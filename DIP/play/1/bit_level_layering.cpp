#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input_image> <output_prefix>" << std::endl;
        return 1;
    }

    cv::Mat image = cv::imread(argv[1], cv::IMREAD_GRAYSCALE);
    if (image.empty())
    {
        std::cerr << "Error: Could not load image." << std::endl;
        return 1;
    }

    std::string output_prefix = argv[2];

    // Bit-plane slicing for 8-bit grayscale image
    for (int bit = 0; bit < 8; ++bit)
    {
        cv::Mat bit_plane = (image & (1 << bit)) * 255; // Extract and scale to 0-255
        std::string filename = output_prefix + "_bit" + std::to_string(bit) + ".png";
        cv::imwrite(filename, bit_plane);
        std::cout << "Saved: " << filename << std::endl;
    }

    return 0;
}