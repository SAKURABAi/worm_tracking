
#include "WormTracking.h"
#include "VirtualCoordinates.h"
#include "DevicePackage.h"
#include "Worm_CV/CONST_PARA.h"

#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtCore/QString>

#include <iostream>
#include <string>

using namespace std;
using namespace VIRTUAL_COORDINATE;

int BW::BINARY_THRESHOLD = 40;
double BW::INITIAL_WORM_AREA = 3500;

//global variables
Galil *controller1 = NULL;         //控制器1
int OPTIMAL_BINARY_THRESHOLD = 40; //最优二值化阈值
double ROI_TRACKING_POSITON = 0.1; //ROI 追踪位置

TRACKING_MODE TrackingMode = NO_TRACKING;
bool IS_RECORDING = false;
bool IS_TRACKING = false;
bool DRAW_REFLINE_FLAG = false;   //是否在DisplayWindow中画辅助线
bool ADAPTIVE_PID = false;
bool ADAPTIVE_BINARY_FLAG = false;
bool BACKBONE_REVERSE = false;

QReadWriteLock read_write_locker;
DalsaImage globalDalsaImage;
cv::Mat    globalAnalyseImage;
ImageSize  globalImageSize;
GridInfo   globalGridInfo;
double     globalCentroid[2];
double     globalRoi[2];
double     globalOverallCenter[2];
double     globalOverallRect[4];
double     globalOriginalOverallRect[4];

//Analyse window position
int AnalyseWinodwStartX = 0;
int AnalyseWindowStartY = 0;

void InitializeGridInfo();
void InitializeDalsaImage();
void InitializeImageSize();
void InitializeOthers();

int main(int argc, char *argv[])
{
    //set local IP address
    system("sudo ~/UpdateIP");

    //initialize global variables
    InitializeGridInfo();
    InitializeDalsaImage();
    InitializeImageSize();
    InitializeOthers();
    
	QLocale::setDefault( QLocale( QLocale::English, QLocale::UnitedStates ) );
	QApplication a(argc, argv);
    //a.setStyle(new QPlastiqueStyle()); 

    QCoreApplication::setOrganizationName( "pku.edu.cn" );
    QCoreApplication::setOrganizationDomain( "pku.edu.cn" );
    QCoreApplication::setApplicationName( "QControlPlane" );
    
    qRegisterMetaType<string>("string");
    qRegisterMetaType<DeviceId>("DeviceId");
    qRegisterMetaType<QImage>("QImage");
    
    QDesktopWidget desktop;
    TrackingWindow window;
    window.show();
    window.move(desktop.width()-window.width()-150, 20);//初始化主界面的位置
    window.DockDisplayWindows();
    
    return a.exec();
}

void InitializeGridInfo()
{
    globalGridInfo.hasRowsAndCols = false;
    globalGridInfo.hasFullFovWidthAndHeight = false;
    globalGridInfo.hasBorders = false;
    globalGridInfo.gridRows = 0;
    globalGridInfo.gridCols = 0;
    globalGridInfo.fullFovWidth = 0;
    globalGridInfo.fullFovHeight = 0;
    globalGridInfo.totalWidth = 0;
    globalGridInfo.totalHeight = 0;
    globalGridInfo.rowPixelToGridDistance = 0;
    globalGridInfo.colPixelToGridDistance = 0;
    globalGridInfo.rowPulseToGridDistance = 0;
    globalGridInfo.colPulseToGridDistance = 0;
    globalGridInfo.topBorder = STAGE_INTIAL_VALUE;
    globalGridInfo.bottomBorder = STAGE_INTIAL_VALUE;
    globalGridInfo.leftBorder = STAGE_INTIAL_VALUE;
    globalGridInfo.rightBorder = STAGE_INTIAL_VALUE;
    globalGridInfo.fullFovImageWidth = DALSACEMERA::FULLIMAGE_WIDTH;
    globalGridInfo.fullFovImageHeight = DALSACEMERA::FULLIMAGE_HEIGHT;
    globalGridInfo.fullFovImageWidth_StageDistance = 1.0 * DALSACEMERA::FULLIMAGE_WIDTH * STAGE_XY_TO_IMAGE_FACTOR;
    globalGridInfo.fullFovImageHeight_StageDistance = 1.0 * DALSACEMERA::FULLIMAGE_HEIGHT * STAGE_XY_TO_IMAGE_FACTOR;
}

void InitializeDalsaImage()
{
    globalDalsaImage.buffer_index = 0;
    globalDalsaImage.count = 0;
    globalDalsaImage.rows = DALSACEMERA::FULLIMAGE_HEIGHT;
    globalDalsaImage.cols = DALSACEMERA::FULLIMAGE_WIDTH;
    globalDalsaImage.data = NULL;
}

void InitializeImageSize()
{
    globalImageSize.imageWidth = DALSACEMERA::FULLIMAGE_WIDTH;
    globalImageSize.imageHeight = DALSACEMERA::FULLIMAGE_HEIGHT;
    globalImageSize.displayWidth = 0;
    globalImageSize.displayHeight = 0;
    globalImageSize.x_position = -1;
    globalImageSize.y_position = -1;
}

void InitializeOthers()
{
    globalCentroid[0] = -1;
    globalCentroid[1] = -1;
    globalRoi[0] = -1;
    globalRoi[1] = -1;
    globalOverallCenter[0] = -1;
    globalOverallCenter[1] = -1;
    globalOverallRect[0] = -1;
    globalOverallRect[1] = -1;
    globalOverallRect[2] = -1;
    globalOverallRect[3] = -1;
}