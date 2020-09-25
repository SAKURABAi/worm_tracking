
#include "TranslationStageControl.h"
#include "DevicePackage.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>

#include <QtGui/QFont>

string TranslationStageControl::OBJECT_NAME = "TranslationStageControl";

TranslationStageControl::TranslationStageControl(QObject* parent):QObject(parent)
{
    controller1 = NULL;
	translationStage = NULL;
	rotationStage = NULL;
    x_MotionThread = NULL;
    y_MotionThread = NULL;
	controller_address = "192.168.0.5";
	
	Create_StageConnectionGroup();
	Create_StageMotionGroup();
}

TranslationStageControl::~TranslationStageControl()
{
	OnController_DisconnectButton();
}

/****** Stage Connection Group ******/
void TranslationStageControl::Create_StageConnectionGroup()
{
	stageConnectionGroup = new QGroupBox;
    
	controller_AddressEdit = new QLineEdit;
	controller_AddressEdit->setText(QString::fromStdString(controller_address));
	controller_connectButton = new QPushButton( tr("Connect") );
	controller_disconnectButton = new QPushButton( tr("Disconnect") );
	controller_disconnectButton->setEnabled( false );
    controller_AddressEdit->setReadOnly(true);

	QObject::connect( controller_connectButton, SIGNAL( clicked() ), this, SLOT( OnController_ConnectButton() ) );
	QObject::connect( controller_disconnectButton, SIGNAL( clicked() ), this, SLOT( OnController_DisconnectButton() ) );
	
	QHBoxLayout *mainLayout = new QHBoxLayout();
	mainLayout->addWidget( new QLabel( tr("Controller1 IP") ) );
	mainLayout->addWidget( controller_AddressEdit );
	mainLayout->addWidget( controller_connectButton );
	mainLayout->addWidget( controller_disconnectButton );
    mainLayout->setSpacing(5);
    mainLayout->setMargin(0);
    
	stageConnectionGroup->setLayout(mainLayout);
}

void TranslationStageControl::OnController_ConnectButton()
{
	controller_address = (controller_AddressEdit->text()).toStdString();
	try{
    	controller1 = new Galil("192.168.0.5");
        controller1->timeout_ms = 500;
    }catch (string e){
        emit SendState(e);
        return;
    }

    if (controller1 != NULL){ //已成功连接控制器
        translationStage = new TranslationStage(controller1);
        rotationStage = new RotationStage(controller1);
        x_MotionThread = new MotionThread(translationStage);
        y_MotionThread = new MotionThread(translationStage);

        QObject::connect(x_MotionThread, SIGNAL(FinishMotion()), this, SLOT(OnMotionFinish()));
        QObject::connect(x_MotionThread, SIGNAL(FinishReturnOrigin(int)), this, SLOT(OnReturnOriginFinish(int)));
        QObject::connect(y_MotionThread, SIGNAL(FinishMotion()), this, SLOT(OnMotionFinish()));
        QObject::connect(y_MotionThread, SIGNAL(FinishReturnOrigin(int)), this, SLOT(OnReturnOriginFinish(int)));

        //translationStage->Connect();
        //rotationStage->Connect();
        bool success = translationStage->Connect();
        cout<<"Connect translationStage: "<<success<<endl;
        if (!success){
            //QMessageBox::critical(NULL, "Error", "Fail to connect translation stage");
            //emit SendState("Fail to connect translation stage.");
            cout<<"Fail to connect translation stage"<<endl;
            OnController_DisconnectButton();
            return;
        }

        success = rotationStage->Connect();
        cout<<"Connect rotationStage: "<<success<<endl;
        if (!success){
            //QMessageBox::critical(NULL, "Error", "Fail to connect rotation stage.");
            //emit SendState("Fail to connect rotation stage");
            cout<<"Fail to connect rotation stage"<<endl;
            OnController_DisconnectButton();
            return;
        }
        cout<<"Connect all stages"<<endl;

        controller_connectButton->setEnabled(false);
        controller_disconnectButton->setEnabled(true);
        emit Broadcast_Controller_Connected();
    }
}

