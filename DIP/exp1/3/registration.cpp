#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char **argv)
{

    std::string img1_path = "../SEU_gray.png";
    std::string img2_path = "../2/custom_shear.png";

    std::string matches_path;
    if (argc >= 3)
    {
        img1_path = argv[1];
        img2_path = argv[2];
        if (argc >= 4)
            matches_path = argv[3];
    }

    // === Step 1. 固定输入 ===
    cv::Mat img1 = cv::imread(img1_path, cv::IMREAD_GRAYSCALE);
    cv::Mat img2 = cv::imread(img2_path, cv::IMREAD_GRAYSCALE);

    if (img1.empty() || img2.empty())
    {
        std::cerr << "❌ 无法读取输入图像，请确保 image1.png 和 image2.png 存在。" << std::endl;
        return -1;
    }

    // === Step 2. 提取 ORB 特征点（如果没有提供匹配文件） ===
    auto orb = cv::ORB::create(1000);
    std::vector<cv::KeyPoint> k1, k2;
    cv::Mat d1, d2;
    orb->detectAndCompute(img1, cv::noArray(), k1, d1);
    orb->detectAndCompute(img2, cv::noArray(), k2, d2);

    std::vector<cv::DMatch> good;
    bool use_matches_file = !matches_path.empty();

    if (!use_matches_file)
    {
        if (d1.empty() || d2.empty())
        {
            std::cerr << "❌ 特征提取失败！" << std::endl;
            return -1;
        }

        // === Step 3. 特征匹配（KNN + 比率测试）===
        cv::BFMatcher matcher(cv::NORM_HAMMING);
        std::vector<std::vector<cv::DMatch>> knn_matches;
        matcher.knnMatch(d1, d2, knn_matches, 2);

        for (auto &m : knn_matches)
        {
            if (m.size() < 2)
                continue;
            if (m[0].distance < 0.75 * m[1].distance)
                good.push_back(m[0]);
        }
    }
    else
    {
        // 从文本文件读取匹配点，每行格式：x1 y1 x2 y2（允许空格或逗号分隔）
        std::ifstream ifs(matches_path);
        if (!ifs)
        {
            std::cerr << "❌ 无法打开匹配点文件：" << matches_path << std::endl;
            return -1;
        }

        std::string line;
        std::vector<cv::KeyPoint> k1_file, k2_file;
        int idx = 0;
        while (std::getline(ifs, line))
        {
            if (line.empty())
                continue;
            // 替换逗号为空格，方便解析
            for (auto &c : line)
                if (c == ',')
                    c = ' ';
            std::istringstream ss(line);
            double x1, y1, x2, y2;
            if (!(ss >> x1 >> y1 >> x2 >> y2))
                continue; // 忽略解析失败的行

            k1_file.emplace_back(cv::KeyPoint(cv::Point2f(static_cast<float>(x1), static_cast<float>(y1)), 1.f));
            k2_file.emplace_back(cv::KeyPoint(cv::Point2f(static_cast<float>(x2), static_cast<float>(y2)), 1.f));
            good.emplace_back(cv::DMatch(idx, idx, 0));
            idx++;
        }

        if (good.empty())
        {
            std::cerr << "❌ 匹配点文件中没有有效匹配。" << std::endl;
            return -1;
        }

        // 用文件中的 keypoints 替换原始 k1/k2，用于绘图和后续构造 p1/p2
        k1 = k1_file;
        k2 = k2_file;
        std::cout << "从文件读取到 " << good.size() << " 对匹配点。" << std::endl;
    }

    std::cout << "检测到 " << good.size() << " 个匹配点。" << std::endl;
    if (good.size() < 4)
    {
        std::cerr << "❌ 匹配点太少，无法估计变换。" << std::endl;
        return -1;
    }

    // === Step 4. 绘制匹配连线图 ===
    cv::Mat match_img;
    cv::drawMatches(img1, k1, img2, k2, good, match_img);
    cv::imwrite("matches.png", match_img);

    // === Step 5. 估计单应矩阵并变换 ===
    std::vector<cv::Point2f> p1, p2;
    for (auto &m : good)
    {
        p1.push_back(k1[m.queryIdx].pt);
        p2.push_back(k2[m.trainIdx].pt);
    }

    cv::Mat H = cv::findHomography(p2, p1, cv::RANSAC);
    if (H.empty())
    {
        std::cerr << "❌ 单应矩阵估计失败！" << std::endl;
        return -1;
    }

    std::cout << "估计的单应矩阵：\n"
              << H << std::endl;

    cv::Mat img2_aligned;
    cv::warpPerspective(img2, img2_aligned, H, img1.size());

    // === Step 6. 叠加对比图 ===
    cv::Mat overlay;
    cv::addWeighted(img1, 0.5, img2_aligned, 0.5, 0, overlay);
    // 将原图和配准后的图并排放入一张图中，输出为 aligned_image.png
    int rows = std::max(img1.rows, img2_aligned.rows);
    int cols = img1.cols + img2_aligned.cols;
    cv::Mat aligned_display(rows, cols, img1.type(), cv::Scalar::all(0));

    // 拷贝原图到左边
    cv::Mat left_roi = aligned_display(cv::Rect(0, 0, img1.cols, img1.rows));
    img1.copyTo(left_roi);

    // 拷贝配准后图到右边
    cv::Mat right_roi = aligned_display(cv::Rect(img1.cols, 0, img2_aligned.cols, img2_aligned.rows));
    img2_aligned.copyTo(right_roi);

    cv::imwrite("aligned_image.png", aligned_display);
    cv::imwrite("overlay.png", overlay);

    std::cout << "✅ 输出文件：\n"
              << " - matches.png : 匹配连线图\n"
              << " - aligned_image.png : 图2配准到图1\n"
              << " - overlay.png : 图1与配准后图2叠加对比\n";

    return 0;
}
