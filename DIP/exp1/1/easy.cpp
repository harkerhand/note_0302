#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

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
    cv::Mat dst;
    cv::resize(src, dst, cv::Size(src.cols * 2, src.rows * 2), 0, 0, cv::INTER_LINEAR);

    // 3. 保存放大后的图像
    std::string src_path = "../SEU_gray.png";
    std::string dst_path = "./easy_resize.png";
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
