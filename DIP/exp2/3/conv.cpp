#include <opencv2/opencv.hpp>
#include <iostream>

cv::Mat conv2D(const cv::Mat &image, const cv::Mat &kernel)
{
    int kRows = kernel.rows;
    int kCols = kernel.cols;
    int padY = kRows / 2;
    int padX = kCols / 2;

    // 镜像翻转填充
    cv::Mat src;
    cv::copyMakeBorder(image, src, padY, padY, padX, padX, cv::BORDER_REFLECT);
    cv::Mat dst(image.size(), image.type());

    for (int y = 0; y < image.rows; ++y)
    {
        for (int x = 0; x < image.cols; ++x)
        {
            double sum = 0.0;
            for (int ky = 0; ky < kRows; ++ky)
            {
                for (int kx = 0; kx < kCols; ++kx)
                {
                    int imgY = y + ky;
                    int imgX = x + kx;
                    sum += src.at<uchar>(imgY, imgX) * kernel.at<double>(ky, kx);
                }
            }
            dst.at<uchar>(y, x) = cv::saturate_cast<uchar>(sum);
        }
    }

    return dst;
}

cv::Mat medianBlur(const cv::Mat &image, const int kSize)
{
    int kRows = kSize;
    int kCols = kSize;
    int padY = kRows / 2;
    int padX = kCols / 2;

    // 镜像翻转填充
    cv::Mat src;
    cv::copyMakeBorder(image, src, padY, padY, padX, padX, cv::BORDER_REFLECT);

    cv::Mat dst(image.size(), image.type());

    for (int y = 0; y < image.rows; ++y)
    {
        for (int x = 0; x < image.cols; ++x)
        {
            std::vector<uchar> neighbors;
            for (int ky = 0; ky < kRows; ++ky)
            {
                for (int kx = 0; kx < kCols; ++kx)
                {
                    int imgY = y + ky;
                    int imgX = x + kx;
                    neighbors.push_back(src.at<uchar>(imgY, imgX));
                }
            }
            std::sort(neighbors.begin(), neighbors.end());
            dst.at<uchar>(y, x) = neighbors[neighbors.size() / 2];
        }
    }

    return dst;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " <image_path> <filter_type>" << std::endl;
        std::cout << "Filter types: box, gaussian, median" << std::endl;
        return 1;
    }

    std::string imagePath = argv[1];
    std::string filterType = argv[2];

    cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
    if (image.empty())
    {
        std::cout << "Error: Could not load image from " << imagePath << std::endl;
        return 1;
    }

    cv::Mat result;
    if (filterType == "box")
    {
        cv::Mat kernel = (cv::Mat_<double>(3, 3) << 1.0 / 9, 1.0 / 9, 1.0 / 9,
                          1.0 / 9, 1.0 / 9, 1.0 / 9,
                          1.0 / 9, 1.0 / 9, 1.0 / 9);
        result = conv2D(image, kernel);
    }
    else if (filterType == "gaussian")
    {
        cv::Mat kernel = (cv::Mat_<double>(3, 3) << 1.0 / 16, 2.0 / 16, 1.0 / 16,
                          2.0 / 16, 4.0 / 16, 2.0 / 16,
                          1.0 / 16, 2.0 / 16, 1.0 / 16);
        result = conv2D(image, kernel);
    }
    else if (filterType == "median")
    {
        result = medianBlur(image, 3);
    }
    else
    {
        std::cout << "Error: Invalid filter type. Choose from: box, gaussian, median" << std::endl;
        return 1;
    }

    // Save the result
    std::string outputPath = imagePath + "_" + filterType + ".png";
    cv::imwrite(outputPath, result);
    std::cout << "Filtered image saved to " << outputPath << std::endl;

    return 0;
}