void TranslationStageControl::OnController_DisconnectButton()
{
    controller_connectButton->setEnabled(true);
    controller_disconnectButton->setEnabled(false);
    
    if (x_MotionThread != NULL){
        QObject::disconnect(x_MotionThread, SIGNAL(FinishMotion()), this, SLOT(OnMotionFinish()));
        QObject::disconnect(x_MotionThread, SIGNAL(FinishReturnOrigin(int)), this, SLOT(OnReturnOriginFinish(int)));
        x_MotionThread->quit();
        delete x_MotionThread;
        x_MotionThread = NULL;
    }
    if (y_MotionThread != NULL){
        QObject::disconnect(y_MotionThread, SIGNAL(FinishMotion()), this, SLOT(OnMotionFinish()));
        QObject::disconnect(y_MotionThread, SIGNAL(FinishReturnOrigin(int)), this, SLOT(OnReturnOriginFinish(int)));
        y_MotionThread->quit();
        delete y_MotionThread;
        y_MotionThread = NULL;
    }
	if (translationStage != NULL){
        if (translationStage->IsConnected()){
        	translationStage->Stop();
        }
    	delete translationStage;
    	translationStage = NULL;
	}
	if (rotationStage != NULL){
        if (rotationStage->IsConnected()){
        	rotationStage->Stop();
        }
    	delete rotationStage;
    	rotationStage = NULL;
	}
	if (controller1 != NULL){
    	delete controller1;
    	controller1 = NULL;
	}
	emit Broadcast_Controller_Disconnected();
}

void TranslationStageControl::OnMotionFinish()
{
    if (globalGridInfo.hasRowsAndCols){
        emit RefreshCurrentPositionSignal();
    }
}

void TranslationStageControl::OnReturnOrigin()
{
    if (x_MotionThread != NULL){
        x_MotionThread->setMethod(X_RETURN_ORIGIN);
        x_MotionThread->start();
        emit SendState("Start X Return Origin");
    }
    if (y_MotionThread != NULL){
        y_MotionThread->setMethod(Y_RETURN_ORIGIN);
        y_MotionThread->start();
        emit SendState("Start Y Return Origin");
    }
}

void TranslationStageControl::OnReturnOriginFinish(int method)
{
    if (method == X_RETURN_ORIGIN)
        emit SendState("Finish X Return Origin");
    else if (method == Y_RETURN_ORIGIN)
        emit SendState("Finish Y Return Origin");
}

