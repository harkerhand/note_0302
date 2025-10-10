#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

cv::Mat bilinearResize(const cv::Mat &src, cv::Size dstSize)
{
    cv::Mat dst(dstSize, src.type());
    double scaleX = static_cast<double>(src.cols) / dstSize.width;
    double scaleY = static_cast<double>(src.rows) / dstSize.height;

    for (int y = 0; y < dstSize.height; ++y)
    {
        for (int x = 0; x < dstSize.width; ++x)
        {
            // 这是初稿的映射方式，发现输出文件大小远小于标准库，考虑到png压缩率，怀疑是边缘像素没有被正确采样
            // 查找资料发现双线性插值的标准做法是基于像素中心进行映射
            // 参考：https://en.wikipedia.org/wiki/Bilinear_interpolation#Algorithm
            // double srcX = x * scaleX;
            // double srcY = y * scaleY;

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

int main()
{
    // 1. 读取原图（灰度）
    cv::Mat src = cv::imread("../SEU_gray.png", cv::IMREAD_GRAYSCALE);
    if (src.empty())
    {
        std::cerr << "Error: Could not open input image!" << std::endl;
        return -1;
    }

    // 2. 放大图像（2倍）
    cv::Mat dst = bilinearResize(src, cv::Size(src.cols * 2, src.rows * 2));

    // 3. 保存放大后的图像
    std::string src_path = "../SEU_gray.png";
    std::string dst_path = "./custom_resize.png";
    cv::imwrite(dst_path, dst);

    // 4. 获取磁盘文件大小（以 KB 显示）
    auto get_file_size_kb = [](const std::string &path) -> double
    {
        if (!fs::exists(path))
            return 0.0;
        return static_cast<double>(fs::file_size(path)) / 1024.0;
    };

    double src_file_size = get_file_size_kb(src_path);
    double dst_file_size = get_file_size_kb(dst_path);

    // 5. 输出结果对比
    std::cout << "\n================= Image Comparison =================\n";
    std::cout << std::left << std::setw(25) << "Property"
              << std::setw(20) << "Original"
              << std::setw(20) << "Resized (2x)"
              << "\n----------------------------------------------------\n";

    std::cout << std::left << std::setw(25) << "Width (cols)"
              << std::setw(20) << src.cols
              << std::setw(20) << dst.cols << '\n';

    std::cout << std::left << std::setw(25) << "Height (rows)"
              << std::setw(20) << src.rows
              << std::setw(20) << dst.rows << '\n';

    std::cout << std::left << std::setw(25) << "Total Pixels"
              << std::setw(20) << src.total()
              << std::setw(20) << dst.total() << '\n';

    std::cout << std::left << std::setw(25) << "Data Size (bytes)"
              << std::setw(20) << src.elemSize() * src.total()
              << std::setw(20) << dst.elemSize() * dst.total() << '\n';

    std::cout << std::left << std::setw(25) << "File Size (KB)"
              << std::setw(20) << std::fixed << std::setprecision(2) << src_file_size
              << std::setw(20) << dst_file_size << '\n';

    std::cout << "====================================================\n\n";

    std::cout << "Saved resized image: " << dst_path << std::endl;

    return 0;
}
