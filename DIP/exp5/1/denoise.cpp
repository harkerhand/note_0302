#include <iostream>
#include <opencv2/opencv.hpp>
#include <wavelib.h>
using namespace cv;
using namespace std;

// 自适应中值滤波
void adaptiveMedianFilter(const Mat &input, Mat &output, int smax)
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

double *mat_to_double_array(const Mat &input_mat)
{
    if (input_mat.empty() || input_mat.channels() != 1 || input_mat.depth() != CV_64F)
    {
        cerr << "Error: Input Mat must be non-empty, single channel, and CV_64F." << endl;
        return nullptr;
    }
    int rows = input_mat.rows;
    int cols = input_mat.cols;
    double *arr = (double *)malloc(rows * cols * sizeof(double));
    if (!arr)
        return nullptr;

    for (int i = 0; i < rows; ++i)
    {
        const double *mat_ptr = input_mat.ptr<double>(i);
        memcpy(arr + i * cols, mat_ptr, cols * sizeof(double));
    }
    return arr;
}

Mat double_array_to_mat(const double *arr, int rows, int cols)
{
    if (!arr)
        return Mat();
    Mat output_mat(rows, cols, CV_64F);

    for (int i = 0; i < rows; ++i)
    {
        double *mat_ptr = output_mat.ptr<double>(i);
        memcpy(mat_ptr, arr + i * cols, cols * sizeof(double));
    }
    return output_mat;
}

// 阈值处理
void thresholding(double *wcoeffs, int start_index, int count, double threshold)
{
    double T = threshold;
    for (int i = start_index; i < start_index + count; ++i)
    {
        if (abs(wcoeffs[i]) < T)
        {
            wcoeffs[i] = 0.0;
        }
        else if (wcoeffs[i] > T)
        {
            wcoeffs[i] -= T;
        }
        else if (wcoeffs[i] < -T)
        {
            wcoeffs[i] += T;
        }
    }
}

// 小波变换
Mat wavelet_denoising(const Mat &noisy_img, int j, const string &wavename, double threshold)
{
    int rows = noisy_img.rows;
    int cols = noisy_img.cols;
    wave_object wave = wave_init(wavename.c_str());
    wt2_object wt = wt2_init(wave, "dwt", rows, cols, j);
    double *input_array = mat_to_double_array(noisy_img);
    double *wcoeffs = dwt2(wt, input_array);
    // 低频子带系数数量
    int cA_len = wt->coeffaccess[0];
    int total_len = wt->outlength;
    // 高频阈值处理
    thresholding(wcoeffs, cA_len, total_len - cA_len, threshold);
    // 重构
    double *output_array = (double *)malloc(rows * cols * sizeof(double));
    idwt2(wt, wcoeffs, output_array);
    free(input_array);
    free(wcoeffs);
    wt2_free(wt);
    wave_free(wave);
    Mat denoised_img = double_array_to_mat(output_array, rows, cols);
    free(output_array);
    return denoised_img;
}

int main()
{
    Mat noisy_img = imread("noise.tif", IMREAD_GRAYSCALE);
    imwrite("noisy_image.bmp", noisy_img);
    Mat double_img;
    noisy_img.convertTo(double_img, CV_64F, 1.0 / 255.0);
    const int j = 4;
    const string wavename = "db4";
    const double threshold = 0.08;
    Mat denoised_img = wavelet_denoising(double_img, j, wavename, threshold);
    denoised_img.convertTo(denoised_img, CV_8U, 255.0);
    imwrite("denoised_wavelet.bmp", denoised_img);
    Mat adaptive_denoised;
    adaptiveMedianFilter(noisy_img, adaptive_denoised, 7);
    imwrite("denoised_adaptive_median.bmp", adaptive_denoised);
}