
//#include "csv.h"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>

#include <unordered_map>
#include<string>

#include <filesystem>
using namespace std;

//using namespace filesystem;
namespace fs = std::experimental::filesystem;



tuple<vector<string>, vector<string>, unordered_map<string,string>> read_name_tag_list(const string& subfolder_path)
	                                                            
{
	ifstream inputFile;
	//string train_img_path = labels_path + "\\" + train_subfolder_address + "/" + train_cow_name;

	//inputFile.open("D:\\Users\\a.kheiri\\Desktop\\20180813_022810.csv");
	inputFile.open(subfolder_path);

	string name;
	string tag;
	string tempString;
	string line = "";

	tuple<vector<string>, vector<string>, unordered_map<string,string> > nameListAndImgList;

	vector<string> cow_name_list;
	vector<string> tag_list;
	unordered_map<string, string> name_tag_match_dict;

	while (getline(inputFile, line))
	{
		stringstream inputString(line);
	
		getline(inputString, name, ',');
		getline(inputString, tempString, ',');
		tag = atoi(tempString.c_str());

		getline(inputString, tempString);

		
		if (name != "rgb")
		{
			cow_name_list.push_back(name);
			tag_list.push_back(tag);
			name_tag_match_dict.insert(pair<string, string>(name, tag));
		}
		line = "";
	}

	get<0>(nameListAndImgList) = cow_name_list;
	get<1>(nameListAndImgList) = tag_list;
	get<2>(nameListAndImgList) = name_tag_match_dict;
	   
	//nameListAndImgList.push_back(tuple(cow_name_list,tag_list));

	return nameListAndImgList;
}


//void Spliter()
// {

tuple <vector<vector<string>>, vector<vector<string>>> preprocess(const string& main_pic,
	                                                              const string& labels_path) 
  {

	string main_dataset_pic = main_pic;
	string main_labels_path = labels_path;

	vector<string> label_folders_names;
/*
	for (const auto& entry : fs::directory_iterator(main_labels_path)) {
		if (entry.is_directory()) {
			label_folders_names.push_back(entry.path().filename().string());
		}
	}*/

	vector<vector<string>> train_name_tag_list;
	vector<vector<string>> train_tag_list;
	vector<vector<string>> test_name_tag_list;
	tuple <vector<vector<string>>, vector<vector<string>>> chosen_test_train_dataset;
	int counter = 0;
	int total_img_from_name_img1 = 0;
	int total_img_from_name_img = 0;

	for (const auto& subfolder_name : fs::directory_iterator(main_dataset_pic))
	{

		counter += 1;
		cout << subfolder_name << endl;

		string subfolder_path = main_labels_path + "/" + subfolder_name.path().filename().string();
		
		vector<string> name_img_rgb;
		vector<string> tag_img_rgb;

		tuple<vector<string>, vector<string>, unordered_map<string, string>>  name_and_tag_list;
		name_and_tag_list = read_name_tag_list(subfolder_path); //main_pic, labels_path);

		vector<string> name_img_init;
		vector<string> tag_img_init;
		unordered_map<string, string>tag_name_dicts;
		//vector<vector<string>> total_name_img_init;

		name_img_init = get<0>(name_and_tag_list);// name_and_tag_list[0];
		tag_img_init  = get<1>(name_and_tag_list);
		tag_name_dicts= get<2>(name_and_tag_list);
		/*total_name_img_init.push_back(name_img_init);
		total_name_img_init.push_back(tag_img_init);*/

		//unordered_map<string, int> name_to_tag;


		vector<vector<string> > name_img;
		vector<vector<string> > tag_img;

		total_img_from_name_img1 += tag_name_dicts.size();

		for (const auto& [name, tag] : tag_name_dicts)

		{
			if (tag != "0" && tag != "1" && count(tag_name_dicts.begin(), tag_name_dicts.end(), make_pair(name, tag)) > 1)
			 {
				name_img.push_back({ name, subfolder_name.path().filename().string(), tag });
			 }
			if (tag != "0" && tag != "1" && count(tag_name_dicts.begin(), tag_name_dicts.end(), make_pair(name, tag)) > 1)
			 {
				tag_img.push_back({ tag, subfolder_name.path().filename().string() });
			 }
		}
/*
		for (const auto& [name, tag] : name_to_tag) 
		{
			if (tag != "0" && tag != "1" && count(name_to_tag.begin(), name_to_tag.end(), make_pair(name, tag)) > 1) 
			{
				name_img.push_back({ name, subfolder_name.path().filename().string(), tag });
			}
			if (tag != "0" && tag != "1" && count(name_to_tag.begin(), name_to_tag.end(), make_pair(name, tag)) > 1) 
			{
				tag_img.push_back({ tag, subfolder_name.path().filename().string() });
			}
		}*/

		total_img_from_name_img += name_img.size();

		if (tag_img.empty()) 
		{
			continue;
		}

		int lenv = 0;
		for (int j = 0; j < tag_img.size() - 1; j++) 
		{
			lenv++;
			
			if (tag_img[j] != tag_img[j + 1])
			{
				int select = rand() % lenv + (j - lenv + 1);
				train_name_tag_list.push_back({ (name_img[select][0]), (tag_img[select][0] )});
				train_tag_list.push_back({ tag_img[select][0] });

				for (int x = j - lenv + 1; x <= j; x++) {
					if (x != select) 
					{
						test_name_tag_list.push_back({ name_img[x][0], tag_img[x][0] });
					}
				}
				lenv = 0;
			}
		}

		lenv++;
		int select = rand() % (tag_img.size() - lenv) + (tag_img.size() - lenv);
		train_name_tag_list.push_back({ name_img[select][0], tag_img[select][0] });
		train_tag_list.push_back({ tag_img[select][0] });

		for (int x = tag_img.size() - lenv; x < tag_img.size(); x++) 
		{
			if (x != select) 
			{
				test_name_tag_list.push_back({ name_img[x][0], tag_img[x][0] });
			}
		}

	}

	cout << "total numbers of labeled images: " << total_img_from_name_img1 << endl;
	cout << "total numbers of suitable labeled images: " << total_img_from_name_img << endl;
	cout << "number of cow in test dataset: " << test_name_tag_list.size() << endl;
	cout << "number of cows train dataset: " << train_name_tag_list.size() << endl;
	
	get<0>(chosen_test_train_dataset) = train_name_tag_list;
	get<1>(chosen_test_train_dataset) = test_name_tag_list;

	return chosen_test_train_dataset;// { train_name_tag_list, test_name_tag_list };

 }

//};