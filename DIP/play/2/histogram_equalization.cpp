#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <cstdint>

int main()
{
    // 加载灰度图像
    cv::Mat img = cv::imread("input.jpg", cv::IMREAD_GRAYSCALE);
    if (img.empty())
    {
        std::cerr << "Error: Cannot open or find the image.\n";
        return -1;
    }

    // Step 1. 计算灰度直方图（0~255）
    std::vector<int> hist(256, 0);
    for (int y = 0; y < img.rows; ++y)
    {
        const uint8_t *row_ptr = img.ptr<uint8_t>(y);
        for (int x = 0; x < img.cols; ++x)
        {
            ++hist[row_ptr[x]];
        }
    }

    // Step 2. 计算累计分布函数（CDF）
    std::vector<int> cdf(256);
    std::partial_sum(hist.begin(), hist.end(), cdf.begin());

    // Step 3. 归一化 CDF 到 [0, 255]
    int total_pixels = img.rows * img.cols;

    // 找到第一个非零的 CDF（避免暗图出现拉伸错误）
    int cdf_min = 0;
    for (int i = 0; i < 256; ++i)
    {
        if (cdf[i] != 0)
        {
            cdf_min = cdf[i];
            break;
        }
    }

    std::vector<uint8_t> lut(256);
    for (int i = 0; i < 256; ++i)
    {
        double normalized = static_cast<double>(cdf[i] - cdf_min) / (total_pixels - cdf_min);
        int equalized_val = std::clamp(static_cast<int>(normalized * 255.0 + 0.5), 0, 255);
        lut[i] = static_cast<uint8_t>(equalized_val);
    }

    // Step 4. 应用查找表（LUT）生成均衡化结果
    cv::Mat equalized(img.size(), img.type());
    for (int y = 0; y < img.rows; ++y)
    {
        const uint8_t *src_row = img.ptr<uint8_t>(y);
        uint8_t *dst_row = equalized.ptr<uint8_t>(y);
        for (int x = 0; x < img.cols; ++x)
        {
            dst_row[x] = lut[src_row[x]];
        }
    }

    // Step 5. 保存结果
    cv::imwrite("output.jpg", equalized);

    return 0;
}
