
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

#include<ctime>

#include <tuple>    // std::tuple, std::get, std::tie, std::ignore

#include <unordered_map>

#include "dataset_random_spliter.h"

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;


Mat read_image(const string& path)//(Mat& img, const string& path)
{
	//Mat& img=0;
	Mat img = imread(path, IMREAD_COLOR);
	return img;
	//img = imread(path, IMREAD_COLOR);
}

Mat orb_descriptor_keypoints(const Mat& img1)
{
	Ptr<ORB> orb = ORB::create(1000);
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



map <string, Mat> descriptors_of_train(string main_dataset_pic)
 {
	int true_pred_process = 0;

	vector<tuple<string, string, int >> train_name_tag_list;
	vector< tuple <tuple<string, string,int>, tuple<int,string> > > train_name_adress_tag_list;
	vector<tuple<string, string, int >> train_name_list;


	vector <tuple <string, DescriptorMatcher> > kp1;
	vector <tuple <string, Mat >> descriptor_train_list;
	map <string, Mat> descriptor_train_dict;

	for (const auto& item : train_name_adress_tag_list) 
	{
		train_name_list.push_back(get<0>(item));
	}
	
	for (const auto& train_data : train_name_list)
	{
		const auto& train_cow_name = get<0>(train_data);
		const auto& train_subfolder_address = get<1>(train_data);
		const auto& train_tag = get<2>(train_data);

		 string train_img_path = main_dataset_pic + "/" + train_subfolder_address + "/" + train_cow_name;
		 auto image_train = read_image(train_img_path);

		const auto train_img_des = orb_descriptor_keypoints(image_train);
		// const auto train_img_des = sift_descriptor_keypoints(image_train);

		descriptor_train_list.push_back({ train_cow_name, train_img_des });
		descriptor_train_dict[train_cow_name] = train_img_des;
	}
}


void matching_test_train(string main_dataset_pic,
	                     vector< tuple<string, string, int> > train_name_list,
	                     map <string, Mat> descriptor_train_dict)
{
	clock_t init_time = clock();

	int counter =0;
	int true_pred_process = 0;
	vector< tuple<string, string, int> > test_name_list;

	map <string, Mat> numbers_pattern_total;
	map < string , tuple<int,int> > pred;
	string pred_cow_in_train;

	vector <tuple<string, string, int, int, string, string> > pred_list;
	/*pred_list.append([test_cow_name, pred_cow_in_train, pred_tag, real_tag,
		test_subfolder_address, train_subfolder_address])*/

	for (const auto& test_data : test_name_list) 
	{
		const auto& test_cow_name = get<0>(test_data);
		const auto& test_subfolder_address = std::get<1>(test_data);
		const auto& test_tag = get<2>(test_data);

		counter += 1;
		const auto test_img_path = main_dataset_pic + "/" + test_subfolder_address + "/" + test_cow_name;
		const auto image_test = read_image(test_img_path);

		const auto test_img_des = orb_descriptor_keypoints(image_test);
		// const auto test_img_des = sift_descriptor_keypoints(image_test);

		int max_keypoint = 0;
		int pred_tag;
		string train_subfolder_address;

		for (const auto& train_data : train_name_list)
		{
			 const auto& train_cow_name = get<0>(train_data);
			 const auto& train_subfolder_address = get<1>(train_data);
			 const auto& train_tag = get<2>(train_data);

			const auto train_img_path = main_dataset_pic + "/" + train_subfolder_address + "/" + train_cow_name;

			const int no_of_pattern = matcher_orb(test_img_des, descriptor_train_dict[train_cow_name]);
			// const int no_of_pattern = matcher_sift(test_img_des, descriptor_train_dict[train_cow_name]);
			// const int no_of_pattern = matcher_flann(test_img_des, descriptor_train_dict[train_cow_name]);

			if (no_of_pattern > max_keypoint)
			{
				max_keypoint = no_of_pattern;
				pred_tag = train_tag;
				pred_cow_in_train = train_cow_name;
			}
		}

		numbers_pattern_total[test_cow_name] = max_keypoint;
		int real_tag = test_tag;

		pred[test_cow_name] = { pred_tag, real_tag };

		pred_list.push_back({ test_cow_name, pred_cow_in_train, pred_tag, real_tag,
							 test_subfolder_address, train_subfolder_address });

		int target ;
		int refrence ;
		for (auto& items : pred)
		{  
			//tuple < int, int > target_refrence;
			//get<0>(test_data)
			target = get<0>( items.second);
			refrence = get<1>(items.second);
						
			if (target == refrence) 
			{
				true_pred_process += 1;
			}
		}
		
		const double acc = true_pred_process / static_cast<double>(pred.size());
		// std::cout << "training accuracy = " << acc << std::endl;

		if (counter % 10 == 0)
		{
			clock_t end = clock();

			cout << "elapsed time: " << clock() - init_time << endl;
			//cout << "numbers of predicted images: " << counter << endl;
			cout << "training accuracy = " << acc << endl;

			//starter = timeit.default_timer();
		}

		true_pred_process = 0;
	}


	// Save Result
	//const int rem = 50;
	//if (pred_list.size() % rem == 0) 
	//{
	//	for (auto it = pred_list.end() - rem; it != pred_list.end(); ++it) {
	//		const auto& x = *it;

	//		pred_list_temp.push_back(x);

	//		if (x[2] != x[3] && std::find(wrong_pred.begin(), wrong_pred.end(), x) == wrong_pred.end()) {
	//			wrong_pred.push_back(x);
	//			wrong_pred_temp.push_back(x);
	//		}
	//	}

	//	// std::ofstream file("D:/Users/a.kheiri/Desktop/save_result/total_reslut.csv", std::ios::app);
	//	// std::ofstream file2("D:/Users/a.kheiri/Desktop/save_result/wrong_predicted.csv", std::ios::app);
	//	// for (const auto& x : pred_list_temp) {
	//	//     file << x[0] << ',' << x[1] << ',' << x[2] << ',' << x[3] << ',' << x[4] << ',' << x[5] << std::endl;
	//	// }
	//	// for (const auto& x : wrong_pred_temp) {
	//	//     file2 << x[0] << ',' << x[1] << ',' << x[2] << ',' << x[3] << ',' << x[4] << ',' << x[5] << std::endl;
	//	// }

	//	wrong_pred_temp.clear();
	//	pred_list_temp.clear();
	//}

}


int main() 
{

	string main_dataset_pic= "G:\\Amir-Kheiri\\Cattle Identifcation\\Datasets\\SarveenFarm\\97\\Final\\final_dataset_onepart_dataset\\pictures";
	string main_dataset_labels = "G:\\Amir-Kheiri\\Cattle Identifcation\\Datasets\\SarveenFarm\\97\\Final\\final_dataset\\Labels";

	tuple <vector<vector<string>>, vector<vector<string>> > datasets;
	vector<vector<string>>train_name_list;

	datasets = preprocess(main_dataset_pic, main_dataset_labels);
	train_name_list = get<0>(datasets);

	map <string, Mat> train_descriptors;
	train_descriptors= descriptors_of_train(main_dataset_pic);


	vector< tuple<string, string, int> > train_name_list,
	map <string, Mat> descriptor_train_dict;

	//vector< tuple<string, string, int> >train_name_list;
	matching_test_train(main_dataset_pic, train_name_list, train_descriptors);
	
	//mathcing_test_train

}