/****** Stage Motion Group ******/
void TranslationStageControl::CreateArrowButtonsAndSetStyle()
{
	upButton = new QToolButton;
    QString styleSheet = tr(".QToolButton:hover{border: 0px; padding: 0px; margin: 0px; background-image: url(icons/HoverUp.png)}")+
                         tr(".QToolButton{border: 0px; padding: 0px; margin: 0px; background-image: url(icons/ArrowUp.png)}");
    upButton->setIconSize( QSize(ARROW_BUTTON_WIDTH, ARROW_BUTTON_HEIGHT) );
    upButton->setStyleSheet(styleSheet);
    
    downButton = new QToolButton;
    styleSheet = tr(".QToolButton:hover{border: 0px; padding: 0px; margin: 0px; background-image: url(icons/HoverDown.png)}")+
                         tr(".QToolButton{border: 0px; padding: 0px; margin: 0px; background-image: url(icons/ArrowDown.png)}");
    downButton->setIconSize( QSize(ARROW_BUTTON_WIDTH, ARROW_BUTTON_HEIGHT) );
    downButton->setStyleSheet(styleSheet);
    
    leftButton = new QToolButton;
    styleSheet = tr(".QToolButton:hover{border: 0px; padding: 0px; margin: 0px; background-image: url(icons/HoverLeft.png)}")+
                         tr(".QToolButton{border: 0px; padding: 0px; margin: 0px; background-image: url(icons/ArrowLeft.png)}");
    leftButton->setIconSize( QSize(ARROW_BUTTON_WIDTH, ARROW_BUTTON_HEIGHT) );
    leftButton->setStyleSheet(styleSheet);
    
    rightButton = new QToolButton;
    styleSheet = tr(".QToolButton:hover{border: 0px; padding: 0px; margin: 0px; background-image: url(icons/HoverRight.png)}")+
                         tr(".QToolButton{border: 0px; padding: 0px; margin: 0px; background-image: url(icons/ArrowRight.png)}");
    rightButton->setIconSize( QSize(ARROW_BUTTON_WIDTH, ARROW_BUTTON_HEIGHT) );
    rightButton->setStyleSheet(styleSheet);
    
    QFont font;
    font.setBold(true);
    stopButton = new QPushButton( tr("Stop") );
    stopButton->setStyleSheet("color: red");
    stopButton->setFont(font);
    stopButton->setFixedSize( QSize(ARROW_BUTTON_WIDTH, ARROW_BUTTON_HEIGHT) );
    
    clockwiseButton = new QToolButton;
    styleSheet = tr(".QToolButton:hover{border: 0px; padding: 0px; margin: 0px; background-image: url(icons/HoverClockwise.png)}")+
                         tr(".QToolButton{border: 0px; padding: 0px; margin: 0px; background-image: url(icons/ArrowClockwise.png)}");
    clockwiseButton->setIconSize( QSize(ARROW_BUTTON_WIDTH, ARROW_BUTTON_HEIGHT) );
    clockwiseButton->setStyleSheet(styleSheet);

	anticlockwiseButton = new QToolButton;
	styleSheet = tr(".QToolButton:hover{border: 0px; padding: 0px; margin: 0px; background-image: url(icons/HoverAnticlockwise.png)}")+
                         tr(".QToolButton{border: 0px; padding: 0px; margin: 0px; background-image: url(icons/ArrowAnticlockwise.png)}");
    anticlockwiseButton->setIconSize( QSize(ARROW_BUTTON_WIDTH, ARROW_BUTTON_HEIGHT) );
    anticlockwiseButton->setStyleSheet(styleSheet);

    QObject::connect( upButton, SIGNAL( clicked() ), this, SLOT( OnUpButton() ) );
    QObject::connect( downButton, SIGNAL( clicked() ), this, SLOT( OnDownButton() ) );
    QObject::connect( leftButton, SIGNAL( clicked() ), this, SLOT( OnLeftButton() ) );
    QObject::connect( rightButton, SIGNAL( clicked() ), this, SLOT( OnRightButton() ) );
    QObject::connect( stopButton, SIGNAL( clicked() ), this, SLOT( OnStopButton() ) );
    QObject::connect( clockwiseButton, SIGNAL( clicked() ), this, SLOT( OnClockwiseButton() ) );
    QObject::connect( anticlockwiseButton, SIGNAL( clicked() ), this, SLOT( OnAnticlockwiseButton() ) );
}

