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
    if (argc > 1)
    {
        int ksize = atoi(argv[1]);
        if (ksize % 2 == 0) // 中值滤波核大小必须为奇数
        {
            cout << "中值滤波核大小必须为奇数" << endl;
            return -1;
        }
        Mat dst;
        medianBlur(src, dst, ksize);
        imwrite("median_size" + to_string(ksize) + ".png", dst);
        return 0;
    }
    return 0;
}
