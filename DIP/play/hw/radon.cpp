// radon.cpp
// C++20, OpenCV >= 4
//
// 编译： g++ -std=c++20 radon.cpp `pkg-config --cflags --libs opencv4` -O2 -o radon
//
// 用法： ./radon input.png sinogram.png profile.png [num_angles] [profile_angle_deg]
//   默认 num_angles = 180 (0..179 deg)
//   默认 profile_angle_deg = 90
//
// 输出：生成 sinogram（灰度图）和指定角度的剖面图（profile）

#include <opencv2/opencv.hpp>
#include <cmath>
#include <iostream>
#include <string>

using namespace cv;
using std::cerr;
using std::cout;
using std::endl;
using std::string;

static Mat rotateImage(const Mat &src, double angle_deg, const Size &dstSize)
{
    // 旋转中心取图像中心；使用双线性插值并填充为 0（黑）
    Point2f center((float)(src.cols / 2.0), (float)(src.rows / 2.0));
    Mat M = getRotationMatrix2D(center, angle_deg, 1.0);

    // 为了把旋转后的图像完整装下，先把仿射矩阵平移到目标图像中心
    // 这里假设 dstSize 是足够大的（通常为对角线长度的正方形）
    // 需要把旋转中心从 src 的中心映射到 dst 的中心
    M.at<double>(0, 2) += (dstSize.width - src.cols) / 2.0;
    M.at<double>(1, 2) += (dstSize.height - src.rows) / 2.0;

    Mat dst;
    warpAffine(src, dst, M, dstSize, INTER_LINEAR, BORDER_CONSTANT, Scalar(0));
    return dst;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " input.png [num_angles] [profile_angle_deg]\n";
        return 0;
    }

    string inPath = argv[1];
    int numAngles = (argc >= 3) ? std::stoi(argv[2]) : 180;
    int profileAngleDeg = (argc >= 4) ? std::stoi(argv[3]) : 90;
    string sinogramOut = "sinogram" + std::to_string(numAngles) + ".png";
    string profileOut = "profile" + std::to_string(profileAngleDeg) + ".png";

    // 读入灰度图
    Mat img = imread(inPath, IMREAD_GRAYSCALE);
    if (img.empty())
    {
        cerr << "Error: cannot load image: " << inPath << endl;
        return 1;
    }

    // 转为 double 精度以避免整型溢出和丢失
    Mat img_d;
    img.convertTo(img_d, CV_64F, 1.0 / 255.0);

    // 为旋转后图像准备合适尺寸：使用对角线长度作为宽高（保证旋转不裁剪）
    double diag = std::sqrt((double)img_d.cols * img_d.cols + (double)img_d.rows * img_d.rows);
    int outSize = (int)std::ceil(diag);
    Size padSize(outSize, outSize);

    // 为了把原图放在中心，创建一个 pad 图像（将 src 放到中心）
    Mat padded = Mat::zeros(padSize, img_d.type());
    int xoff = (padSize.width - img_d.cols) / 2;
    int yoff = (padSize.height - img_d.rows) / 2;
    img_d.copyTo(padded(Rect(xoff, yoff, img_d.cols, img_d.rows)));

    // sinogram: 每一行对应一个角度，每列对应投影位置 s
    Mat sinogram = Mat::zeros(numAngles, outSize, CV_64F);

    // 角度从 0 到 180（不含 180）等分
    for (int i = 0; i < numAngles; ++i)
    {
        double theta = (180.0 * i) / numAngles; // degrees
        // 为了得到角度 theta 的投影，我们把图像逆向旋转 -theta（使该投影变为竖直方向上的列和）
        Mat rotated = rotateImage(padded, -theta, padSize);

        // 对列方向求和 => 得到长度为 outSize 的投影向量
        Mat projection;
        // 把求和结果放到 projection（1 x outSize）
        reduce(rotated, projection, 0, REDUCE_SUM, CV_64F);

        for (int c = 0; c < outSize; ++c)
        {
            sinogram.at<double>(i, c) = projection.at<double>(0, c);
        }
    }

    // 归一化 sinogram 到 [0,255] 以方便保存显示
    double smin, smax;
    minMaxLoc(sinogram, &smin, &smax);
    Mat sinogram_norm;
    if (smax - smin < 1e-12)
    {
        sinogram_norm = Mat::zeros(sinogram.size(), CV_8U);
    }
    else
    {
        sinogram.convertTo(sinogram_norm, CV_8U, 255.0 / (smax - smin), -smin * 255.0 / (smax - smin));
    }

    imwrite(sinogramOut, sinogram_norm);
    cout << "Saved sinogram to " << sinogramOut << endl;

    // 生成 profile 图像（剖面）: 找到最接近的角度索引
    // 将角度映射回索引
    int angleIndex = (int)std::round((profileAngleDeg / 180.0) * numAngles) % numAngles;
    if (angleIndex < 0)
        angleIndex += numAngles;

    // 取出该行投影（double）
    Mat profileRow = sinogram.row(angleIndex); // 1 x outSize

    // 归一化 profile 到画布高度
    int plotW = outSize;
    int plotH = 400;
    Mat plot = Mat::zeros(plotH, plotW, CV_8U);

    // 将 profileRow 线性映射到 [0, plotH-1]
    double pmin, pmax;
    minMaxLoc(profileRow, &pmin, &pmax);
    if (pmax - pmin < 1e-12)
    {
        // 全零投影 -> 空白图
    }
    else
    {
        // 画折线（从左到右）
        std::vector<Point> pts;
        pts.reserve(plotW);
        for (int x = 0; x < plotW; ++x)
        {
            double v = profileRow.at<double>(0, x);
            double nv = (v - pmin) / (pmax - pmin);                // 0..1
            int y = plotH - 1 - (int)std::round(nv * (plotH - 1)); // 底部为 0
            pts.emplace_back(x, y);
        }
        // 把点连起来（白色折线）
        for (size_t k = 1; k < pts.size(); ++k)
        {
            line(plot, pts[k - 1], pts[k], Scalar(255), 1, LINE_AA);
        }
    }

    imwrite(profileOut, plot);
    cout << "Saved profile (angle " << profileAngleDeg << " deg, index " << angleIndex << ") to " << profileOut << endl;

    return 0;
}
