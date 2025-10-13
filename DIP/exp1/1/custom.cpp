#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

// 最近邻插值（2D）
cv::Mat nearestNeighborResize(const cv::Mat &src, cv::Size dstSize)
{
    cv::Mat dst(dstSize, src.type());
    double scaleX = static_cast<double>(src.cols) / dstSize.width;
    double scaleY = static_cast<double>(src.rows) / dstSize.height;

    for (int y = 0; y < dstSize.height; ++y)
    {
        for (int x = 0; x < dstSize.width; ++x)
        {
            int srcX = std::clamp(static_cast<int>(std::round(x * scaleX)), 0, src.cols - 1);
            int srcY = std::clamp(static_cast<int>(std::round(y * scaleY)), 0, src.rows - 1);
            dst.at<uchar>(y, x) = src.at<uchar>(srcY, srcX);
        }
    }
    return dst;
}

// 双线性插值（2D）
cv::Mat bilinearResize(const cv::Mat &src, cv::Size dstSize)
{
    cv::Mat dst(dstSize, src.type());
    double scaleX = static_cast<double>(src.cols) / dstSize.width;
    double scaleY = static_cast<double>(src.rows) / dstSize.height;

    for (int y = 0; y < dstSize.height; ++y)
    {
        for (int x = 0; x < dstSize.width; ++x)
        {
            double srcX = (x + 0.5) * scaleX - 0.5;
            double srcY = (y + 0.5) * scaleY - 0.5;

            int x1 = std::clamp(static_cast<int>(std::floor(srcX)), 0, src.cols - 1);
            int y1 = std::clamp(static_cast<int>(std::floor(srcY)), 0, src.rows - 1);
            int x2 = std::clamp(x1 + 1, 0, src.cols - 1);
            int y2 = std::clamp(y1 + 1, 0, src.rows - 1);

            double dx = srcX - x1;
            double dy = srcY - y1;

            double f11 = src.at<uchar>(y1, x1);
            double f12 = src.at<uchar>(y1, x2);
            double f21 = src.at<uchar>(y2, x1);
            double f22 = src.at<uchar>(y2, x2);

            double value = f11 * (1 - dx) * (1 - dy) +
                           f12 * dx * (1 - dy) +
                           f21 * (1 - dx) * dy +
                           f22 * dx * dy;

            dst.at<uchar>(y, x) = static_cast<uchar>(std::round(value));
        }
    }
    return dst;
}

// 获取文件大小
double get_file_size_kb(const std::string &path)
{
    if (!fs::exists(path))
        return 0.0;
    return static_cast<double>(fs::file_size(path)) / 1024.0;
}

int main()
{
    cv::Mat src = cv::imread("../SEU_gray.png", cv::IMREAD_GRAYSCALE);
    if (src.empty())
    {
        std::cerr << "Error: Cannot open image!\n";
        return -1;
    }

    cv::Mat nn = nearestNeighborResize(src, cv::Size(src.cols * 2, src.rows * 2));
    cv::Mat bl = bilinearResize(src, cv::Size(src.cols * 2, src.rows * 2));

    cv::imwrite("./resize_nn.png", nn);
    cv::imwrite("./resize_bilinear.png", bl);

    std::cout << "\n================= Interpolation Comparison =================\n";
    std::cout << std::left << std::setw(20) << "Method"
              << std::setw(10) << "Width"
              << std::setw(10) << "Height"
              << std::setw(10) << "File Size (KB)"
              << "\n------------------------------------------------------------\n";

    std::cout << std::left << std::setw(20) << "Original"
              << std::setw(10) << src.cols
              << std::setw(10) << src.rows
              << std::setw(10) << std::fixed << std::setprecision(2) << get_file_size_kb("../SEU_gray.png") << "\n";

    std::cout << std::left << std::setw(20) << "Nearest Neighbor"
              << std::setw(10) << nn.cols
              << std::setw(10) << nn.rows
              << std::setw(10) << std::fixed << std::setprecision(2) << get_file_size_kb("./resize_nn.png") << "\n";

    std::cout << std::left << std::setw(20) << "Bilinear"
              << std::setw(10) << bl.cols
              << std::setw(10) << bl.rows
              << std::setw(10) << get_file_size_kb("./resize_bilinear.png") << "\n";

    std::cout << "============================================================\n";

    return 0;
}