void TranslationStageControl::Create_StageMotionGroup()
{
	stageMotionGroup = new QGroupBox;
	CreateArrowButtonsAndSetStyle();

    //create widgets
    motionStepEdit = new QDoubleSpinBox;
    motionStepEdit->setMaximum(10000.0);
	motionStepEdit->setMinimum(0.1);
	motionStepEdit->setValue(2000.0);
	motionStepEdit->setSingleStep(500.0);
    
    rotationDegreeEdit  = new QDoubleSpinBox;
    rotationDegreeEdit->setMaximum(360.0);
    rotationDegreeEdit->setMinimum(0.01);
    rotationDegreeEdit->setValue(0.1);
    motionStepEdit->setSingleStep(1.0);
    
    borderSettingLockerButton = new QPushButton(tr("Unlock"));
	borderSettingLockerButton->setMinimumWidth(60);
	borderSettingLockerButton->setMaximumWidth(70);
	
    setUpBorderButton = new QPushButton( tr("Set") );
    setDownBorderButton = new QPushButton( tr("Set") );
    setLeftBorderButton = new QPushButton( tr("Set") );
    setRightBorderButton =  new QPushButton( tr("Set") );

    //set widgets property
    motionStepEdit->setMaximumWidth(155);
    motionStepEdit->setMinimumWidth(145);
    setUpBorderButton->setFixedWidth(50);
    setDownBorderButton->setFixedWidth(50);
    setLeftBorderButton->setFixedWidth(50);
    setRightBorderButton->setFixedWidth(50);
    
    //connect signals and slots
    QObject::connect(borderSettingLockerButton, SIGNAL(clicked()), this, SLOT(OnLockBorderSettingButton()));   
    QObject::connect( setUpBorderButton, SIGNAL( clicked() ), this, SLOT( OnSetUpBorderButton() ) );
    QObject::connect( setDownBorderButton, SIGNAL( clicked() ), this, SLOT( OnSetDownBorderButton() ) );
    QObject::connect( setLeftBorderButton, SIGNAL( clicked() ), this, SLOT( OnSetLeftBorderButton() ) );
    QObject::connect( setRightBorderButton, SIGNAL( clicked() ), this, SLOT( OnSetRightBorderButton() ) );

    //layout for arrow buttons
    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(upButton, 0, 1);
    gridLayout->addWidget(leftButton, 1, 0);
    gridLayout->addWidget(stopButton, 1, 1);
    gridLayout->addWidget(rightButton, 1, 2);
    gridLayout->addWidget(downButton, 2, 1);
    gridLayout->setMargin(0);
    gridLayout->setSpacing(0);
    
    QHBoxLayout *stepLayout = new QHBoxLayout;
    stepLayout->addWidget(new QLabel( tr("Step") ));
    stepLayout->addWidget(motionStepEdit);
    stepLayout->addWidget(new QLabel( tr("um") ));
    stepLayout->addWidget(new QLabel( tr("Rotation") ));
    stepLayout->addWidget(rotationDegreeEdit);
    stepLayout->addWidget(new QLabel( tr("degree") ));
    stepLayout->addStretch();
    stepLayout->setMargin(0);
    stepLayout->setSpacing(5);
    stepLayout->insertSpacing(3, 10);
    
    QGridLayout *gridLayout1 = new QGridLayout;
    gridLayout1->addWidget(setUpBorderButton, 0, 1, 1, 1, Qt::AlignCenter);
    gridLayout1->addWidget(setLeftBorderButton, 1, 0, Qt::AlignRight);
    gridLayout1->addLayout(gridLayout, 1, 1, Qt::AlignCenter);
    gridLayout1->addWidget(setRightBorderButton, 1, 2, Qt::AlignLeft);
    gridLayout1->addWidget(setDownBorderButton, 2, 1, 1, 1, Qt::AlignCenter);
    gridLayout1->addWidget(borderSettingLockerButton, 2, 2, 1, 1, Qt::AlignRight);
    gridLayout1->setMargin(0);
    gridLayout1->setSpacing(10);
    
    QVBoxLayout *rotationLayout = new QVBoxLayout;
    rotationLayout->addWidget(clockwiseButton);
    rotationLayout->addWidget(anticlockwiseButton);
    
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addLayout(gridLayout1);
    hLayout->addLayout(rotationLayout);
    hLayout->insertSpacing(1,20);
    hLayout->addStretch();
    
    QVBoxLayout *vLayout= new QVBoxLayout;
    vLayout->addLayout(stepLayout);
    vLayout->addLayout(hLayout);
    vLayout->setMargin(5);
    vLayout->setSpacing(5);
    
    stageMotionGroup->setLayout(vLayout);
    BorderSettingEnabled(false);
}

void TranslationStageControl::OnLockBorderSettingButton()
{
	static bool isButtonClicked = true;
	if (isButtonClicked){
		isButtonClicked = false;
		borderSettingLockerButton->setText(tr("Lock"));
		BorderSettingEnabled(true);
	}
	else{
		isButtonClicked = true;
		borderSettingLockerButton->setText(tr("Unlock"));
		BorderSettingEnabled(false);
	}
}

void TranslationStageControl::BorderSettingEnabled(bool ok)
{
	setUpBorderButton->setEnabled(ok);
	setRightBorderButton->setEnabled(ok);
	setDownBorderButton->setEnabled(ok);
	setLeftBorderButton->setEnabled(ok);
}

double TranslationStageControl::Get_MotionStep()
{
    double step = motionStepEdit->value();
    return step/TRANSLATION_STAGE::X_PRECISION;;//1 um=10pulse
}

