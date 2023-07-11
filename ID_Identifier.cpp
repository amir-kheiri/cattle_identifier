
#include <iostream>

//#include <tuple>  

//
//#include <opencv2/opencv.hpp>
//#include <opencv2/xfeatures2d.hpp>
//#include <opencv2/core.hpp>
//#include <opencv2/core/types.hpp>
//#include <opencv2/imgcodecs.hpp>
//#include <opencv2/highgui.hpp>
//#include <opencv2/core/mat.hpp>
//#include <opencv2/features2d.hpp>
//#include <opencv2/calib3d.hpp>
//#include <opencv2/flann.hpp>


#include <opencv2/opencv.hpp>
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/features2d.hpp"

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

tuple <Mat, Mat> orb_descriptor_keypoints(const Mat& img1)
{
	Ptr<ORB> orb = ORB::create(500);
	vector<KeyPoint> kp1;
	Mat des1;
	orb->detectAndCompute(img1, Mat(), kp1, des1);
	return des1, kp1;
}

int matcher_orb(Mat& im1, Mat& im2, const Mat& des_test, const Mat& des_train)
{
	//BFMatcher matcher(NORM_HAMMING, true);

	vector<DMatch> matches;
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
	//Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::BRUTEFORCE);

	matcher->match(des_test, des_train, matches, Mat());
	//brute_force.match(des_test, des_train, matches);
	return matches.size();

	// Sort matches by score
	std::sort(matches.begin(), matches.end());

	// Remove not so good matches
	const float GOOD_MATCH_PERCENT = 0.15f;

	const int numGoodMatches = matches.size() * GOOD_MATCH_PERCENT;
	matches.erase(matches.begin() + numGoodMatches, matches.end());

	// Draw top matches
	Mat imMatches;
	drawMatches(im1, kp1, im2, kp2, matches, imMatches);
	imwrite("matches.jpg", imMatches);


}

//Mat sift_descriptor_keypoints(const Mat& img1)
//{
//	Ptr<SIFT> sift = SIFT::create();
//	vector<KeyPoint> kp1;
//	Mat des1;
//	sift->detectAndCompute(img1, noArray(), kp1, des1);
//	return des1;
//}
//
//int matcher_sift(const Mat& des_test, const Mat& des_train)
//{
//	BFMatcher bf(NORM_L2);
//	vector<vector<DMatch>> matches;
//	bf.knnMatch(des_test, des_train, matches, 2);
//
//	vector<DMatch> good_match;
//	for (size_t i = 0; i < matches.size(); i++)
//	{
//		if (matches[i][0].distance < 0.85 * matches[i][1].distance)
//		{
//			good_match.push_back(matches[i][0]);
//		}
//	}
//	return good_match.size();
//}



map <string, Mat> descriptors_of_train(string main_dataset_pic, vector<vector<string>>train_name_adress_tag_list)
 {

	int true_pred_process = 0;

	//vector<tuple<string, string, int >> train_name_tag_list;
	//vector< tuple <tuple<string, string,int>, tuple<int,string> > > train_name_adress_tag_list;
	vector<tuple<string, string, int >> train_name_list;


	vector<KeyPoint> keypoints1, keypoints2;
	vector <tuple <string, Mat >> descriptor_train_list;
	map <string, Mat> descriptor_train_dict;
/*
	for (const auto& item : train_name_adress_tag_list) 
	{
		train_name_list.push_back(get<0>(item));
	}
	*/
	for (const auto& train_data : train_name_adress_tag_list)// train_name_list)
	{
		/*
		const auto& train_cow_name = get<0>(train_data);
		const auto& train_subfolder_address = get<1>(train_data);
		const auto& train_tag = get<2>(train_data);*/

		const auto& train_cow_name = train_data[0];
		const auto& train_subfolder_address = train_data[1];
		const auto& train_tag = train_data[2];

		string train_img_path = main_dataset_pic + "\\" + train_subfolder_address + "\\" + train_cow_name;
		auto image_train = read_image(train_img_path);

		const auto train_img_des = orb_descriptor_keypoints(image_train);
		// const auto train_img_des = sift_descriptor_keypoints(image_train);
		
		descriptor_train_list.push_back({ train_cow_name, train_img_des });
		descriptor_train_dict[train_cow_name] = train_img_des;	
	}
	return descriptor_train_dict;
}



