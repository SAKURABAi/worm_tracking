#pragma once

#include "CONST_PARA.h"
#include <opencv2/highgui/highgui.hpp>

// Candidate_Center_Points结构体表示中心线候选点
class Candidate_Points {
private:
	int point_num; // 中心点的个数
	int cood[SKELETONIZE::POINT_NUM_MAX][2];//候选点坐标
	int hash_table[WORM::IMAGE_SIZE + 1];//hash表，表中每项存储的是中心线候选点x坐标小于该位置的数目
	int current_line;
	mutable int range_x[2], range_y[2];
	Candidate_Points(const Candidate_Points &) {}
	Candidate_Points & operator= (const Candidate_Points &) { return *this; }
	void Range_Calc(std::vector<int> & base_points) const;

public:
	Candidate_Points() {
		Reset();
	};
	void Reset() {
		point_num = 0;
		current_line = 0;
		hash_table[0] = 0;
	}
	int Get_Point_Num() const {
		return point_num;
	}
	const double * Get_Center(std::vector<int> & points) const;
	const int * Get_Point(int index) const {
		return cood[index];
	}
	inline bool Is_Point_Nearby(int index1, int index2) const {
		return cood[index1][0] - cood[index2][0] < 2 && cood[index2][0] - cood[index1][0] < 2
			&& cood[index1][1] - cood[index2][1] < 2 && cood[index2][1] - cood[index1][1] < 2;
	}
	void Query_Points_Nearby(std::vector<int> & base_points, std::vector<int> & nearby_points) const;
	int Query_Points_By_Pointer(const double * base_point, const double * direct_vec) const;
	void Add_Line();
	void Add_Point_To_Line(int y);
	static void persistence(void *obj_ptr, std::string out_file);
	static void anti_persistence(void * obj_ptr, std::string in_file);
	std::string getPointStr(std::vector<int> & points) const;
	std::string getWholeStr() const;
};

class Candidate_Points_Detect {
private:
	cv::Mat binary_image;
	cv::Mat distance_matrix;
	cv::Mat distance_matrix_modification; // @kevin
	double(*laplacian_matrix)[WORM::IMAGE_SIZE];
	int Worm_XY_Range[4];
	double area, adhesion_width;
	static const double HALF_THRES;
	float Worm_Edge_Width[BW::MAX_EDGE_POINT_NUM]; //used to shrink the binary worm region

	void Distance_Retrace(double width_max);
	//void Calc_LapMat_Of_Inner_Part() const;
	void Calc_LapMat_Of_Inner_Part(cv::Mat & dist_mat) const; // @kevin
	inline bool Lap_Value_Small_Enough(int i, int j) const;
	//void Catch_Candidate_By_LapMat(Candidate_Points &candidate_Points);
	void Catch_Candidate_By_LapMat(cv::Mat dist_mat, Candidate_Points &candidate_Points);// @kevin
	void Contour_Range_Get(std::vector<cv::Point> worm_contour, int * Range);
	void Denoise_And_Worm_Locate(double area);

	// @kevin
	int Adaptive_Threshold(const cv::Mat & image, double area);
	double Shrink_Width(cv::Mat worm_region);
	void Worm_Shrink(const cv::Mat & worm_image, double area);
	void Distance_Modification(double width);
	void ImageDilate(cv::Mat &img);
	void ImageErode(cv::Mat &img);
	void ImageFillHole(cv::Mat &img, double area);

public:
	Candidate_Points_Detect() {
		laplacian_matrix = new double[WORM::IMAGE_SIZE][WORM::IMAGE_SIZE];
	}
	~Candidate_Points_Detect() {
		delete[] laplacian_matrix;
	}
	
	void Save2File(std::string binary_cache_dir, std::string dist_cache_dir, std::string lap_cache_dir, std::string pic_num) const;
	double Get_Dist(double x, double y) const;
	double Get_Area() const {
		return area;
	};
	void Detect_Points(const cv::Mat & binary_image, Candidate_Points & candidate_points, double width = WORM::INF, double area = WORM::INF);
};