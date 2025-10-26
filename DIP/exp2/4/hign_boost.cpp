#include <opencv2/opencv.hpp>
#include <iostream>

cv::Mat highBoostFilter(const cv::Mat &image, float k = 1.5, int kernelSize = 3, double sigma = 1.0)
{
    cv::Mat smooth;
    // 高斯平滑
    cv::GaussianBlur(image, smooth, cv::Size(kernelSize, kernelSize), sigma);

    cv::Mat highboost;
    // 高提升滤波公式
    highboost = (1 + k) * image - k * smooth;

    // 保证像素值在0-255
    cv::Mat result;
    highboost.convertTo(result, CV_8U);
    return result;
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

    cv::Mat result = highBoostFilter(image, 15, 3, 1.0);

    cv::imwrite(imagePath + "_highboost.png", result);

    return 0;
}
