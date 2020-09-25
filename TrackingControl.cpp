
#include "TrackingControl.h"
#include "DevicePackage.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMessageBox>

#include <QtCore/QDir>
#include "Worm_CV/stdafx.h"

void CLEAR_WORMCV_PROCESS_FRAME_NUM();

#define SLEEP_TIME 500000 //100ms
string TrackingControl::OBJECT_NAME = "TrackingControl";

TrackingControl::TrackingControl(DalsaCamera* camera, QObject *parent):QObject(parent)
{
    scale = 0.1;
    MaxFrameRate = DALSACEMERA::MAX_FRAMERATE;
    MinFrameRate = DALSACEMERA::MIN_FRAMERATE;
    MinExposureTime = DALSACEMERA::MIN_EXPOSURETIME;
	MaxExposureTime = DALSACEMERA::MAX_EXPOSURETIME;

    triggerMode = INTERNAL;
	dalsaCamera = camera;
	trackingParametersSettingDialog = NULL;
	translationStage = NULL;

    // Initialize threads
    displayThread = NULL;
    analyseThread = NULL;
    centroidTrackThread = NULL;
    roiTrackThread = NULL;
    recordThread = NULL;

    staringImagingThread = NULL;
    cruisingThread = NULL;

	Create_DisplayAndAnalyzeThreads();
	Create_InitialControlGroup();
	Create_RealtimeControlGroup();

	displayButton->setEnabled(false);
	analyseButton->setEnabled(false);
	SetExposureTimeGroupEnabled(false);
	SetFrameRateGroupEnabled(false);
	SetRealtimeGroupEnabled(false);

    //连接信号与槽，接受来自与camera所广播的信号
    QObject::connect(dalsaCamera, SIGNAL(Broadcast_Image_Recieved()), this, SLOT(Image_Recieved()));
}

TrackingControl::~TrackingControl()
{
    dalsaCamera = NULL;

    if (translationStage != NULL){
        delete translationStage;
        translationStage = NULL;
    }

    if (displayThread != NULL){
        QuitDisplay();
        ClearThread(displayThread);
    }
    if (analyseThread != NULL){
        QuitAnalyse();
        ClearThread(analyseThread);
    }

	if (centroidTrackThread != NULL){
    	QuitCentroidTrack();
    	ClearThread(centroidTrackThread);
	}
	if (roiTrackThread != NULL){
    	QuitRoiTrack();
    	ClearThread(roiTrackThread);
	}
	if (recordThread != NULL){
    	QuitRecord();
    	ClearThread(recordThread);
	}

    if (staringImagingThread != NULL){
        QuitStaringImaging();
        ClearThread(staringImagingThread);
    }
    if (cruisingThread != NULL){
        QuitCruising();
        ClearThread(cruisingThread);
    }
}

void TrackingControl::CloseParametersSettingDialog()
{
    if (trackingParametersSettingDialog != NULL){
        trackingParametersSettingDialog->close();
        QObject::disconnect(trackingParametersSettingDialog, SIGNAL(UpdateWormAreaSignal()), roiTrackThread, SLOT(UpdateWormAreaSlot()));
        delete trackingParametersSettingDialog;
        trackingParametersSettingDialog = NULL;
    }
}

void TrackingControl::Create_DisplayAndAnalyzeThreads()
{
    displayThread = new DisplayThread();
    displayThread->start();

    analyseThread = new AnalyseThread();
	analyseThread->start();


}

// Hint: The thread should have quited before clearing
void TrackingControl::ClearThread(QThread* thread)
{
    if (thread != NULL){
        delete thread;
        thread = NULL;
    }
}

