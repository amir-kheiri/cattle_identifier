
#include <iostream>
//#include <tuple> 

#include<chrono>
#include <ctime>
#include <Windows.h>
#include <algorithm>

#include <opencv2/opencv.hpp>
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/features2d.hpp"

#include "dataset_random_spliter.h"

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;


Mat read_image( string& path)//(Mat& img, const string& path)
{
	//Mat& img=0;
	Mat img = imread(path, IMREAD_COLOR);
	return img;
	//img = imread(path, IMREAD_COLOR);
}

tuple <Mat, vector<KeyPoint>> orb_descriptor_keypoints( Mat& img1)
{
	//Ptr<ORB> orb = ORB::create(400);
	Ptr<Feature2D> orb = ORB::create(400);
	vector<KeyPoint> kp1;
	Mat des1;
	orb->detectAndCompute(img1, Mat(), kp1, des1);
	return tuple(des1 , kp1);
}


vector<DMatch> matcher_orb( Mat& des_test,  Mat& des_train )
{
	BFMatcher matcher(NORM_HAMMING, true);

	vector<DMatch> matches;
	//Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::BRUTEFORCE);
	//Ptr<cv::DescriptorMatcher> matcher = BFMatcher::create(cv::NORM_HAMMING); 

	matcher.match( des_train, des_test, matches );

	// Sort matches by score
	//sort(matches.begin(), matches.end());

	// Remove not  good matches
	//const float GOOD_MATCH_PERCENT = 0.4f;
	//int numGoodMatches = matches.size() * GOOD_MATCH_PERCENT;
	//matches.erase(matches.begin() + numGoodMatches, matches.end());
	//returnr matches.size()
/*
	BruteForceMatcher<L2<float> > matcher;
	vector<DMatch> matches;
	matcher.match(descriptors1, descriptors2, matches);*/
	return matches;
}

void draw_matches(Mat& im1, Mat& im2, vector<KeyPoint> kp1, vector<KeyPoint> kp2, vector<DMatch> matches)
{
	// Draw  matches
	Mat imMatches;
	drawMatches(im1, kp1, im2, kp2, matches, imMatches);
	imwrite("matches.jpg", imMatches);
}


