
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/flann.hpp>
#include <numeric>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>

#include <unordered_map>


using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;


void read_image(Mat& image, const string& img_path)
{
	imgage = imread(img_path, IMREAD_COLOR);
}

Mat orb_descriptor_keypoints(const Mat& img1)
{
	Ptr<ORB> orb = ORB::create(500);
	vector<KeyPoint> kp1;
	Mat des1;
	orb->detectAndCompute(img1, noArray(), kp1, des1);
	return des1;
}

int matcher_orb(const Mat& des_test, const Mat& des_train)
{
	BFMatcher brute_force(NORM_HAMMING, true);

	vector<DMatch> matches;
	brute_force.match(des_test, des_train, matches);
	return matches.size();
}

Mat sift_descriptor_keypoints(const Mat& img1)
{
	Ptr<SIFT> sift = SIFT::create();
	vector<KeyPoint> kp1;
	Mat des1;
	sift->detectAndCompute(img1, noArray(), kp1, des1);
	return des1;
}

int matcher_sift(const Mat& des_test, const Mat& des_train)
{

	BFMatcher bf(NORM_L2);
	vector<vector<DMatch>> matches;
	bf.knnMatch(des_test, des_train, matches, 2);

	vector<DMatch> good_match;
	for (size_t i = 0; i < matches.size(); i++)
	{
		if (matches[i][0].distance < 0.85 * matches[i][1].distance)
		{
			good_match.push_back(matches[i][0]);
		}
	}
	return good_match.size();
}