void TranslationStageControl::OnUpButton()
{
    
    /*虚拟坐标系中设备之间的对应关系：
      虚拟坐标系Dalsa CCD(X+) <-> Stage(X-), Dalsa CCD(Y+) <-> Stage(Y+)，
      图像运动与平移台运动是相对的，因此Y轴向负方向运动*/
    if (translationStage==NULL || !translationStage->IsConnected()) {return;}
    try {
        //long step = motionStepEdit->value();
        //double distance = step*(-STAGE_Y_POSITIVE)/TRANSLATION_STAGE::Y_PRECISION;
        double pulse = Get_MotionStep();
        pulse = pulse*(-STAGE_Y_POSITIVE);

        y_MotionThread->setMethod(Y_MOTION);
        y_MotionThread->setYDistance(pulse);
        y_MotionThread->start();
    } catch (QException e){
        emit SendState(e.getMessage());
    }
}

void TranslationStageControl::OnDownButton()
{
    /*虚拟坐标系中设备之间的对应关系：
      虚拟坐标系Dalsa CCD(X+) <-> Stage(X-), Dalsa CCD(Y+) <-> Stage(Y+)，
      图像运动与平移台运动是相对的，因此Y轴向正方向运动*/
    if (translationStage==NULL || !translationStage->IsConnected()) {return;}
    try {
        //long step = motionStepEdit->value();
        //double pulse = step*(STAGE_Y_POSITIVE)/TRANSLATION_STAGE::Y_PRECISION;
        double pulse = Get_MotionStep();
        pulse = pulse*STAGE_Y_POSITIVE;

        y_MotionThread->setMethod(Y_MOTION);
        y_MotionThread->setYDistance(pulse);
        y_MotionThread->start();      
    } catch (QException e){
        emit SendState(e.getMessage());
    }
}

void TranslationStageControl::OnLeftButton()
{
    /*虚拟坐标系中设备之间的对应关系：
      虚拟坐标系Dalsa CCD(X+) <-> Stage(X-), Dalsa CCD(Y+) <-> Stage(Y+)，
      图像向左运动（X-）对应与平移台X轴（X+）*/
    if (translationStage==NULL || !translationStage->IsConnected()) {return;}
    try {
        //long step = motionStepEdit->value();
        //double pulse = step*(STAGE_X_POSITIVE)/TRANSLATION_STAGE::X_PRECISION;
        double pulse = Get_MotionStep();
        pulse = pulse*STAGE_X_POSITIVE;

        x_MotionThread->setMethod(X_MOTION);
        x_MotionThread->setXDistance(pulse);
        x_MotionThread->start();     
    } catch (QException e){
        emit SendState(e.getMessage());
    }
}

void TranslationStageControl::OnRightButton()
{
    /*虚拟坐标系中设备之间的对应关系：
      虚拟坐标系Dalsa CCD(X+) <-> Stage(X-), Dalsa CCD(Y+) <-> Stage(Y+)，
      图像向右运动（X+）对应与平移台X轴（X-）*/
    if (translationStage==NULL || !translationStage->IsConnected()) {return;}
    try {
        //long step = motionStepEdit->value();
        //double pulse = step*(-STAGE_X_POSITIVE)/TRANSLATION_STAGE::X_PRECISION;
        double pulse = Get_MotionStep();
        pulse = pulse*(-STAGE_X_POSITIVE);

        x_MotionThread->setMethod(X_MOTION);
        x_MotionThread->setXDistance(pulse);
        x_MotionThread->start();
    } catch (QException e){
        emit SendState(e.getMessage());
    }
}

void TranslationStageControl::OnStopButton()
{
    if (translationStage==NULL || !translationStage->IsConnected()) {return;}
    try {
        translationStage->Stop();
        rotationStage->Stop();
    } catch (QException e){
        emit SendState(e.getMessage());
    }
}

