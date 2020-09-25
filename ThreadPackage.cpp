
#include "ThreadPackage.h"
#include "DevicePackage.h"
#include "VirtualCoordinates.h"
#include <sys/timeb.h>
#include <QtGui/QColor>
using namespace VIRTUAL_COORDINATE;

#define NANO 1000000
#define FOCUS_SWITCH_INTERVAL 24

#define MAX_X_OFFSET 150
#define MAX_Y_OFFSET 150
#define MAXLOOPS_START_CHECK_OFFSET 100

void Remove_Region_Outside_Worm(Mat & binary_image, double & worm_area);
void Remove_Region_Inside_Worm(Mat & binary_image, double & worm_area);

bool CheckOffset(double x, double y);
void ImageWrite(ImageStreamParams *imageParams);
void SaveTrackingResult(TrackingResult &result);

bool IsRectInCircle(double *rect_region);
void GetWormRegionRect(double backbone[][2], int num, double margin, double *rect_region, double *roi_range);
void UpdateRect(double backbone[][2], int num, double margin, double *rect_region, double *roi_range, int focus_region);
void SaveStaringImagingResult(StaringImagingResult &result);

///////////////////////////////  DisplayThread  //////////////////////////////////
DisplayThread::DisplayThread()
{
	image_num = 0;
    stopDisplay = true;
    readyDisplay = false;
    quitDisplay = false;
}

void DisplayThread::run()
{
    while (true){
        if (quitDisplay){
            break;
        }
        else if(!stopDisplay && readyDisplay){
        	if (globalDalsaImage.data == NULL){
        		cout<<"DisplayThread: Dalsa Image is NULL" <<endl;
                continue;
	        }
	        ++image_num;
	        
	        // Synchronize image and feature points during tracking
        	emit DisplaySignal();
	        readyDisplay = false;
        }
    }
}

///////////////////////////////  AnalyseThread  //////////////////////////////////
AnalyseThread::AnalyseThread()
{
	image_num = 0; 
	stopAnalyse = true;
    readyAnalyse = false;
    quitAnalyse = false;
    //Worm_Area = BW::INITIAL_WORM_AREA;
	//Binary_Threshold = BW::BINARY_THRESHOLD;
}

/*void AnalyseThread::Get_Binary_Threshold(Mat & worm_image)
{
	// Compute the histogram of grayscale image
	cv::Mat hist;
	int histSize = 256;
	float range[] = { 0, 256 };
	const float *histRange = { range };
	calcHist(& worm_image, 1, 0, Mat(), hist, 1, &histSize, &histRange);
	double hist_values[256];
	for (unsigned i = 0; i < 256; ++i){
		hist_values[i] = hist.at<float>(i, 0);
	}

	// Compute the adaptive threshold accroding to the worm area
	double hist_threshold = Worm_Area*BW::THRESHOLD_AREA_PROPORTION;
	for (int i = 255; i >= 0; --i){
		if (hist_values[i] >= hist_threshold){
			Binary_Threshold = i+2;
			OPTIMAL_BINARY_THRESHOLD = Binary_Threshold; //Update global viarable value
			break;
		}
	}
}

bool AnalyseThread::Worm_Image_Denoise(Mat & worm_image)
{
	worm_image = worm_image > Binary_Threshold;

	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	
	//Remove the dots outside the worm
	findContours(worm_image, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
	if (contours.size() <= 0){
		Worm_Area = BW::INITIAL_WORM_AREA;
		//throw QException(OBJECT_NAME, "GetWormCentroid()", "Fail to extract worm region");
		//cout<<"Worm_Image_Denoise(): Fail to extract worm region"<<endl;
		return false;
	}

	Select_Minimum select_contours_by_area(WORM::INF, -1);
	double contour_area_temp;
	for (int i = 0; i >= 0; i = hierarchy[i][0]){
		contour_area_temp = contourArea(contours[i]);

		int j = hierarchy[i][2];
		while (j >= 0) {
			contour_area_temp -= contourArea(contours[j]);
			j = hierarchy[j][0];
		}
		select_contours_by_area.Renew(abs(Worm_Area - contour_area_temp), i);
	}
	int contour_select = select_contours_by_area.Get_Min_Index();
	Worm_Area = contourArea(contours[contour_select]);

	if (contour_select == -1){
		cout<< "Worm_Image_Denoise(): Can't get connected components of the worm"<<endl;
		return false;
	}
	for (int i = 0; i >= 0; i = hierarchy[i][0])
		drawContours(worm_image, contours, i, 255 * (i == contour_select), CV_FILLED, 8, hierarchy);

	//Remove the dots inside the worm
	contours.clear();
	hierarchy.clear();
	worm_image = ~worm_image;
	findContours(worm_image, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
	for (int i = 0; i >= 0; i = hierarchy[i][0]) {
		double hole_area = contourArea(contours[i]);
		bool retain_hole = hole_area > Worm_Area * BW::MINIMUM_HOLE_PROPORTION;
		if (!retain_hole)
			Worm_Area += hole_area;
		drawContours(worm_image, contours, i, 255 * retain_hole, CV_FILLED, 8, hierarchy);
	}
	worm_image = ~worm_image;
	BW::INITIAL_WORM_AREA = Worm_Area; //Update the worm area in namespace

	return true;
}*/

void AnalyseThread::run()
{
	double worm_area = 0;
    while (true){
        if (quitAnalyse){
            break;
        }
        else if(!stopAnalyse && readyAnalyse){
        	if (globalImageSize.imageWidth != WORM::IMAGE_SIZE || globalImageSize.imageHeight != WORM::IMAGE_SIZE){
            	readyAnalyse = false;
            	continue;
            }
            if (globalDalsaImage.data == NULL){ 
                //cout<<"AnalyseThread: Dalsa Image is NULL"<<endl;
                continue;
            }

            read_write_locker.lockForRead();
            int rows = globalDalsaImage.rows;
            int cols = globalDalsaImage.cols;
            Mat src = cv::Mat(rows,cols,CV_8UC1,(void*)globalDalsaImage.data);
            src.copyTo(globalAnalyseImage);
            globalAnalyseImage = globalAnalyseImage > OPTIMAL_BINARY_THRESHOLD;
            read_write_locker.unlock();
            ++image_num;

            try{
            	if (image_num % 3 == 0){
	            	//Get_Binary_Threshold(globalAnalyseImage);
		            //Worm_Image_Denoise(globalAnalyseImage);
		            Remove_Region_Outside_Worm(globalAnalyseImage, worm_area);
			        Remove_Region_Inside_Worm(globalAnalyseImage, worm_area);
			        imshow("Analyse", globalAnalyseImage);
			    }
		        //emit AnalyseSignal();
	        } catch(QException e){
	        	cout<<"AnalyseThread: "<<e.getMessage()<<endl;
	        }
	        readyAnalyse = false;
        }
    }
}