void TrackingControl::Create_InitialControlGroup()
{
	initialControlGroup = new QGroupBox;
    
    cameraConnectButton = new QPushButton( tr("Connect") );
    displayButton = new QPushButton( tr("Display") );
    analyseButton = new QPushButton( tr("Analyse") );        
    displayButton->setEnabled(false);
	analyseButton->setEnabled(false);
	
	//exposure time settings
	QLabel* exposureTimeLabel = new QLabel("Exposure Time");
	QLabel* timeLabel = new QLabel("us");
	exposureTimeEdit = new QLineEdit;
	addExposureTimeButton = new QPushButton("+10%");
	subtractExposureTimeButton = new QPushButton("-10%");
	QString exposureTimeRange = tr("Exposure Time: (")+QString::number(MinExposureTime)+tr(", ")+QString::number(MaxExposureTime)+tr(") us");
	exposureTimeEdit->setToolTip(exposureTimeRange);
	
	//frame rate settings
	QLabel* frameRateLabel = new QLabel(tr("FrameRate"));
	QLabel* frameRateUnit = new QLabel(tr("Hz"));
	frameRateEdit = new QLineEdit;
	QString frameRateRange = tr("FrameRate: (")+QString::number(MinFrameRate)+tr(", ")+QString::number(MaxFrameRate)+tr(") Hz");
	frameRateEdit->setToolTip(frameRateRange);
	triggerModeBox = new QComboBox;
	triggerModeBox->setMinimumWidth(85);
	triggerModeBox->setMaximumWidth(95);
	
    //connect signals to slots
    QObject::connect( cameraConnectButton, SIGNAL( clicked() ), this, SLOT( OnCameraConnectButton() ) );
    QObject::connect( displayButton, SIGNAL( clicked() ), this, SLOT( OnDisplayButton() ) );
    QObject::connect( analyseButton, SIGNAL( clicked() ), this, SLOT( OnAnalyseButton() ) );
    
	QObject::connect(exposureTimeEdit, SIGNAL(returnPressed()), this, SLOT(OnExposureTimeChanged()));
	QObject::connect(addExposureTimeButton, SIGNAL(clicked()), this, SLOT(OnAddExposureTimeButton()));
	QObject::connect(subtractExposureTimeButton, SIGNAL(clicked()), this, SLOT(OnSubtractExposureTimeButton()));
	
	QObject::connect(frameRateEdit, SIGNAL(returnPressed()), this, SLOT(OnFrameRateChanged()));
	QObject::connect(triggerModeBox, SIGNAL(activated(int)), this, SLOT(OnTriggerModeChanged()));
		
	//set layout
    QHBoxLayout* controlLayout = new QHBoxLayout;
	controlLayout->addWidget(cameraConnectButton);
	controlLayout->addWidget(displayButton);
	controlLayout->addWidget(analyseButton);

	QHBoxLayout* exposureTimeLayout1 = new QHBoxLayout;
	exposureTimeLayout1->addWidget(exposureTimeLabel);
	exposureTimeLayout1->addWidget(exposureTimeEdit);
	exposureTimeLayout1->addWidget(timeLabel);
	exposureTimeLayout1->addWidget(addExposureTimeButton);
	exposureTimeLayout1->addWidget(subtractExposureTimeButton);
	
    QHBoxLayout *frameRateLayout = new QHBoxLayout;
    frameRateLayout->addWidget(frameRateLabel);
    frameRateLayout->addWidget(frameRateEdit);
    frameRateLayout->addWidget(frameRateUnit);
    frameRateLayout->addWidget(new QLabel("Trigger Mode") );
    frameRateLayout->addWidget(triggerModeBox);
    frameRateLayout->setMargin(0);
    //frameRateLayout->insertSpacing(2,5);

    QVBoxLayout *initialcontrolLayout = new QVBoxLayout;
	initialcontrolLayout->addLayout(controlLayout);
	initialcontrolLayout->addLayout(exposureTimeLayout1);
	initialcontrolLayout->addLayout(frameRateLayout);
	initialcontrolLayout->setContentsMargins(0,5,0,0);
	initialcontrolLayout->setSpacing(5);

    initialControlGroup->setLayout(initialcontrolLayout);
    
    FillTriggerModeBox();
	//初始化时相机还没有连接，此时设置控件的状态为不可访问
	SetExposureTimeGroupEnabled(false);
	SetFrameRateGroupEnabled(false);
	triggerModeBox->setEnabled(false);
}

