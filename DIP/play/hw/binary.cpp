#include <opencv2/opencv.hpp>
#include <iostream>

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: binarize <input_gray_image> <output_binary_image>\n";
        return 1;
    }

    std::string inputPath = argv[1];
    std::string outputPath = argv[2];

    // 读取图像（灰度）
    cv::Mat gray = cv::imread(inputPath, cv::IMREAD_GRAYSCALE);
    if (gray.empty())
    {
        std::cerr << "Error: failed to load image: " << inputPath << "\n";
        return 1;
    }

    // ---- 自动阈值处理（Otsu 二值化） ----
    cv::Mat binary;
    cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    // 如果你的灰度图反色了，可改成 THRESH_BINARY_INV
    // cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);

    // 保存结果
    if (!cv::imwrite(outputPath, binary))
    {
        std::cerr << "Error: failed to save image: " << outputPath << "\n";
        return 1;
    }

    std::cout << "Binary image saved to " << outputPath << "\n";
    return 0;
}