///////////////////////////////  CentroidTrackThread  //////////////////////////////////
CentroidTrackThread::CentroidTrackThread()
{
	stopCentroidTrack = true;
	readyCentroidTrack = false;
	quitCentroidTrack = false;
	image_num = 0;
	tracking_loops = 0;
}

void CentroidTrackThread::run()
{
	while(true){
    	if (quitCentroidTrack){
        	break;
    	}
        else if(!stopCentroidTrack && readyCentroidTrack){
			CentroidTrack();
			readyCentroidTrack = false;
		}
	}
}

void CentroidTrackThread::CentroidTrack()
{
	if (globalDalsaImage.data == NULL){ 
    	cout<<"CentroidTrack: Dalsa Image Is Null"<<endl;
    	return; 
	}

    ++image_num;
    read_write_locker.lockForRead();
    int rows = globalDalsaImage.rows;
    int cols = globalDalsaImage.cols;
    cv::Mat(rows,cols,CV_8UC1,(void*)globalDalsaImage.data).copyTo(src_image);
    read_write_locker.unlock();

    binary_image = src_image > OPTIMAL_BINARY_THRESHOLD;

    //Test the time consumption of Centroid Computing
    //struct timespec ts,te;
    //clock_gettime(CLOCK_REALTIME, &ts);
    double centroid[2], x = WORM::IMAGE_SIZE/2, y = WORM::IMAGE_SIZE/2;
    try{
    	wormCentroid.GetWormCentroid(binary_image, centroid);
    	
    	// drive stage to track worm
    	/*clock_gettime(CLOCK_REALTIME, &te);
    	double time_cost = 1000*(te.tv_sec - ts.tv_sec) + (double)(te.tv_nsec-ts.tv_nsec)/NANO;
    	cout<<"Centroid Computing Time Comsumption: "<<time_cost<<endl;*/
    		
    	x = centroid[0], y = centroid[1];
    	if (x>500){x = 500;} else if (x<10){x = 10;}
        if (y>500){y = 500;} else if (y<10){y = 10;}
    	cout << "centroid x offset:" << x-WORM::IMAGE_SIZE/2 << " , centroid y offset:" << y-WORM::IMAGE_SIZE/2<< endl;
    	
    	double x_stage_move = 0, y_stage_move = 0;
    	bool bCheckOffset = CheckOffset(x,y);

    	//Compute the stage motion steps
        if (tracking_loops < MAXLOOPS_START_CHECK_OFFSET || 
        	(tracking_loops >= MAXLOOPS_START_CHECK_OFFSET && bCheckOffset)){
        	globalCentroid[0] = x;
        	globalCentroid[1] = y;
      
        	int x_image_offset = x-WORM::IMAGE_SIZE/2;
    		int y_image_offset = y-WORM::IMAGE_SIZE/2;
    		x_stage_move = STAGE_XY_TO_IMAGE_MATRIX[0][0]*x_image_offset + STAGE_XY_TO_IMAGE_MATRIX[0][1]*y_image_offset;
    		y_stage_move = STAGE_XY_TO_IMAGE_MATRIX[1][0]*x_image_offset + STAGE_XY_TO_IMAGE_MATRIX[1][1]*y_image_offset;
        }
        else if (tracking_loops >= MAXLOOPS_START_CHECK_OFFSET && !bCheckOffset){ //Don't move
        	x_stage_move = 0;
        	y_stage_move = 0;
        }
        TrackingMode = CENTROID_TRACKING;
        
        // Nove stage
        if (controller1!=NULL && (long(x_stage_move)!=0 || long(y_stage_move)!=0)){
        	char strCommand[32];
        	if (ADAPTIVE_PID){
        		sprintf(strCommand, "xSTEP=%ld;ySTEP=%ld\r",long(-x_stage_move),long(-y_stage_move));
        	}
        	else{
        		sprintf(strCommand,"IPX=%ld;IPY=%ld\r",long(-x_stage_move),long(-y_stage_move));
        	}
            controller1->write(string(strCommand));
            ++tracking_loops;
    	}
	
    } catch (QException e){
    	cout<<"Pic: "<<image_num<<", "<<e.getMessage()<<endl;
    	TrackingMode = NO_TRACKING;
    	//return;
    }
	
	//save current image and tracking point
	ImageStreamParams imageParams;
	char imagename[256], trackingResultFilename[256];;
	struct timeb currentTimeb;
	ftime(&currentTimeb);
	long long timestamp = ((long long)currentTimeb.time)*1000 + currentTimeb.millitm; //get current timestamp (millisecs)

	sprintf(imagename, "tracking_video/Image_%lld.bin", timestamp);
	imageParams.filename = imagename;
	imageParams.format = 1;
	imageParams.tracking_point[0] = globalCentroid[0];
	imageParams.tracking_point[1] = globalCentroid[1];
	imageParams.data = &src_image;

	#ifdef SAVE_ALL_TRACKING_DATA
		ImageWrite(&imageParams);
	#endif
	
    //Save tracking results
    TrackingResult result;
	sprintf(trackingResultFilename, "video/CentroidTracking/Image_%lld.res", timestamp);
	result.filename = trackingResultFilename;
	result.length_error = 0;
	result.imageOffset[0] = x-WORM::IMAGE_SIZE/2;
	result.imageOffset[1] = y-WORM::IMAGE_SIZE/2;
	result.roiPosition = ROI_TRACKING_POSITON;
	
	//get stage positions
	try{
		memset(result.stagePosition, 0, sizeof(result.stagePosition));
		if (controller1 != NULL){
	    	string pos = controller1->command("MG _TPX,_TPY\r");
	    	strncpy(result.stagePosition, pos.c_str(), sizeof(result.stagePosition));
		}
	} catch (string e){
		cout<<"Save Stage Position: "<<e<<endl;
	}
	
	//rewrite the image into file
	if (IS_RECORDING){
		sprintf(imagename, "video/CentroidTracking/Image_%lld.bin", timestamp);
		imageParams.filename = imagename;
		ImageWrite(&imageParams);
		SaveTrackingResult(result);
	}
}