//Create Realtime Control Group
void TrackingControl::Create_RealtimeControlGroup()
{
	realtimeControlGroup = new QGroupBox;
     
    centroidTrackButton = new QPushButton( tr("Centroid Track") );
    roiTrackButton = new QPushButton( tr("Centerline Track") );
    recordButton = new QPushButton( tr("Record") );
    trackingParametersButton = new QPushButton( tr("Tracking Parameters") );
    saveButton = new QPushButton( tr("Save") );
    dualImagingButton = new QPushButton( tr("Dual Imaging") );
    cruisingButton = new QPushButton( tr("Overall Track") );
    staringButton = new QPushButton( tr("ROI Track") );

    QObject::connect( centroidTrackButton, SIGNAL( clicked() ), this, SLOT( OnCentroidTrackButton() ) );
    QObject::connect( roiTrackButton, SIGNAL( clicked() ), this, SLOT( OnRoiTrackButton() ) );
    QObject::connect( recordButton, SIGNAL( clicked() ), this, SLOT( OnRecordButton() ) );
    QObject::connect( trackingParametersButton, SIGNAL( clicked() ), this, SLOT( OnTrackingParametersButton() ) );
    QObject::connect( saveButton, SIGNAL( clicked() ), this, SLOT( OnSaveButton() ) );
    QObject::connect( dualImagingButton, SIGNAL( clicked() ), this, SLOT( OnDualImagingButton() ) );
    QObject::connect( cruisingButton, SIGNAL( clicked() ), this, SLOT( OnCruisingButton() ) );
    QObject::connect( staringButton, SIGNAL( clicked() ), this, SLOT( OnStaringImagingButton() ) );
    
    QGridLayout *realTimeControl_Layout = new QGridLayout;
    realTimeControl_Layout->addWidget(centroidTrackButton, 0, 0);
    realTimeControl_Layout->addWidget(roiTrackButton, 0, 1);
    realTimeControl_Layout->addWidget(staringButton, 1, 0);
    realTimeControl_Layout->addWidget(cruisingButton, 1, 1);
    realTimeControl_Layout->addWidget(trackingParametersButton, 2, 0);
    realTimeControl_Layout->addWidget(dualImagingButton, 2, 1);
    realTimeControl_Layout->addWidget(recordButton, 3, 0);
    realTimeControl_Layout->addWidget(saveButton, 3, 1);
    realTimeControl_Layout->setContentsMargins(0,5,0,0);
    realTimeControl_Layout->setHorizontalSpacing(40);
    realTimeControl_Layout->setVerticalSpacing(10);
    
    realtimeControlGroup->setLayout(realTimeControl_Layout);
}

/****** Initial Control Group Slots ******/
void TrackingControl::OnCameraConnectButton()
{
    static bool IsWork = false;
	if (IsWork && (dalsaCamera!=NULL)){
		dalsaCamera->Disconnect();
		IsWork = false;
		cameraConnectButton->setText( tr("Connect") );
		
		//set groups state
		displayButton->setEnabled(false);
		analyseButton->setEnabled(false);
        triggerModeBox->setEnabled(false);

		SetExposureTimeGroupEnabled(false);
		SetFrameRateGroupEnabled(false);
		SetRealtimeGroupEnabled(false);
	}
	else {
		if ((dalsaCamera!=NULL) && (dalsaCamera->Connect())){
			IsWork = true;
			cameraConnectButton->setText( tr("Disconnect") );
			
			//Set groups state
			displayButton->setEnabled(true);
        	analyseButton->setEnabled(true);
            triggerModeBox->setEnabled(true);

        	//Get camera parameters and set related widgets
            if (dalsaCamera->GetTriggerMode(triggerMode)){
                if (triggerMode == INTERNAL)
                    triggerModeBox->setCurrentIndex(0);
                else if (triggerMode == EXTERNAL)
                    triggerModeBox->setCurrentIndex(1);
            }
            if (triggerMode == INTERNAL){
                SetExposureTimeGroupEnabled(true);
                SetFrameRateGroupEnabled(true);

                //Update current exposure time and frame rate
                InitializeExposureTime();
                InitializeFrameRate();
            }
        	SetRealtimeGroupEnabled(true);
		}
		else {
			QMessageBox::critical(NULL, "Error", "No dalsa camera connection");
		}
	}
}

