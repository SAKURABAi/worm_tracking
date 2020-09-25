/****************************************************************************
    TrackingControl
****************************************************************************/

#ifndef _TRACKING_CONTROL_H_
#define _TRACKING_CONTROL_H_

#include "DalsaCamera.h"
#include "GlobalParameters.h"
#include "ThreadPackage.h"
#include "TrackingParametersSettingDialog.h"
#include "TranslationStage.h"
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QComboBox>

class TrackingControl : public QObject
{
	Q_OBJECT
public:
	static string OBJECT_NAME;

	explicit TrackingControl(DalsaCamera* camera, QObject *parent=0);
	~TrackingControl();

	//Create Initial Control Group
	void Create_InitialControlGroup();
	
	inline QGroupBox* Get_InitialControlGroup() const { 
    	return initialControlGroup; 
	}
	inline void Set_InitialControlGroupTitle(const QString &title){
		initialControlGroup->setTitle(title);
	}

	//Create Realtime Control Group
	void Create_RealtimeControlGroup();
	
	inline QGroupBox* Get_RealtimeControlGroup() const {
    	return realtimeControlGroup; 
	}
	inline void Set_ReatimeControlGroupTitle(const QString &title){
		realtimeControlGroup->setTitle(title);
	}

	inline DisplayThread* GetDisplayThread() const{
		return displayThread;
	}
	inline AnalyseThread* GetAnalyseThread() const{
		return analyseThread;
	}
    
    void CloseParametersSettingDialog();

public slots:
    void Image_Recieved();//接受图像已获得信号，用于后续处理
    void Controller1_Disconnected();
    void Controller1_Connected();
    
protected:
    void FillTriggerModeBox();
	inline void SetExposureTimeGroupEnabled(bool ok){
		exposureTimeEdit->setEnabled(ok);
		addExposureTimeButton->setEnabled(ok);
		subtractExposureTimeButton->setEnabled(ok);
	}

    inline void SetFrameRateGroupEnabled(bool ok){
		frameRateEdit->setEnabled(ok);
    }
    
    inline void SetRealtimeGroupEnabled(bool ok){
        centroidTrackButton->setEnabled(ok);
        roiTrackButton->setEnabled(ok);
        recordButton->setEnabled(ok);
        trackingParametersButton->setEnabled(ok);
        saveButton->setEnabled(ok);
        dualImagingButton->setEnabled(ok);
        staringButton->setEnabled(ok);
        cruisingButton->setEnabled(ok);
    }
    
    inline void UpdateMaxExposureTime(){
    	MaxExposureTime = UINT64((1.0e6/(frameRate*1.0))-75);
    	QString exposureTimeRange = tr("Exposure Time: (")+QString::number(MinExposureTime)+tr(", ")+QString::number(MaxExposureTime)+tr(") us");
		exposureTimeEdit->setToolTip(exposureTimeRange);
    }

protected slots:
	/****** Initial Control Group Slots ******/
	void OnCameraConnectButton();
    void OnDisplayButton();
    void OnAnalyseButton();

    void InitializeExposureTime();//初始化exposure time(从相机读取参数)
    void OnExposureTimeChanged();
	void OnAddExposureTimeButton();
	void OnSubtractExposureTimeButton();
	
	void InitializeFrameRate(); //初始化frame rate(从相机读取参数)
	void OnFrameRateChanged();
	void OnTriggerModeChanged();

	/****** Realtime Control Group Slots ******/
	void OnCentroidTrackButton();
    void OnRoiTrackButton();
    void OnRecordButton();
    void OnTrackingParametersButton();
    void OnSaveButton();
    void OnDualImagingButton();
    void OnCruisingButton();
    void OnStaringImagingButton();
    
private:
    // Atom adaptive PID control
    bool EndAdaptivePID();
    bool StartAdaptivePID();
    bool SetStageInitialPID();

    void EnterAdaptivePID();
    void ExitAdaptivePID();

    void ClearThread(QThread* );
    void Create_DisplayAndAnalyzeThreads();//创建线程
    void StartDisplay();
    void StopDisplay();
    void QuitDisplay();

    void StartAnalyse();
    void StopAnalyse();
    void QuitAnalyse();
    
    void StartCentroidTrack();
    void StopCentroidTrack();
    void QuitCentroidTrack();
    
    void StartRoiTrack();
    void StopRoiTrack();
    void QuitRoiTrack();
    
    void StartStaringImaging();
    void StopStaringImaging();
    void QuitStaringImaging();

    void StartCruising();
    void StopCruising();
    void QuitCruising();

    void StartRecord();
    void StopRecord();
    void QuitRecord();

	/****** Initial Control Group ******/
	QGroupBox *initialControlGroup;

	QPushButton *cameraConnectButton;
    QPushButton *displayButton;
    QPushButton *analyseButton;

	QLineEdit* exposureTimeEdit;
	QPushButton* addExposureTimeButton;
	QPushButton* subtractExposureTimeButton;
	QLineEdit* frameRateEdit;
	QComboBox* triggerModeBox;

	double scale;//增加/减少曝光时间的比例系数，初始化为10%
	UINT64 exposureTime;
	UINT64 MinExposureTime;
	UINT64 MaxExposureTime;
	UINT32 frameRate;
	UINT32 MinFrameRate;
	UINT32 MaxFrameRate;

	/****** Realtime Control Group ******/	
	QGroupBox* realtimeControlGroup;
	QPushButton *centroidTrackButton;
    QPushButton *roiTrackButton;
    QPushButton *recordButton;
    QPushButton *trackingParametersButton;
    QPushButton *saveButton;
    QPushButton *dualImagingButton;
    QPushButton *cruisingButton;
    QPushButton *staringButton;
    TrackingParametersSettingDialog *trackingParametersSettingDialog;
    
    DisplayThread* displayThread;
    AnalyseThread *analyseThread;
	CentroidTrackThread *centroidTrackThread;
	RoiTrackThread *roiTrackThread;
    StaringImagingThread *staringImagingThread;
    CruisingThread *cruisingThread;
	RecordThread *recordThread;
	
	DalsaCamera *dalsaCamera;
	DALSA_TRIGGERMODE triggerMode;
    TranslationStage* translationStage;
};

#endif
