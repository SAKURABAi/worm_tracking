﻿#include "stdafx.h"

using namespace std;
using namespace cv;
using namespace WORM;

const double Candidate_Points_Detect::HALF_THRES = BW::LAPLACIAN_THRES / 2;

//private:
void Candidate_Points_Detect::Distance_Retrace(double width_max) {
	const double RAP_SLOPE = BW::RAP_THRESHOLD / (1 - BW::RAP_THRESHOLD);
	for (int i = 0; i < distance_matrix.rows; ++i) {
		float * dist_row = distance_matrix.ptr<float>(i);
		for (int j = 0; j < distance_matrix.cols; ++j)
			if (dist_row[j] > BW::RAP_THRESHOLD * width_max)
				dist_row[j] = fmod(width_max - static_cast<double>(dist_row[j]), width_max) * RAP_SLOPE;
	}
}

/*void Candidate_Points_Detect::Calc_LapMat_Of_Inner_Part() const {
	const float * dist_row[IMAGE_SIZE];
	for (int i = 0; i < distance_matrix.rows; ++i)
		dist_row[i] = distance_matrix.ptr<float>(i);

	for (int i = 1; i < distance_matrix.rows - 1; ++i)
		for (int j = 1; j < distance_matrix.cols - 1; ++j)
			if (dist_row[i][j] > 2) {
				laplacian_matrix[i][j] = dist_row[i - 1][j] + dist_row[i + 1][j] + dist_row[i][j - 1] + dist_row[i][j + 1] - 4 * dist_row[i][j];
				if (laplacian_matrix[i][j] > 0)
					laplacian_matrix[i][j] = 0;
			}
			else
				laplacian_matrix[i][j] = 0;
	for (int i = 0; i < distance_matrix.rows; ++i) {
		laplacian_matrix[i][0] = 0;
		laplacian_matrix[i][distance_matrix.cols - 1] = 0;
	}
	for (int i = 1; i < distance_matrix.cols - 1; ++i) {
		laplacian_matrix[0][i] = 0;
		laplacian_matrix[distance_matrix.rows - 1][i] = 0;
	}
}*/
void Candidate_Points_Detect::Calc_LapMat_Of_Inner_Part(cv::Mat & dist_mat) const {
	const float * dist_row[IMAGE_SIZE];
	for (int i = 0; i < dist_mat.rows; ++i)
		dist_row[i] = dist_mat.ptr<float>(i);

	for (int i = 1; i < dist_mat.rows - 1; ++i)
		for (int j = 1; j < dist_mat.cols - 1; ++j)
			if (dist_row[i][j] > BW::BOUNDARY_WIDTH) {
				laplacian_matrix[i][j] = dist_row[i - 1][j] + dist_row[i + 1][j] + dist_row[i][j - 1] + dist_row[i][j + 1] - 4 * dist_row[i][j];
				if (laplacian_matrix[i][j] > 0)
					laplacian_matrix[i][j] = 0;
			}
			else
				laplacian_matrix[i][j] = 0;
	for (int i = 0; i < dist_mat.rows; ++i) {
		laplacian_matrix[i][0] = 0;
		laplacian_matrix[i][dist_mat.cols - 1] = 0;
	}
	for (int i = 1; i < dist_mat.cols - 1; ++i) {
		laplacian_matrix[0][i] = 0;
		laplacian_matrix[dist_mat.rows - 1][i] = 0;
	}
}

bool Candidate_Points_Detect::Lap_Value_Small_Enough(int i, int j) const {
	if (laplacian_matrix[i][j] < BW::LAPLACIAN_THRES)
		return true;
	if (laplacian_matrix[i][j] >= HALF_THRES)
		return false;
	return
		(laplacian_matrix[i - 1][j - 1] < HALF_THRES ? 1 : 0) +
		(laplacian_matrix[i - 1][j] < HALF_THRES ? 1 : 0) +
		(laplacian_matrix[i - 1][j + 1] < HALF_THRES ? 1 : 0) +
		(laplacian_matrix[i][j - 1] < HALF_THRES ? 1 : 0) +
		(laplacian_matrix[i][j + 1] < HALF_THRES ? 1 : 0) +
		(laplacian_matrix[i + 1][j - 1] < HALF_THRES ? 1 : 0) +
		(laplacian_matrix[i + 1][j] < HALF_THRES ? 1 : 0) +
		(laplacian_matrix[i + 1][j + 1] < HALF_THRES ? 1 : 0) >= 4;
}

