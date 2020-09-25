#pragma once

#include <iostream>

//#define __OUTPUT_DEBUG_INFO
#define __OUTPUT_STAGE_INFO

namespace WORM {
	const int IMAGE_SIZE = 512;
	const double INF = 1.0E10;
	const double PI = 3.14159265359;
	const double POSITION = 0.1;
}

namespace BW {
	const int SIDE_WIDTH = 4;
	const double BOUNDARY_WIDTH = 3;          // @kevin, 边缘宽度，在laplacian变换时不考虑此处的点
	const double MINIMUM_HOLE_PROPORTION = 0.02;
	extern int BINARY_THRESHOLD;
	const int MAX_BINARY_THRESHOLD = 70; // For current illumination, the binary threshold must be small
	extern double INITIAL_WORM_AREA;
	const double WORM_AREA_THRESHOLD = 0.6;
	const double LAPLACIAN_THRES = -0.3 * 4;	//对距离做laplacian变换后的矩阵中小于此值的点被取为中心线候选点
	const double RAP_THRESHOLD = 0.6;			//宽度大于线虫平均宽度的该倍数时，认为线虫发生了粘连
	const double THRESHOLD_AREA_PROPORTION = 0.3;
	const int MAX_EDGE_POINT_NUM = 1024;
	const double SHRINK_LAPLACE_THRESHOLD = -2 * 4;
	const double SHRINK_MIN_WIDTH = 3.0;
	const double SHRINK_COMPENSATION_WIDTH = 2.0;
}

namespace SKELETONIZE {
	const int POINT_NUM_MAX = 1024;
	const int DEGREE_MAX = 5;		//结点最大度数
	const int STORAGE_MAX = 200;
	const int METRICS_MAX = 18;
	const double ANGLE_THRESHOLD_NAN = WORM::PI / 3;   //骨架化时若找不到相邻点则在前方该角度内进行搜索
	const double ALPHA = 2;   //骨架化时若找不到相邻点进行搜索时的参数，alpha越大表示优先选择角度不变的元素
	const double ANGLE_ERROR = 1E-6;
	const int WORM_SPEED = 5; //线虫帧间爬过的长度（以体宽计）

	// @kevin
	const double WORM_TURNING_ANGLE = WORM::PI / 3;
	const double FORWARD_DIST_PORTION = 3;
}

namespace ROOT_SMOOTH {
	const int PARTITION_NUM = 100;       //将线虫中心线等分的份数
	const int SMOOTH_DETAIL_LEVEL = 1;   //非负，参数越大平滑细节越好，同时越耗时间
	const int SMOOTH_SCOPE = 5;          //平滑的规模，为0表示不做平滑，参数越大整体越平滑化
	const double ZERO_BOUND = 1.5;
	const int MULTIPLIER = 2 * SMOOTH_DETAIL_LEVEL + 1;
	const int FULL_PARTS = MULTIPLIER * (2 * SMOOTH_SCOPE + 1);
	const int HALF_PARTS = (FULL_PARTS - 1) / 2;
}

const std::string CACHE_DIR = "./cache_data/";

class Simple_Exception {
private:
	std::string message;
public:
	explicit Simple_Exception(std::string message) :message(message) {}
	void Add_message(std::string new_message) {
		message += new_message;
	}
	std::string Get_message() {
		return message;
	}
};
