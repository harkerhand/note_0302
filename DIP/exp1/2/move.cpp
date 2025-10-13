#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <filesystem>

#include <cmath>

namespace fs = std::filesystem;

/*
 * @function myGetMoveMatrix
 * @brief  生成平移变换的齐次 3x3 矩阵
 * @param  tx x 方向平移
 * @param  ty y 方向平移
 * @return     3x3 齐次变换矩阵
 */
cv::Mat myGetMoveMatrix(double tx, double ty)
{
    cv::Mat move_mat = (cv::Mat_<double>(3, 3) << 1, 0, tx,
                        0, 1, ty,
                        0, 0, 1);
    return move_mat;
}

/*
 * @function myMove
 * @brief  使用逆映射和双线性插值执行平移变换（与 shear.cpp 风格一致）
 * @param  src 输入图像
 * @param  tx  x 方向平移（可以为负）
 * @param  ty  y 方向平移（可以为负）
 * @return     变换后的图像
 */
cv::Mat myMove(const cv::Mat &src, double tx, double ty)
{
    cv::Mat move_mat = myGetMoveMatrix(tx, ty);
    cv::Mat inv_move = move_mat.inv();

    cv::Mat dst(src.rows, src.cols, src.type(), cv::Scalar(0));

    for (int y = 0; y < dst.rows; ++y)
    {
        for (int x = 0; x < dst.cols; ++x)
        {
            double srcX = inv_move.at<double>(0, 0) * x +
                          inv_move.at<double>(0, 1) * y +
                          inv_move.at<double>(0, 2);
            double srcY = inv_move.at<double>(1, 0) * x +
                          inv_move.at<double>(1, 1) * y +
                          inv_move.at<double>(1, 2);

            if (srcX < 0 || srcX >= src.cols - 1 || srcY < 0 || srcY >= src.rows - 1)
                continue;

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