///////////////////////////////  RoiTrackThread  //////////////////////////////////
RoiTrackThread::RoiTrackThread()
{
	stopRoiTrack = true;
	readyRoiTrack = false;
	quitRoiTrack = false;
	backbone = NULL;
	image_num = 0;
	tracking_loops = 0;
}

void RoiTrackThread::Initialize()
{
	stopRoiTrack = true;
	readyRoiTrack = false;
	quitRoiTrack = false;

	search_backbone.initialize();
	image_num = 0;
	tracking_loops = 0;
}

void RoiTrackThread::UpdateWormAreaSlot()
{
	search_backbone.Set_Area(BW::INITIAL_WORM_AREA);
	image_num = 0;
	tracking_loops = 0;
	backbone = NULL;
}

void RoiTrackThread::run()
{
	while(true){
    	if (quitRoiTrack){
        	break;
    	}
		if(!stopRoiTrack && readyRoiTrack){
			RoiTrack();
			readyRoiTrack = false;
		}
	}
}

void RoiTrackThread::RoiTrack()
{
	if (globalDalsaImage.data == NULL){ return; }
	
	++image_num;
	read_write_locker.lockForRead();
	int rows = globalDalsaImage.rows;
    int cols = globalDalsaImage.cols;
	cv::Mat(rows,cols,CV_8UC1,(void*)globalDalsaImage.data).copyTo(src_image);
    read_write_locker.unlock();
    
    //Testing the time consumption of ROI Position Computing
    struct timespec ts,te;
    clock_gettime(CLOCK_REALTIME, &ts);
    double x = WORM::IMAGE_SIZE/2,y = WORM::IMAGE_SIZE/2;
    
	try{
		//cout<<"Start to cal centerline"<<endl;
		backbone = (Backbone*)search_backbone.Search(src_image, QString::number(image_num).toStdString());
		if (BACKBONE_REVERSE) { backbone->reverse(); }
		
	    //	clock_gettime(CLOCK_REALTIME, &te);
        //	double time_cost = 1000*(te.tv_sec - ts.tv_sec) + (double)(te.tv_nsec-ts.tv_nsec)/NANO;
        //	cout<<"ROI Computing Time Comsumption: "<<time_cost<<" ms"<<endl;

    	if (!search_backbone.lengthError()){
    		int RoiPosition = int(ROI_TRACKING_POSITON*(ROOT_SMOOTH::PARTITION_NUM + 1))-1;//减1因为从0开始计数
    		if (RoiPosition < 0 || RoiPosition>=(ROOT_SMOOTH::PARTITION_NUM+1)){
    	    	cout<<" Invalid ROI Position"<<endl;
    	    	TrackingMode = NO_TRACKING;
    	    	return;
    		}
    		x = backbone->cood[RoiPosition][1];
    		y = backbone->cood[RoiPosition][0];
    		
    		//Testing the time consumption of sending commands
    		//clock_gettime(CLOCK_REALTIME, &ts);
    		if (x>500){x = 500;} else if (x<10){x = 10;}
    	    if (y>500){y = 500;} else if (y<10){y = 10;}
    	    cout << "roi x offset:" << x-WORM::IMAGE_SIZE/2 << " , roi y offset:" << y-WORM::IMAGE_SIZE/2<<endl<<endl;

    	    double x_stage_move = 0, y_stage_move = 0;
    	    bool bCheckOffset = CheckOffset(x,y);
    	    //Compute the stage motion steps
    	    if (tracking_loops < MAXLOOPS_START_CHECK_OFFSET || 
    	    	(tracking_loops >= MAXLOOPS_START_CHECK_OFFSET && bCheckOffset)){
    	    	globalRoi[0] = x;
    	    	globalRoi[1] = y;

    	    	double x_image_offset = x-WORM::IMAGE_SIZE/2;
    	    	double y_image_offset = y-WORM::IMAGE_SIZE/2;
    			x_stage_move = STAGE_XY_TO_IMAGE_MATRIX[0][0]*x_image_offset + STAGE_XY_TO_IMAGE_MATRIX[0][1]*y_image_offset;
    			y_stage_move = STAGE_XY_TO_IMAGE_MATRIX[1][0]*x_image_offset + STAGE_XY_TO_IMAGE_MATRIX[1][1]*y_image_offset;
    	    }
    	    else if (tracking_loops >= MAXLOOPS_START_CHECK_OFFSET && !bCheckOffset){ //Don't move
    	    	x_stage_move = 0;
    	    	y_stage_move = 0;
    	    }
    	    TrackingMode = ROI_TRACKING;
    	    
    		//Drive stage to compensate the position error
    		if (controller1!=NULL && (long(x_stage_move)!=0 || long(y_stage_move)!=0)){
    	    	char strCommand[32];
    	    	if (ADAPTIVE_PID){
    	    		sprintf(strCommand, "xSTEP=%ld;ySTEP=%ld\r",long(-x_stage_move),long(-y_stage_move));
    	    	}
    	    	else{
    	    		sprintf(strCommand,"IPX=%ld;IPY=%ld\r",long(-x_stage_move),long(-y_stage_move));
    	    	}
    	    	try{
    		        controller1->write(string(strCommand));
    		    } catch (string e){
    		    	cout<<"ROITracking: "<<e<<endl;
    		    }
    	        ++tracking_loops;
    		}
    	}
	
	} catch (Simple_Exception *e){
		cout << e->Get_message() << endl;
		TrackingMode = NO_TRACKING;
		// return;
	}
	
	//Testing the time consumption of saving images
	//clock_gettime(CLOCK_REALTIME, &ts);

	struct timeb currentTimeb;
	ftime(&currentTimeb);
	long long timestamp = ((long long)currentTimeb.time)*1000 + currentTimeb.millitm; //get current timestamp (millisecs)

	//save current image and tracking point
	ImageStreamParams imageParams;
	char imagename[256], trackingResultFilename[256];
	
	sprintf(imagename, "tracking_video/Image_%lld.bin", timestamp);
	imageParams.filename = imagename;
	imageParams.format = 1;
	imageParams.tracking_point[0] = globalRoi[0];
	imageParams.tracking_point[1] = globalRoi[1];
	imageParams.data = &src_image;

	#ifdef SAVE_ALL_TRACKING_DATA
		ImageWrite(&imageParams);
	#endif

	/*clock_gettime(CLOCK_REALTIME, &te);
	time_cost = 1000*(te.tv_sec - ts.tv_sec) + (double)(te.tv_nsec-ts.tv_nsec)/NANO;
	cout<<"Saving images Comsumption: "<<time_cost<<" ms"<<endl;*/

	// Save Tracking results
	TrackingResult result;
	sprintf(trackingResultFilename, "video/ROITracking/Image_%lld.res", timestamp);
	//sprintf(trackingResultFilename, "video/ROITracking/Image_%ld.res", image_num);
	result.filename = trackingResultFilename;
	result.length_error = search_backbone.lengthError();
	result.imageOffset[0] = x-WORM::IMAGE_SIZE/2;
	result.imageOffset[1] = y-WORM::IMAGE_SIZE/2;
	result.roiPosition = ROI_TRACKING_POSITON;
	// save centerline
	for (int i=0; i<ROOT_SMOOTH::PARTITION_NUM+1; ++i){
		result.centerline[i][0] = backbone->cood[i][0];
		result.centerline[i][1] = backbone->cood[i][1];
	}
	// Get stage positions
	try{
		memset(result.stagePosition, 0, sizeof(result.stagePosition));
		if (controller1 != NULL){
	    	string pos = controller1->command("MG _TPX,_TPY\r");
	    	strncpy(result.stagePosition, pos.c_str(), sizeof(result.stagePosition));
		}
	} catch (string e){
		cout<<"Save Stage Position: "<<e<<endl;
	}

	//rewrite the image into file
	if (IS_RECORDING){
		sprintf(imagename,"video/ROITracking/Image_%lld.bin", timestamp);
		imageParams.filename = imagename;
		ImageWrite(&imageParams);
		SaveTrackingResult(result);
	}

	clock_gettime(CLOCK_REALTIME, &te);
	double time_cost = 1000*(te.tv_sec - ts.tv_sec) + (double)(te.tv_nsec-ts.tv_nsec)/NANO;
	cout<<"ROI Tracking Time Comsumption: "<<time_cost<<" ms"<<endl;
}

