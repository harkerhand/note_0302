#include <opencv2/opencv.hpp>
#include <iostream>
#include <algorithm>

/// @brief 扫描核覆盖区域，只有当所有像素都为255时，中心像素才设为255
/// @param src
/// @param kernelSize
/// @return
cv::Mat myErode(const cv::Mat &src, int kernelSize)
{
    cv::Mat dst = cv::Mat::zeros(src.size(), src.type());
    int offset = kernelSize / 2;

    for (int y = offset; y < src.rows - offset; ++y)
    {
        for (int x = offset; x < src.cols - offset; ++x)
        {
            uchar minVal = 255;
            // 遍历核区域
            for (int ky = -offset; ky <= offset; ++ky)
            {
                for (int kx = -offset; kx <= offset; ++kx)
                {
                    minVal = std::min(minVal, src.at<uchar>(y + ky, x + kx));
                }
            }
            dst.at<uchar>(y, x) = minVal;
        }
    }
    return dst;
}

/// @brief 扫描核覆盖区域，只要有一个像素为255，中心像素就设为255（取最大值）
/// @param src
/// @param kernelSize
/// @return
cv::Mat myDilate(const cv::Mat &src, int kernelSize)
{
    cv::Mat dst = cv::Mat::zeros(src.size(), src.type());
    int offset = kernelSize / 2;

    for (int y = offset; y < src.rows - offset; ++y)
    {
        for (int x = offset; x < src.cols - offset; ++x)
        {
            uchar maxVal = 0;
            // 遍历核区域
            for (int ky = -offset; ky <= offset; ++ky)
            {
                for (int kx = -offset; kx <= offset; ++kx)
                {
                    maxVal = std::max(maxVal, src.at<uchar>(y + ky, x + kx));
                }
            }
            dst.at<uchar>(y, x) = maxVal;
        }
    }
    return dst;
}

int main()
{
    cv::Mat img1 = cv::imread("charactor_01.png", cv::IMREAD_GRAYSCALE);
    cv::Mat img2 = cv::imread("charactor_02.png", cv::IMREAD_GRAYSCALE);

    if (img1.empty() || img2.empty())
        return -1;

    cv::Mat thresh1, thresh2;
    cv::threshold(img1, thresh1, 127, 255, cv::THRESH_BINARY);
    cv::threshold(img2, thresh2, 127, 255, cv::THRESH_BINARY);

    int kSize = 7;

    // --- 任务 A: 开运算 (先腐蚀后膨胀) ---
    cv::Mat res_opening = myDilate(myErode(thresh1, kSize), kSize);

    // --- 任务 B: 闭运算 (先膨胀后腐蚀) ---
    cv::Mat res_closing = myErode(myDilate(thresh2, kSize), kSize);

    cv::imwrite("res_opening.bmp", res_opening);
    cv::imwrite("res_closing.bmp", res_closing);

    return 0;
}