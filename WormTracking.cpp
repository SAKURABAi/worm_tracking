
#include "WormTracking.h"
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QDesktopWidget>

TrackingWindow::TrackingWindow(QWidget *parent, Qt::WindowFlags flags):QMainWindow(parent, flags)
{
	setAttribute(Qt::WA_StaticContents);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    
    stageParamsDialog = NULL;
    autoFocusSetting = NULL;
    imageSaveSetting = NULL;
    translationStageControl = NULL;
    gridTracking = NULL;
    roiSetting = NULL;
    trackingControl = NULL;

    dalsaCamera = new DalsaCamera(this);
    CreateLayout();
    
    //Create display window
    displayWindow = new DisplayWindow(true);
    displayWindow->SetContextMenuEnabled(true);
    displayWindow->SetMouseMoveEnabled(true);
    displayWindow->SetTitle( tr("Display") );
    displayWindow->show();
    
    //Create analyse window
    analyseWindow = NULL;
    namedWindow("Analyse", WINDOW_NORMAL);
    /*analyseWindow = new DisplayWindow(false);
    analyseWindow->SetContextMenuEnabled(false);
    displayWindow->SetMouseMoveEnabled(false);
    analyseWindow->SetTitle( tr("Analyse") );
    analyseWindow->resize(500,500);
    analyseWindow->show();*/
    
    //Register Mouse Response
    QObject::connect(translationStageControl, SIGNAL(RefreshCurrentPositionSignal()), gridTracking, SLOT(refreshCurrentPosition()));
    QObject::connect(gridTracking, SIGNAL(ReturnOriginSignal()), translationStageControl, SLOT(OnReturnOrigin()));
    
    QObject::connect(displayWindow, SIGNAL(FinishMouseResponseSignal()), roiSetting, SLOT(FinishMouseResponse()));
    QObject::connect(roiSetting, SIGNAL(StartMouseResponseSignal()), displayWindow, SLOT(StartMouseResponse()));
    
    QObject::connect(trackingControl->GetDisplayThread(), SIGNAL(DisplaySignal()), this, SLOT(DisplayImage()));
    //QObject::connect(trackingControl->GetAnalyseThread(), SIGNAL(AnalyseSignal()), this, SLOT(AnalyseImage()));
}

TrackingWindow::~TrackingWindow()
{
    // Close display and analyze window
    if (displayWindow != NULL){
        delete displayWindow;
        displayWindow = NULL;
    }
    if (analyseWindow != NULL){
        delete analyseWindow;
        analyseWindow = NULL;
    }
    if (dalsaCamera!=NULL){
        dalsaCamera->Disconnect();
        delete dalsaCamera;
        dalsaCamera = NULL;
    }
    // Clear dalsa image
    if (globalDalsaImage.data != NULL){
        delete globalDalsaImage.data;
        globalDalsaImage.data = NULL;
    }
}

void TrackingWindow::closeEvent(QCloseEvent *event)
{
    if (displayWindow != NULL){
        delete displayWindow;
        displayWindow = NULL;
    }
    if (trackingControl != NULL){
        trackingControl->CloseParametersSettingDialog();
    }
    destroyAllWindows();
}

void TrackingWindow::DockDisplayWindows()
{
    QDesktopWidget desktop;
    int desktop_width = desktop.width();
    int window_width = this->width();
    displayWindow->move(desktop_width-window_width-displayWindow->width()-155,20);

    AnalyseWinodwStartX = desktop_width-window_width-500-155;
    AnalyseWindowStartY = 50+displayWindow->height();
    //analyseWindow->move(AnalyseWinodwStartX,AnalyseWindowStartY);
    resizeWindow("Analyse", 500, 400);
    moveWindow("Analyse", AnalyseWinodwStartX, AnalyseWindowStartY);
}

void TrackingWindow::CreateMenus()
{
	//create actions
	exitAction = new QAction(tr("Exit"), this);
    exitAction->setShortcut(tr("Ctrl+Q"));
    exitAction->setStatusTip(tr("Exit Tracking System"));
	aboutAction = new QAction(tr("About"), this);
	configAction = new QAction(tr("Configuration"), this);
	stageParamsAction = new QAction(tr("Stage Parameters"), this);
	
	//connect action trigger to relative slots
	connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
	connect(stageParamsAction, SIGNAL(triggered()), this, SLOT(SetStageParameters()));
	
	//create menus and add actions to the menu
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(exitAction);

    settingMenu = new QMenu(tr("S&ettings"), this);
    settingMenu->addAction(configAction);
    settingMenu->addAction(stageParamsAction);

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(aboutAction);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(settingMenu);
    menuBar()->addMenu(helpMenu);
}

