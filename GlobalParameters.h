/****************************************************************************
    GlobalParameters 定义程序中常用的结构体以及全局变量
****************************************************************************/
#ifndef _GLOBAL_PARAMETERS_H_
#define _GLOBAL_PARAMETERS_H_

#include "Util.h"
#include <opencv2/core/core.hpp>
#include <QtCore/QReadWriteLock>

namespace DALSACEMERA{
	const UINT32 FULLIMAGE_WIDTH = 2432;   //full image width
	const UINT32 FULLIMAGE_HEIGHT = 1728;  //full image height
	const int MAX_EXPOSURETIME = 199925;//us
	const int MIN_EXPOSURETIME = 20;
	const int MAX_FRAMERATE = 35;
	const int MIN_FRAMERATE = 1;
	const int X_CENTER = 1460;//1451
	const int Y_CENTER = 932; // 934
	const int X_CENTER_OFFSET = X_CENTER - FULLIMAGE_WIDTH/2;
	const int Y_CENTER_OFFSET = Y_CENTER - FULLIMAGE_HEIGHT/2;
}

namespace STARING_IMAGING{
    const double CIRCLE_RADIUS = 50;
    const double CIRCLE_CENTER[2] = {256,256};
    const double CIRCLE_RADIUS_TOL = 5;
    const double ROI_POSITION_THRESHOLD = 0.1;
    const double HEAD_RANGE[2] = {0, 0.5};
    const double TAIL_RANGE[2] = {0.5, 1};
}

struct DalsaImage
{
    int buffer_index;
    UINT64 count;
    long counterStamp;
    UINT32 cols;
    UINT32 rows;
    uchar* data;
};

struct ImageSize
{
    int imageWidth;//图像大小
    int imageHeight;
	int roiWidth; //roi 图像大小
	int roiHeight;
    int displayWidth;//Qt中显示图像窗口大小
    int displayHeight;
    int x_position;//x,y的位置对应与Qt中显示的窗口
    int y_position;
};

struct GridInfo
{
    bool hasRowsAndCols;   //判断是否已经生成rows和cols
    bool hasFullFovWidthAndHeight;//是否已经生成网格信息（单网格的宽、高）
    bool hasBorders;       //是否已设置网格边界，该边界用于生成网格
    int gridRows;
    int gridCols;
    int fullFovWidth;   //全图像对应到显示网格区域的宽度
    int fullFovHeight;  //全图像对应到显示网格区域的高度
    int totalWidth;     //显示网格区域的总宽度
    int totalHeight;    //显示网格区域的总高度
    double rowPixelToGridDistance;//图像上行像素对应到显示网格区域的距离
    double colPixelToGridDistance;//图像上列像素对应到显示网格区域的距离
    double rowPulseToGridDistance;//平移台的竖向1个脉冲对应到网格区域的距离
    double colPulseToGridDistance;//平移台的横向1个脉冲对应到网格区域的距离
    long topBorder;
    long bottomBorder;
    long leftBorder;
    long rightBorder;
    int fullFovImageWidth;//全图像的宽(像素)
    int fullFovImageHeight;
    double fullFovImageWidth_StageDistance;//全图像对应到平移台的脉冲
    double fullFovImageHeight_StageDistance;    
};

struct ImageStreamParams
{
    char *filename;
    cv::Mat *data;
    int format;
    double tracking_point[2];
};

struct TrackingResult{
    char *filename;
    bool length_error;
    double imageOffset[2];
    char stagePosition[256];
    double centerline[101][2];
    double roiPosition;
};

struct StaringImagingResult{
    char *filename;
    bool length_error;
    double margin;
    double rect_region[4];
    double roi_range[2];
    double circle[3];
    double centerline[101][2];
    int focus_region;
};

enum TRACKING_MODE{ NO_TRACKING, CENTROID_TRACKING, ROI_TRACKING, STARING_IMAGING_TRACKING, CRUISING_TRACKING};

//global variables
extern TRACKING_MODE TrackingMode;
extern bool DRAW_REFLINE_FLAG;
extern bool ADAPTIVE_PID;
extern bool IS_RECORDING;
extern bool IS_TRACKING;
extern bool ADAPTIVE_BINARY_FLAG;

extern int OPTIMAL_BINARY_THRESHOLD;
extern double ROI_TRACKING_POSITON;
extern bool   BACKBONE_REVERSE;

extern QReadWriteLock read_write_locker;
extern DalsaImage globalDalsaImage;
extern ImageSize  globalImageSize;
extern cv::Mat    globalAnalyseImage;
extern GridInfo   globalGridInfo;
extern double     globalCentroid[2];
extern double     globalRoi[2];
extern double     globalOverallCenter[2];
extern double     globalOverallRect[4];
extern double     globalOriginalOverallRect[4];

extern int AnalyseWinodwStartX; //Analyse window position
extern int AnalyseWindowStartY;

#endif
