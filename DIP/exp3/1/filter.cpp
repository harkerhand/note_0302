#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// 单通道中心化
void fftShiftSingle(Mat &mag)
{
    int cx = mag.cols / 2;
    int cy = mag.rows / 2;

    Mat q0(mag, Rect(0, 0, cx, cy));   // 左上
    Mat q1(mag, Rect(cx, 0, cx, cy));  // 右上
    Mat q2(mag, Rect(0, cy, cx, cy));  // 左下
    Mat q3(mag, Rect(cx, cy, cx, cy)); // 右下

    Mat tmp;
    // 交换左上 <-> 右下
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);

    // 交换右上 <-> 左下
    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);
}

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

// 构建竖直陷波滤波器，直接作用于中心化频谱
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
            // 阻带竖直带，中心在 cx
            if (std::abs(x - cx) <= bandHalfWidth && std::abs(y - cy) >= skipCenterRadius)
            {
                filter.at<cv::Vec2f>(y, x)[0] = 0.0f; // 实部
                filter.at<cv::Vec2f>(y, x)[1] = 0.0f; // 虚部
            }
        }
    }

    return filter;
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

    // 构建复数矩阵
    Mat planes[] = {Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F)};
    Mat complexI;
    merge(planes, 2, complexI);

    // DFT
    dft(complexI, complexI);

    // 中心化频谱
    split(complexI, planes);
    fftShiftSingle(planes[0]);
    fftShiftSingle(planes[1]);
    merge(planes, 2, complexI);

    // 频谱
    Mat spectrum = spectrumToImage(complexI);
    imwrite("fft_result.png", spectrum);

    // 构建陷波滤波器
    int bandHalfWidth = 2;    // 阻带半宽度
    int skipCenterRadius = 6; // 中心跳过半径
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
    // 先中心化回去
    split(filteredComplex, planes);
    fftShiftSingle(planes[0]);
    fftShiftSingle(planes[1]);
    merge(planes, 2, filteredComplex);

    Mat invDFT;
    dft(filteredComplex, invDFT, DFT_INVERSE | DFT_REAL_OUTPUT | DFT_SCALE);

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
    // 逆中心化
    split(noiseSpectrum, planes);
    fftShiftSingle(planes[0]);
    fftShiftSingle(planes[1]);
    merge(planes, 2, noiseSpectrum);
    // 逆 DFT 得到噪声图像
    Mat noiseImage;
    dft(noiseSpectrum, noiseImage, DFT_INVERSE | DFT_REAL_OUTPUT | DFT_SCALE);
    noiseImage = noiseImage(Rect(0, 0, src.cols, src.rows));
    normalize(noiseImage, noiseImage, 0, 255, NORM_MINMAX);
    noiseImage.convertTo(noiseImage, CV_8U);
    imwrite("extracted_noise.png", noiseImage);

    return 0;
}