void TrackingControl::OnDisplayButton()
{
    static bool IsWork = false;
    if (IsWork){
        IsWork = false;
        displayButton->setText( tr("Display") );
        StopDisplay();
    }
    else {
        IsWork = true;
        displayButton->setText( tr("Stop Display") );
        StartDisplay();
    }
}

void TrackingControl::OnAnalyseButton()
{
    static bool IsWork = false;
    if (IsWork){
        IsWork = false;
        analyseButton->setText( tr("Analyse") );
        StopAnalyse();
    }
    else {
        IsWork = true;
        analyseButton->setText( tr("Stop Analyse") );
        StartAnalyse();
    }
}

//初始化exposure time(从相机读取参数)
void TrackingControl::InitializeExposureTime()
{
    bool success = dalsaCamera->GetExposureTime(exposureTime);
    if(success && (exposureTime >= MinExposureTime && exposureTime <= MaxExposureTime)){
        exposureTimeEdit->setText(QString::number(exposureTime));
    }
    else{
        QMessageBox::critical(NULL, "Error", "Fail to update exposure time");
    }
}

void TrackingControl::OnExposureTimeChanged()
{
	UINT64 time = (exposureTimeEdit->text()).toULong();
	if (time < MinExposureTime || time > MaxExposureTime){
		QMessageBox::critical(NULL, "Error", "Invalid exposure time");
		return;
	}
	try{
		dalsaCamera->SetExposureTime(time);
		exposureTime = time;
		exposureTimeEdit->setText(QString::number(exposureTime));
	}catch(QException e){
    	QMessageBox::critical(NULL, "Error", "Cannot set exposure time");
	}
}

void TrackingControl::OnAddExposureTimeButton()
{
	UINT64 time = long(exposureTime*(1+scale));
	if (time < MinExposureTime || time > MaxExposureTime){
		QMessageBox::critical(NULL, "Error", "Invalid exposure time");
		return;
	}

	//update exposure time
	try{
		dalsaCamera->SetExposureTime(time);
		exposureTime = time;
		exposureTimeEdit->setText(QString::number(exposureTime));
	}catch(QException e){
    	QMessageBox::critical(NULL, "Error", "Cannot add exposure time");
	}
}

void TrackingControl::OnSubtractExposureTimeButton()
{
	UINT64 time = long(exposureTime*(1-scale));
	if (time < MinExposureTime || time > MaxExposureTime){
		QMessageBox::critical(NULL, "Error", "Invalid exposure time");
		return;
	}

	try{
		dalsaCamera->SetExposureTime(time);
		exposureTime = time;
		exposureTimeEdit->setText(QString::number(exposureTime));
	}catch(QException e){
    	QMessageBox::critical(NULL, "Error", "Cannot subtract exposure time");
	}
}

//初始化frame rate(从相机读取参数)
void TrackingControl::InitializeFrameRate()
{
    bool success = dalsaCamera->GetFrameRate(frameRate);
    if(success && (frameRate >= MinFrameRate && frameRate <= MaxFrameRate)){
        frameRateEdit->setText(QString::number(frameRate));
    }
    else{
        QMessageBox::critical(NULL, "Error", "Fail to update framerate");
    }
}

void TrackingControl::OnFrameRateChanged()
{
	UINT32 rate = (frameRateEdit->text()).toULong();
	if (rate < MinFrameRate || rate > MaxFrameRate){
		QMessageBox::critical(NULL, "Error", "Invalid frame rate");
		return;
	}
	
	try{
		dalsaCamera->SetFrameRate(rate);
		frameRate = rate;
        frameRateEdit->setText(QString::number(frameRate));
        UpdateMaxExposureTime();
	}catch(QException e){
    	QMessageBox::critical(NULL, "Error", "Cannot set framerate");
	}
}