///////////////////////////////  StaringImagingThread  //////////////////////////////////
StaringImagingThread::StaringImagingThread()
{
	stopStaringImaging = true;
	readyStaringImaging = false;
	quitStaringImaging = false;
	isFirstImage = true;
	backbone = NULL;
	image_num = 0;
	focus_region = 0;
}

void StaringImagingThread::Initialize()
{
	stopStaringImaging = true;
	readyStaringImaging = false;
	quitStaringImaging = false;
	isFirstImage = true;
	backbone = NULL;
	image_num = 0;
	focus_region = 0;
	search_backbone.initialize();
}

void StaringImagingThread::run()
{
	while(true){
    	if (quitStaringImaging){
        	break;
    	}
		if(!stopStaringImaging && readyStaringImaging){
			StaringImaging();
			readyStaringImaging = false;
		}
	}
}

void StaringImagingThread::StaringImaging()
{
	if (globalDalsaImage.data == NULL){ return; }
	
	++image_num;
	read_write_locker.lockForRead();
	int rows = globalDalsaImage.rows;
    int cols = globalDalsaImage.cols;
	cv::Mat(rows,cols,CV_8UC1,(void*)globalDalsaImage.data).copyTo(src_image);
    read_write_locker.unlock();
    
	try{
		backbone = (Backbone*)search_backbone.Search(src_image, QString::number(image_num).toStdString());
		if (BACKBONE_REVERSE) { backbone->reverse(); }
	} catch (Simple_Exception *e){
		cout << e->Get_message() << endl;
		TrackingMode = NO_TRACKING;
		return;
	}

	double worm_width = search_backbone.Get_WormWidth();
	double rect_region[4], roi_range[2], margin = worm_width;
	if (focus_region == 0){
		roi_range[0] = STARING_IMAGING::HEAD_RANGE[0];
		roi_range[1] = STARING_IMAGING::HEAD_RANGE[1];
	}
	else if (focus_region == 1){
		roi_range[0] = STARING_IMAGING::TAIL_RANGE[0];
		roi_range[1] = STARING_IMAGING::TAIL_RANGE[1];
	}

	double x_image_offset = 0, y_image_offset = 0;
	if (!search_backbone.lengthError()){
	    TrackingMode = STARING_IMAGING_TRACKING;
	    GetWormRegionRect(backbone->cood, backbone->length, margin, rect_region, roi_range);
	    globalOriginalOverallRect[0] = rect_region[0];
	    globalOriginalOverallRect[1] = rect_region[1];
	    globalOriginalOverallRect[2] = rect_region[2];
	    globalOriginalOverallRect[3] = rect_region[3];
	    cout<<"Original ROI Range: (" << roi_range[0]<<", "<<roi_range[1]<<")"<<endl;

	    UpdateRect(backbone->cood, backbone->length, margin, rect_region, roi_range, focus_region);
	    globalOverallRect[0] = rect_region[0];
	    globalOverallRect[1] = rect_region[1];
	    globalOverallRect[2] = rect_region[2];
	    globalOverallRect[3] = rect_region[3];
	    cout<<"Updated ROI Range: (" << roi_range[0]<<", "<<roi_range[1]<<")"<<endl<<endl;
	    
	    // Make the center of worm region centered in NCI
	    globalOverallCenter[0] = (rect_region[0] + rect_region[1])/2;
    	globalOverallCenter[1] = (rect_region[2] + rect_region[3])/2;
    	x_image_offset = globalOverallCenter[0] - WORM::IMAGE_SIZE/2;
    	y_image_offset = globalOverallCenter[1] - WORM::IMAGE_SIZE/2;

	   /* if (isFirstImage){
	    	globalOverallCenter[0] = (rect_region[0] + rect_region[1])/2;
	    	globalOverallCenter[1] = (rect_region[2] + rect_region[3])/2;
	    	x_image_offset = globalOverallCenter[0] - WORM::IMAGE_SIZE/2;
	    	y_image_offset = globalOverallCenter[1] - WORM::IMAGE_SIZE/2;
	    	isFirstImage = false;
	    }
	    else {
	    	if (IsRectInCircle(rect_region)){
	    		x_image_offset = 0;
	    		y_image_offset = 0;
	    	}
	    	else{
	    		globalOverallCenter[0] = (rect_region[0] + rect_region[1])/2;
		    	globalOverallCenter[1] = (rect_region[2] + rect_region[3])/2;
		    	x_image_offset = globalOverallCenter[0] - WORM::IMAGE_SIZE/2;
		    	y_image_offset = globalOverallCenter[1] - WORM::IMAGE_SIZE/2;
	    	}
	    }*/
		double x_stage_move = STAGE_XY_TO_IMAGE_MATRIX[0][0]*x_image_offset + STAGE_XY_TO_IMAGE_MATRIX[0][1]*y_image_offset;
		double y_stage_move = STAGE_XY_TO_IMAGE_MATRIX[1][0]*x_image_offset + STAGE_XY_TO_IMAGE_MATRIX[1][1]*y_image_offset;

		//Drive stage to compensate the position error
		if (controller1!=NULL && (long(x_stage_move)!=0 || long(y_stage_move)!=0)){
	    	char strCommand[32];
	    	if (ADAPTIVE_PID){
	    		sprintf(strCommand, "xSTEP=%ld;ySTEP=%ld\r",long(-x_stage_move),long(-y_stage_move));
	    	}
	    	else{
	    		sprintf(strCommand,"IPX=%ld;IPY=%ld\r",long(-x_stage_move),long(-y_stage_move));
	    	}
	    	try{
		        controller1->write(string(strCommand));
		    } catch (string e){
		    	cout<<"StaringImaging: "<<e<<endl;
		    }
		}
	}
	
	struct timeb currentTimeb;
	ftime(&currentTimeb);
	long long timestamp = ((long long)currentTimeb.time)*1000 + currentTimeb.millitm; //get current timestamp (millisecs)

	//save current image and tracking point
	ImageStreamParams imageParams;
	char imagename[256];
	sprintf(imagename, "tracking_video/Image_%lld.bin", timestamp);

	imageParams.filename = imagename;
	imageParams.format = 1;
	imageParams.tracking_point[0] = globalOverallCenter[0];
	imageParams.tracking_point[1] = globalOverallCenter[1];
	imageParams.data = &src_image;
	#ifdef SAVE_ALL_TRACKING_DATA
		ImageWrite(&imageParams);
	#endif

	// Save Staring Imaging results
	StaringImagingResult result;
	char staringImagingResultFilename[256];
	sprintf(staringImagingResultFilename, "video/ROITracking/Image_%lld.res", timestamp);

	result.filename = staringImagingResultFilename;
	result.length_error = search_backbone.lengthError();
	result.rect_region[0] = rect_region[0];
	result.rect_region[1] = rect_region[1];
	result.rect_region[2] = rect_region[2];
	result.rect_region[3] = rect_region[3];
	result.margin = margin;
	result.circle[0] = STARING_IMAGING::CIRCLE_RADIUS;
	result.circle[1] = STARING_IMAGING::CIRCLE_CENTER[0];
	result.circle[2] = STARING_IMAGING::CIRCLE_CENTER[1];
	result.roi_range[0] = roi_range[0];
	result.roi_range[1] = roi_range[1];
	result.focus_region = focus_region;

	for (int i=0; i<ROOT_SMOOTH::PARTITION_NUM+1; ++i){
		result.centerline[i][0] = backbone->cood[i][0];
		result.centerline[i][1] = backbone->cood[i][1];
	}
	if (IS_RECORDING){
		sprintf(imagename,"video/ROITracking/Image_%lld.bin", timestamp);
		imageParams.filename = imagename;
		ImageWrite(&imageParams);
		SaveStaringImagingResult(result);
	}
}

