#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

int main()
{
    cv::Mat src = cv::imread("ct_1.png", cv::IMREAD_GRAYSCALE);

    // 高斯模糊减少噪声干扰，有助于霍夫变换的准确性
    cv::GaussianBlur(src, src, cv::Size(9, 9), 2, 2);

    // 参数说明:
    // dp=1: 累加器分辨率与图像分辨率相同
    // minDist=50: 检测到的圆心之间的最小距离
    // param1=100: Canny边缘检测的高阈值
    // param2=30: 累加器阈值（越小检测到的圆越多）
    // minRadius=10, maxRadius=40: 根据CT中血管大小设定的半径范围
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(src, circles, cv::HOUGH_GRADIENT, 1, 50, 100, 30, 10, 40);

    cv::Mat result;
    cv::cvtColor(src, result, cv::COLOR_GRAY2BGR);
    for (const auto &c : circles)
    {
        cv::Point center(std::round(c[0]), std::round(c[1]));
        int radius = std::round(c[2]);

        // 仅筛选图像中心区域的圆（规避边缘干扰）
        if (center.x > src.cols * 0.4 && center.x < src.cols * 0.6 &&
            center.y > src.rows * 0.4 && center.y < src.rows * 0.7)
        {

            cv::circle(result, center, radius, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
            cv::circle(result, center, 2, cv::Scalar(0, 255, 0), 3, cv::LINE_AA);
        }
    }

    // 5. 保存结果
    cv::imwrite("detected_vessel.png", result);

    return 0;
}