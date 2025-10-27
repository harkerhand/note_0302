#include <opencv2/opencv.hpp>
#include <iostream>
std::vector<cv::Mat> secondOrderFilter(const cv::Mat &image_in, bool use8 = false, double k = 1.0)
{
    // 转为 double 精度
    cv::Mat image;
    image_in.convertTo(image, CV_64F);

    // 定义卷积核
    cv::Mat kernel;
    if (use8)
    {
        // -8 核
        kernel = (cv::Mat_<double>(3, 3) << 1, 1, 1,
                  1, -8, 1,
                  1, 1, 1);
    }
    else
    {
        // -4 核
        kernel = (cv::Mat_<double>(3, 3) << 0, 1, 0,
                  1, -4, 1,
                  0, 1, 0);
    }

    // 卷积运算
    cv::Mat laplace;
    cv::filter2D(image, laplace, CV_64F, kernel);

    // 增强结果：image - k * laplace
    cv::Mat enhanced = image - k * laplace;

    // 限制范围 [0, 255]
    cv::Mat laplace_clipped, enhanced_clipped;
    cv::min(cv::max(laplace, 0.0), 255.0, laplace_clipped);
    cv::min(cv::max(enhanced, 0.0), 255.0, enhanced_clipped);

    // 转回 8-bit
    cv::Mat laplace_u8, enhanced_u8;
    laplace_clipped.convertTo(laplace_u8, CV_8U);
    enhanced_clipped.convertTo(enhanced_u8, CV_8U);

    return {image_in, laplace_u8, enhanced_u8};
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " <image_path> <kernel_type>\n";
        std::cout << "kernel_type: 4 or 8 (for -4 or -8 kernel)" << std::endl;
        return 1;
    }

    std::string imagePath = argv[1];
    int kernelType = std::stoi(argv[2]);

    cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
    if (image.empty())
    {
        std::cerr << "Error: Could not load image from " << imagePath << std::endl;
        return 1;
    }

    // 二阶导滤波
    bool use8 = (kernelType == 8);
    auto result_vec = secondOrderFilter(image, use8, 1.0);

    // 拼接显示结果
    cv::Mat result;
    cv::hconcat(result_vec, result);

    std::string outName = imagePath + (use8 ? "_laplace8.png" : "_laplace4.png");
    cv::imwrite(outName, result);

    std::cout << "Saved: " << outName << std::endl;

    return 0;
}