///////////////////////////////  CruisingThread  //////////////////////////////////
CruisingThread::CruisingThread()
{
	stopCruising = true;
	readyCruising = false;
	quitCruising = false;
	isFirstImage = true;
	backbone = NULL;
	image_num = 0;
	focus_region = 0;
}

void CruisingThread::Initialize()
{
	stopCruising = true;
	readyCruising = false;
	quitCruising = false;
	isFirstImage = true;
	backbone = NULL;
	image_num = 0;
	direction = 0;
	focus_region = 0;
	search_backbone.initialize();
}

void CruisingThread::run()
{
	while(true){
    	if (quitCruising){
        	break;
    	}
		if(!stopCruising && readyCruising){
			Cruising();
			readyCruising = false;
		}
	}
}

void CruisingThread::Cruising()
{
	if (globalDalsaImage.data == NULL){ return; }
	
	struct timespec ts,te;
    clock_gettime(CLOCK_REALTIME, &ts);

	++image_num;
	read_write_locker.lockForRead();
	int rows = globalDalsaImage.rows;
    int cols = globalDalsaImage.cols;
	cv::Mat(rows,cols,CV_8UC1,(void*)globalDalsaImage.data).copyTo(src_image);
    read_write_locker.unlock();
    
	try{
		backbone = (Backbone*)search_backbone.Search(src_image, QString::number(image_num).toStdString());
		if (BACKBONE_REVERSE) { backbone->reverse(); }
	} catch (Simple_Exception *e){
		cout << e->Get_message() << endl;
		TrackingMode = NO_TRACKING;
		return;
	}

	// update focus region (head or tail)
	/*double worm_width = search_backbone.Get_WormWidth();
	double rect_region[4], roi_range[2], margin = worm_width;
	if (image_num % FOCUS_SWITCH_INTERVAL == 1){
		focus_region = ((focus_region+1) % 2);
	}
	if (focus_region == 0){
		roi_range[0] = STARING_IMAGING::HEAD_RANGE[0];
		roi_range[1] = STARING_IMAGING::HEAD_RANGE[1];
	}
	else if (focus_region == 1){
		roi_range[0] = STARING_IMAGING::TAIL_RANGE[0];
		roi_range[1] = STARING_IMAGING::TAIL_RANGE[1];
	}*/
	
	double worm_width = search_backbone.Get_WormWidth();
	double rect_region[4], roi_range[2], margin = worm_width;
	int Times = 20;
	int Repeat_Interval = 4;
	double Roi_Increment = 0.2;
	double Overlap_Region = 0.2;
	int t = image_num % Times;
    if (t == 0){ t = Times; }
    if (direction == 0){
    	roi_range[0] = (ceil(1.0*t/Repeat_Interval)-1)*Roi_Increment;
		roi_range[1] = ((roi_range[0]+Overlap_Region)<1.0 ? (roi_range[0]+Overlap_Region) : 1.0);
		focus_region = 0;
    }
    else if (direction == 1){
    	roi_range[1] = 1-(ceil(1.0*t/Repeat_Interval)-1)*Roi_Increment;
    	roi_range[0] = ((roi_range[1]-Overlap_Region)>0 ? (roi_range[1]-Overlap_Region) : 0);
    	focus_region = 1;
    }
    if (t == Times){ direction = 1 - direction; } // Moving the ROI region along centerline
  
	double x_image_offset = 0, y_image_offset = 0;
	if (!search_backbone.lengthError()){
	    TrackingMode = STARING_IMAGING_TRACKING;
	    GetWormRegionRect(backbone->cood, backbone->length, margin, rect_region, roi_range);
	    globalOriginalOverallRect[0] = rect_region[0];
	    globalOriginalOverallRect[1] = rect_region[1];
	    globalOriginalOverallRect[2] = rect_region[2];
	    globalOriginalOverallRect[3] = rect_region[3];
	    cout<<"Original ROI Range: (" << roi_range[0]<<", "<<roi_range[1]<<")"<<endl;

	    UpdateRect(backbone->cood, backbone->length, margin, rect_region, roi_range, focus_region);
		globalOverallRect[0] = rect_region[0];
	    globalOverallRect[1] = rect_region[1];
	    globalOverallRect[2] = rect_region[2];
	    globalOverallRect[3] = rect_region[3];
	    cout<<"Updated ROI Range: (" << roi_range[0]<<", "<<roi_range[1]<<")"<<endl;

	    if (isFirstImage){
	    	globalOverallCenter[0] = (rect_region[0] + rect_region[1])/2;
	    	globalOverallCenter[1] = (rect_region[2] + rect_region[3])/2;
	    	x_image_offset = globalOverallCenter[0] - WORM::IMAGE_SIZE/2;
	    	y_image_offset = globalOverallCenter[1] - WORM::IMAGE_SIZE/2;
	    	isFirstImage = false;
	    }
	    else {
	    	if (IsRectInCircle(rect_region)){
	    		x_image_offset = 0;
	    		y_image_offset = 0;
	    	}
	    	else{
	    		globalOverallCenter[0] = (rect_region[0] + rect_region[1])/2;
		    	globalOverallCenter[1] = (rect_region[2] + rect_region[3])/2;
		    	x_image_offset = globalOverallCenter[0] - WORM::IMAGE_SIZE/2;
		    	y_image_offset = globalOverallCenter[1] - WORM::IMAGE_SIZE/2;
	    	}
	    }
	    cout <<"X offset:" << x_image_offset << " , Y offset:" << y_image_offset<<endl;

		double x_stage_move = STAGE_XY_TO_IMAGE_MATRIX[0][0]*x_image_offset + STAGE_XY_TO_IMAGE_MATRIX[0][1]*y_image_offset;
		double y_stage_move = STAGE_XY_TO_IMAGE_MATRIX[1][0]*x_image_offset + STAGE_XY_TO_IMAGE_MATRIX[1][1]*y_image_offset;

		//Drive stage to compensate the position error
		if (controller1!=NULL && (long(x_stage_move)!=0 || long(y_stage_move)!=0)){
	    	char strCommand[32];
	    	if (ADAPTIVE_PID){
	    		sprintf(strCommand, "xSTEP=%ld;ySTEP=%ld\r",long(-x_stage_move),long(-y_stage_move));
	    	}
	    	else{
	    		sprintf(strCommand,"IPX=%ld;IPY=%ld\r",long(-x_stage_move),long(-y_stage_move));
	    	}
	    	try{
		        controller1->write(string(strCommand));
		    } catch (string e){
		    	cout<<"Cruising: "<<e<<endl;
		    }
		}
	}
	
	struct timeb currentTimeb;
	ftime(&currentTimeb);
	long long timestamp = ((long long)currentTimeb.time)*1000 + currentTimeb.millitm; //get current timestamp (millisecs)

	//save current image and tracking point
	ImageStreamParams imageParams;
	char imagename[256], staringImagingResultFilename[256];
	
	sprintf(imagename, "tracking_video/Image_%lld.bin", timestamp);
	imageParams.filename = imagename;
	imageParams.format = 1;
	imageParams.tracking_point[0] = globalOverallCenter[0];
	imageParams.tracking_point[1] = globalOverallCenter[1];
	imageParams.data = &src_image;
	#ifdef SAVE_ALL_TRACKING_DATA
		ImageWrite(&imageParams);
	#endif

	// Save Staring Imaging results
	StaringImagingResult result;
	sprintf(staringImagingResultFilename, "video/ROITracking/Image_%lld.res", timestamp);

	result.filename = staringImagingResultFilename;
	result.length_error = search_backbone.lengthError();
	result.rect_region[0] = rect_region[0];
	result.rect_region[1] = rect_region[1];
	result.rect_region[2] = rect_region[2];
	result.rect_region[3] = rect_region[3];
	result.margin = margin;
	result.circle[0] = STARING_IMAGING::CIRCLE_RADIUS;
	result.circle[1] = STARING_IMAGING::CIRCLE_CENTER[0];
	result.circle[2] = STARING_IMAGING::CIRCLE_CENTER[1];
	result.roi_range[0] = roi_range[0];
	result.roi_range[1] = roi_range[1];
	result.focus_region = focus_region;

	for (int i=0; i<ROOT_SMOOTH::PARTITION_NUM+1; ++i){
		result.centerline[i][0] = backbone->cood[i][0];
		result.centerline[i][1] = backbone->cood[i][1];
	}
	if (IS_RECORDING){
		sprintf(imagename,"video/ROITracking/Image_%lld.bin", timestamp);
		imageParams.filename = imagename;
		ImageWrite(&imageParams);
		SaveStaringImagingResult(result);
	}

	clock_gettime(CLOCK_REALTIME, &te);
	double time_cost = 1000*(te.tv_sec - ts.tv_sec) + (double)(te.tv_nsec-ts.tv_nsec)/NANO;
	cout<<"Cruising time comsumption: "<<time_cost<<" ms"<<endl<<endl;

}

