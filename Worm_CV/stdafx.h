#pragma once

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cmath>
#include <string>
#include "Search_Backbone.h"

template<typename T>
double Compute_Median(T* data, int num){
	sort(data, data + num);
	if (num % 2 == 0){
		return (data[num / 2] + data[num / 2 - 1]) / 2.0;
	}
	else{
		return 1.0*data[num / 2];
	}
}

template<typename T1, typename T2>
double Point_Dist_Square(const T1 * p1, const T2 * p2) {
	return (p1[0] - p2[0])*(p1[0] - p2[0]) + (p1[1] - p2[1])*(p1[1] - p2[1]);
}

template<typename T1, typename T2>
double Included_Angle_Tan(const T1 * vec_1, const T2 * vec_2) {
	// 做旋转变换将vec_1旋转到y轴正方向后vec_2的方向
	double angle[2] = { vec_2[0] * vec_1[1] - vec_2[1] * vec_1[0], vec_1[0] * vec_2[0] + vec_1[1] * vec_2[1] };
	if (angle[0] < 0)
		angle[0] = -angle[0];
	return angle[1] <= 1E-5 ? -1 : angle[0] / angle[1];
}

template<typename T1, typename T2>
double Cal_Cos_Theta( const T1* vec_1, const T2* vec_2){
	double cos_theta = (vec_1[0]*vec_2[0] + vec_1[1]*vec_2[1])/(sqrt(vec_1[0]*vec_1[0]+vec_1[1]*vec_1[1])*
	sqrt(vec_2[0]*vec_2[0]+vec_2[1]*vec_2[1]));
	return cos_theta;
}

template<typename T>
double Calc_Clockwise_Angle(const T* p0, const T* p1, const T* p2) {
	double angle_clockwise = atan2(p2[0] - p1[0], p2[1] - p1[1]) - atan2(p0[0] - p1[0], p0[1] - p1[1]);
	if (angle_clockwise < 0)
		angle_clockwise += 2 * WORM::PI;
	return angle_clockwise;
}

// 有序数组二分查找
template<class T>
int Binary_Chop(const T * ordered_array, T element_to_locate, int left, int right) {
	if (element_to_locate < ordered_array[left] || element_to_locate > ordered_array[right])
		return -1;
	if (element_to_locate == ordered_array[right])
		return right;
	while (right - left > 1) {
		int mid = (left + right) / 2;
		if (ordered_array[mid] < element_to_locate)
			left = mid;
		else if (ordered_array[mid] > element_to_locate)
			right = mid;
		else return mid;
	}
	return left;
}

inline std::string int2str(int num) {
	std::stringstream stream;
	std::string s;
	stream << num;
	stream >> s;
	return s;
}

class Select_Minimum {
private:
	double min_Val;
	int min_Index;
public:
	Select_Minimum(double initial_val, int initial_index) :
		min_Val(initial_val), min_Index(initial_index) {
	};
	void Renew(double new_val, int new_index) {
		if (new_val < min_Val) {
			min_Val = new_val;
			min_Index = new_index;
		}
	}
	int Get_Min_Index() const { return min_Index; }
	double Get_Min_Val() const { return min_Val; }
};

template<typename T>
void Save_Mat_To_File(const cv::Mat & mat, std::string file_dir) {
	int rows = mat.rows;
	int cols = mat.cols;
	std::ofstream file(file_dir.c_str(), std::ios::binary);
	file.write(reinterpret_cast<char *>(&rows), sizeof(int));
	file.write(reinterpret_cast<char *>(&cols), sizeof(int));
	for (int i = 0; i < mat.rows; ++i) {
		const T * ptr = mat.ptr<T>(i);
		file.write(reinterpret_cast<const char *>(ptr), mat.cols * sizeof(T));
	}
	file.close();
}