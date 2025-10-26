#include <opencv2/opencv.hpp>
#include <iostream>

cv::Mat conv2D(const cv::Mat &image, const int kSize, std::function<double(std::vector<uchar>)> getValue)
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
            dst.at<uchar>(y, x) = cv::saturate_cast<uchar>(getValue(neighbors));
        }
    }

    return dst;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " <image_path> <filter_type>" << std::endl;
        std::cout << "Filter types: box, gaussian, median, f2" << std::endl;
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

        result = conv2D(image, 3, [](std::vector<uchar> neighbors)
                        {
            double sum = 0.0;
            for (uchar val : neighbors)
            {
                sum += val;
            }
            return sum / neighbors.size(); });
    }
    else if (filterType == "gaussian")
    {

        result = conv2D(image, 3, [](std::vector<uchar> neighbors)
                        {
            // 3x3 高斯核权重
            std::vector<double> weights = {
                1/16.0, 2/16.0, 1/16.0,
                2/16.0, 4/16.0, 2/16.0,
                1/16.0, 2/16.0, 1/16.0
            };
            double sum = 0.0;
            for (size_t i = 0; i < neighbors.size(); ++i)
            {
                sum += neighbors[i] * weights[i];
            }
            return sum; });
    }
    else if (filterType == "median")
    {
        result = conv2D(image, 3, [](std::vector<uchar> neighbors)
                        {
            std::sort(neighbors.begin(), neighbors.end());
            return neighbors[neighbors.size() / 2]; });
    }
    else if (filterType == "f2")
    {
        cv::Mat tmp = conv2D(image, 3, [](std::vector<uchar> neighbors)
                             { return neighbors[0] * 1 + neighbors[1] * 1 + neighbors[2] * 1 +
                                      neighbors[3] * 1 + neighbors[4] * -8 + neighbors[5] * 1 +
                                      neighbors[6] * 1 + neighbors[7] * 1 + neighbors[8] * 1; });
        result = image - 5 * tmp;
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
