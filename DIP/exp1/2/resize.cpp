#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

/*
 * @function myResize
 * @brief  自定义缩放函数
 * @param  src      输入图像
 * @param  scale_x  水平方向缩放比例
 * @param  scale_y  垂直方向缩放比例
 * @return          缩放后的图像
 * @note 使用双线性插值法进行插值
 */
cv::Mat myResize(const cv::Mat &src, double scale_x, double scale_y)
{
    int dst_w = static_cast<int>(src.cols * scale_x);
    int dst_h = static_cast<int>(src.rows * scale_y);

    cv::Mat dst(dst_h, dst_w, src.type());

    for (int y = 0; y < dst_h; ++y)
    {
        for (int x = 0; x < dst_w; ++x)
        {
            // 基于像素中心进行坐标映射
            double srcX = (x + 0.5) / scale_x - 0.5;
            double srcY = (y + 0.5) / scale_y - 0.5;

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

int main(int argc, char **argv)
{
    double scale_x = 2.0, scale_y = 2.0;

    if (argc == 3)
    {
        scale_x = std::stod(argv[1]);
        scale_y = std::stod(argv[2]);
    }

    // 1. 读取原图（灰度）
    cv::Mat src = cv::imread("../SEU_gray.png", cv::IMREAD_GRAYSCALE);
    if (src.empty())
    {
        std::cerr << "Error: Could not open input image!" << std::endl;
        return -1;
    }

    // 2. 使用 OpenCV 自带函数缩放
    cv::Mat dst_cv;
    cv::resize(src, dst_cv, cv::Size(), scale_x, scale_y, cv::INTER_LINEAR);

    // 3. 使用自定义函数缩放
    cv::Mat dst_custom = myResize(src, scale_x, scale_y);

    // 4. 保存输出结果
    cv::imwrite("./easy_resize.png", dst_cv);
    cv::imwrite("./custom_resize.png", dst_custom);

    // 5. 对变换后的图像进行逆变换并保存
    // 5.1 使用 OpenCV 的函数进行逆变换
    cv::Mat dst_inv_cv;
    cv::resize(dst_cv, dst_inv_cv, src.size(), 0, 0, cv::INTER_LINEAR);
    std::string dst_path = "./easy_resize_inv.png";
    cv::imwrite(dst_path, dst_inv_cv);
    // 5.2 使用自定义函数进行逆变换
    cv::Mat dst_inv_custom;
    dst_inv_custom = myResize(dst_custom, 1.0 / scale_x, 1.0 / scale_y);
    dst_path = "./custom_resize_inv.png";
    cv::imwrite(dst_path, dst_inv_custom);
    return 0;
}