/*void Candidate_Points_Detect::Catch_Candidate_By_LapMat(Candidate_Points &candidate_Points) {
	for (int i = 0; i < Worm_XY_Range[2] - 1; i++)
		candidate_Points.Add_Line();
	for (int i = 1; i < distance_matrix.rows - 1; ++i) {
		candidate_Points.Add_Line();
		for (int j = 1; j < distance_matrix.cols - 1; ++j){
			if (Lap_Value_Small_Enough(i, j)){
				candidate_Points.Add_Point_To_Line(j + Worm_XY_Range[0]);
				}
		}
	}
	candidate_Points.Add_Line();
	candidate_Points.Add_Line();
}*/

// @kevin
void Candidate_Points_Detect::Catch_Candidate_By_LapMat(cv::Mat dist_mat, Candidate_Points &candidate_Points) {
	for (int i = 0; i <= Worm_XY_Range[2]; i++)
		candidate_Points.Add_Line();

	float* dist_row = NULL;
	for (int i = 1; i < dist_mat.rows - 1; ++i) {
		candidate_Points.Add_Line();
		dist_row = dist_mat.ptr<float>(i);
		for (int j = 1; j < dist_mat.cols - 1; ++j){
			if (Lap_Value_Small_Enough(i, j)/*&& dist_row[j]<adhesion_width*/){
				candidate_Points.Add_Point_To_Line(j + Worm_XY_Range[0] + 1);
			}
		}
	}
	candidate_Points.Add_Line();
	candidate_Points.Add_Line();
}

void Candidate_Points_Detect::Contour_Range_Get(vector<Point> worm_contour, int * Range) {
	int top = worm_contour[0].x,
		bottom = worm_contour[0].x,
		left = worm_contour[0].y,
		right = worm_contour[0].y;
	for (unsigned i = 1; i < worm_contour.size(); ++i) {
		if (worm_contour[i].x < top)	top = worm_contour[i].x;
		if (worm_contour[i].x > bottom) bottom = worm_contour[i].x;
		if (worm_contour[i].y < left)	left = worm_contour[i].y;
		if (worm_contour[i].y > right)	right = worm_contour[i].y;
	}
	Range[0] = max(top - BW::SIDE_WIDTH, 1);
	Range[1] = min(bottom + BW::SIDE_WIDTH, WORM::IMAGE_SIZE);
	Range[2] = max(left - BW::SIDE_WIDTH, 1);
	Range[3] = min(right + BW::SIDE_WIDTH, WORM::IMAGE_SIZE);
}