void TrackingControl::FillTriggerModeBox()
{
    triggerModeBox->addItem("Internal");
    triggerModeBox->addItem("External");
}

void TrackingControl::OnTriggerModeChanged()
{
    QString modeType = triggerModeBox->currentText();
    if (modeType == "Internal"){
        triggerMode = INTERNAL;
        SetExposureTimeGroupEnabled(true);
        SetFrameRateGroupEnabled(true);
    }
    else if (modeType == "External"){
        triggerMode = EXTERNAL;

        exposureTimeEdit->setText("NA");
        frameRateEdit->setText("NA");
        SetExposureTimeGroupEnabled(false);
        SetFrameRateGroupEnabled(false);
    }
    dalsaCamera->SetTriggerMode(triggerMode);

    //Update current exposure time and frame rate
    if (triggerMode == INTERNAL){
        usleep(2*DALSE_SERIAL_QUERY_DELAY);
        InitializeExposureTime();
        InitializeFrameRate();
    }
}

/****** Realtime Control Group Slots ******/
bool TrackingControl::SetStageInitialPID()
{
    bool success = false;
    try{
        translationStage->X_Set_PID(TRANSLATION_STAGE::X_PID_INITIAL);//Set to initial PID
        translationStage->Y_Set_PID(TRANSLATION_STAGE::Y_PID_INITIAL);
        success = true;
    } catch(QException e){
        cout<<e.getMessage()<<endl;
    }
    usleep(500000);
    return success;
}

bool TrackingControl::EndAdaptivePID()
{
    bool success = false;
    try{
        translationStage->WriteCommand("HX\r");
        usleep(500000);
        translationStage->WriteCommand("XQ#EXIT\r");
        usleep(500000);
        translationStage->X_Set_PID(TRANSLATION_STAGE::X_PID_INITIAL);
        translationStage->Y_Set_PID(TRANSLATION_STAGE::Y_PID_INITIAL);
        success = true;         
    } catch(std::string e){
        cout<<e<<endl;
    } catch (QException e){
        cout<<e.getMessage()<<endl;
    }
    usleep(500000);
    return success;
}
void TrackingControl::EnterAdaptivePID()
{
    usleep(500000);
    bool success = StartAdaptivePID();
    int index = 0;
    /*while (!success && index<3){*/
    if (!success){
        usleep(500000);
        success = StartAdaptivePID();
        ++index;
    }
    ADAPTIVE_PID = true;
}

bool TrackingControl::StartAdaptivePID()
{
    bool success = false;
    try{
        translationStage->WriteCommand("XQ#WORM\r");
        success = true;         
    } catch(std::string e){
        cout<<e<<endl;
    } catch (QException e){
        cout<<e.getMessage()<<endl;
    }
    usleep(500000);
    return success;
}
void TrackingControl::ExitAdaptivePID()
{
    ADAPTIVE_PID = false;
    usleep(500000);
    bool success = EndAdaptivePID();
    int index = 0;
    /*while (!success && index<3){*/
    if (!success){
        usleep(500000);
        success = EndAdaptivePID();
        ++index;
    }
}

void TrackingControl::OnCentroidTrackButton()
{
    static bool IsWork = false;
    if (IsWork){
        // Quit the centoid track thread
        if (centroidTrackThread != NULL){
            QuitCentroidTrack();
            cout<<"Quit centroid track thread\n";
        }

        IsWork = false;
        DRAW_REFLINE_FLAG = false;//Stop to draw reference lines
        IS_TRACKING = false;
        centroidTrackButton->setText( tr("Centroid Track") );
        ExitAdaptivePID(); // exit adaptive PID
    }
    else {
        EnterAdaptivePID(); //enter adaptive PID
        IsWork = true;
        DRAW_REFLINE_FLAG = true;//Start to draw reference lines
        IS_TRACKING = true;
        centroidTrackButton->setText( tr("Stop Centroid Track") );

        // Start thread to centroid track
        if (centroidTrackThread == NULL){
            centroidTrackThread = new CentroidTrackThread();
            cout<<"Create centroid track thread\n";
        }
        centroidTrackThread->start();
        centroidTrackThread->Initialize();
        StartCentroidTrack();
    }
}

