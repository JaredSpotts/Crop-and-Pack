#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <opencv2/core.hpp>

namespace fs = std::filesystem;

struct PipelineContext {
    int pivot_x = 0;
    int pivot_y = 0;

    std::string character;
    std::string variation;
    std::string animation_name;
    std::string animation_type;
    std::string role;

    int frame_width = 0;
    int frame_height = 0;

    // width and height of a cropped frame
    int crop_width = 0;
    int crop_height = 0;

    int rows = 0;
    int cols = 0;
    size_t frame_count = 0;

    cv::Rect crop_bb;
};

struct Pass {
    fs::path path;
    std::string name;
    bool write_sprite_sheet = true;
    cv::Mat sprite_sheet;
    PipelineContext& context;

    // e.g. <name of source pass>, <source channel>, <destination channel>
    std::vector<std::string> channel_replace_relation;
    std::vector<std::string> img_paths;

    Pass(PipelineContext& pipeline_context, const fs::path& pass_path, const std::string& pass_name);

    void initialize_sprite_sheet();
    void write_frame(const cv::Mat& frame, const int frame_idx);
};

struct FrameResult {
    size_t frame_idx;
    std::vector<cv::Mat> frames;
};