void TranslationStageControl::OnSetUpBorderButton()
{
    //虚拟坐标系Dalsa CCD(X+) <-> Stage(X-), Dalsa CCD(Y+) <-> Stage(Y+)，
    //当前位置为图像中心点出的位置，向上移动才到真正的上边缘
    if (translationStage==NULL || !translationStage->IsConnected()) {return;}
    try {
        upBorder = translationStage->Y_GetCurrentPosition();
        globalGridInfo.topBorder = upBorder+long((-STAGE_Y_POSITIVE)*globalGridInfo.fullFovImageHeight_StageDistance/2);
        
        char buffer[32] = "\0";
    	sprintf(buffer, "Set top border : %ld", upBorder);
    	emit SendState(string(buffer));
    }
    catch (QException e){
        emit SendState(e.getMessage());
    }
}

void TranslationStageControl::OnSetDownBorderButton()
{
    //虚拟坐标系Dalsa CCD(X+) <-> Stage(X-), Dalsa CCD(Y+) <-> Stage(Y+)，
    //当前位置为图像中心点出的位置，向下移动才到真正的下边缘
    if (translationStage==NULL || !translationStage->IsConnected()) {return;}
    try {
        downBorder = translationStage->Y_GetCurrentPosition();
        globalGridInfo.bottomBorder = downBorder+long((STAGE_Y_POSITIVE)*globalGridInfo.fullFovImageHeight_StageDistance/2);
        
        char buffer[32] = "\0";
    	sprintf(buffer, "Set bottom border : %ld", downBorder);
    	emit SendState(string(buffer));
    }catch (QException e){
        emit SendState(e.getMessage());
    }
}

void TranslationStageControl::OnSetLeftBorderButton()
{
    //虚拟坐标系Dalsa CCD(X+) <-> Stage(X-), Dalsa CCD(Y+) <-> Stage(Y+)，
    //当前位置为图像中心点的位置，向左移动才到真正的左边缘
    if (translationStage==NULL || !translationStage->IsConnected()) {return;}
    try {
        leftBorder = translationStage->X_GetCurrentPosition();
        globalGridInfo.leftBorder = leftBorder+long((STAGE_X_POSITIVE)*globalGridInfo.fullFovImageWidth_StageDistance/2);

    	char buffer[32] = "\0";
    	sprintf(buffer, "Set left border : %ld", leftBorder);
    	emit SendState(string(buffer));
    } catch (QException e){
        emit SendState(e.getMessage());
    }
}

void TranslationStageControl::OnSetRightBorderButton()
{
    //虚拟坐标系Dalsa CCD(X+) <-> Stage(X-), Dalsa CCD(Y+) <-> Stage(Y+)，
    //当前位置为图像中心点出的位置，向右移动才到真正的右边缘
    if (translationStage==NULL || !translationStage->IsConnected()) {return;}
    try {
        rightBorder = translationStage->X_GetCurrentPosition();
    	globalGridInfo.rightBorder = rightBorder + long((-STAGE_X_POSITIVE)*globalGridInfo.fullFovImageWidth_StageDistance/2);

    	char buffer[32] = "\0";
    	sprintf(buffer, "Set right border : %ld", rightBorder);
    	emit SendState(string(buffer));
    } catch (QException e){
        emit SendState(e.getMessage());
    }
}

double TranslationStageControl::Get_RotationDegree()
{
    //double step = (rotationDegreeEdit->text()).toDouble();
    double step = rotationDegreeEdit->value();
    return step*2000;//1 degree=2000pulse
}

void TranslationStageControl::OnClockwiseButton()
{
    //在图像空间上看，顺时针旋转为负值
    if (rotationStage==NULL || !rotationStage->IsConnected()) {return;}
    try{
        double pulse = Get_RotationDegree();
        rotationStage->Move_Openloop_Realtime(-pulse);
    } catch(QException e){
        emit SendState(e.getMessage());
    }
}
    
void TranslationStageControl::OnAnticlockwiseButton()
{
    //在图像空间上看，逆时针旋转为正值
    if (rotationStage==NULL || !rotationStage->IsConnected()) {return;}
    try{
        double pulse = Get_RotationDegree();
        rotationStage->Move_Openloop_Realtime(pulse);
    } catch(QException e){
        emit SendState(e.getMessage());
    }
}
