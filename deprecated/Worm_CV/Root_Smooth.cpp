#include "stdafx.h"

using namespace std;
using namespace ROOT_SMOOTH;

//Root_Smooth:private
void Root_Smooth::Interpolate(){
	// ==================================计算插值系数====================================
	double coefficent[ROOT_SMOOTH::FULL_PARTS];
	double dh = ZERO_BOUND / (SMOOTH_SCOPE * MULTIPLIER);
	double sum[MULTIPLIER] = {};
	// 取正态曲线在[-ZERO_BOUND,ZERO_BOUND]区间上SMOOTH_SCOPE * MULTIPLIER等分的值为权重因子，并对其分组归一化
	for (int i = 1;i <= HALF_PARTS;++ i){
		double value = exp(-dh*dh*i*i/2);
		coefficent[HALF_PARTS - i] = value;
		coefficent[HALF_PARTS + i] = value;
		sum[(HALF_PARTS - i) % MULTIPLIER] += value;
		sum[(HALF_PARTS + i) % MULTIPLIER] += value;
	}
	coefficent[HALF_PARTS] = 1;
	sum[SMOOTH_DETAIL_LEVEL] += 1;
	for (int i = 0;i < FULL_PARTS;++ i)
		coefficent[i] /= sum[i % 3];
	// ==================================正式插值过程=================================
	for (int i = 0;i < INTERPOLATE_NUM;++ i){
		interpolate_coodinate[i][0] = 0;
		interpolate_coodinate[i][1] = 0;
	}
	// 进行加密后的样板卷积
	for (int i = 0;i < origin_num;++ i)
		for (int j = 0;j < FULL_PARTS;++ j){
			int index = i * MULTIPLIER + j - HALF_PARTS;
			if (index < 0 || index >= INTERPOLATE_NUM)
				continue;
			interpolate_coodinate[index][0] += coodinate[i][0] * coefficent[j];
			interpolate_coodinate[index][1] += coodinate[i][1] * coefficent[j];
		}
	// 两端使用镜像点进行插值
	for (int i = 0;i < SMOOTH_SCOPE;++ i){
		double symmetry_cood[2];
		// 首端
		symmetry_cood[0] = 2 * coodinate[0][0] - coodinate[i+1][0];
		symmetry_cood[1] = 2 * coodinate[0][1] - coodinate[i+1][1];
		for (int j = 0;j < FULL_PARTS;++ j){
			int index = (-1-i) * MULTIPLIER + j - HALF_PARTS;
			if (index < 0 || index >= INTERPOLATE_NUM)
				continue;
			interpolate_coodinate[index][0] += symmetry_cood[0] * coefficent[j];
			interpolate_coodinate[index][1] += symmetry_cood[1] * coefficent[j];
		}
		// 尾端
		symmetry_cood[0] = 2 * coodinate[origin_num-1][0] - coodinate[origin_num-i-2][0];
		symmetry_cood[1] = 2 * coodinate[origin_num-1][1] - coodinate[origin_num-i-2][1];
		for (int j = 0;j < FULL_PARTS;++ j){
			int index = (origin_num+i) * MULTIPLIER + j - HALF_PARTS;
			if (index < 0 || index >= INTERPOLATE_NUM)
				continue;
			interpolate_coodinate[index][0] += symmetry_cood[0] * coefficent[j];
			interpolate_coodinate[index][1] += symmetry_cood[1] * coefficent[j];
		}
	}
}
void Root_Smooth::Equal_Divide(int partition_num){
	// 记录首尾坐标
	coodinate[0][0] = interpolate_coodinate[0][0];
	coodinate[0][1] = interpolate_coodinate[0][1];
	coodinate[partition_num][0] = interpolate_coodinate[INTERPOLATE_NUM-1][0];
	coodinate[partition_num][1] = interpolate_coodinate[INTERPOLATE_NUM-1][1];
	// 计算每个点的距离参数
	double * length_each = new double[INTERPOLATE_NUM];
	length_each[0] = 0;
	for (int i = 1;i < INTERPOLATE_NUM;++ i)
		length_each[i] = length_each[i-1] + sqrt(Point_Dist_Square(interpolate_coodinate[i], interpolate_coodinate[i-1]));
	double full_length = length_each[INTERPOLATE_NUM - 1];
	double segment_length = full_length / partition_num;
	int index ,last_index = 0;
	double alpha;
	// 计算等分点
	for (int i = 1;i < partition_num;++ i){
		index = Binary_Chop(length_each, segment_length * i, last_index, INTERPOLATE_NUM - 1);
		alpha = (segment_length * i - length_each[index]) / (length_each[index+1] - length_each[index]);
		coodinate[i][0] = interpolate_coodinate[index+1][0] * alpha + interpolate_coodinate[index][0] * (1-alpha);
		coodinate[i][1] = interpolate_coodinate[index+1][1] * alpha + interpolate_coodinate[index][1] * (1-alpha);
	}
	delete[] length_each;
}
//Root_Smooth:public
void Root_Smooth::Interpolate_And_Equal_Divide(Backbone & centerline_to_smooth, int partition_num){
	// 初始化并进行插值
	origin_num = centerline_to_smooth.length;
	INTERPOLATE_NUM = origin_num * MULTIPLIER - 2;
	coodinate = centerline_to_smooth.cood;

	interpolate_coodinate = new double[INTERPOLATE_NUM][2];
	Interpolate();

	delete[] coodinate;
	coodinate = new double[partition_num + 1][2];
	
	// 进行等分，并将结果存回到原有centerline中
	Equal_Divide(partition_num);
	centerline_to_smooth.cood = coodinate;
	centerline_to_smooth.length = partition_num + 1;
	centerline_to_smooth.size = partition_num + 1;
	delete[] interpolate_coodinate;
}