///////////////////////////////  RecordThread  //////////////////////////////////
RecordThread::RecordThread()
{
	stopRecord = true;
	readyRecord = false;
	quitRecord = false;
}

void RecordThread::run()
{
	while(true){
    	if (quitRecord){
        	break;
    	}
		if(!stopRecord && readyRecord){
			if (!IS_TRACKING){
				Record();
				cout<<"Record image"<<endl;
			}
			readyRecord = false;
		}
	}
}

void RecordThread::Record()
{
	UINT64 counter;
	char imagename[256];
	ImageStreamParams imageParams;
	
	// get current timestamp (millisecs)
	struct timeb currentTimeb;
	ftime(&currentTimeb);
	long long timestamp = ((long long)currentTimeb.time)*1000 + currentTimeb.millitm;

	if (globalDalsaImage.data == NULL){ return; }
	counter = globalDalsaImage.count;
	sprintf(imagename, "video/Without_Tracking/Image_%lld.bin", timestamp);

	imageParams.filename = imagename;
	imageParams.format = 1;
	imageParams.tracking_point[0] = -1;
	imageParams.tracking_point[1] = -1;

	// writw image raw data into file
	read_write_locker.lockForRead();
	int rows = globalDalsaImage.rows;
    int cols = globalDalsaImage.cols;
	cv::Mat(rows,cols,CV_8UC1,(void*)globalDalsaImage.data).copyTo(src_image);
	read_write_locker.unlock();
	
	imageParams.data = &src_image;
	ImageWrite(&imageParams);
	
	// Read stage positions and save into file
	TrackingResult result;
	char trackingResultFilename[256];
	try{
		memset(result.stagePosition, 0, sizeof(result.stagePosition));
		if (controller1 != NULL){
	    	string pos = controller1->command("MG _TPX,_TPY\r");
	    	strncpy(result.stagePosition, pos.c_str(), sizeof(result.stagePosition));
		}
	} catch (string e){
		cout<<"Save Stage Position: "<<e<<endl;
	}
	sprintf(trackingResultFilename, "video/Without_Tracking/Image_%lld.res", timestamp);
	result.filename = trackingResultFilename;
	result.length_error = 0;
	result.imageOffset[0] = 0;
	result.imageOffset[1] = 0;
    SaveTrackingResult(result);
}

