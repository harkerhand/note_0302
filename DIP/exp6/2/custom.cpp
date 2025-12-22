#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>

int main()
{
    // 1. 读取图像
    cv::Mat src = cv::imread("ct_crop.png", cv::IMREAD_GRAYSCALE);
    if (src.empty())
        return -1;

    // 预处理：高斯模糊平滑噪声
    cv::Mat blurred;
    cv::GaussianBlur(src, blurred, cv::Size(9, 9), 2, 2);

    // 2. 计算图像梯度 (Sobel 算子)
    cv::Mat grad_x, grad_y;
    cv::Sobel(blurred, grad_x, CV_32F, 1, 0);
    cv::Sobel(blurred, grad_y, CV_32F, 0, 1);

    // 3. 定义参数空间与累加器
    int minRadius = 15;
    int maxRadius = 35;
    int rows = src.rows;
    int cols = src.cols;

    struct CircleCandidate
    {
        int x, y, r, votes;
    };
    std::vector<CircleCandidate> candidates;

    // 遍历半径范围
    for (int r = minRadius; r <= maxRadius; ++r)
    {
        // 创建当前半径下的 2D 累加器（画布大小）
        cv::Mat accumulator = cv::Mat::zeros(rows, cols, CV_32S);

        for (int y = 0; y < rows; ++y)
        {
            for (int x = 0; x < cols; ++x)
            {
                float gx = grad_x.at<float>(y, x);
                float gy = grad_y.at<float>(y, x);
                float magnitude = std::sqrt(gx * gx + gy * gy);

                if (magnitude > 100.0f)
                {
                    // 利用梯度方向：圆心一定在梯度向量的方向上
                    // a = x +/- r * cos(theta), b = y +/- r * sin(theta)
                    float cosTheta = gx / magnitude;
                    float sinTheta = gy / magnitude;

                    // 沿着梯度正反两个方向各投一票（因为不知道圆在亮侧还是暗侧）
                    for (int side : {-1, 1})
                    {
                        int cx = std::round(x + side * r * cosTheta);
                        int cy = std::round(y + side * r * sinTheta);

                        if (cx >= 0 && cx < cols && cy >= 0 && cy < rows)
                        {
                            accumulator.at<int>(cy, cx)++;
                        }
                    }
                }
            }
        }

        // 寻找当前半径平面下投票数极高的点
        double min_val, max_val;
        cv::Point max_loc;
        cv::minMaxLoc(accumulator, &min_val, &max_val, nullptr, &max_loc);

        if (max_val > 25)
        {
            candidates.push_back({max_loc.x, max_loc.y, r, (int)max_val});
        }
    }

    // --- 核心算法结束 ---

    // 4. 绘制结果
    cv::Mat result;
    cv::cvtColor(src, result, cv::COLOR_GRAY2BGR);

    for (const auto &c : candidates)
    {

        cv::circle(result, cv::Point(c.x, c.y), c.r, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
        cv::circle(result, cv::Point(c.x, c.y), 2, cv::Scalar(0, 255, 0), -1);
    }

    cv::imwrite("manual_detected_vessel.png", result);
    return 0;
}