void Candidate_Points_Detect::Denoise_And_Worm_Locate(double area) {
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	findContours(binary_image, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);//调用CV函数获取轮廓线
	Select_Minimum select_contours_by_area(WORM::INF, -1);
	double contour_area_temp;
	for (int i = 0; i >= 0; i = hierarchy[i][0]) {
		contour_area_temp = contourArea(contours[i]);
		int j = hierarchy[i][2];
		while (j >= 0) {
			contour_area_temp -= contourArea(contours[j]);
			j = hierarchy[j][0];
		}
		select_contours_by_area.Renew(abs(area - contour_area_temp), i);
	}
	int contour_select = select_contours_by_area.Get_Min_Index();
	this->area = contourArea(contours[contour_select]);
	if (contour_select == -1)
		throw new Simple_Exception("Can't get connected components of the worm");

	Contour_Range_Get(contours[contour_select], Worm_XY_Range);
	// Retain a contour whose area is closest to area
	for (int i = 0; i >= 0; i = hierarchy[i][0])
		drawContours(binary_image, contours, i, 255 * (i == contour_select), CV_FILLED, 8, hierarchy);

	//这个binary_image表示裁剪之后的图像数组，采用指针的方式返回，也可以使用引用类型的返回
	binary_image = Mat(binary_image,
		Range(Worm_XY_Range[2], Worm_XY_Range[3]),
		Range(Worm_XY_Range[0], Worm_XY_Range[1]));

	//----------------------填充线虫区域内部的空白杂点-----------------------
	contours.clear();//清空所有数据
	hierarchy.clear();
	binary_image = ~binary_image;
	//操作会影响binary_image，必须全部重新填充，并且需要补上边缘
	findContours(binary_image, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//填充线虫区域内部的空白杂点
	for (int i = 0; i >= 0; i = hierarchy[i][0]) {
		double hole_area = contourArea(contours[i]);
		bool retain_hole = hole_area > this->area * BW::MINIMUM_HOLE_PROPORTION;
		if (!retain_hole)
			this->area += hole_area;
		drawContours(binary_image, contours, i, 255 * retain_hole, CV_FILLED, 8, hierarchy);
	}
	binary_image = ~binary_image;

	//修补图像边缘
	for (int i = 0; i < binary_image.rows; ++i) {
		uchar * Mi = binary_image.ptr<uchar>(i);
		Mi[0] = 0;
		Mi[binary_image.cols - 1] = 0;
	}
	uchar * first_line = binary_image.ptr<uchar>(0);
	uchar * end_line = binary_image.ptr<uchar>(binary_image.rows - 1);
	for (int i = 1; i < binary_image.cols - 1; ++i) {
		first_line[i] = 0;
		end_line[i] = 0;
	}
}

int Candidate_Points_Detect::Adaptive_Threshold(const cv::Mat & image, double area)
{
	// Compute the histogram of grayscale image
	cv::Mat hist;
	int histSize = 256;
	float range[] = { 0, 256 };
	const float *histRange = { range };
	calcHist(&image, 1, 0, Mat(), hist, 1, &histSize, &histRange);
	double hist_values[256];
	for (unsigned i = 0; i < 256; ++i){
		hist_values[i] = hist.at<float>(i, 0);
	}

	// Compute the adaptive threshold accroding to the worm area
	double hist_threshold = area*BW::THRESHOLD_AREA_PROPORTION;
	for (int i = 255; i >= 0; --i){
		if (hist_values[i] >= hist_threshold){
			return (i+3);
		}
	}
}

double Candidate_Points_Detect::Shrink_Width(cv::Mat worm_region)
{
	const uchar *img_row[IMAGE_SIZE];
	for (int i = 0; i < worm_region.rows; ++i)
		img_row[i] = worm_region.ptr<uchar>(i);

	int point_num = 0;
	for (int i = 1; i < worm_region.rows - 1; ++i){
		uchar* binary_img_row = binary_image.ptr<uchar>(i);
		float* dist_row = distance_matrix.ptr<float>(i);
		for (int j = 1; j < worm_region.cols - 1; ++j){
			laplacian_matrix[i][j] = img_row[i - 1][j] + img_row[i + 1][j] + img_row[i][j - 1] + img_row[i][j + 1] - 4 * img_row[i][j];
			if (binary_img_row[j] > 0 && laplacian_matrix[i][j] < BW::SHRINK_LAPLACE_THRESHOLD){
				Worm_Edge_Width[point_num] = dist_row[j];
				point_num = point_num + 1;
			}
		}
	}
	double median_width = Compute_Median<float>(Worm_Edge_Width, point_num);

	// 计算Worm_Edge_Width的中值
	return max(median_width - BW::SHRINK_COMPENSATION_WIDTH, BW::SHRINK_MIN_WIDTH);
}

void Candidate_Points_Detect::Worm_Shrink(const cv::Mat & worm_image, double area)
{
	// 获取线虫图像区域
	cv::Mat worm_region = Mat(worm_image, Range(Worm_XY_Range[2], Worm_XY_Range[3]),
		Range(Worm_XY_Range[0], Worm_XY_Range[1]));

	distanceTransform(binary_image, distance_matrix, CV_DIST_L2, CV_DIST_MASK_PRECISE);// 计算初始二值图像的距离矩阵
	double shrink_width = Shrink_Width(worm_region);// 对线虫区域做Laplace变换，计算线虫区域收缩宽度
	binary_image = distance_matrix > shrink_width; // 对线虫二值图像进行收缩处理
}

// Use laplacian matrix and intial distance_matrix to modify the binary image
void Candidate_Points_Detect::Distance_Modification(double width)
{
	this->adhesion_width = BW::RAP_THRESHOLD*width;
	float* dist_row = NULL;
	for (int i = 1; i < distance_matrix.rows - 1; ++i){
		dist_row = distance_matrix.ptr<float>(i);
		for (int j = 1; j < distance_matrix.cols - 1; ++j){
			if (dist_row[j] > adhesion_width && laplacian_matrix[i][j] < BW::LAPLACIAN_THRES){
				binary_image.at<uchar>(i, j) = 0;
			}
		}
	}
}

void Candidate_Points_Detect::ImageDilate(cv::Mat &img){
	uchar *last_row = NULL, *current_row = NULL, *next_row = NULL;
	uchar *last_img_row = NULL, *current_img_row = NULL, *next_img_row = NULL;
	for (int i = 2; i < binary_image.rows - 2; ++i){
		// rows of binary image
		last_row = binary_image.ptr<uchar>(i - 1);
		current_row = binary_image.ptr<uchar>(i);
		next_row = binary_image.ptr<uchar>(i + 1);

		// rows of img
		last_img_row = img.ptr<uchar>(i - 1);
		current_img_row = img.ptr<uchar>(i);
		next_img_row = img.ptr<uchar>(i + 1);
		for (int j = 2; j < binary_image.cols - 2; ++j){ 
			if ((current_row[j] & 0xFF) &&
				((last_row[j - 1] & 0xFF) + (last_row[j] & 0xFF) + (last_row[j + 1] & 0xFF) +
				(current_row[j - 1] & 0xFF) + (current_row[j + 1] & 0xFF) + (next_row[j - 1] & 0xFF) +
				(next_row[j] & 0xFF) + (next_row[j + 1] & 0xFF) != 8)){
				last_img_row[j - 1] = 0xFF;
				last_img_row[j] = 0xFF;
				last_img_row[j + 1] = 0xFF;
				current_img_row[j - 1] = 0xFF;
				current_img_row[j + 1] = 0xFF;
				next_img_row[j - 1] = 0xFF;
				next_img_row[j] = 0xFF;
				next_img_row[j + 1] = 0xFF;
			}
			else if (current_row[j] & 0xFF)
				current_img_row[j] = 0xFF;
		}
	}
}

void Candidate_Points_Detect::ImageErode(cv::Mat &img){
	uchar *last_row = NULL, *current_row = NULL, *next_row = NULL;
	for (int i = 2; i < binary_image.rows - 2; ++i){
		last_row = binary_image.ptr<uchar>(i - 1);
		current_row = binary_image.ptr<uchar>(i);
		next_row = binary_image.ptr<uchar>(i + 1);
		for (int j = 2; j < binary_image.cols - 2; ++j){
			if ((current_row[j] & 0xFF) &&
				((last_row[j - 1] & 0xFF) + (last_row[j] & 0xFF) + (last_row[j + 1] & 0xFF) +
				(current_row[j - 1] & 0xFF) + (current_row[j + 1] & 0xFF) + (next_row[j - 1] & 0xFF) +
				(next_row[j] & 0xFF) + (next_row[j + 1] & 0xFF) < 8)){
				//img.at<uchar>(i, j) = 0x0;
				current_row[j] = 0x0;
			}
		}
	}
}

void Candidate_Points_Detect::ImageFillHole(cv::Mat &img, double area){
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	img = ~img;
	//操作会影响binary_image，必须全部重新填充，并且需要补上边缘
	findContours(img, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//填充线虫区域内部的空白杂点
	for (int i = 0; i >= 0; i = hierarchy[i][0]) {
		double hole_area = contourArea(contours[i]);
		bool retain_hole = hole_area > area * BW::MINIMUM_HOLE_PROPORTION / 2;
		drawContours(img, contours, i, 0xFF * retain_hole, CV_FILLED, 8, hierarchy);
	}
	img = ~img;

	// 处理边缘
	for (int i = 0; i < img.rows; ++i) {
		uchar * Mi = img.ptr<uchar>(i);
		Mi[0] = 0;
		Mi[binary_image.cols - 1] = 0;
	}
	uchar * first_line = img.ptr<uchar>(0);
	uchar * end_line = img.ptr<uchar>(img.rows - 1);
	for (int i = 1; i < img.cols - 1; ++i) {
		first_line[i] = 0;
		end_line[i] = 0;
	}
}

//public:
void Candidate_Points_Detect::Save2File(string binary_cache_dir, string dist_cache_dir, string lap_cache_dir, string pic_num) const {
	//Save_Mat_To_File<uchar>(binary_image, binary_cache_dir + pic_num);
	ofstream file((binary_cache_dir + pic_num).c_str(), ios::binary);
	int rows = binary_image.rows;
	int cols = binary_image.cols;
	file.write(reinterpret_cast<const char *>(Worm_XY_Range), sizeof(int) * 4);
	file.write(reinterpret_cast<char *>(&rows), sizeof(int));
	file.write(reinterpret_cast<char *>(&cols), sizeof(int));
	for (int i = 0; i < binary_image.rows; ++i) {
		const uchar * ptr = binary_image.ptr<uchar>(i);
		file.write(reinterpret_cast<const char *>(ptr), binary_image.cols * sizeof(uchar));
	}
	file.close();

	Save_Mat_To_File<float>(distance_matrix_modification, dist_cache_dir + pic_num);
	ofstream lap_file((lap_cache_dir + pic_num).c_str(), ios::binary);
	rows = distance_matrix.rows;
	cols = distance_matrix.cols;
	lap_file.write(reinterpret_cast<char *>(&rows), sizeof(int));
	lap_file.write(reinterpret_cast<char *>(&cols), sizeof(int));
	for (int i = 0; i < distance_matrix.rows; ++i)
		lap_file.write(reinterpret_cast<char *>(laplacian_matrix[i]), distance_matrix.cols * sizeof(double));
	lap_file.close();
}

double Candidate_Points_Detect::Get_Dist(double x, double y) const {
	x -= Worm_XY_Range[2];
	y -= Worm_XY_Range[0];
	if (x < 0 || y < 0 || x >= distance_matrix.rows || y >= distance_matrix.cols)
		return 0;
	double alpha, beta;
	int x0 = int(x), y0 = int(y);
	alpha = x - x0;
	beta = y - y0;
	return (distance_matrix.at<float>(x, y) * (1 - alpha) * (1 - beta) +
		distance_matrix.at<float>(x + 1, y) * alpha * (1 - beta) +
		distance_matrix.at<float>(x, y + 1) * (1 - alpha) * beta +
		distance_matrix.at<float>(x + 1, y + 1) * alpha * beta);
}

void Candidate_Points_Detect::Detect_Points(const Mat & image, Candidate_Points & candidate_points, double width, double area)
{
	candidate_points.Reset();
	
	//int binary_threshold = Adaptive_Threshold(image,area);
	int binary_threshold = BW::BINARY_THRESHOLD;
	if (binary_threshold > BW::MAX_BINARY_THRESHOLD){
		throw new Simple_Exception("Candidate_Points_Detect: Adaptive Threshold Error!");
	}
	cout<<"Worm area is: "<<area<<", Binary threshold is: "<< binary_threshold << endl;

	binary_image = image > binary_threshold;
	Denoise_And_Worm_Locate(area);

	Mat new_binary_image = cv::Mat::zeros(binary_image.rows, binary_image.cols, CV_8UC1);
	ImageDilate(new_binary_image);
	ImageFillHole(new_binary_image, area);
	ImageErode(new_binary_image);
	new_binary_image.copyTo(binary_image);
	Worm_Shrink(image, area);
	
	// @kevin-20170410
	distanceTransform(binary_image, distance_matrix, CV_DIST_L2, CV_DIST_MASK_PRECISE);
	Calc_LapMat_Of_Inner_Part(distance_matrix);
	Distance_Modification(width);
	distanceTransform(binary_image, distance_matrix_modification, CV_DIST_L2, CV_DIST_MASK_PRECISE);
	Calc_LapMat_Of_Inner_Part(distance_matrix_modification);
	Catch_Candidate_By_LapMat(distance_matrix_modification, candidate_points);

	/*distanceTransform(binary_image, distance_matrix, CV_DIST_L2, CV_DIST_MASK_PRECISE);
	Distance_Retrace(width);
	Calc_LapMat_Of_Inner_Part(); 
	Catch_Candidate_By_LapMat(candidate_points);*/
}