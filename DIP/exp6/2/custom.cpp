#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>

int main()
{
    cv::Mat src = cv::imread("ct_crop.png", cv::IMREAD_GRAYSCALE);
    if (src.empty())
        return -1;

    cv::Mat blurred;
    cv::GaussianBlur(src, blurred, cv::Size(9, 9), 2, 2);

    cv::Mat grad_x, grad_y;
    cv::Sobel(blurred, grad_x, CV_32F, 1, 0);
    cv::Sobel(blurred, grad_y, CV_32F, 0, 1);

    struct EdgePoint
    {
        int x;
        int y;
        double cosTheta;
        double sinTheta;
    };

    std::vector<EdgePoint> edge_points;
    for (int y = 0; y < blurred.rows; ++y)
    {
        for (int x = 0; x < blurred.cols; ++x)
        {
            float gx = grad_x.at<float>(y, x);
            float gy = grad_y.at<float>(y, x);
            float magnitude = std::sqrt(gx * gx + gy * gy);
            float cosTheta = gx / magnitude;
            float sinTheta = gy / magnitude;

            if (magnitude > 100.0f)
            {
                edge_points.emplace_back(EdgePoint{x, y, cosTheta, sinTheta});
            }
        }
    }

    int minRadius = 15;
    int maxRadius = 35;
    int rows = src.rows;
    int cols = src.cols;

    struct CircleCandidate
    {
        int x, y, r, votes;
    };
    std::vector<CircleCandidate> candidates;

    for (int r = minRadius; r <= maxRadius; ++r)
    {
        cv::Mat accumulator = cv::Mat::zeros(rows, cols, CV_32S);

        for (const auto &pt : edge_points)

        {
            int x = pt.x;
            int y = pt.y;
            double cosTheta = pt.cosTheta;
            double sinTheta = pt.sinTheta;

            // 沿着梯度正反两个方向各投一票
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

        // 寻找当前半径平面下投票数极高的点
        double min_val, max_val;
        cv::Point max_loc;
        cv::minMaxLoc(accumulator, &min_val, &max_val, nullptr, &max_loc);

        double pi = 2 * std::acos(0);
        if (max_val > 2 * 2 * std::acos(0) * r / 6.5)
        {
            candidates.push_back({max_loc.x, max_loc.y, r, (int)max_val});
        }
    }

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