#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// 频谱可视化
Mat spectrumToImage(const Mat &complexImg)
{
    Mat planes[2];
    split(complexImg, planes); // 拆成实部和虚部
    Mat mag;
    magnitude(planes[0], planes[1], mag);     // 计算幅度
    mag += 1;                                 // 避免 log(0)
    log(mag, mag);                            // 对数尺度
    normalize(mag, mag, 0, 255, NORM_MINMAX); // 归一化到0-255
    Mat mag8U;
    mag.convertTo(mag8U, CV_8U);
    return mag8U;
}

// 构建竖直陷波滤波器
// size: 频谱大小
// bandHalfWidth: 阻带半宽度（像素）
// skipCenterRadius: 中心跳过半径（像素）
cv::Mat createNotchFilterComplex(cv::Size size, int bandHalfWidth, int skipCenterRadius)
{
    cv::Mat filter = cv::Mat::ones(size, CV_32FC2); // 复数矩阵
    if (bandHalfWidth < 0)
        return filter;
    int cx = size.width / 2;  // 频谱中心 x
    int cy = size.height / 2; // 频谱中心 y

    for (int y = 0; y < size.height; ++y)
    {
        for (int x = 0; x < size.width; ++x)
        {
            if (std::abs(x - cx) <= bandHalfWidth && std::abs(y - cy) >= skipCenterRadius)
            {
                filter.at<cv::Vec2f>(y, x)[0] = 0.0f;
                filter.at<cv::Vec2f>(y, x)[1] = 0.0f;
            }
        }
    }

    return filter;
}

// 空间域中心化: 原图乘 (-1)^(x+y)
void centerShift(Mat &img)
{
    Mat maskY = Mat::ones(img.rows, 1, CV_32F);
    Mat maskX = Mat::ones(1, img.cols, CV_32F);

    for (int i = 0; i < img.rows; i++)
        maskY.at<float>(i, 0) = (i % 2 == 0) ? 1 : -1;
    for (int j = 0; j < img.cols; j++)
        maskX.at<float>(0, j) = (j % 2 == 0) ? 1 : -1;

    Mat mask = maskY * maskX;
    multiply(img, mask, img);
}

int main()
{
    // 读取图像
    Mat src = imread("cassini.tif", IMREAD_GRAYSCALE);
    if (src.empty())
    {
        cout << "无法读取图像" << endl;
        return -1;
    }
    // 原图
    imwrite("original_image.png", src);
    src.convertTo(src, CV_32F); // 转为浮点型

    // 扩展到最优大小（加快 DFT）
    Mat padded;
    int m = getOptimalDFTSize(src.rows);
    int n = getOptimalDFTSize(src.cols);
    copyMakeBorder(src, padded, 0, m - src.rows, 0, n - src.cols, BORDER_CONSTANT, Scalar::all(0));

    // 空间域中心化
    centerShift(padded);

    // 构建复数矩阵
    Mat planes[] = {padded, Mat::zeros(padded.size(), padded.type())};
    Mat complexI;
    merge(planes, 2, complexI);

    // DFT
    dft(complexI, complexI);

    // 频谱
    Mat spectrum = spectrumToImage(complexI);
    imwrite("fft_result.png", spectrum);

    // 构建陷波滤波器
    int bandHalfWidth = 2;     // 阻带半宽度
    int skipCenterRadius = 10; // 中心跳过半径
    Mat notchFilter = createNotchFilterComplex(complexI.size(), bandHalfWidth, skipCenterRadius);

    // 可视化滤波器频谱
    Mat filterSpectrum = spectrumToImage(notchFilter);
    imwrite("notch_filter.png", filterSpectrum);

    // 应用滤波器
    Mat filteredComplex;
    mulSpectrums(complexI, notchFilter, filteredComplex, 0);

    // 可视化滤波后频谱
    Mat filteredSpectrum = spectrumToImage(filteredComplex);
    imwrite("fft_filtered_result.png", filteredSpectrum);

    // 逆 DFT
    Mat invDFT;
    dft(filteredComplex, invDFT, DFT_INVERSE | DFT_REAL_OUTPUT | DFT_SCALE);
    // 逆空间域中心化
    centerShift(invDFT);
    // 裁剪到原始大小
    invDFT = invDFT(Rect(0, 0, src.cols, src.rows));
    normalize(invDFT, invDFT, 0, 255, NORM_MINMAX);
    invDFT.convertTo(invDFT, CV_8U);
    imwrite("fft_filtered_image.png", invDFT);

    // 构造反滤波器提取噪声
    Mat inverseFilter = Mat::ones(notchFilter.size(), CV_32FC2) - notchFilter;
    // 提取噪声频谱
    Mat noiseSpectrum;
    mulSpectrums(complexI, inverseFilter, noiseSpectrum, 0);
    // 逆 DFT 得到噪声图像
    Mat noiseImage;
    dft(noiseSpectrum, noiseImage, DFT_INVERSE | DFT_REAL_OUTPUT | DFT_SCALE);
    // 逆空间域中心化
    centerShift(noiseImage);
    // 裁剪到原始大小
    noiseImage = noiseImage(Rect(0, 0, src.cols, src.rows));
    normalize(noiseImage, noiseImage, 0, 255, NORM_MINMAX);
    noiseImage.convertTo(noiseImage, CV_8U);
    imwrite("extracted_noise.png", noiseImage);

    return 0;
}
