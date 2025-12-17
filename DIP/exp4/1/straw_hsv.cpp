#include <opencv2/opencv.hpp>
using namespace cv;

int main()
{
    // 1. 加载图像
    Mat img = imread("strawberries_fullcolor.tif");
    if (img.empty())
    {
        printf("Error: Could not open image 'strawberries_fullcolor.tif'.\n");
        return -1;
    }

    imwrite("strawberries_fullcolor.bmp", img);

    Mat hsv;
    cvtColor(img, hsv, COLOR_BGR2HSV);

    Scalar red_lower_1(0, 120, 70);
    Scalar red_upper_1(10, 255, 255);
    Scalar red_lower_2(170, 120, 70);
    Scalar red_upper_2(179, 255, 255);

    Scalar green_lower(20, 130, 0);
    Scalar green_upper(50, 255, 210);

    Mat redMask1, redMask2, redMaskTotal, greenMask;

    inRange(hsv, red_lower_1, red_upper_1, redMask1);
    inRange(hsv, red_lower_2, red_upper_2, redMask2);
    bitwise_or(redMask1, redMask2, redMaskTotal); // 合并两个红色掩膜
    Mat redResult = img.clone();
    redResult.setTo(Scalar(127, 127, 127), ~redMaskTotal);
    imwrite("strawberries_red_hsv_partial_pyref.bmp", redResult);

    // 创建绿色掩膜
    inRange(hsv, green_lower, green_upper, greenMask);
    Mat greenResult = img.clone();
    greenResult.setTo(Scalar(127, 127, 127), ~greenMask);
    imwrite("strawberries_green_hsv_pyref.bmp", greenResult);

    Mat fullMask;
    bitwise_or(redMaskTotal, greenMask, fullMask);
    Mat fullResult = img.clone();
    fullResult.setTo(Scalar(127, 127, 127), ~fullMask);
    imwrite("strawberries_red_green_hsv_partial_pyref.bmp", fullResult);
    return 0;
}