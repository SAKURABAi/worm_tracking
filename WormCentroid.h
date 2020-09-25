
#ifndef _WORM_CENTROID_H_
#define _WORM_CENTROID_H_

#include "QException.h"
#include <iostream>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

class WormCentroid
{
public:
	static string OBJECT_NAME;
	WormCentroid();

	void GetWormCentroid(Mat &worm_image, double* centroid);
	inline double GetCurrentWormArea() {return Worm_Area;}

protected:
	void GetWormContourRange(vector<Point> worm_contour);

private:
	double Worm_Area;
	double last_centroid[2];
	double current_centroid[2];
	int worm_range[4];
};

#endif