#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

cv::Mat PowerLawTrans(const cv::Mat &src, double gamma)
{
    cv::Mat dst = cv::Mat::zeros(src.size(), src.type());
    for (int y = 0; y < src.rows; ++y)
    {
        const uint8_t *src_row = src.ptr<uint8_t>(y);
        uint8_t *dst_row = dst.ptr<uint8_t>(y);
        for (int x = 0; x < src.cols; ++x)
        {
            double normalized = static_cast<double>(src_row[x]) / 255.0;
            double powered = std::pow(normalized, gamma);
            int scaled = std::round(powered * 255.0);
            dst_row[x] = static_cast<uint8_t>(std::clamp(scaled, 0, 255));
        }
    }
    return dst;
}

int main(int argc, char **argv)
{
    double gamma;
    std::string image_path;
    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " <image_path> <gamma_value>\n";
        std::cout << "Using default gamma value: " << gamma << "\n";
        return -1;
    }
    else
    {
        image_path = argv[1];
        gamma = std::stod(argv[2]);
    }

    cv::Mat src = cv::imread(image_path, cv::IMREAD_GRAYSCALE);
    if (src.empty())
    {
        std::cerr << "Error: Cannot open image!\n";
        return -1;
    }

    cv::Mat transed = PowerLawTrans(src, gamma);

    cv::imwrite("./transed_" + image_path + ".png", transed);

    return 0;
}