void TrackingWindow::CreateLayout()
{
    gridTracking = new GridTracking();//This object will be used in left and right panel
    gridTracking->Set_FullFovSearchGridGroupTitle(tr("FullFOV Grid Search"));
    gridTracking->Set_GridGroupTitle(tr("FullFOV Grid"));
    
 	leftPanel = CreateLeftPanel();
    rightPanel = CreateRightpanel();
	CreateMenus();

    //Connect signal to relative slot
    QObject::connect(translationStageControl, SIGNAL(SendState(const string)), this, SLOT(ShowState(string)));
    QObject::connect(roiSetting, SIGNAL(SendState(const string)), this, SLOT(ShowState(const string)));
    QObject::connect(gridTracking, SIGNAL(SendState(const string)), this, SLOT(ShowState(const string)));    
    QObject::connect(translationStageControl, SIGNAL(Broadcast_Controller_Connected()), roiSetting, SLOT(Controller1_Connected()));
    QObject::connect(translationStageControl, SIGNAL(Broadcast_Controller_Disconnected()), roiSetting, SLOT(Controller1_Disconnected()));
    QObject::connect(translationStageControl, SIGNAL(Broadcast_Controller_Connected()), gridTracking, SLOT(Controller1_Connected()));
    QObject::connect(translationStageControl, SIGNAL(Broadcast_Controller_Disconnected()), gridTracking, SLOT(Controller1_Disconnected()));
    QObject::connect(translationStageControl, SIGNAL(Broadcast_Controller_Connected()), trackingControl, SLOT(Controller1_Connected()));
    QObject::connect(translationStageControl, SIGNAL(Broadcast_Controller_Disconnected()), trackingControl, SLOT(Controller1_Disconnected()));

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel);
    //mainLayout->insertSpacing(1,5);//主窗口中的空隙
    
    QFrame *mainWindow = new QFrame;//主窗口
	mainWindow->setLayout(mainLayout);

	setFixedWidth(885);//left_max + right_max + mid
	setCentralWidget( mainWindow );	
	setWindowTitle( tr("Worm Tracking System") );
}

QFrame* TrackingWindow::CreateLeftPanel()
{
    QFrame *leftFrame = new QFrame(this);
    leftFrame->setFrameShape(QFrame::NoFrame);
    
    translationStageControl = new TranslationStageControl(leftFrame);
    translationStageControl->Set_StageConnectionGroupTitle(tr("Stage Connection"));
    translationStageControl->Set_StageMotionGroupTitle(tr("Stage Motion"));
    
    roiSetting = new ROISetting(dalsaCamera,leftFrame);
    roiSetting->SetTitle(tr("ROI Setting"));
    trackingControl = new TrackingControl(dalsaCamera,leftFrame);
    trackingControl->Set_InitialControlGroupTitle(tr("Initial Control"));
    trackingControl->Set_ReatimeControlGroupTitle(tr("Reatime Control"));
    
    QGroupBox *stageConnectionGroup = translationStageControl->Get_StageConnectionGroup();
    QGroupBox *stageMotionGroup = translationStageControl->Get_StageMotionGroup();
    QGroupBox *initialControl = trackingControl->Get_InitialControlGroup();
    QGroupBox *realtimeControl = trackingControl->Get_RealtimeControlGroup();
    //QGroupBox *fullFovSearchGrid = gridTracking->Get_FullFovSearchGridGroup();
    
    QFont font;
    font.setPointSize(9);
    QLabel *copyrightLabel = new QLabel( QString::fromLocal8Bit("© 2014 Heng Mao Lab, Peking University") );
    copyrightLabel->setFixedHeight(15);
    copyrightLabel->setFont(font);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(stageConnectionGroup);
    leftLayout->addWidget(initialControl);
    leftLayout->addWidget(stageMotionGroup);

    // @kevin - silence full fov search grid for easy operation
    // @date 2017-11-07
    //leftLayout->addWidget(fullFovSearchGrid);
    
    leftLayout->addWidget(roiSetting->GetRoiSettingGroup());
    leftLayout->addWidget(realtimeControl);
    leftLayout->addWidget(copyrightLabel);
    leftLayout->setMargin(0);
    leftLayout->setSpacing(0);
    leftLayout->insertSpacing(6,5);
    
    leftFrame->setLayout(leftLayout);
    leftFrame->setMaximumWidth(445);
    leftFrame->setMinimumWidth(430);
    
    return leftFrame;
}

QFrame* TrackingWindow::CreateRightpanel()
{
    QFrame *rightFrame = new QFrame(this);
    rightFrame->setFrameShape(QFrame::NoFrame);

    autoFocusSetting = new AutoFocusSetting(rightFrame);
    autoFocusSetting->SetTitle( tr("AutoFocus Setting") );
    imageSaveSetting = new ImageSaveSetting(rightFrame);
    imageSaveSetting->SetTitle( tr("Image Save Setting") );
    
    stateDisplayEdit = new QTextEdit(rightFrame);
    stateDisplayEdit->setReadOnly(true);

    QHBoxLayout *stateDisplayEditLayout = new QHBoxLayout;
    stateDisplayEditLayout->insertSpacing(0, 10);
    stateDisplayEditLayout->addWidget(stateDisplayEdit);
    stateDisplayEditLayout->insertSpacing(-1, 10);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(gridTracking->Get_GridGroup());
    rightLayout->addWidget(autoFocusSetting->Get_AutoFocusSettingGroup());
    rightLayout->addWidget(imageSaveSetting->Get_ImageSaveSettingGroup());
    rightLayout->addLayout(stateDisplayEditLayout);
    rightLayout->setMargin(0);

    rightFrame->setLayout(rightLayout);
    rightFrame->setMaximumWidth(440);
    rightFrame->setMinimumWidth(410);
    return rightFrame;
}

void TrackingWindow::SetStageParameters()
{
    if (stageParamsDialog == NULL){
        stageParamsDialog = new DeviceParametersDialog();
        stageParamsDialog->show();
    }else{
        stageParamsDialog->raise();
        stageParamsDialog->show();
    }
}

void TrackingWindow::DisplayImage()
{
    int width = globalDalsaImage.cols;
    int height = globalDalsaImage.rows;
    displayWindow->ShowImage((void*)globalDalsaImage.data, width, height);
}

 void TrackingWindow::AnalyseImage()
 {
    int width = globalAnalyseImage.cols;
    int height = globalAnalyseImage.rows;
    //analyseWindow->ShowImage((void*)globalAnalyseImage.data, width, height);
 }