void TrackingControl::OnRoiTrackButton()
{
    static bool IsWork = false;
    if (IsWork){
        // Quit ROI track thread and disconnect the correspoding slots
        if (roiTrackThread != NULL){
            if (trackingParametersSettingDialog != NULL){
                disconnect(trackingParametersSettingDialog, SIGNAL(UpdateWormAreaSignal()), roiTrackThread, SLOT(UpdateWormAreaSlot()));
            }
            QuitRoiTrack();
            cout<<"Quit ROI track thread\n";
        }
        IsWork = false;
        DRAW_REFLINE_FLAG = false;//Stop to draw reference lines
        IS_TRACKING = false;
        roiTrackButton->setText( tr("Centerline Track") );
        ExitAdaptivePID(); // exit adaptive PID
    }
    else {
        EnterAdaptivePID(); //enter adaptive PID
        IsWork = true;
        DRAW_REFLINE_FLAG = true; //Start to draw reference lines
        IS_TRACKING = true;
        roiTrackButton->setText( tr("Stop Centerline Track") ); 

        // Start ROI track
        if (roiTrackThread == NULL){
            roiTrackThread = new RoiTrackThread();
            if (trackingParametersSettingDialog != NULL){
                QObject::connect(trackingParametersSettingDialog, SIGNAL(UpdateWormAreaSignal()), roiTrackThread, SLOT(UpdateWormAreaSlot()));
            }
            cout<<"Create ROI track thread\n";
        }
        roiTrackThread->start();
        roiTrackThread->Initialize();
        StartRoiTrack();
    }
}

void TrackingControl::OnRecordButton()
{
    static bool IsWork = false;
    if (IsWork){
        IsWork = false;
        recordButton->setText( tr("Record") );
        // Quit record thread
        if (recordThread != NULL){
            QuitRecord();
            cout<<"Quit record thread\n";
        }
        IS_RECORDING = false;
    }
    else {
        IsWork = true;
        recordButton->setText( tr("Stop Record") );
        // Start record thread
        if (recordThread == NULL){
            recordThread = new RecordThread();
            cout<<"Created record thread\n";
        }
        recordThread->start();
        StartRecord();
        IS_RECORDING = true;
    }
}

void TrackingControl::OnTrackingParametersButton()
{
    if (trackingParametersSettingDialog == NULL){
        trackingParametersSettingDialog = new TrackingParametersSettingDialog(translationStage);
        trackingParametersSettingDialog->show();
    }
    else{
        trackingParametersSettingDialog->raise();
        trackingParametersSettingDialog->show();
    }
}

void TrackingControl::OnSaveButton()
{
    static UINT64 count = 0;
    QString folder = "./image/";
    QString filename = folder + tr("image_") + QString::number(count) + tr(".tiff");
    
    read_write_locker.lockForRead();
    if (globalDalsaImage.data != NULL){
        int rows = globalDalsaImage.rows;
        int cols = globalDalsaImage.cols;
        cv::Mat image = cv::Mat(rows,cols,CV_8UC1,(void*)globalDalsaImage.data);
        imwrite(filename.toStdString(), image);
        ++count;
    }
    read_write_locker.unlock();
}

void TrackingControl::OnDualImagingButton()
{
}

