#include <opencv2/opencv.hpp>
using namespace cv;

int main()
{
    Mat img = imread("strawberries_fullcolor.tif");
    if (img.empty())
        return -1;

    // B,G,R
    Vec3b redRef = {50, 50, 200};
    Vec3b greenRef = {40, 150, 140};

    int redThresh = 120;
    int greenThresh = 120;

    Mat redMask(img.rows, img.cols, CV_8UC1);
    Mat greenMask(img.rows, img.cols, CV_8UC1);

    for (int y = 0; y < img.rows; y++)
    {
        for (int x = 0; x < img.cols; x++)
        {
            Vec3b p = img.at<Vec3b>(y, x);

            int dRed =
                abs(p[0] - redRef[0]) +
                abs(p[1] - redRef[1]) +
                abs(p[2] - redRef[2]);

            int dGreen =
                abs(p[0] - greenRef[0]) +
                abs(p[1] - greenRef[1]) +
                abs(p[2] - greenRef[2]);

            redMask.at<uchar>(y, x) = (dRed < redThresh) ? 255 : 0;
            greenMask.at<uchar>(y, x) = (dGreen < greenThresh) ? 255 : 0;
        }
    }

    Mat fullMask;
    bitwise_or(redMask, greenMask, fullMask);
    Mat result = img.clone();
    result.setTo(Scalar(127, 127, 127), ~fullMask);
    imwrite("strawberries_segmented.bmp", result);
    imwrite("strawberries_red.bmp", redMask);
    imwrite("strawberries_green.bmp", greenMask);
    imwrite("strawberries_mask.bmp", fullMask);

    return 0;
}