///////////////////////////////  sub funcitons  //////////////////////////////////
void ImageWrite(ImageStreamParams *imageParams)
{
	Mat* image = imageParams -> data;
	int i = 0, j = 0, rows = image -> rows, cols = image -> cols;
	double tracking_point[2];
	tracking_point[0] =  imageParams->tracking_point[0];
	tracking_point[1] =  imageParams->tracking_point[1];

	uchar *Mi;
	FILE* outputImage;
	if(imageParams -> format == 1){
		outputImage = fopen(imageParams->filename, "wb");//以二进制方式写文件
		if(outputImage == NULL){//如果文件打开失败，显示错误并退出
			printf("ImageWrite: cannot open the file!\n");
			fclose(outputImage);
			exit(-1);
		}
		//setvbuf(outputImage, buffer, _IOFBF, BUFFER_SIZE);//设置缓存
		fwrite(&rows, sizeof(int), 1, outputImage);//写文件的行数
		fwrite(&cols, sizeof(int), 1, outputImage);//写文件的列数
		fwrite(tracking_point,   sizeof(double), 1, outputImage);//写文件的行数
		fwrite(tracking_point+1, sizeof(double), 1, outputImage);//写文件的列数

		for(i = 0 ; i < rows; ++i){
			Mi = image -> ptr<uchar>(i);
			fwrite(Mi, sizeof(uchar), cols, outputImage);//写入以Mi为首地址的cols个数据
		}
		fflush(outputImage);//更新缓冲区，令缓冲区中的数据输出到磁盘
		fclose(outputImage);//关闭文件		
	}
	else if(imageParams -> format == 2){
		outputImage = fopen(imageParams->filename, "wt");//以文本方式读写文件
		if(outputImage == NULL){//如果文件打开失败，显示错误并退出
			printf("ImageWrite: cannot open the file!\n");
			fclose(outputImage);
			exit(-1);	
		}
		//setvbuf(outputImage, buffer, _IOFBF, BUFFER_SIZE);//设置缓存
		//写入图像数据的行数和列数
		fprintf(outputImage, "%d,%d\n",rows, cols);
		for(i = 0 ; i < rows; ++i){
			Mi = image -> ptr<uchar>(i);
			for(j = 0 ; j < cols; ++j){
				fprintf(outputImage, "%u,",Mi[j]);//写入以Mi为首地址的cols个数据
			}
			fprintf(outputImage, "\n");
		}
		fflush(outputImage);//更新缓冲区，令缓冲区中的数据输出到磁盘
		fclose(outputImage);//关闭文件		
	}
}

bool CheckOffset(double x, double y)
{
	if (abs(x-WORM::IMAGE_SIZE/2)>MAX_X_OFFSET || abs(y-WORM::IMAGE_SIZE/2)>MAX_Y_OFFSET){
		return false;
	}
	return true;
}

void SaveTrackingResult(TrackingResult &result)
{
	FILE* file = fopen(result.filename, "wb");
	fwrite(&result.length_error, sizeof(bool), 1, file);
	fwrite(result.imageOffset, sizeof(double), 2, file);
	fwrite(result.stagePosition, sizeof(char), sizeof(result.stagePosition), file);
	fwrite(result.centerline, sizeof(double), 2*(ROOT_SMOOTH::PARTITION_NUM+1), file);
	fwrite(&result.roiPosition, sizeof(double), 1, file);
	fclose(file);
}

