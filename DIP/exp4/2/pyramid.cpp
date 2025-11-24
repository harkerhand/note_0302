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

    for (int i = 0; i < levels - 1; ++i)
    {
        Mat down;
        // pyrDown 执行高斯模糊 + 下采样
        pyrDown(gaussian_pyramid[i], down);
        gaussian_pyramid.push_back(down);
    }

    // 构建残差金字塔
    vector<Mat> laplacian_pyramid;

    for (int i = 0; i < levels - 1; ++i)
    {
        Mat up;
        // 将下一层图像上采样并放大到当前层的大小
        pyrUp(gaussian_pyramid[i + 1], up, gaussian_pyramid[i].size());

        // 计算残差: L_i = G_i - Expand(G_{i+1})
        Mat residual;
        subtract(gaussian_pyramid[i], up, residual);
        laplacian_pyramid.push_back(residual);
    }
    // 最后一层特殊处理，直接存储最顶层的近似图像
    laplacian_pyramid.push_back(gaussian_pyramid.back());

    for (int i = 0; i < levels; ++i)
    {
        string title_approx = std::format("Approximation (Gaussian) Level {}", i);
        string title_resid = std::format("Residual (Laplacian) Level {}", i);

        Mat show_approx, show_resid;
        gaussian_pyramid[i].convertTo(show_approx, CV_8U);
        normalize(laplacian_pyramid[i], show_resid, 0, 255, NORM_MINMAX, CV_8U);

        imwrite(std::format("gaussian_level_{}.bmp", i), show_approx);
        imwrite(std::format("laplacian_level_{}.bmp", i), show_resid);
    }

    // 验证重建
    // 重建公式: G_i = L_i + Expand(G_{i+1})
    Mat reconstructed = laplacian_pyramid.back(); // 从顶层开始

    for (int i = levels - 2; i >= 0; --i)
    {
        Mat up;
        pyrUp(reconstructed, up, laplacian_pyramid[i].size());
        add(laplacian_pyramid[i], up, reconstructed);
    }

    // 转换回 8位显示
    Mat show_recon;
    reconstructed.convertTo(show_recon, CV_8U);
    imwrite("reconstructed_image.bmp", show_recon);

    // 计算与原图的差异
    Mat diff;
    Mat original_8u;
    img.convertTo(original_8u, CV_8U);
    absdiff(original_8u, show_recon, diff);
    cout << "重建误差 (Sum of Diff): " << sum(diff)[0] << endl;
    return 0;
}