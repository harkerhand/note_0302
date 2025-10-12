#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

/*
 * @function myGetRotationMatrix2D
 * @brief  自定义计算旋转矩阵的函数
 * @param  center 旋转中心
 * @param  angle  旋转角度（逆时针，单位：度）
 * @return        3x3 的旋转矩阵
 * @note OpenCV 的 getRotationMatrix2D 返回的是 2x3 矩阵，这里为了方便学习理解，返回一个完整的 3x3 矩阵
 */
cv::Mat myGetRotationMatrix2D(cv::Point2f center, double angle)
{
    double alpha = std::cos(angle * CV_PI / 180.0);
    double beta = std::sin(angle * CV_PI / 180.0);

    cv::Mat rot_mat(3, 3, CV_64F);
    rot_mat.at<double>(0, 0) = alpha;
    rot_mat.at<double>(0, 1) = beta;
    rot_mat.at<double>(0, 2) = (1 - alpha) * center.x - beta * center.y;
    rot_mat.at<double>(1, 0) = -beta;
    rot_mat.at<double>(1, 1) = alpha;
    rot_mat.at<double>(1, 2) = beta * center.x + (1 - alpha) * center.y;
    rot_mat.at<double>(2, 0) = 0.0;
    rot_mat.at<double>(2, 1) = 0.0;
    rot_mat.at<double>(2, 2) = 1.0;

    return rot_mat;
}

/*
 * @function myRotate
 * @brief  自定义旋转函数
 * @param  src          输入图像
 * @param  rotate_angle 旋转角度（逆时针，单位：度）
 * @return              旋转后的图像
 * @note 使用双线性插值法进行插值
 */
cv::Mat myRotate(const cv::Mat &src, int rotate_angle)
{
    // 计算旋转矩阵
    cv::Point2f center(src.cols / 2.0f, src.rows / 2.0f);
    cv::Mat rot_mat = myGetRotationMatrix2D(center, rotate_angle);

    // 创建目标图像
    cv::Mat dst = cv::Mat::zeros(src.rows, src.cols, src.type());

    // 遍历目标图像的每个像素
    for (int y = 0; y < dst.rows; ++y)
    {
        for (int x = 0; x < dst.cols; ++x)
        {
            // 计算源图像中的对应位置
            cv::Mat inv_rot = rot_mat.inv();
            double srcX = inv_rot.at<double>(0, 0) * x + inv_rot.at<double>(0, 1) * y + inv_rot.at<double>(0, 2);
            double srcY = inv_rot.at<double>(1, 0) * x + inv_rot.at<double>(1, 1) * y + inv_rot.at<double>(1, 2);

            // 双线性插值
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
    // 默认旋转的角度（逆时针）
    int rotate_angle = 45;

    // 如果提供了参数，解析参数
    if (argc == 2)
    {
        rotate_angle = std::stoi(argv[1]);
    }

    // 1. 读取原图（灰度）
    cv::Mat src = cv::imread("../SEU_gray.png", cv::IMREAD_GRAYSCALE);
    if (src.empty())
    {
        std::cerr << "Error: Could not open input image!" << std::endl;
        return -1;
    }

    // 2. 创建目标图像并旋转图像
    cv::Mat dst = cv::Mat::zeros(src.rows, src.cols, src.type());
    cv::Point2f center(src.cols / 2.0f, src.rows / 2.0f);
    cv::Mat rot_mat = cv::getRotationMatrix2D(center, rotate_angle, 1.0);
    cv::warpAffine(src, dst, rot_mat, dst.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0));

    // 3. 使用自定义的旋转函数
    cv::Mat dst_custom = myRotate(src, rotate_angle);

    // 4. 保存旋转后的图像
    std::string dst_path = "./easy_rotate.png";
    cv::imwrite(dst_path, dst);
    dst_path = "./custom_rotate.png";
    cv::imwrite(dst_path, dst_custom);

    return 0;
}