void TrackingControl::OnStaringImagingButton()
{
   static bool IsWork = false;
    if (IsWork){
        // Quit staringImaging thread
        if (staringImagingThread != NULL){
            QuitStaringImaging();
        }
        IsWork = false;
        DRAW_REFLINE_FLAG = false;
        IS_TRACKING = false;
        staringButton->setText( tr("ROI Track") );
        ExitAdaptivePID(); // exit adaptive PID
    }
    else {
        EnterAdaptivePID(); //enter adaptive PID
        IsWork = true;
        DRAW_REFLINE_FLAG = true;
        IS_TRACKING = true;
        staringButton->setText( tr("Stop ROI Track") ); 

        // Start staringImaging thread
        if (staringImagingThread == NULL){
            staringImagingThread = new StaringImagingThread();
        }
        staringImagingThread->start();
        staringImagingThread->Initialize();
        StartStaringImaging();
    }
}

void TrackingControl::OnCruisingButton()
{
   static bool IsWork = false;
    if (IsWork){
        // Quit crusing thread
        if (cruisingThread != NULL){
            QuitCruising();
        }
        
        IsWork = false;
        DRAW_REFLINE_FLAG = false;
        IS_TRACKING = false;
        cruisingButton->setText( tr("Overall Track") );
        ExitAdaptivePID(); // exit adaptive PID
    }
    else {
        EnterAdaptivePID(); //enter adaptive PID
        IsWork = true;
        DRAW_REFLINE_FLAG = true;
        IS_TRACKING = true;
        cruisingButton->setText( tr("Stop Overall Track") );

        // Start cruising thread
        if (cruisingThread == NULL){
            cruisingThread = new CruisingThread();
        }
        cruisingThread->start();
        cruisingThread->Initialize();
        StartCruising();
    }
}

//图像已获得，唤醒线程处理相关事务
void TrackingControl::Image_Recieved()
{
    if (displayThread != NULL && !displayThread->GetStopDisplay()){
        StartDisplay();
    }
    if (analyseThread != NULL && !analyseThread->GetStopAnalyse()){
        StartAnalyse();
    }
    if (centroidTrackThread != NULL && !centroidTrackThread->GetStopCentroidTrack()){
        StartCentroidTrack();
    }
    if (roiTrackThread != NULL && !roiTrackThread->GetStopRoiTrack()){
        StartRoiTrack();
    }
    if (staringImagingThread != NULL && !staringImagingThread->GetStopStaringImaging()){
        StartStaringImaging();
    }
    if (cruisingThread != NULL && !cruisingThread->GetStopCruising()){
        StartCruising();
    }
    if (recordThread != NULL && !recordThread->GetStopRecord()){
        StartRecord();
    }
}

void TrackingControl::Controller1_Connected()
{
    try{
        translationStage = new TranslationStage(controller1);
        translationStage->Connect();
    }catch (string e){
        cout<<e<<endl;
        return;
    }catch (QException e){
        cout<<e.getMessage()<<endl;
        return;
    }
    cout<<"TrackingControl connect stage"<<endl;
}

void TrackingControl::Controller1_Disconnected()
{
    if (translationStage!=NULL){
        delete translationStage;
        translationStage = NULL;
    }
}

/************************* Thread Operations *************************/
// Displat thread   
void TrackingControl::StartDisplay()
{
    if (displayThread != NULL){
        displayThread->SetReadyDisplay(true);
        displayThread->SetStopDisplay(false);
    }
}
void TrackingControl::StopDisplay()
{
    if (displayThread != NULL){
        displayThread->SetStopDisplay(true);
    }
}
void TrackingControl::QuitDisplay()
{
    if (displayThread != NULL){
        displayThread->SetQuitDisplay(true);
        usleep(SLEEP_TIME);
        displayThread->quit();
    }
}

// Analyze thread
void TrackingControl::StartAnalyse()
{
    if (analyseThread != NULL){
        analyseThread->SetReadyAnalyse(true);
        analyseThread->SetStopAnalyse(false);
    }
}
void TrackingControl::StopAnalyse()
{
    if (analyseThread != NULL){
        analyseThread->SetStopAnalyse(true);
    }
}
void TrackingControl::QuitAnalyse()
{
    if (analyseThread != NULL){
        analyseThread->SetQuitAnalyse(true);
        usleep(SLEEP_TIME);
        analyseThread->quit();
    }
}

