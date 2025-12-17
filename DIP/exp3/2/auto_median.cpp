#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;
int main(int argc, char **argv)
{
    Mat src = imread("ckt_saltpep_prob_pt25.tif", IMREAD_GRAYSCALE);
    if (src.empty())
    {
        cout << "无法读取图像" << endl;
        return -1;
    }

    // 默认最大窗口大小
    int Smax = 7;
    if (argc > 1)
    {
        Smax = atoi(argv[1]);
        if (Smax < 3 || Smax % 2 == 0)
        {
            cout << "最大窗口大小必须为奇数且至少为3" << endl;
            return -1;
        }
    }

    // 自适应中值滤波实现
    auto adaptiveMedianFilter = [&](const Mat &input, Mat &output, int smax)
    {
        output.create(input.size(), input.type());

        int rows = input.rows;
        int cols = input.cols;
        int pad = smax / 2; // 最大半径

        Mat padded;
        copyMakeBorder(input, padded, pad, pad, pad, pad, BORDER_REPLICATE);

        for (int y = 0; y < rows; ++y)
        {
            for (int x = 0; x < cols; ++x)
            {
                int zxy = padded.at<uchar>(y + pad, x + pad);
                int finalPixel = zxy;
                int lastMedian = zxy;

                bool pixelSet = false;

                for (int s = 3; s <= smax; s += 2)
                {
                    int half = s / 2;
                    vector<int> window;
                    window.reserve(s * s);
                    for (int ry = -half; ry <= half; ++ry)
                    {
                        const uchar *prow = padded.ptr<uchar>(y + pad + ry);
                        for (int rx = -half; rx <= half; ++rx)
                        {
                            window.push_back(prow[x + pad + rx]);
                        }
                    }
                    sort(window.begin(), window.end());
                    int zmin = window.front();
                    int zmax = window.back();
                    int zmed = window[window.size() / 2];
                    lastMedian = zmed;

                    int A1 = zmed - zmin;
                    int A2 = zmed - zmax;

                    // Stage A
                    if (A1 > 0 && A2 < 0)
                    {
                        // Stage B
                        int B1 = zxy - zmin;
                        int B2 = zxy - zmax;
                        if (B1 > 0 && B2 < 0)
                        {
                            finalPixel = zxy;
                        }
                        else
                        {
                            finalPixel = zmed;
                        }
                        pixelSet = true;
                        break;
                    }
                    // 否则增大窗口继续
                }

                if (!pixelSet)
                    finalPixel = lastMedian;

                output.at<uchar>(y, x) = static_cast<uchar>(finalPixel);
            }
        }
    };

    Mat dst;
    adaptiveMedianFilter(src, dst, Smax);
    string outname = "adaptive_median_smax" + to_string(Smax) + ".png";
    imwrite(outname, dst);
    return 0;
}
