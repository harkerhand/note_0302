#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <filesystem>

#include <cmath>

namespace fs = std::filesystem;

/*
 * @function myGetMoveMatrix
 * @brief  生成缩放变换的齐次 3x3 矩阵
 * @param  scale_x x 方向缩放
 * @param  scale_y y 方向缩放
 * @return     3x3 齐次变换矩阵
 */
cv::Mat myGetMoveMatrix(double scale_x, double scale_y)
{
    cv::Mat move_mat = (cv::Mat_<double>(3, 3) << scale_x, 0, 0,
                        0, scale_y, 0,
                        0, 0, 1);
    return move_mat;
}

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
    cv::Mat scale_mat = myGetMoveMatrix(scale_x, scale_y);
    cv::Mat inv_scale = scale_mat.inv();

    // 将输出 dst 的尺寸设置为原始图像尺寸，超出的部分隐式丢弃
    int dst_w = src.cols;
    int dst_h = src.rows;

    cv::Mat dst(dst_h, dst_w, src.type(), cv::Scalar(0));

    for (int y = 0; y < dst_h; ++y)
    {
        for (int x = 0; x < dst_w; ++x)
        {
            double srcX = inv_scale.at<double>(0, 0) * x +
                          inv_scale.at<double>(0, 1) * y +
                          inv_scale.at<double>(0, 2);
            double srcY = inv_scale.at<double>(1, 0) * x +
                          inv_scale.at<double>(1, 1) * y +
                          inv_scale.at<double>(1, 2);

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
    int dst_w = static_cast<int>(src.cols * scale_x);
    int dst_h = static_cast<int>(src.rows * scale_y);
    cv::Mat dst_cv;
    // 使用仿射缩放矩阵和 warpAffine（目标尺寸设为 src.size()，超出部分丢弃）
    cv::Mat scale_mat = (cv::Mat_<double>(2, 3) << scale_x, 0, 0, 0, scale_y, 0);
    cv::warpAffine(src, dst_cv, scale_mat, src.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0));

    // 3. 使用自定义函数缩放
    cv::Mat dst_custom = myResize(src, scale_x, scale_y);

    // 4. 保存输出结果
    // 把放缩后超出原始图片的部分丢弃（裁剪到原始尺寸，放在左上角）
    cv::Mat dst_cv_cropped(src.rows, src.cols, dst_cv.type(), cv::Scalar(0));
    int copy_w = std::min(dst_w, src.cols);
    int copy_h = std::min(dst_h, src.rows);
    if (copy_w > 0 && copy_h > 0)
    {
        dst_cv(cv::Rect(0, 0, copy_w, copy_h)).copyTo(dst_cv_cropped(cv::Rect(0, 0, copy_w, copy_h)));
    }

    cv::Mat dst_custom_cropped(src.rows, src.cols, dst_custom.type(), cv::Scalar(0));
    if (copy_w > 0 && copy_h > 0)
    {
        dst_custom(cv::Rect(0, 0, copy_w, copy_h)).copyTo(dst_custom_cropped(cv::Rect(0, 0, copy_w, copy_h)));
    }

    cv::imwrite("./easy_resize.png", dst_cv_cropped);
    cv::imwrite("./custom_resize.png", dst_custom_cropped);

    // 5. 对变换后的图像进行逆变换并保存
    // 5.1 OpenCV 逆变换：使用 invertAffineTransform 得到逆仿射矩阵
    cv::Mat inv_affine;
    cv::invertAffineTransform(scale_mat, inv_affine);
    cv::Mat easy_inv;
    // 使用裁剪后的缩放图像进行逆变换
    cv::warpAffine(dst_cv_cropped, easy_inv, inv_affine, src.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0));
    std::string dst_path = "./easy_resize_inv.png";
    cv::imwrite(dst_path, easy_inv);

    // 5.2 使用自定义函数进行逆变换（缩放的逆为 1/scale）
    cv::Mat dst_inv_custom;
    dst_inv_custom = myResize(dst_custom_cropped, 1.0 / scale_x, 1.0 / scale_y);
    dst_path = "./custom_resize_inv.png";
    cv::imwrite(dst_path, dst_inv_custom);
    return 0;
}
