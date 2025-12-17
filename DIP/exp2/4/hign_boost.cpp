#include <opencv2/opencv.hpp>
#include <iostream>

// 返回 Vector<cv::Mat>, 包含 平滑图 Mask 和 高提升图
std::vector<cv::Mat> highBoostFilter(const cv::Mat &image_in, float k = 1.5, int kernelSize = 3, double sigma = 1.0)
{

    cv::Mat image;
    image_in.convertTo(image, CV_64F);

    cv::Mat smooth;
    // 高斯平滑
    cv::GaussianBlur(image, smooth, cv::Size(kernelSize, kernelSize), sigma);

    cv::Mat mask = image - smooth;
    cv::Mat highboost = image + k * mask;

    // 限制范围 [0, 255]
    cv::min(cv::max(smooth, 0.0), 255.0, smooth);
    cv::min(cv::max(mask, 0.0), 255.0, mask);
    cv::min(cv::max(highboost, 0.0), 255.0, highboost);

    cv::Mat smooth_u8, mask_u8, highboost_u8;
    smooth.convertTo(smooth_u8, CV_8U);
    mask.convertTo(mask_u8, CV_8U);
    highboost.convertTo(highboost_u8, CV_8U);
    return {smooth_u8, mask_u8, highboost_u8};
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <image_path>" << std::endl;
        return 1;
    }

    std::string imagePath = argv[1];

    cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
    if (image.empty())
    {
        std::cout << "Error: Could not load image from " << imagePath << std::endl;
        return 1;
    }

    std::vector<cv::Mat> result_vec = highBoostFilter(image, 1.5, 11, 5);
    cv::Mat result;
    cv::hconcat(result_vec, result);
    cv::hconcat(image, result, result);

    cv::imwrite(imagePath + "_highboost.png", result);

    return 0;
}