tuple <Mat, vector<KeyPoint>> sift_descriptor_keypoints(const Mat& img1)
{
	Ptr<SIFT> sift = SIFT::create();
	vector<KeyPoint> kp1;
	Mat des1;
	sift->detectAndCompute(img1, Mat(), kp1, des1);

	return tuple(des1, kp1);;
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



tuple <map<string,Mat>, map<string,vector<KeyPoint>>> descriptors_of_train(string main_dataset_pic,
	                                                                       vector<vector<string>>train_name_adress_tag_list)
 {
	int true_pred_process = 0;

	//vector<tuple<string, string, int >> train_name_tag_list;
	//vector< tuple <tuple<string, string,int>, tuple<int,string> > > train_name_adress_tag_list;
	vector<tuple<string, string, int >> train_name_list;


	vector<KeyPoint> keypoints1, keypoints2;
	vector <tuple <string, Mat >> descriptor_train_list;
	map <string, Mat> descriptor_train_dict;
	map < string, vector<KeyPoint> > keypoints_train_dict;
	tuple < map <string, Mat>, map < string, vector<KeyPoint> >> return_descriptor_and_keypoints;
	tuple <Mat, vector<KeyPoint>>train_img_des_kp;

/*
	for (const auto& item : train_name_adress_tag_list) 
	{
		train_name_list.push_back(get<0>(item));
	}
	*/
	for ( auto& train_data : train_name_adress_tag_list)// train_name_list)
	{
		/*
		const auto& train_cow_name = get<0>(train_data);
		const auto& train_subfolder_address = get<1>(train_data);
		const auto& train_tag = get<2>(train_data);*/

		auto& train_cow_name = train_data[0];
		auto& train_subfolder_address = train_data[1];
		auto& train_tag = train_data[2];

		string train_img_path = main_dataset_pic + "\\" + train_subfolder_address + "\\" + train_cow_name;
		auto image_train = read_image(train_img_path);


		auto train_img_des_kp = orb_descriptor_keypoints(image_train);
		auto train_img_des = get<0>(train_img_des_kp);
		auto train_img_kp  = get<1>(train_img_des_kp);
/*
		auto train_img_des_kp = sift_descriptor_keypoints(image_train);
		auto train_img_des = get<0>(train_img_des_kp);
		auto train_img_kp  = get<1>(train_img_des_kp);*/
		
		descriptor_train_list.push_back({ train_cow_name, train_img_des });
		descriptor_train_dict[train_cow_name] = train_img_des;
		keypoints_train_dict[train_cow_name] = train_img_kp ;
		
	}
	get<0>(return_descriptor_and_keypoints) = descriptor_train_dict;
	get<1>(return_descriptor_and_keypoints) = keypoints_train_dict;

	return return_descriptor_and_keypoints;
}


auto matching_test_train(string& main_dataset_pic,
	                     vector<vector<string>>& train_name_list,
	                     vector<vector<string>>& test_name_list,
	                     map<string,Mat>& descriptor_train_dict)
{
//	map <string, Mat> descriptor_train_dict;
	
	int counter = 0;
	int true_pred_process = 0;

	map <string, Mat> numbers_pattern_total;
	map <string, tuple<int,int> > pred;
	string pred_cow_in_train;
	tuple <Mat, vector<KeyPoint>>test_img_des_kp;
	
	vector <tuple<string, string, int, int, string, string> > pred_list;

	/*pred_list.append([test_cow_name, pred_cow_in_train, pred_tag, real_tag,
		test_subfolder_address, train_subfolder_address])*/

	auto t_start = chrono::high_resolution_clock::now();
	int no_of_matches =0;
	int tt = 0;

	for ( auto& test_data : test_name_list) 
	{
		//clock_t init_time = clock();
		
		auto test_cow_name = test_data[0];
		auto test_subfolder_address = test_data[1];
		auto test_tag = test_data[2];

		counter += 1;
		auto test_img_path = main_dataset_pic + "\\" + test_subfolder_address + "\\" + test_cow_name;
		auto image_test = read_image(test_img_path);

		test_img_des_kp = orb_descriptor_keypoints(image_test);
		auto test_img_des = get<0>(test_img_des_kp);
		auto test_img_kp  = get<1>(test_img_des_kp);

		//test_img_des_kp = sift_descriptor_keypoints(image_test);
		//auto test_img_des = get<0>(test_img_des_kp);
		//auto test_img_kp = get<1>(test_img_des_kp);
	

		int max_keypoint = 0;
		int pred_tag;
		string train_subfolder_address;

		for ( auto train_data : train_name_list)
		{			
			auto train_cow_name = train_data[0];
			auto train_subfolder_address = train_data[1];
			auto train_tag = train_data[2];

		/*	auto train_img_path= main_dataset_pic+ "\\" + train_subfolder_address +"\\" + train_cow_name;
			auto image_train = read_image(train_img_path);*/

			auto matches_result = matcher_orb( test_img_des, descriptor_train_dict[train_cow_name] );
			no_of_matches = matches_result.size(); // number of matched keypointes

			//const int no_of_pattern = matcher_sift(test_img_des, descriptor_train_dict[train_cow_name]);
			// const int no_of_pattern = matcher_flann(test_img_des, descriptor_train_dict[train_cow_name]);
			
			if (no_of_matches > max_keypoint)
			{
				max_keypoint = no_of_matches;
				pred_tag = stoi(train_tag);
				pred_cow_in_train = train_cow_name;
			}
		}

		numbers_pattern_total[test_cow_name] = max_keypoint;
		int real_tag = stoi(test_tag);

		pred[test_cow_name] = { pred_tag, real_tag };

		pred_list.push_back({ test_cow_name, pred_cow_in_train, pred_tag, real_tag,
							 test_subfolder_address, train_subfolder_address });

		int target;
		int refrence;
		for (auto& items : pred)
		{  						
			target  = get<0>(items.second);
			refrence= get<1>(items.second);
						
			if (target == refrence) 
			{
				true_pred_process += 1;
			}
		}
		
		 double acc = true_pred_process / static_cast<double>(pred.size());
		// std::cout << "training accuracy = " << acc << std::endl;

		if (counter % 10 == 0)
		{
			//clock_t end = clock();
			auto t_end = chrono::high_resolution_clock::now();
			//double elapsed_time_ms = chrono::duration< double, std::milli>(t_end - t_start).count();
			auto duration = chrono::duration_cast<chrono::milliseconds>(t_end - t_start);
		
			//cout << "elapsed time: " << (end-init_time)/ (double)CLOCKS_PER_SEC << endl;
			cout << "elapsed time =  " << duration.count() << endl;
			
			t_start = chrono::high_resolution_clock::now();

			cout << "numbers of predicted images: " << counter << endl;
			cout << "training accuracy = " << acc << endl;
		}
		
		true_pred_process = 0;
	}

}


int main() 
{
	   //Tehran_University Images and Labels
	//string main_dataset_pic = "G:\\Amir-Kheiri\\Cattle Identifcation\\Datasets\\SarveenFarm\\Tehran_University1402\\selected_frames\\rgb_color";
	//string main_dataset_labels = "G:\\Amir-Kheiri\\Cattle Identifcation\\Datasets\\SarveenFarm\\Tehran_University1402\\selected_frames\\labels";

		//97 Images and Labels
	string main_dataset_pic= "G:\\Amir-Kheiri\\Cattle Identifcation\\Datasets\\SarveenFarm\\97\\Final\\final_dataset_onepart_dataset\\pictures";
	string main_dataset_labels= "G:\\Amir-Kheiri\\Cattle Identifcation\\Datasets\\SarveenFarm\\97\\Final\\final_dataset\\Labels";

	tuple <vector<vector<string>>, vector<vector<string>>> datasets;
	vector<vector<string>> train_name_list;
	vector<vector<string>> test_name_list;

	datasets = preprocess(main_dataset_pic, main_dataset_labels);
	train_name_list = get<0>(datasets);
	test_name_list  = get<1>(datasets);

	map <string, Mat> train_descriptors;
	map <string, vector<KeyPoint>> train_keypoints;

	tuple < map<string,Mat>, map<string,vector<KeyPoint> > > train_descriptors_and_keypoints;

	train_descriptors_and_keypoints = descriptors_of_train(main_dataset_pic, train_name_list);
	
	train_descriptors = get<0>(train_descriptors_and_keypoints);
	train_keypoints = get<1>(train_descriptors_and_keypoints);

	matching_test_train(main_dataset_pic, train_name_list, test_name_list, train_descriptors);

	//draw_mathcing(test_image, train_image, kp_test, kp_train,)
	map <string, Mat> test2;

}