#pragma once

class Root_Smooth{
private:
	int origin_num;
	int INTERPOLATE_NUM;
	double (* interpolate_coodinate)[2];
	double (* coodinate)[2];

	void Interpolate();
	void Equal_Divide(int partition_num);
public:
	void Interpolate_And_Equal_Divide(Backbone & centerline_to_smooth, int partition_num);
};