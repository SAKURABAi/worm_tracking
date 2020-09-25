/****************************************************************************
    ThreadPackage 包含TrackingSystem中线程
****************************************************************************/
#ifndef _THREAD_PACKAGE_H_
#define _THREAD_PACKAGE_H_

#include "Worm_CV/stdafx.h"
#include "WormCentroid.h"
#include "GlobalParameters.h"
#include <QtCore/QThread>
#include <opencv2/core/core.hpp>

///////////////////////////////  DisplayThread  //////////////////////////////////
class DisplayThread : public QThread
{
    Q_OBJECT
public:
    DisplayThread();

signals:
    void DisplaySignal();

public slots:
    inline bool GetReadyDisplay(){ return readyDisplay; }
    inline void SetReadyDisplay(bool ok){ readyDisplay = ok; }
    inline bool GetStopDisplay() { return stopDisplay; }
    inline void SetStopDisplay(bool ok){ stopDisplay = ok; }
    inline bool GetQuitDisplay(){ return quitDisplay; }
    inline void SetQuitDisplay(bool ok){ quitDisplay = ok; }
    
protected:
    void run();
    
private:
	long image_num;
    volatile bool stopDisplay;
    volatile bool readyDisplay;
    volatile bool quitDisplay;
};

///////////////////////////////  AnalyseThread  //////////////////////////////////
class AnalyseThread : public QThread
{
    Q_OBJECT
public:
    AnalyseThread();
    //void Get_Binary_Threshold(Mat & worm_image);
    //bool Worm_Image_Denoise(Mat & binary_image);
    
signals:
    void AnalyseSignal();
       
public slots:
    inline bool GetReadyAnalyse(){ return readyAnalyse; }
    inline void SetReadyAnalyse(bool ok){ readyAnalyse = ok; }
    inline bool GetStopAnalyse() { return stopAnalyse; }
    inline void SetStopAnalyse(bool ok){ stopAnalyse = ok; }
    inline bool GetQuitAnalyse(){ return quitAnalyse; }
    inline void SetQuitAnalyse(bool ok){ quitAnalyse = ok; }
    
protected:
    void run();
    
private:
	//double Worm_Area;
	//int Binary_Threshold;
	long image_num;
    volatile bool stopAnalyse;
    volatile bool readyAnalyse;
    volatile bool quitAnalyse;
};

///////////////////////////////  CentroidTrackThread  //////////////////////////////////
class CentroidTrackThread : public QThread
{
	Q_OBJECT
public:
	CentroidTrackThread();
	void CentroidTrack();
	void Initialize(){ 
		readyCentroidTrack = false;
		stopCentroidTrack = true;
		quitCentroidTrack = false;
		tracking_loops = 0; 
	}

public slots:
	inline bool GetReadyCentroidTrack() const { return readyCentroidTrack; }
	inline void SetReadyCentroidTrack(const bool ok){ readyCentroidTrack = ok; }
	inline bool GetStopCentroidTrack() const { return stopCentroidTrack; }
	inline void SetStopCentroidTrack(const bool ok){ stopCentroidTrack = ok; }
	inline bool GetQuitCentroidTrack() const { return quitCentroidTrack; }
	inline void SetQuitCentroidTrack(const bool ok){ quitCentroidTrack = ok; }

protected:
	virtual void run();

private:
	volatile bool stopCentroidTrack;
	volatile bool readyCentroidTrack;
	volatile bool quitCentroidTrack;

	long tracking_loops;
	long image_num;
	WormCentroid wormCentroid;
	cv::Mat src_image, binary_image;
};

///////////////////////////////  RoiTrackThread  //////////////////////////////////
class RoiTrackThread : public QThread
{
	Q_OBJECT
public:
	RoiTrackThread();
	void RoiTrack();
    void Initialize();

public slots:
	inline bool GetReadyRoiTrack() const { return readyRoiTrack; }
	inline void SetReadyRoiTrack(const bool ok){ readyRoiTrack = ok; }
	inline bool GetStopRoiTrack() const { return stopRoiTrack; }
	inline void SetStopRoiTrack(const bool ok){ stopRoiTrack = ok; }
	inline bool GetQuitRoiTrack() const { return quitRoiTrack; }
	inline void SetQuitRoiTrack(const bool ok){ quitRoiTrack = ok; }
	void UpdateWormAreaSlot();

protected:
	virtual void run();

private:
	volatile bool stopRoiTrack;
	volatile bool readyRoiTrack;
	volatile bool quitRoiTrack;
	
	long tracking_loops;
	long image_num;
	TrackingResult result;
	Backbone* backbone;
	Search_Backbone search_backbone;
	cv::Mat src_image;
};

///////////////////////////////  StaringImagingThread  //////////////////////////////////
class StaringImagingThread : public QThread
{
	Q_OBJECT
public:
	StaringImagingThread();
	void StaringImaging();
	void Initialize();

public slots:
	inline bool GetReadyStaringImaging() const { return readyStaringImaging; }
	inline void SetReadyStaringImaging(const bool ok){ readyStaringImaging = ok; }
	inline bool GetStopStaringImaging() const { return stopStaringImaging; }
	inline void SetStopStaringImaging(const bool ok){ stopStaringImaging = ok; }
	inline bool GetQuitStaringImaging() const { return quitStaringImaging; }
	inline void SetQuitStaringImaging(const bool ok){ quitStaringImaging = ok; }

protected:
	virtual void run();

private:
	volatile bool stopStaringImaging;
	volatile bool readyStaringImaging;
	volatile bool quitStaringImaging;

	bool isFirstImage;
	long image_num;
	int focus_region;
	Backbone* backbone;
	Search_Backbone search_backbone;
	cv::Mat src_image;
};

///////////////////////////////  CruisingThread  //////////////////////////////////
class CruisingThread : public QThread
{
	Q_OBJECT
public:
	CruisingThread();
	void Cruising();
	void Initialize();

public slots:
	inline bool GetReadyCruising() const { return readyCruising; }
	inline void SetReadyCruising(const bool ok){ readyCruising = ok; }
	inline bool GetStopCruising() const { return stopCruising; }
	inline void SetStopCruising(const bool ok){ stopCruising = ok; }
	inline bool GetQuitCruising() const { return quitCruising; }
	inline void SetQuitCruising(const bool ok){ quitCruising = ok; }

protected:
	virtual void run();

private:
	volatile bool stopCruising;
	volatile bool readyCruising;
	volatile bool quitCruising;

	bool isFirstImage;
	int direction;
	long image_num;
	long focus_region;
	Backbone* backbone;
	Search_Backbone search_backbone;
	cv::Mat src_image;
};

///////////////////////////////  RecordThread  //////////////////////////////////
class RecordThread : public QThread
{
	Q_OBJECT
public:
	RecordThread();
	void Record();

public slots:
	inline bool GetReadyRecord() const { return readyRecord; }
	inline void SetReadyRecord(const bool ok){ readyRecord = ok; }
	inline bool GetStopRecord() const { return stopRecord; }
	inline void SetStopRecord(const bool ok){ stopRecord = ok; }
	inline bool GetQuitRecord() const { return quitRecord; }
	inline void SetQuitRecord(const bool ok){ quitRecord = ok; }

protected:
	virtual void run();

private:
	cv::Mat src_image;
	volatile bool stopRecord;
	volatile bool readyRecord;
	volatile bool quitRecord;
};

#endif
