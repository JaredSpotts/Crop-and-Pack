#include <iostream>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <iomanip>

#include "render_pass.h"

using json = nlohmann::json;
using namespace std;
namespace fs = std::filesystem;

// Globals
fs::path base_path;
bool skip_render_helper = false;
bool skip_importer = false;
vector<Pass> passes;
string write_name = "godot_info";


json render_info;

PipelineContext pipeline_context;



template<typename T>
void print_setting(string level, string label, T value){
    if(level == "info"){
        spdlog::info(label, value);
    }
    else if(level == "error"){
        spdlog::error(label, value);
    }
}

void compute_crop_box(Pass& pass){
    fs::path path = pass.path;
    int threshold_value = 5;
    int left_bound = pipeline_context.frame_width;
    int top_bound = pipeline_context.frame_height;
    int right_bound = 0;
    int bottom_bound = 0;
    cv::Point pivot(pipeline_context.pivot_x, pipeline_context.pivot_y);
    double alpha = 0.25;
    
    for (auto &img_path : fs::directory_iterator(path)){
        cv::Mat img_grey = cv::imread(img_path.path().string(), cv::IMREAD_GRAYSCALE);
        cv::Mat img_color = cv::imread(img_path.path().string(), cv::IMREAD_COLOR);
        cv::Mat mask;

        if (img_grey.empty() || img_color.empty()) {
            print_setting("error", "failed to load image: {}", img_path.path().string());
            continue;
        }


        cv::threshold(img_grey, mask, threshold_value, 255, cv::THRESH_BINARY);

        cv::Rect rect = cv::boundingRect(mask);
        int rect_left = rect.x;
        int rect_top = rect.y;
        int rect_right = rect.x + rect.width;
        int rect_bottom = rect.y + rect.height;
        string rect_str = "{" + to_string(rect_left) + "," + to_string(rect_top) +
                                "," + to_string(rect_right) + "," + to_string(rect_bottom) + "}";
        //print_setting("info", "Rect left, top, right, bottom: {}", rect_str);

        // update bound box

        left_bound = min(left_bound, rect_left);
        top_bound = min(top_bound, rect_top);
        right_bound = max(right_bound, rect_right);
        bottom_bound = max(bottom_bound, rect_bottom);
    }
    string bb_str = "{" + to_string(left_bound) + "," + to_string(top_bound) +
                                "," + to_string(right_bound) + "," + to_string(bottom_bound) + "}";
    print_setting("info", "Bounding box left, top, right, bottom: {}", bb_str);
    cv::Rect crop_bb(left_bound, top_bound, right_bound - left_bound, bottom_bound - top_bound);
    pipeline_context.crop_bb = crop_bb;
    // for (auto &img_path : fs::directory_iterator(path)){
    //     cv::Mat img_color = cv::imread(img_path.path().string(), cv::IMREAD_COLOR);
    //     cv::Mat overlay = img_color.clone();
    //     cv::rectangle(overlay, crop_bb, cv::Scalar(0, 255, 0), -1); // filled
    //     cv::circle(overlay, pivot, 10, cv::Scalar(255, 0, 0), 3);
    //     cv::addWeighted(overlay, alpha, img_color, 1 - alpha, 0, img_color);
    //     cv::imshow("Bounding Box Debug", img_color);
    //     cv::waitKey(0);
    // }

    // update pivot point
    pipeline_context.pivot_x -= left_bound;
    pipeline_context.pivot_y -= top_bound;

    // set the width and height of spirite sheets
    pipeline_context.crop_width = (right_bound - left_bound);
    pipeline_context.crop_height = (bottom_bound - top_bound);
}

bool validate_filepath(fs::path path, string filename = ""){
    if (!filename.empty()){
        path = path / filename;
    }
    if (fs::exists(path)){
        return true;
    }

    return false;
}

void parse_json(){
    
    base_path = render_info["filepath"].get<string>();
    print_setting("info", "file path: {}", base_path.string());

    skip_render_helper = render_info["settings"]["skip_render_helper"];
    print_setting("info", "skip render helper: {}", skip_render_helper);

    skip_importer = render_info["settings"]["skip_importer"];
    print_setting("info", "skip godot importer: {}", skip_importer);




    // Pivot point

    
    pipeline_context.pivot_x = render_info["pivot"]["x"];
    pipeline_context.pivot_y = render_info["pivot"]["y"];
    string pivot = to_string(pipeline_context.pivot_x) + "," + to_string(pipeline_context.pivot_y);
    print_setting("info", "pivot (x,y): ({})", pivot);

    pipeline_context.character = render_info["identity"]["character"];
    print_setting("info", "character name: {}", pipeline_context.character);

    pipeline_context.variation = render_info["identity"]["variation"];
    print_setting("info", "variation: {}", pipeline_context.variation);

    pipeline_context.animation_name = render_info["identity"]["animation_name"];
    print_setting("info", "animation name: {}", pipeline_context.animation_name);

    pipeline_context.animation_type = render_info["render"]["animation_type"];
    print_setting("info", "animation type: {}", pipeline_context.animation_type);

    pipeline_context.frame_width = render_info["render"]["resolution"]["width"];
    print_setting("info", "starting width: {}", pipeline_context.frame_width);
    pipeline_context.frame_height = render_info["render"]["resolution"]["height"];
    print_setting("info", "starting height: {}", pipeline_context.frame_height);

    // columns and rows

    pipeline_context.rows = render_info["layout"]["rows"];
    print_setting("info", "rows: {}", pipeline_context.rows);

    pipeline_context.cols = render_info["layout"]["cols"];
    print_setting("info", "cols: {}", pipeline_context.cols);

    pipeline_context.frame_count = pipeline_context.cols * pipeline_context.rows;
    print_setting("info", "total frame count: {}", pipeline_context.frame_count);



    for (const auto& pass : render_info["render"]["passes"]) {
        string pass_str = pass.get<string>();

        fs::path path = base_path / (pass_str);
        if (!validate_filepath(path)){
            print_setting("error", "file location {} not found", path.string());
        }

        print_setting("info", "render pass: {}", pass_str);
        passes.emplace_back(pipeline_context, path, pass_str);
    }
    for (auto &pass : passes){
        for (auto &img_path : fs::directory_iterator(pass.path)){
            pass.img_paths.push_back(img_path.path().string());
        }
        sort(pass.img_paths.begin(), pass.img_paths.end());
    }

}



