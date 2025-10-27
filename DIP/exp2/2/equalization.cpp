#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <cstdint>

using namespace cv;

// 绘制灰度直方图
static cv::Mat draw_histogram(const std::vector<int> &hist, cv::Size size = {256, 200})
{
    int hist_w = size.width, hist_h = size.height;
    int max_val = *std::max_element(hist.begin(), hist.end());

    cv::Mat hist_img(hist_h, hist_w, CV_8UC3, cv::Scalar(255, 255, 255));

    for (int i = 0; i < 256; ++i)
    {
        int h = cvRound((double)hist[i] / max_val * hist_h);
        cv::line(hist_img, Point(i, hist_h),
                 Point(i, hist_h - h),
                 Scalar(0, 0, 0), 1);
    }
    return hist_img;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input_image>\n";
        return -1;
    }

    std::string input_image_path = argv[1];
    cv::Mat img = cv::imread(input_image_path, cv::IMREAD_GRAYSCALE);
    if (img.empty())
    {
        std::cerr << "Error: Cannot open or find the image.\n";
        return -1;
    }

    // Step 1. 计算原图直方图
    std::vector<int> hist(256, 0);
    for (int y = 0; y < img.rows; ++y)
    {
        const uint8_t *row_ptr = img.ptr<uint8_t>(y);
        for (int x = 0; x < img.cols; ++x)
            ++hist[row_ptr[x]];
    }

    // Step 2. 计算CDF
    std::vector<int> cdf(256);
    std::partial_sum(hist.begin(), hist.end(), cdf.begin());

    // Step 3. 归一化CDF
    int total_pixels = img.rows * img.cols;

    std::vector<uint8_t> lut(256);
    for (int i = 0; i < 256; ++i)
    {
        double normalized = static_cast<double>(cdf[i]) / total_pixels;
        int equalized_val = std::round(normalized * 255.0);
        lut[i] = static_cast<uint8_t>(equalized_val);
    }

    // Step 4. 应用LUT生成均衡化图像
    cv::Mat equalized(img.size(), img.type());
    for (int y = 0; y < img.rows; ++y)
    {
        const uint8_t *src_row = img.ptr<uint8_t>(y);
        uint8_t *dst_row = equalized.ptr<uint8_t>(y);
        for (int x = 0; x < img.cols; ++x)
            dst_row[x] = lut[src_row[x]];
    }

    // Step 5. 计算均衡化后的直方图
    std::vector<int> hist_eq(256, 0);
    for (int y = 0; y < equalized.rows; ++y)
    {
        const uint8_t *row_ptr = equalized.ptr<uint8_t>(y);
        for (int x = 0; x < equalized.cols; ++x)
            ++hist_eq[row_ptr[x]];
    }

    // Step 6. 绘制直方图并缩放到图像大小
    cv::Mat hist_img = draw_histogram(hist);
    cv::Mat hist_eq_img = draw_histogram(hist_eq);

    cv::resize(hist_img, hist_img, img.size());
    cv::resize(hist_eq_img, hist_eq_img, img.size());
    cv::cvtColor(hist_img, hist_img, cv::COLOR_BGR2GRAY);
    cv::cvtColor(hist_eq_img, hist_eq_img, cv::COLOR_BGR2GRAY);

    // 拼接成大图：上（原图+直方图），下（均衡化后+直方图）
    cv::Mat top, bottom, final;
    cv::hconcat(img, hist_img, top);
    cv::hconcat(equalized, hist_eq_img, bottom);
    cv::vconcat(top, bottom, final);

    // Step 7. 保存
    cv::imwrite("./eq_visual_" + input_image_path + ".png", final);

    return 0;
}
