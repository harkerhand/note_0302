#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <functional>

cv::Mat conv2D_matrix(const cv::Mat &image, const cv::Mat &kernel)
{

    int kSize = kernel.rows;
    int pad = kSize / 2;

    // 镜像填充
    cv::Mat src;
    cv::copyMakeBorder(image, src, pad, pad, pad, pad, cv::BORDER_REFLECT);

    cv::Mat dst(image.size(), CV_8U);

    for (int y = 0; y < image.rows; ++y)
    {
        for (int x = 0; x < image.cols; ++x)
        {
            cv::Rect roi(x, y, kSize, kSize);
            cv::Mat patch = src(roi);

            // 转为 double 类型进行乘法
            cv::Mat patch64;
            patch.convertTo(patch64, CV_64F);

            double val = cv::sum(patch64.mul(kernel))[0];
            dst.at<uchar>(y, x) = cv::saturate_cast<uchar>(val);
        }
    }

    return dst;
}

cv::Mat makeBoxKernel(int kSize)
{
    cv::Mat kernel = cv::Mat::ones(kSize, kSize, CV_64F);
    kernel /= (kSize * kSize);
    return kernel;
}

cv::Mat makeGaussianKernel(int kSize, double sigma)
{
    cv::Mat kernel1D = cv::getGaussianKernel(kSize, sigma, CV_64F);
    return kernel1D * kernel1D.t();
}

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        std::cout << "Usage: " << argv[0] << " <image_path> <filter_type> <kernel_size>\n";
        std::cout << "Filter types: box, gaussian, median\n";
        return 1;
    }

    std::string imagePath = argv[1];
    std::string filterType = argv[2];
    int kSize = std::stoi(argv[3]);

    if (kSize <= 0 || kSize % 2 == 0)
    {
        std::cerr << "Error: kernel size must be a positive odd number.\n";
        return 1;
    }

    cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
    if (image.empty())
    {
        std::cerr << "Error: Could not load image from " << imagePath << std::endl;
        return 1;
    }

    cv::Mat result;

    if (filterType == "box")
    {
        cv::Mat kernel = makeBoxKernel(kSize);
        result = conv2D_matrix(image, kernel);
    }
    else if (filterType == "gaussian")
    {
        double sigma = 0.3 * ((kSize - 1) * 0.5 - 1) + 0.8;
        cv::Mat kernel = makeGaussianKernel(kSize, sigma);
        result = conv2D_matrix(image, kernel);
    }
    else if (filterType == "median")
    {
        cv::medianBlur(image, result, kSize);
    }
    else
    {
        std::cerr << "Error: Invalid filter type. Choose from: box, gaussian, median.\n";
        return 1;
    }

    std::string outputPath = imagePath + "_" + filterType + "_k" + std::to_string(kSize) + ".png";
    cv::imwrite(outputPath, result);
    std::cout << "Filtered image saved to " << outputPath << std::endl;

    return 0;
}
