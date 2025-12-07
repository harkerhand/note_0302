#include <opencv2/opencv.hpp>

using namespace cv;

void addVisibleWatermark(const Mat &original, const Mat &watermark, Mat &output, double alpha)
{
    Mat padded_watermark = Mat::zeros(original.size(), original.type());

    Rect roi(original.cols - watermark.cols, original.rows - watermark.rows, watermark.cols, watermark.rows);
    watermark.copyTo(padded_watermark(roi));

    addWeighted(original, 1.0, padded_watermark, alpha, 0, output);
}

void addInvisibleWatermark(const Mat &original, const Mat &watermark, Mat &output)
{
    Mat padded_watermark = Mat::zeros(original.size(), original.type());

    Rect roi(original.cols - watermark.cols, original.rows - watermark.rows, watermark.cols, watermark.rows);
    watermark.copyTo(padded_watermark(roi));

    Mat low2_bits = (padded_watermark & 0xC0) / 64;
    Mat high6_bits = original & 0xFC;
    output = high6_bits | low2_bits;
}

void extractInvisibleWatermark(const Mat &watermarked, Mat &extracted_watermark)
{
    Mat low2_bits = watermarked & 0x03;
    multiply(low2_bits, 64, extracted_watermark, 1, watermarked.type());
}

int main()
{
    Mat original = imread("patient_xray.png", IMREAD_GRAYSCALE);
    Mat watermark = imread("patient_no.png", IMREAD_GRAYSCALE);
    Mat output;
    double alpha = 0.3;
    addVisibleWatermark(original, watermark, output, alpha);
    imwrite("watermarked_xray.png", output);
    Mat invisibleOutput;
    addInvisibleWatermark(original, watermark, invisibleOutput);
    imwrite("invisible_watermarked_xray.png", invisibleOutput);
    Mat extractedWatermark;
    extractInvisibleWatermark(invisibleOutput, extractedWatermark);
    imwrite("extracted_watermark.png", extractedWatermark);
    return 0;
}