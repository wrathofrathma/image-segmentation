#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include "SLIC.hpp"

using namespace std;




int main(){
	//Meaningful variables.
	std::string input_directory = "in/";
	std::string output_directory = "out/";
	unsigned int K = 200;
	unsigned int E = 10;
	//Our containers.
	std::vector<std::string> file_list;
	std::vector<Slic> images;
	//Read every file in the input directory and load it into our image list.
	for(const auto & file : filesystem::directory_iterator(input_directory))
		file_list.push_back(file.path());
	for(int i=0; i<file_list.size(); i++)
		images.emplace_back(Slic(file_list[i]));

	for(Slic image : images){
		image.loadToGraph();
		image.generateSuperpixels(K,E);
		std::cout << "Write" << std::endl;
		image.writeToDisk(output_directory);
	}


	return 0;
}
