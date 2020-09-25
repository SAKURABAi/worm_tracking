
//计算图像清晰度
#include <opencv2/core/core.hpp>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <string.h>

using namespace std;
using namespace cv;

template <typename T> inline T Square(T a){ return a*a; }
double Depth(const Mat &image, char *type)
{
	int cols = image.cols, rows = image.rows;
	double outfocus_depth = 0, temp = 0;
    const uchar * Mi_prev, * Mi_current, * Mi_next;

	if( strcmp(type, "Absolute") == 0) {
		for (int i = 0; i < rows-1; ++i){
			Mi_current = image.ptr<uchar>(i);
			Mi_next = image.ptr<uchar>(i+1);
			for(int j = 0; j < cols;  ++j){
				outfocus_depth += abs(Mi_current[j] - Mi_next[j]);
			}
		}
	}
	else if(strcmp(type, "Roberts") == 0){
		for (int i = 0; i < rows-1; ++i){
			Mi_current = image.ptr<uchar>(i);
			Mi_next = image.ptr<uchar>(i+1);
			for(int j = 0; j < cols-1;  ++j){
				outfocus_depth += abs(Mi_current[j] - Mi_next[j+1]);
				outfocus_depth += abs(Mi_next[j] - Mi_current[j+1]);
			}
		}
	}
	else if(strcmp(type, "CrandientModule") == 0){
		for (int i = 0; i < rows-1; ++i){
			Mi_current = image.ptr<uchar>(i);
			Mi_next = image.ptr<uchar>(i+1);
			for(int j = 0; j < cols-1;  ++j){
				temp = Square(Mi_next[j] - Mi_current[j]) + Square(Mi_current[j+1] - Mi_next[j]);
				outfocus_depth += sqrt(temp);
			}
		}
	}
	else if(strcmp(type, "GrandientSquare") == 0){
		for (int i = 0; i < rows-1; ++i){
			Mi_current = image.ptr<uchar>(i);
			Mi_next = image.ptr<uchar>(i+1);
			for(int j = 0; j < cols-1;  ++j){
				temp = Square(Mi_next[j] - Mi_current[j]) + Square(Mi_current[j+1] - Mi_next[j]);
				outfocus_depth += temp;
			}
		}
	}
	else if(strcmp(type, "Laplacian") == 0){
		for (int i = 1; i < rows-1; ++i){
			Mi_prev = image.ptr<uchar>(i-1);
			Mi_current = image.ptr<uchar>(i);
			Mi_next = image.ptr<uchar>(i+1);
			for(int j = 1; j < cols-1; ++j){
				temp = 4*Mi_current[j] - Mi_current[j+1] - Mi_current[j-1] - Mi_prev[j] - Mi_next[j];
				outfocus_depth += Square(temp);
			}
		}
	}
	else if(strcmp(type, "LaplacianA") == 0){//laplacian for 4 directions
		for (int i = 1; i < rows-1; ++i){
			Mi_prev = image.ptr<uchar>(i-1);
			Mi_current = image.ptr<uchar>(i);
			Mi_next = image.ptr<uchar>(i+1);
			for(int j = 1; j < cols-1;  ++j){
				temp = Square(Mi_current[j] - Mi_current[j+1]) + Square(Mi_current[j] - Mi_current[j-1])+ 
				                                 Square(Mi_current[j] - Mi_prev[j]) + Square(Mi_current[j] - Mi_next[j]);
				outfocus_depth += temp;
			}
		}
	}
	else if(strcmp(type, "LaplacianB") == 0){//laplacian for 8 directions
		for (int i = 1; i < rows-1; ++i){
			Mi_prev = image.ptr<uchar>(i-1);
			Mi_current = image.ptr<uchar>(i);
			Mi_next = image.ptr<uchar>(i+1);
			for(int j = 1; j < cols-1;  ++j){
				temp = Square(Mi_current[j] - Mi_prev[j-1]) + Square(Mi_current[j] - Mi_prev[j]) + Square(Mi_current[j] - Mi_prev[j+1])+
				       Square(Mi_current[j] - Mi_current[j-1]) + Square(Mi_current[j] - Mi_current[j+1]) +
				       Square(Mi_current[j] - Mi_next[j-1]) + Square(Mi_current[j] - Mi_next[j]) + Square(Mi_current[j] - Mi_next[j+1]);
				outfocus_depth += temp;
			}
		}
	}
	else if(strcmp(type, "Tenengrad") == 0){
		for (int i = 1; i < rows-1; ++i){
			Mi_prev = image.ptr<uchar>(i-1);
			Mi_current = image.ptr<uchar>(i);
			Mi_next = image.ptr<uchar>(i+1);
			for(int j = 1; j < cols-1; ++j){
				temp = Mi_prev[j-1] - Mi_next[j-1] + 2*(Mi_prev[j] - Mi_next[j]) + Mi_prev[j+1] - Mi_next[j+1];
				outfocus_depth += Square(temp);
				temp = Mi_prev[j+1] - Mi_prev[j-1] + 2*(Mi_current[j+1] - Mi_current[j-1]) + Mi_next[j+1] - Mi_next[j-1];
				outfocus_depth += Square(temp);
			}
		}
	}
	else if(strcmp(type, "Sobel3") == 0){
		for (int i = 1; i < rows-1; ++i){
			Mi_prev = image.ptr<uchar>(i-1);
			Mi_current = image.ptr<uchar>(i);
			Mi_next = image.ptr<uchar>(i+1);
			for(int j = 1; j < cols-1;  ++j){
				temp = Mi_prev[j-1] - Mi_next[j-1] + 2*(Mi_prev[j] - Mi_next[j]) + Mi_prev[j+1] - Mi_next[j+1];
				outfocus_depth += Square(temp);

				temp = Mi_prev[j+1] - Mi_prev[j-1] + 2*(Mi_current[j+1] - Mi_current[j-1]) + Mi_next[j+1] - Mi_next[j-1];
				outfocus_depth += Square(temp);

				temp = Mi_prev[j] - Mi_current[j-1] + 2*(Mi_prev[j+1] - Mi_next[j-1]) + Mi_current[j+1] - Mi_next[j];
				outfocus_depth += Square(temp);
			}
		}
	}
	else if(strcmp(type, "Sobel4") == 0){
		for (int i = 1; i < rows-1; ++i){
			Mi_prev = image.ptr<uchar>(i-1);
			Mi_current = image.ptr<uchar>(i);
			Mi_next = image.ptr<uchar>(i+1);
			for(int j = 1; j < cols-1; ++j){
				temp = Mi_prev[j-1] - Mi_next[j-1] + 2*(Mi_prev[j] - Mi_next[j]) + Mi_prev[j+1] - Mi_next[j+1];
				outfocus_depth += Square(temp);

				temp = Mi_prev[j+1] - Mi_prev[j-1] + 2*(Mi_current[j+1] - Mi_current[j-1]) + Mi_next[j+1] - Mi_next[j-1];
				outfocus_depth += Square(temp);

				temp = Mi_prev[j] - Mi_current[j-1] + 2*(Mi_prev[j+1] - Mi_next[j-1]) + Mi_current[j+1] - Mi_next[j];
				outfocus_depth += Square(temp);

				temp = Mi_current[j-1] - Mi_next[j] + 2*(Mi_prev[j-1] - Mi_next[j+1]) + Mi_prev[j] - Mi_current[j+1];
				outfocus_depth += Square(temp);
			}
		}
	}
	else if(strcmp(type, "Robinson") == 0){
		for (int i = 1; i < rows-1; ++i){
			Mi_prev = image.ptr<uchar>(i-1);
			Mi_current = image.ptr<uchar>(i);
			Mi_next = image.ptr<uchar>(i+1);
			for(int j = 1; j < cols-1; ++j){
				temp = Mi_prev[j-1] + Mi_prev[j] + Mi_prev[j+1] + Mi_current[j-1] - 
				     2*Mi_current[j] + Mi_current[j+1] -
				       Mi_next[j-1] - Mi_next[j] - Mi_next[j+1];
				outfocus_depth += Square(temp);

				temp = Mi_prev[j-1] + Mi_prev[j] + Mi_prev[j+1] - Mi_current[j-1] - 
				     2*Mi_current[j] + Mi_current[j+1] -
				       Mi_next[j-1] - Mi_next[j] + Mi_next[j+1];
				outfocus_depth += Square(temp);

				temp = -Mi_prev[j-1] + Mi_prev[j] + Mi_prev[j+1] - Mi_current[j-1] - 
				      2*Mi_current[j] + Mi_current[j+1] -
				        Mi_next[j-1] + Mi_next[j] + Mi_next[j+1];
				outfocus_depth += Square(temp);
			}
		}
	}
	else if(strcmp(type, "Kirsch") == 0){
		for (int i = 1; i < rows-1; ++i){
			Mi_prev = image.ptr<uchar>(i-1);
			Mi_current = image.ptr<uchar>(i);
			Mi_next = image.ptr<uchar>(i+1);
			for(int j = 1; j < cols-1; ++j){
				temp = 3*Mi_prev[j-1] + 3*Mi_prev[j] + 3*Mi_prev[j+1] +
				        3*Mi_current[j-1] + 3*Mi_current[j+1] -
				        5*Mi_next[j-1] - 5*Mi_next[j] - 5*Mi_next[j+1];
				outfocus_depth += Square(temp);

				temp = 3*Mi_prev[j-1] + 3*Mi_prev[j] + 3*Mi_prev[j+1] -
				        5*Mi_current[j-1] -  3*Mi_current[j+1] -
				        5*Mi_next[j-1] - 5*Mi_next[j] + 3*Mi_next[j+1];
				outfocus_depth += Square(temp);

				temp = -5*Mi_prev[j-1] + 3*Mi_prev[j] + 3*Mi_prev[j+1] -
				        5*Mi_current[j-1]  + 3*Mi_current[j+1] -
				        5*Mi_next[j-1] + 3*Mi_next[j] + 3*Mi_next[j+1];
				outfocus_depth += Square(temp);
			}
		}
	}

	return outfocus_depth;
}
