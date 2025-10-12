#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

cv::Mat myMove(const cv::Mat &src, int move_x, int move_y)
{
    cv::Mat dst = cv::Mat::zeros(src.rows, src.cols, src.type());
    for (int y = 0; y < src.rows; ++y)
    {
        for (int x = 0; x < src.cols; ++x)
        {
            int new_x = x + move_x;
            int new_y = y + move_y;
            if (new_x >= 0 && new_x < dst.cols && new_y >= 0 && new_y < dst.rows)
            {
                dst.at<uchar>(new_y, new_x) = src.at<uchar>(y, x);
            }
        }
    }
    return dst;
}

int main(int argc, char **argv)
{
    // 默认移动的像素数
    int move_x = 80;
    int move_y = 80;

    // 如果提供了参数，解析参数
    if (argc == 3)
    {
        move_x = std::stoi(argv[1]);
        move_y = std::stoi(argv[2]);
    }

    // 1. 读取原图（灰度）
    cv::Mat src = cv::imread("../SEU_gray.png", cv::IMREAD_GRAYSCALE);
    if (src.empty())
    {
        std::cerr << "Error: Could not open input image!" << std::endl;
        return -1;
    }

    // 2. 创建目标图像并移动图像
    cv::Mat M = (cv::Mat_<double>(2, 3) << 1, 0, move_x,
                 0, 1, move_y);
    cv::Mat dst;
    cv::warpAffine(src, dst, M, src.size(), cv::INTER_NEAREST, cv::BORDER_CONSTANT, cv::Scalar(0));

    // 3. 使用自定义的移动函数
    cv::Mat dst_custom = myMove(src, move_x, move_y);
    // 验证自定义函数的结果是否与标准库一致
    if (cv::countNonZero(dst != dst_custom) == 0)
    {
        std::cout << "Custom move function works correctly." << std::endl;
    }
    else
    {
        std::cout << "Custom move function has discrepancies." << std::endl;
    }

    // 4. 保存移动后的图像
    std::string dst_path = "./easy_move.png";
    cv::imwrite(dst_path, dst);
    dst_path = "./custom_move.png";
    cv::imwrite(dst_path, dst_custom);

    // 5. 对变换后的图像进行逆变换并保存
    // 5.1 使用OpenCV的warpAffine进行逆变换
    cv::Mat M_inv;
    cv::invertAffineTransform(M, M_inv);
    cv::Mat dst_inv;
    cv::warpAffine(dst, dst_inv, M_inv, dst.size(), cv::INTER_NEAREST, cv::BORDER_CONSTANT, cv::Scalar(0));
    dst_path = "./easy_move_inv.png";
    cv::imwrite(dst_path, dst_inv);

    // 5.2 使用自定义函数进行逆变换
    cv::Mat dst_custom_inv = myMove(dst_custom, -move_x, -move_y);
    dst_path = "./custom_move_inv.png";
    cv::imwrite(dst_path, dst_custom_inv);

    return 0;
}
