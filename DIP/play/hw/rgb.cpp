#include <opencv2/opencv.hpp>
#include <iostream>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: rgb_components <input_color_image>\n";
        return 0;
    }

    std::string inputPath = argv[1];
    std::string outPrefix = "output";

    cv::Mat img = cv::imread(inputPath, cv::IMREAD_COLOR);

    // 分离三个通道：B, G, R
    std::vector<cv::Mat> bgr;
    cv::split(img, bgr);

    // OpenCV 分离顺序：bgr[0] = B, bgr[1] = G, bgr[2] = R
    // 单色显示器显示时，每个通道直接当作灰度图

    cv::imwrite(outPrefix + "_R.png", bgr[2]);
    cv::imwrite(outPrefix + "_G.png", bgr[1]);
    cv::imwrite(outPrefix + "_B.png", bgr[0]);

    std::cout << "Saved: "
              << outPrefix << "_R.png, "
              << outPrefix << "_G.png, "
              << outPrefix << "_B.png\n";

    return 0;
}