FrameResult crop_frame(int frame_idx, const vector<Pass>& passes){
    // TODO add color channel remapping
    FrameResult result;
    result.frame_idx = frame_idx;
    for (const auto &pass : passes){
        if (!pass.write_sprite_sheet){continue;}
        
        cv::Mat frame = cv::imread(pass.img_paths.at(frame_idx), cv::IMREAD_UNCHANGED);
        cv::Mat cropped_frame = frame(pass.context.crop_bb).clone();
        result.frames.push_back(cropped_frame);
        
        // double alpha = 0.25;
        // cv::Point pivot(pipeline_context.pivot_x, pipeline_context.pivot_y);
        // cv::Mat overlay = cropped_frame.clone();
        // cv::circle(overlay, pivot, 10, cv::Scalar(255, 0, 0), 3);
        // cv::addWeighted(overlay, alpha, cropped_frame, 1 - alpha, 0, cropped_frame);
        // cv::imshow("Bounding Box Debug", cropped_frame);
        // cv::waitKey(0);
    }
    return result;
}
void make_spritesheets(){
    // initialize sprite sheets
    vector<size_t> writable_pass_idx;
    size_t i = 0;
    for (auto &pass : passes){
        if (pass.write_sprite_sheet){
            pass.initialize_sprite_sheet();
            writable_pass_idx.push_back(i);
        }
        ++i;
    }

    for (int i = 0; i < pipeline_context.frame_count; i++){
        //print_setting("info", "cropping frame {} start", i);
        FrameResult result = crop_frame(i, passes);
        //print_setting("info", "cropping frame {} finish", i);

            // write the cropped frame to the sprite sheets.
            int x = 0;
            for (size_t j : writable_pass_idx){
                if (result.frames.at(x).empty()){
                    print_setting("error", "Empty frame at {}", result.frame_idx);
                }
                //print_setting("info", "writing frame {} start", i);
                passes.at(j).write_frame(result.frames.at(x++), result.frame_idx);
                //print_setting("info", "writing frame {} finish", i);
            }
    }
}

void write_sprite_sheets(){
    for (auto &pass : passes){
        if (pass.write_sprite_sheet){
            fs::path name_path = base_path / pass.name;
            string name_string = name_path.string() + ".png";
            bool save_successful = cv::imwrite(name_string, pass.sprite_sheet);
            if (save_successful){
                print_setting("info", "Sprite sheet saved at {}", name_string);
            }
            else {
                print_setting("error", "Sprite sheet failed to save at {}", name_string);
            }
        }
    }
}

void write_json(const string& write_path){
    json godot_info = {
        {"filepath", base_path.string()},
        {"settings", {
            {"skip_importer", skip_importer}
            }
        },
        {"identity", {
                {"character", pipeline_context.character},
                {"variation", pipeline_context.variation},
                {"animation_name", pipeline_context.animation_name}
            }
        },
        {"render", {
            {"passes", render_info["render"]["passes"]},
            {"fps", render_info["render"]["fps"]},
            {"resolution", {
                    {"width", pipeline_context.crop_width * pipeline_context.cols},
                    {"height", pipeline_context.crop_height * pipeline_context.rows}
                    }
                },
            {"animation_type", pipeline_context.animation_type},
            {"loop", render_info["render"]["loop"]},
            {"render_scale", render_info["render"]["render_scale"]}
        
            }
        },
        {"layout", {
            {"rows", pipeline_context.rows},
            {"cols", pipeline_context.cols}
            }
        },
        {"pivot", {
            {"x", pipeline_context.pivot_x},
            {"y", pipeline_context.pivot_y}
            }
        }
    };
    print_setting("info", "Wrote {}", write_name);
    std::ofstream out_file(write_path);
    out_file << std::setw(4) << godot_info;
    out_file.close();
}

int main(int argc, char* argv[]) {
    string render_info_path;
    for (int i = 1; i < argc; ++i){
        string arg = argv[i];
        
        if (arg == "--renders"){
            if (i + 1 < argc){
                render_info_path = argv[++i];
            }
        }
    }
    if (render_info_path.empty()){
        cout << "No render path input" << endl;
        return -1;
    }


    std::ifstream info_file(render_info_path);
    if (!info_file){
        cout << "No render info json file found" << endl;
        return -1;
    }
    info_file >> render_info;
    info_file.close();

    parse_json();
    compute_crop_box(passes.at(0));
    make_spritesheets();
    write_sprite_sheets();
    
    fs::path write_path = base_path / (write_name + ".json");
    write_json(write_path.string());

    return 0;
}
