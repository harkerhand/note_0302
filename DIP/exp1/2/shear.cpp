#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

/*
 * @function myGetShearMatrix
 * @brief  生成倾斜（剪切）变换矩阵
 * @param  shx  x方向剪切系数
 * @param  shy  y方向剪切系数
 * @return      3x3 齐次变换矩阵
 */
cv::Mat myGetShearMatrix(double shx, double shy)
{
    cv::Mat shear_mat = (cv::Mat_<double>(3, 3) << 1, shx, 0,
                         shy, 1, 0,
                         0, 0, 1);
    return shear_mat;
}

/*
 * @function myShear
 * @brief  对图像进行倾斜变换
 * @param  src 输入图像
 * @param  shx x方向剪切系数
 * @param  shy y方向剪切系数
 * @return     变换后的图像
 * @note 使用双线性插值
 */
cv::Mat myShear(const cv::Mat &src, double shx, double shy)
{
    cv::Mat shear_mat = myGetShearMatrix(shx, shy);
    cv::Mat inv_shear = shear_mat.inv();

    cv::Mat dst(src.rows, src.cols, src.type(), cv::Scalar(0));

    for (int y = 0; y < dst.rows; ++y)
    {
        for (int x = 0; x < dst.cols; ++x)
        {
            double srcX = inv_shear.at<double>(0, 0) * x +
                          inv_shear.at<double>(0, 1) * y +
                          inv_shear.at<double>(0, 2);
            double srcY = inv_shear.at<double>(1, 0) * x +
                          inv_shear.at<double>(1, 1) * y +
                          inv_shear.at<double>(1, 2);

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
    double shx = 0.3, shy = 0.0;

    if (argc == 3)
    {
        shx = std::stod(argv[1]);
        shy = std::stod(argv[2]);
    }

    // 1. 读取原图（灰度）
    cv::Mat src = cv::imread("../SEU_gray.png", cv::IMREAD_GRAYSCALE);
    if (src.empty())
    {
        std::cerr << "Error: Could not open input image!" << std::endl;
        return -1;
    }

    // 2. 使用 OpenCV 内置函数
    cv::Mat shear_mat = (cv::Mat_<double>(2, 3) << 1, shx, 0, shy, 1, 0);
    cv::Mat dst_cv;
    cv::warpAffine(src, dst_cv, shear_mat, src.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0));

    // 3. 使用自定义函数
    cv::Mat dst_custom = myShear(src, shx, shy);

    // 4. 保存结果
    cv::imwrite("./easy_shear.png", dst_cv);
    cv::imwrite("./custom_shear.png", dst_custom);

    // 5. 对变换后的图像进行逆变换并保存
    // 5.1 OpenCV 逆变换：使用 invertAffineTransform 得到逆仿射矩阵
    cv::Mat inv_affine;
    cv::invertAffineTransform(shear_mat, inv_affine);
    cv::Mat easy_inv;
    cv::warpAffine(dst_cv, easy_inv, inv_affine, dst_cv.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0));
    cv::imwrite("./easy_shear_inv.png", easy_inv);

    // 5.2 自定义逆变换：如果剪切矩阵可逆，则计算逆剪切系数并应用 myShear
    // 原剪切矩阵为 [1 shx; shy 1]，其行列式 det = 1 - shx*shy
    double det = 1.0 - shx * shy;
    if (std::abs(det) < 1e-12)
    {
        std::cerr << "Warning: shear matrix is not invertible (det ~= 0). Skipping custom inverse." << std::endl;
    }
    else
    {
        // 逆矩阵为 (1/det) * [1 -shx; -shy 1]
        double ishx = -shx / det; // 注意：逆矩阵对应的 shear 参数
        double ishy = -shy / det;

        cv::Mat custom_inv = myShear(dst_custom, ishx, ishy);
        cv::imwrite("./custom_shear_inv.png", custom_inv);
    }

    return 0;
}