void Remove_Region_Outside_Worm(Mat & binary_image, double & worm_area)
{
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	findContours(binary_image, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);//去除背景中的杂点
	if (contours.size() <= 0){
		throw QException("Fail to extract worm region");
	}

	worm_area = 0;
	for (int i = 0; i >= 0; i = hierarchy[i][0]){
		double area = contourArea(contours[i]);
		bool is_worm = abs(area - BW::INITIAL_WORM_AREA) < BW::INITIAL_WORM_AREA*BW::WORM_AREA_THRESHOLD;
		if (is_worm){
			if (worm_area < area) worm_area = area;
		}
		drawContours(binary_image, contours, i, 255*is_worm, CV_FILLED, 8, hierarchy);
	}
}

void Remove_Region_Inside_Worm(Mat & binary_image,double & worm_area)
{
	binary_image = ~binary_image;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	double tmp_area = worm_area;
	findContours(binary_image, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
	/*if (contours.size() <= 0){
		throw QException("Fail to extract worm region");
	}*/

	for (int i = 0; i >=0; i=hierarchy[i][0]){
		double area = contourArea(contours[i]);
		bool retain_hole = area < worm_area*BW::MINIMUM_HOLE_PROPORTION;
		if (retain_hole){
			tmp_area += area;
		}
		drawContours(binary_image, contours, i, 255*(!retain_hole), CV_FILLED, 8, hierarchy);
	}
	binary_image = ~binary_image;
	worm_area = tmp_area;
}

void GetWormRegionRect(double (*backbone)[2], int backbone_num, double margin, double *rect_region, double *roi_range)
{
	// backbone(:,1): y position, backbone(:,2): x position
	// initialize rectangle region
	rect_region[0] = 0;
	rect_region[1] = 0;
	rect_region[2] = 0;
	rect_region[3] = 0;

	int num1 = int(backbone_num * roi_range[0]);
	int num2 = int(backbone_num * roi_range[1]);
	if (num2 > backbone_num-1) { num2 = backbone_num-1; }

	for (int i = num1; i <= num2; ++i){
		if (i == num1){
			rect_region[0] = backbone[num1][1];
			rect_region[1] = backbone[num1][1];
			rect_region[2] = backbone[num1][0];
			rect_region[3] = backbone[num1][0];
		}
		if (rect_region[0] > backbone[i][1]){ rect_region[0] = backbone[i][1]; }
		if (rect_region[1] < backbone[i][1]){ rect_region[1] = backbone[i][1]; }
		if (rect_region[2] > backbone[i][0]){ rect_region[2] = backbone[i][0]; }
		if (rect_region[3] < backbone[i][0]){ rect_region[3] = backbone[i][0]; }
	}
	rect_region[0] -= margin;
	rect_region[1] += margin;
	rect_region[2] -= margin;
	rect_region[3] += margin;
}

void UpdateRect(double (*backbone)[2], int backbone_num, double margin, double *rect_region, double *roi_range, int focus_region)
{
	double rect_radius = sqrt((rect_region[1] - rect_region[0])*(rect_region[1] - rect_region[0]) + 
		                      (rect_region[3] - rect_region[2])*(rect_region[3] - rect_region[2]))/2;
	double p1 = roi_range[0];
	double p2 = roi_range[1];

	while (rect_radius > STARING_IMAGING::CIRCLE_RADIUS - STARING_IMAGING::CIRCLE_RADIUS_TOL){
		if (focus_region == 0){
			p2 = p2 - 0.05;
			if (abs(p2 - p1) < STARING_IMAGING::ROI_POSITION_THRESHOLD){
				p2 = p1 + STARING_IMAGING::ROI_POSITION_THRESHOLD;
				roi_range[0] = p1;
				roi_range[1] = p2;
				GetWormRegionRect(backbone, backbone_num, margin, rect_region, roi_range);
				break;
			}
		}
		else if(focus_region == 1){
			p1 = p1 + 0.05;
			if (abs(p2 - p1) < STARING_IMAGING::ROI_POSITION_THRESHOLD){
				p1 = p2 - STARING_IMAGING::ROI_POSITION_THRESHOLD;
				roi_range[0] = p1;
				roi_range[1] = p2;
				GetWormRegionRect(backbone, backbone_num, margin, rect_region, roi_range);
				break;
			}
		}
		roi_range[0] = p1;
		roi_range[1] = p2;
		GetWormRegionRect(backbone, backbone_num, margin, rect_region, roi_range);
		rect_radius = sqrt((rect_region[1] - rect_region[0])*(rect_region[1] - rect_region[0]) + 
		                   (rect_region[3] - rect_region[2])*(rect_region[3] - rect_region[2]))/2;
	}
}

bool IsRectInCircle(double *rect_region) 
{
	const double *center = STARING_IMAGING::CIRCLE_CENTER;
	double up_left_dist = (rect_region[0] - center[0])*(rect_region[0] - center[0]) +
	                      (rect_region[2] - center[1])*(rect_region[2] - center[1]);
	double up_right_dist = (rect_region[1] - center[0])*(rect_region[1] - center[0]) +
	                      (rect_region[2] - center[1])*(rect_region[2] - center[1]);
	double down_left_dist = (rect_region[0] - center[0])*(rect_region[0] - center[0]) +
	                      (rect_region[3] - center[1])*(rect_region[3] - center[1]);
	double down_right_dist = (rect_region[1] - center[0])*(rect_region[1] - center[0]) +
	                      (rect_region[3] - center[1])*(rect_region[3] - center[1]);
    double max_dist = max(up_left_dist, up_right_dist);
    max_dist = max(max_dist, down_left_dist);
    max_dist = max(max_dist, down_right_dist);

    return (max_dist < ((STARING_IMAGING::CIRCLE_RADIUS - STARING_IMAGING::CIRCLE_RADIUS_TOL)*
    	(STARING_IMAGING::CIRCLE_RADIUS - STARING_IMAGING::CIRCLE_RADIUS_TOL)));
}

void SaveStaringImagingResult(StaringImagingResult &result)
{
	FILE* file = fopen(result.filename, "wb");
	fwrite(&result.length_error, sizeof(bool), 1, file);
	fwrite(result.circle, sizeof(double), 3, file);
	fwrite(result.rect_region, sizeof(double), 4, file);
	fwrite(&result.margin, sizeof(double), 1, file);
	fwrite(result.roi_range, sizeof(double), 2, file);
	fwrite(result.centerline, sizeof(double), 2*(ROOT_SMOOTH::PARTITION_NUM+1), file);
	fwrite(&result.focus_region, sizeof(int), 1, file);
	fclose(file);
}