// Centroid tracking thread
void TrackingControl::StartCentroidTrack()
{
    if (centroidTrackThread == NULL){
        return;
    }

    //首先检查RoiTrackThread是否开启，若开启则使该线程睡眠
    if (roiTrackThread != NULL && !roiTrackThread->GetStopRoiTrack()){
        StopRoiTrack();
        roiTrackButton->setText( tr("ROI Track") );
    }
    centroidTrackThread->SetReadyCentroidTrack(true);
    centroidTrackThread->SetStopCentroidTrack(false);
}
void TrackingControl::StopCentroidTrack()
{
    if (centroidTrackThread != NULL){
        centroidTrackThread->SetStopCentroidTrack(true);
        TrackingMode = NO_TRACKING;
    }
}
void TrackingControl::QuitCentroidTrack()
{
    if (centroidTrackThread != NULL){
        centroidTrackThread->SetQuitCentroidTrack(true);
        usleep(SLEEP_TIME);
        centroidTrackThread->quit();
        usleep(SLEEP_TIME);
    }
}

// ROI tracking thread
void TrackingControl::StartRoiTrack()
{
    if (roiTrackThread == NULL){
        return;
    }
    
    //首先检查CentroidTrackThread是否开启，若开启则使该线程睡眠
    if (centroidTrackThread != NULL && !centroidTrackThread->GetStopCentroidTrack()){
        StopCentroidTrack();
        centroidTrackButton->setText( tr("Centroid Track") );
    }
    roiTrackThread->SetReadyRoiTrack(true);
    roiTrackThread->SetStopRoiTrack(false);
}
void TrackingControl::StopRoiTrack()
{
    if (roiTrackThread != NULL){
        roiTrackThread->SetStopRoiTrack(true);
        TrackingMode = NO_TRACKING;
    }
}
void TrackingControl::QuitRoiTrack()
{
    if (roiTrackThread != NULL){
        roiTrackThread->SetQuitRoiTrack(true);
        usleep(SLEEP_TIME);
        roiTrackThread->quit();
        usleep(SLEEP_TIME);
    }
}

// Staring imaging thread
void TrackingControl::StartStaringImaging()
{
    if (staringImagingThread != NULL){
        staringImagingThread->SetReadyStaringImaging(true);
        staringImagingThread->SetStopStaringImaging(false);
    }
}

void TrackingControl::StopStaringImaging()
{
    if (staringImagingThread != NULL){
        staringImagingThread->SetStopStaringImaging(true);
    }
}

void TrackingControl::QuitStaringImaging()
{
    if (staringImagingThread != NULL){
        staringImagingThread->SetQuitStaringImaging(true);
        usleep(SLEEP_TIME);
        staringImagingThread->quit();
        usleep(SLEEP_TIME);
    }
}

// Curing thread
void TrackingControl::StartCruising()
{
    if (cruisingThread != NULL){
        cruisingThread->SetReadyCruising(true);
        cruisingThread->SetStopCruising(false);
    }
}

void TrackingControl::StopCruising()
{
    if (cruisingThread != NULL){
        cruisingThread->SetStopCruising(true);
    }
}

void TrackingControl::QuitCruising()
{
    if (cruisingThread != NULL){
        cruisingThread->SetQuitCruising(true);
        usleep(SLEEP_TIME);
        cruisingThread->quit();
        usleep(SLEEP_TIME);
    }
}

// Record thread
void TrackingControl::StartRecord()
{
    if (recordThread != NULL){
        recordThread->SetReadyRecord(true);
        recordThread->SetStopRecord(false);
        recordThread->SetQuitRecord(false);
    }
}
void TrackingControl::StopRecord()
{
    if (recordThread != NULL){
        recordThread->SetStopRecord(true);
    }
}
void TrackingControl::QuitRecord()
{
    if (recordThread != NULL){
        recordThread->SetQuitRecord(true);
        usleep(SLEEP_TIME);
        recordThread->quit();
        usleep(SLEEP_TIME);
    }
}
