#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>

using namespace cv;
using namespace std;

int main()
{
    string image_path = "car.jpg";
    Mat img = imread(image_path, IMREAD_GRAYSCALE);
    img.convertTo(img, CV_32F);

    // 构建近似金字塔
    int levels = 3;
    vector<Mat> gaussian_pyramid;
    gaussian_pyramid.push_back(img); // Level 0 (原始图像 j)

    for (int i = 0; i < levels; ++i)
    {
        Mat down;
        // 高斯模糊
        GaussianBlur(gaussian_pyramid[i], down, Size(5, 5), 0);
        // 缩小一半
        resize(down, down, Size(down.cols / 2, down.rows / 2));

        gaussian_pyramid.push_back(down);
    }

    // 构建残差金字塔
    vector<Mat> laplacian_pyramid;

    for (int i = 0; i < levels; ++i)
    {
        Mat up;
        // 将下一层图像上采样并放大到当前层的大小
        // 先放大到当前层大小
        resize(gaussian_pyramid[i + 1], up, gaussian_pyramid[i].size());
        // 再进行高斯模糊
        GaussianBlur(up, up, Size(5, 5), 0);

        // 计算残差: L_i = G_i - Expand(G_{i+1})
        Mat residual;
        subtract(gaussian_pyramid[i], up, residual);
        laplacian_pyramid.push_back(residual);
    }

    for (int i = 0; i < levels; ++i)
    {
        Mat show_approx, show_resid;
        gaussian_pyramid[i].convertTo(show_approx, CV_8U);
        normalize(laplacian_pyramid[i], show_resid, 0, 255, NORM_MINMAX, CV_8U);

        imwrite(std::format("gaussian_level_{}.bmp", i), show_approx);
        imwrite(std::format("laplacian_level_{}.bmp", i), show_resid);
    }
    return 0;
}