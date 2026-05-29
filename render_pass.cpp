#include "render_pass.h"
#include <opencv2/opencv.hpp>

Pass::Pass(PipelineContext& pipeline_context, const fs::path& pass_path, const std::string& pass_name)
    : path(pass_path),
      name(pass_name),
      context(pipeline_context) {}

void Pass::initialize_sprite_sheet() {
    // TODO make it compatible with 16 bit images
    sprite_sheet = cv::Mat(
        context.crop_height * context.rows,
        context.crop_width * context.cols,
        CV_8UC4,
        cv::Scalar(0, 0, 0, 0)
    );
}

void Pass::write_frame(const cv::Mat& frame, const int frame_idx){
    // TODO maybe replace crop width and height with frame.cols and frame.height 
    int x_pos = int(frame_idx % context.cols) * context.crop_width;
    int y_pos = (frame_idx / context.cols) * context.crop_height;

    cv::Rect roi(x_pos, y_pos, frame.cols, frame.rows);

    // std::cout << sprite_sheet.cols << " x " << sprite_sheet.rows << std::endl;
    // std::cout << x_pos << ", " << y_pos << ", " << frame.cols << ", " << frame.rows << std::endl;
    cv::Mat destinationROI =  sprite_sheet(roi);
    frame.copyTo(destinationROI);
    // if (frame_idx == 0 || frame_idx == context.frame_count - 1){
    //     cv::imshow("frame", frame);
    //     cv::waitKey(0);
    //     cv::imshow(name, sprite_sheet);
    //     cv::waitKey(0);
    // }
};