
#include "WormCentroid.h"
#include "Worm_CV/CONST_PARA.h"

#define WORM_AREA_THRESHOLD 0.3

string WormCentroid::OBJECT_NAME = "WormCentroid";

WormCentroid::WormCentroid()
{
	last_centroid = {255, 255};
	current_centroid = {255, 255};
	worm_range = {0,0,0,0};
	Worm_Area = BW::INITIAL_WORM_AREA;
}

void WormCentroid::GetWormContourRange(vector<Point> worm_contour)
{
	int top = worm_contour[0].x,
		bottom = worm_contour[0].x,
		left = worm_contour[0].y,
		right = worm_contour[0].y;
	for (unsigned int i = 1; i < worm_contour.size(); ++i) {
		if (worm_contour[i].x < top)	top = worm_contour[i].x;
		if (worm_contour[i].x > bottom) bottom = worm_contour[i].x;
		if (worm_contour[i].y < left)	left = worm_contour[i].y;
		if (worm_contour[i].y > right)	right = worm_contour[i].y;
	}
	worm_range[0] = max(top - BW::SIDE_WIDTH, 1);
	worm_range[1] = min(bottom + BW::SIDE_WIDTH, WORM::IMAGE_SIZE);
	worm_range[2] = max(left - BW::SIDE_WIDTH, 1);
	worm_range[3] = min(right + BW::SIDE_WIDTH, WORM::IMAGE_SIZE);
}

void WormCentroid::GetWormCentroid(Mat &worm_image, double* centroid)
{
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;

	//Remove the dots outside the worm
	findContours(worm_image, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
	if (contours.size() <= 0){
		Worm_Area = BW::INITIAL_WORM_AREA;
		throw QException(OBJECT_NAME, "GetWormCentroid()", "Fail to extract worm region");
	}

	for (int i = 0; i >= 0; i = hierarchy[i][0]){
		double area = contourArea(contours[i]);
		bool is_worm = abs(area - Worm_Area) < Worm_Area*WORM_AREA_THRESHOLD;
		drawContours(worm_image, contours, i, 255*is_worm, CV_FILLED, 8, hierarchy);
	}
	
	//Remove the dots inside the worm
	worm_image = ~worm_image;
	contours.clear();
	hierarchy.clear();
	findContours(worm_image, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
	for (int i = 0; i >=0; i=hierarchy[i][0]){
		double area = contourArea(contours[i]);
		bool retain_hole = area < Worm_Area*BW::MINIMUM_HOLE_PROPORTION;
		drawContours(worm_image, contours, i, 255*(!retain_hole), CV_FILLED, 8, hierarchy);
	}
	worm_image = ~worm_image;

	//compute the centroid of the worm
	int worm_index = -1;
	double center_distance = 512*512*2;

	contours.clear();
	hierarchy.clear();

	findContours(worm_image, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
	if (contours.size() <= 0){
		Worm_Area = BW::INITIAL_WORM_AREA;
		throw QException(OBJECT_NAME, "GetWormCentroid()", "No Worm");
	}

	for (int i = 0; i >=0; i=hierarchy[i][0]){
		Moments moment = moments(contours[i]);
		if (moment.m00 < 1.0e-3){
			throw QException(OBJECT_NAME, "GetWormCentroid()", "Cannot compute the moment of worm image");
		}
		
		double x = 1.0*moment.m10/moment.m00;
		double y = 1.0*moment.m01/moment.m00;
		double distance = (x-last_centroid[0])*(x-last_centroid[0]) + (y-last_centroid[1])*(y-last_centroid[1]);
		if (distance < center_distance){
			center_distance = distance;
			worm_index = i; //寻找与图像中心距离最近的线虫contour
		}
	}
	if (worm_index != -1){
		Worm_Area = contourArea(contours[worm_index]); //update worm area
		Moments moment = moments(contours[worm_index]);
		current_centroid[0] = 1.0*moment.m10/moment.m00;
		current_centroid[1] = 1.0*moment.m01/moment.m00;
		last_centroid[0] = current_centroid[0];
		last_centroid[1] = current_centroid[1];

		//返回线虫质心
		centroid[0] = current_centroid[0];
		centroid[1] = current_centroid[1];
	}
}