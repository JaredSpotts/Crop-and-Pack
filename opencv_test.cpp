#include <iostream>
#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: opencv_test <image_path>\n";
        return -1;
    }

    std::string image_path = argv[1];

    // Load image (unchanged = keeps channels as-is)
    cv::Mat image = cv::imread(image_path, cv::IMREAD_UNCHANGED);

    if (image.empty()) {
        std::cout << "Failed to load image: " << image_path << "\n";
        return -1;
    }

    std::cout << "Image loaded successfully!\n";
    std::cout << "Width: " << image.cols << "\n";
    std::cout << "Height: " << image.rows << "\n";
    std::cout << "Channels: " << image.channels() << "\n";

    return 0;
}