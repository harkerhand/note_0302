#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath>

using namespace cv;
using namespace std;

/// @brief 使用双阈值 Otsu 方法计算两个最佳阈值
pair<int, int> getMultiOtsuThresholds(const Mat &src)
{
    // 计算直方图
    int histSize = 256;
    float range[] = {0, 256};
    const float *histRange = {range};
    Mat hist;
    calcHist(&src, 1, 0, Mat(), hist, 1, &histSize, &histRange);
    vector<double> W(256, 0.0); // 像素量前缀和
    vector<double> M(256, 0.0); // 灰度强度前缀和
    float *hData = hist.ptr<float>();
    W[0] = hData[0];
    M[0] = 0.0;
    for (int i = 1; i < 256; i++)
    {
        W[i] = W[i - 1] + hData[i];
        M[i] = M[i - 1] + i * hData[i];
    }

    float maxVariance = -1;
    pair<int, int> thresholds = {0, 0};

    // 穷举搜索两个阈值 k1, k2 (0 < k1 < k2 < 255)
    for (int k1 = 1; k1 < 254; k1++)
    {
        for (int k2 = k1 + 1; k2 < 255; k2++)
        {

            // 类别 0: [0, k1]
            double w0 = W[k1];
            double mSum0 = M[k1];
            // 类别 1: [k1 + 1, k2]
            double w1 = W[k2] - W[k1];
            double mSum1 = M[k2] - M[k1];
            // 类别 2: [k2 + 1, 255]
            double w2 = W[255] - W[k2];
            double mSum2 = M[255] - M[k2];

            if (w0 <= 0 || w1 <= 0 || w2 <= 0)
                continue;

            // 类间方差（理论上还要除以 totalPixels，但不影响最大值位置，忽略以简化计算）
            float variance = mSum0 * mSum0 / w0 + mSum1 * mSum1 / w1 + mSum2 * mSum2 / w2;
            if (variance > maxVariance)
            {
                maxVariance = variance;
                thresholds = {k1, k2};
            }
        }
    }
    return thresholds;
}

int main()
{
    // 加载图像
    Mat img = imread("raw.tif", IMREAD_GRAYSCALE);
    if (img.empty())
        return -1;
    Mat imgBlur;
    GaussianBlur(img, imgBlur, Size(5, 5), 1.5);

    // 单阈值 Otsu
    Mat singleOtsu;
    // THRESH_OTSU: 使用 Otsu 方法自动计算阈值
    double threshVal = threshold(imgBlur, singleOtsu, 0, 255, THRESH_BINARY | THRESH_OTSU);
    cout << "单阈值 Otsu 结果: " << threshVal << endl;

    // 双阈值 Otsu
    auto [k1, k2] = getMultiOtsuThresholds(imgBlur);
    cout << "双阈值 Otsu 结果: k1 = " << k1 << ", k2 = " << k2 << endl;

    // 应用双阈值分割 (结果分为 0, 127, 255 三个亮度级)
    Mat multiOtsu = Mat::zeros(img.size(), CV_8UC1);
    for (int r = 0; r < img.rows; r++)
    {
        for (int c = 0; c < img.cols; c++)
        {
            uchar pixel = img.at<uchar>(r, c);
            if (pixel <= k1)
                multiOtsu.at<uchar>(r, c) = 0;
            else if (pixel <= k2)
                multiOtsu.at<uchar>(r, c) = 127;
            else
                multiOtsu.at<uchar>(r, c) = 255;
        }
    }

    // 保存结果
    imwrite("original_image.bmp", img);
    imwrite("single_otsu_result.bmp", singleOtsu);
    imwrite("multi_otsu_result.bmp", multiOtsu);

    return 0;
}