void matching_test_train(string main_dataset_pic, vector<vector<string>> train_name_list,
	                     vector<vector<string>> test_name_list, map <string,Mat> descriptor_train_dict)
{
//	map <string, Mat> descriptor_train_dict;
	
	//clock_t init_time = clock();

	int counter =0;
	int true_pred_process = 0;

	map <string, Mat> numbers_pattern_total;
	map <string , tuple<int,int> > pred;
	string pred_cow_in_train;

	vector <tuple<string, string, int, int, string, string> > pred_list;

	/*pred_list.append([test_cow_name, pred_cow_in_train, pred_tag, real_tag,
		test_subfolder_address, train_subfolder_address])*/

	for (const auto& test_data : test_name_list) 
	{
		const auto& test_cow_name = test_data[0];
		const auto& test_subfolder_address = test_data[1];
		const auto& test_tag = test_data[2];

		counter += 1;
		const auto test_img_path = main_dataset_pic + "\\" + test_subfolder_address + "\\" + test_cow_name;
		const auto image_test = read_image(test_img_path);

		const auto test_img_des = orb_descriptor_keypoints(image_test);
		// const auto test_img_des = sift_descriptor_keypoints(image_test);

		int max_keypoint = 0;
		int pred_tag;
		string train_subfolder_address;

		for (const auto& train_data : train_name_list)
		{			
			const auto& train_cow_name = train_data[0];
			const auto& train_subfolder_address = train_data[1];
			const auto& train_tag = train_data[2];

			auto train_img_path= main_dataset_pic+ "\\" + train_subfolder_address +"\\" + train_cow_name;
			auto image_train = read_image(train_img_path);

			int no_of_pattern = matcher_orb(image_test, image_train, test_img_des, descriptor_train_dict[train_cow_name]);
			//const int no_of_pattern = matcher_sift(test_img_des, descriptor_train_dict[train_cow_name]);
			// const int no_of_pattern = matcher_flann(test_img_des, descriptor_train_dict[train_cow_name]);

			if (no_of_pattern > max_keypoint)
			{
				max_keypoint = no_of_pattern;
				pred_tag = stoi(train_tag);
				pred_cow_in_train = train_cow_name;
			}
		}

		numbers_pattern_total[test_cow_name] = max_keypoint;
		int real_tag = stoi(test_tag);

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
		//	clock_t end = clock();

		//	cout << "elapsed time: " << clock() - init_time << endl;
			//cout << "numbers of predicted images: " << counter << endl;
			cout << "training accuracy = " << acc << endl;

			//starter = timeit.default_timer();
		}

		true_pred_process = 0;
	}

	//return true;
}


int main() 
{
	//Tehran_University Images and Labels
	string main_dataset_pic = "G:\\Amir-Kheiri\\Cattle Identifcation\\Datasets\\SarveenFarm\\Tehran_University1402\\selected_frames\\rgb_color";
	string main_dataset_labels = "G:\\Amir-Kheiri\\Cattle Identifcation\\Datasets\\SarveenFarm\\Tehran_University1402\\selected_frames\\labels";

		//97 Images and Labels
	//'string main_dataset_pic= "G:\\Amir-Kheiri\\Cattle Identifcation\\Datasets\\SarveenFarm\\97\\Final\\final_dataset_onepart_dataset\\pictures";
	//string main_dataset_labels= "G:\\Amir-Kheiri\\Cattle Identifcation\\Datasets\\SarveenFarm\\97\\Final\\final_dataset\\Labels";
	tuple <vector<vector<string>>, vector<vector<string>>> datasets;
	vector<vector<string>> train_name_list;
	vector<vector<string>> test_name_list;

	datasets = preprocess(main_dataset_pic, main_dataset_labels);
	train_name_list = get<0>(datasets);
	test_name_list  = get<1>(datasets);

	map <string, Mat> train_descriptors;

	train_descriptors= descriptors_of_train(main_dataset_pic, train_name_list);
	matching_test_train(main_dataset_pic, train_name_list, test_name_list, train_descriptors);

	map <string, Mat> test2;

	//bool result= matching_test_train(main_dataset_pic, train_name_list, test_name_list, train_descriptors);
	int re = 3;
	

}