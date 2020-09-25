/****************************************************************************
	Z2轴AutoFocus
****************************************************************************/

#include "Z2_AutoFocus.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QMessageBox>

#include <QtGui/QFont>
#include <QtCore/QString>

#define IMAGE_MAX 64
extern double Depth(const cv::Mat &image, char *type);

string Z2_AutoFocusDialog::OBJECT_NAME = "Z2_AutoFocusDialog";

Z2_AutoFocusDialog::Z2_AutoFocusDialog(QWidget *parent, Qt::WindowFlags flags):QDialog(parent, flags)
{
	controller2 = NULL;
	stage = NULL;
	motionThread = NULL;

	z2_step = 0;
	coarse_motion_times = Z2_STAGE::Z2_COARSE_FOCUS_TIMES;
	coarse_up_point = AUTOFOCUS_INITIAL_POINT;
	coarse_down_point = AUTOFOCUS_INITIAL_POINT;
	coarse_focus_point = 0;

	fine_motion_times = Z2_STAGE::Z2_FINE_FOCUS_TIMES;
	fine_focus_point = 0;

	CreateLayout();
	CreateActions();
	InitialDock();
	
   try{
	    controller2 = new Galil("192.168.0.5");
        controller2 -> timeout_ms = 500;

        stage = new Z2Stage(controller2); //controller2 isn't NULL
        stage->Connect();
    } catch(string e){
    	cout<<e<<endl;
    	return;
        //stateTextEdit->setText(QString::fromStdString(e));
    } catch(QException e){
    	cout<<e.getMessage()<<endl;
    	return;
       //stateTextEdit->setText(QString::fromStdString(e.getMessage()));
    }
    motionThread = new MotionThread(stage);
    connect( motionThread, SIGNAL(FinishMotion()), this, SLOT(OnFinishMotion()) );
    connect( motionThread, SIGNAL(FinishReturnOrigin(int)), this, SLOT(OnFinishReturnOrigin(int)) );

	//Open controller
	/*if (controller1 == NULL){
		stateTextEdit->setText("No stage connection");
		return;
	}*/
	/*try{
		//controller2 = new Galil("192.168.0.5");
        //controller2 -> timeout_ms = 500;

        stage = new Z2Stage(controller2); //controller2 isn't NULL
        stage->Connect();

        motionThread = new MotionThread(stage);
        connect( motionThread, SIGNAL(FinishMotion()), this, SLOT(OnFinishMotion()) );
        connect( motionThread, SIGNAL(FinishReturnOrigin(int)), this, SLOT(OnFinishReturnOrigin(int)) );
    } catch(string e){
    	cout<<e<<endl;
       stateTextEdit->setText(QString::fromStdString(e));
    } catch(QException e){
       stateTextEdit->setText(QString::fromStdString(e.getMessage()));
    }*/
}

Z2_AutoFocusDialog::~Z2_AutoFocusDialog()
{
	if (motionThread != NULL){
		disconnect( motionThread, SIGNAL(FinishMotion()), this, SLOT(OnFinishMotion()) );
		disconnect( motionThread, SIGNAL(FinishReturnOrigin(int)), this, SLOT(OnFinishReturnOrigin(int)) );
		delete motionThread;
		motionThread = NULL;
	}
	if (stage != NULL){
    	stage->Stop();
    	delete stage;
    	stage = NULL;
	}
}

void Z2_AutoFocusDialog::CreateActions()
{
	saveStateAction = new QAction(tr("&Save  State"), stateTextEdit);
	saveStateAction->setShortcut(tr("Ctrl+S"));
	
	clearStateAction = new QAction(tr("Clea&r  State"), stateTextEdit);
	clearStateAction->setShortcut(tr("Ctrl+R"));
	
	stateTextEdit->addAction(saveStateAction);
    stateTextEdit->addAction(clearStateAction);
    stateTextEdit->setContextMenuPolicy(Qt::ActionsContextMenu);
	QObject::connect( saveStateAction, SIGNAL( triggered() ), this, SLOT( OnSaveStateAction() ) );
	QObject::connect( clearStateAction, SIGNAL( triggered() ), this, SLOT( OnClearStateAction() ) );
}

void Z2_AutoFocusDialog::CreateLayout()
{
    QFont font, colorFont;
	font.setPointSize(9);
	colorFont.setBold(true);

	//coarse focus setting
	coarseFocusGroup = new QGroupBox( tr("Coarse Focus Setting"));
	QVBoxLayout *coarseFocusLayout = new QVBoxLayout;
	coarseFocusTimesEdit = new QSpinBox;
	setUpPointButton = new QPushButton( tr("Set Up Point") );
	setDownPointButton = new QPushButton( tr("Set Down Point") );
	coarseFocusButton = new QPushButton( tr("Coarse Focus") );
	
	coarseFocusButton->setFixedWidth(145);
	coarseFocusTimesEdit->setFixedWidth(75);
	coarseFocusTimesEdit->setMinimum(7);
	coarseFocusTimesEdit->setMaximum(21);
	coarseFocusTimesEdit->setValue(Z2_STAGE::Z2_COARSE_FOCUS_TIMES);
	coarseFocusTimesEdit->setSingleStep(2);
	
	QHBoxLayout* coarseFocusTimesLayout = new QHBoxLayout;
	coarseFocusTimesLayout->addWidget(new QLabel("Times") );
	coarseFocusTimesLayout->addWidget(coarseFocusTimesEdit);
	coarseFocusTimesLayout->addWidget(coarseFocusButton);
	coarseFocusTimesLayout->insertSpacing(2,23);
	
	QHBoxLayout* coarseFocusButtonsLayout = new QHBoxLayout;
	coarseFocusButtonsLayout->addWidget(setUpPointButton);
	coarseFocusButtonsLayout->addWidget(setDownPointButton);
	coarseFocusButtonsLayout->setSpacing(30);
	
	coarseFocusLayout->addLayout(coarseFocusButtonsLayout);
	coarseFocusLayout->addLayout(coarseFocusTimesLayout);
	coarseFocusLayout->setSpacing(20);
	coarseFocusGroup->setLayout(coarseFocusLayout);

	//fine focus setting
	fineFocusGroup = new QGroupBox( tr("Fine Focus Setting") );
	QHBoxLayout *fineFocusLayout = new QHBoxLayout;
	fineFocusButton = new QPushButton( tr("Fine Focus") );
	fineFocusTimesEdit = new QSpinBox;
	
	fineFocusButton->setFixedWidth(145);
	fineFocusTimesEdit->setFixedWidth(75);
	fineFocusTimesEdit->setMinimum(5);
	fineFocusTimesEdit->setMaximum(21);
	fineFocusTimesEdit->setValue(Z2_STAGE::Z2_FINE_FOCUS_TIMES);
	fineFocusTimesEdit->setSingleStep(2);
	
	fineFocusLayout->addWidget(new QLabel("Times") );
	fineFocusLayout->addWidget(fineFocusTimesEdit);
	fineFocusLayout->addWidget(fineFocusButton);
	fineFocusLayout->insertSpacing(2,37);
	fineFocusGroup->setLayout(fineFocusLayout);
	
	fineFocusButton->setFont(colorFont);
	fineFocusButton->setStyleSheet("color: green");
	coarseFocusButton->setFont(colorFont);
	coarseFocusButton->setStyleSheet("color: green");

	//Quick Focus Group
	quickFocusGroup = new QGroupBox("Quick Focus Setting");
	quickFocusButton = new QPushButton("Quick Focus");
	quitQuickFocusButton = new QPushButton("Quit Focus");

	quickFocusButton->setFont(colorFont);
	quickFocusButton->setStyleSheet("color: green");
	quitQuickFocusButton->setFont(colorFont);
	quitQuickFocusButton->setStyleSheet("color: red");

	QHBoxLayout *quickFocusLayout = new QHBoxLayout;
	quickFocusLayout->addWidget(quickFocusButton);
	quickFocusLayout->addWidget(quitQuickFocusButton);
	quickFocusLayout->setSpacing(30);
	quickFocusGroup->setLayout(quickFocusLayout);

	//控制组
	initialStateSetting = new QGroupBox( tr("Z2 Stage Control") );
	z2StepEdit = new QLineEdit;
	moveUpButton = new QPushButton( tr("Move Up") );
	moveDownButton = new QPushButton( tr("Move Down") );
	originReturnButton = new QPushButton( tr("Return Origin") );
	stopButton = new QPushButton( tr("Stop") );
	originReturnButton->setMinimumHeight(35);
	stopButton->setMinimumHeight(35);
	stopButton->setFont(colorFont);
	stopButton->setStyleSheet("color: red");
	originReturnButton->setFont(colorFont);
	originReturnButton->setStyleSheet("color: blue");

	z2StepEdit->setMinimumWidth(80);
	z2StepEdit->setMaximumWidth(100);
	z2StepEdit->setText( QString::number(1000) );
	QLabel *unitDetail = new QLabel( tr("um [1pulse = 1.25um]") );
	unitDetail->setFont(font);
	
	QVBoxLayout *controlLayout = new QVBoxLayout;
	QHBoxLayout* z2StepLayout = new QHBoxLayout;
	z2StepLayout->addWidget(new QLabel("Step"));
	z2StepLayout->addWidget(z2StepEdit);
	z2StepLayout->addWidget(unitDetail);
	
	QGridLayout *controlButtonsLayout = new QGridLayout;
	controlButtonsLayout->addWidget(moveUpButton, 0, 0);
	controlButtonsLayout->addWidget(moveDownButton, 0, 1);
	controlButtonsLayout->addWidget(stopButton, 1, 0);
	controlButtonsLayout->addWidget(originReturnButton, 1, 1);
	controlButtonsLayout->setHorizontalSpacing(30);
	controlButtonsLayout->setVerticalSpacing(20);
	
	controlLayout->addLayout(z2StepLayout);
	controlLayout->addLayout(controlButtonsLayout);
	controlLayout->insertSpacing(1,10);
	initialStateSetting->setLayout(controlLayout);
	
	///////////////////////////////////////////////////////////////////
	stateTextEdit = new QTextEdit;
	stateTextEdit->setReadOnly( true );//设置文本框是只读的
	stateTextEdit->setMaximumHeight(150);

	currentPositionGroup = new QGroupBox( tr("Current Position") );
	positionSlider = new DistanceSlider(Qt::Horizontal);
	positionSlider->setMaximum(100);
	positionSlider->setMinimum(0);
	
	positionLabel = new QLabel;
	positionLabel->setMinimumWidth(50);
	positionLabel->setMaximumWidth(60);
	positionLabel->setAlignment(Qt::AlignCenter);
	
	QHBoxLayout *hBoxLayout5 = new QHBoxLayout;
	hBoxLayout5->addWidget(positionSlider);
	hBoxLayout5->addWidget(positionLabel);
	currentPositionGroup->setLayout(hBoxLayout5);

    //连接控制组中控件的信号和槽
	QObject::connect( moveUpButton, SIGNAL( clicked() ), this, SLOT( OnMoveUpButton() ) );
	QObject::connect( moveDownButton, SIGNAL( clicked() ), this, SLOT( OnMoveDownButton() ) );
	QObject::connect( originReturnButton, SIGNAL( clicked() ), this, SLOT( OnReturnOriginButton() ) );
	QObject::connect( stopButton, SIGNAL( clicked() ), this, SLOT( OnStopButton() ) );
	
	QObject::connect( coarseFocusButton, SIGNAL( clicked() ), this, SLOT( OnCoarseFocusButton() ) );
	QObject::connect( setUpPointButton, SIGNAL( clicked() ), this, SLOT( OnSetUpPointButton() ) );
	QObject::connect( setDownPointButton, SIGNAL( clicked() ), this, SLOT( OnSetDownPointButton() ) );
	QObject::connect( fineFocusButton, SIGNAL( clicked() ), this, SLOT( OnFineFocusButton() ) );
	
	QObject::connect( quickFocusButton, SIGNAL( clicked() ), this, SLOT( OnQuickFocusButton() ) );
	QObject::connect( quitQuickFocusButton, SIGNAL( clicked() ), this, SLOT( OnQuitQuickFocusButton() ) );
	
	//定义布局管理器，将以上控件加入到布局中
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(initialStateSetting);
	mainLayout->addWidget(quickFocusGroup);
	mainLayout->addWidget(coarseFocusGroup);
	mainLayout->addWidget(fineFocusGroup);
	mainLayout->addWidget(currentPositionGroup);
	mainLayout->addWidget(stateTextEdit);
	
	mainLayout->addStretch();
	setLayout(mainLayout);
	setMaximumWidth(350);
	setWindowTitle( tr("Z2 AutoFocus") );
}

void Z2_AutoFocusDialog::InitialDock()
{
    QDesktopWidget desktop;
    int desktop_width = desktop.width();
    int window_width = 890;
    this->move(desktop_width-window_width-100,20);
}

void Z2_AutoFocusDialog::OnFinishMotion()
{
	stateTextEdit->append(motionThread->getDescription());
}

void Z2_AutoFocusDialog::OnFinishReturnOrigin(int stage_type)
{
	if (stage_type == RETURN_ORIGIN){
		stateTextEdit->append("ReturnOrigin: Finish");

		moveUpButton->setEnabled(true);
		moveDownButton->setEnabled(true);
		stage->Set_CurrentPosition(0);
		positionSlider->setValue(0);
		positionLabel->setText( QString::number(0) );	
	}
}

void Z2_AutoFocusDialog::OnSaveStateAction()
{
}

void Z2_AutoFocusDialog::OnClearStateAction()
{
	stateTextEdit->setText("");
	
	z2_step = 0;
	coarse_motion_times = Z2_STAGE::Z2_COARSE_FOCUS_TIMES;
	coarse_up_point  = AUTOFOCUS_INITIAL_POINT;//单位为pulse
	coarse_down_point = AUTOFOCUS_INITIAL_POINT;
	coarse_focus_point = 0;
	
	fine_motion_times = Z2_STAGE::Z2_FINE_FOCUS_TIMES;
	fine_focus_point = 0;
}

void Z2_AutoFocusDialog::OnStopButton()
{
    if (stage==NULL || !stage->IsConnected()){
        return;
    }

    try{
    	stage->Stop();
    } catch (QException e){
    	stateTextEdit->append(QString::fromStdString(e.getMessage()));
    }
}

void Z2_AutoFocusDialog::Get_Z2Step()
{
	double step = (z2StepEdit->text()).toDouble();
    z2_step = step/Z2_STAGE::Z2_PRECISION;//convert step to pulse
}

void Z2_AutoFocusDialog::Get_CoarseTimes()
{
	coarse_motion_times = coarseFocusTimesEdit->value();
}

void Z2_AutoFocusDialog::Get_FineTimes()
{
	fine_motion_times = fineFocusTimesEdit->value();
}

//向上移动升降台(Z轴)
void Z2_AutoFocusDialog::OnMoveUpButton()
{
	Get_Z2Step();		
	if(stage==NULL || !stage->IsConnected()){
		stateTextEdit->setText( tr("Move Up: stage no connection") );
		return;
	}
	try{
    	stage->Move_Openloop_Unrealtime(z2_step*Z_POSITIVE);
	} catch(QException e){
    	stateTextEdit->setText(QString::fromStdString(e.getMessage()));
	}
	DisplaySliderState();
}

//向下移动升降台(Z轴)
void Z2_AutoFocusDialog::OnMoveDownButton()
{
	Get_Z2Step();
	if(stage==NULL || !stage->IsConnected()){
		stateTextEdit->setText( tr("Move Down: stage no connection") );
		return;
	}

	try{
    	stage->Move_Openloop_Unrealtime(z2_step*(-Z_POSITIVE));
	} catch(QException e){
    	stateTextEdit->setText(QString::fromStdString(e.getMessage()));
    	return;
	}
	DisplaySliderState();
}

//设置上部升降台(Z轴)的位置
void Z2_AutoFocusDialog::OnSetUpPointButton()
{
	if(stage==NULL || !stage->IsConnected()){
		stateTextEdit->setText( tr("Set top Point: stage no connection") );
		return;
	}

	try{
		coarse_up_point = stage->Get_CurrentPosition();
	} catch(QException e){
		stateTextEdit->setText(QString::fromStdString(e.getMessage()));
		return;
	}
	
	QString status;
	status = tr("Z2 axis top point: ") + QString::number(coarse_up_point);
	stateTextEdit->append(status);
}

//设置下部升降台(Z轴)的位置
void Z2_AutoFocusDialog::OnSetDownPointButton()
{
	if(stage==NULL || !stage->IsConnected()){
		stateTextEdit->setText( tr("Set Bottom Point: stage no connection") );
		return;
	}

	try{
		coarse_down_point = stage->Get_CurrentPosition();
	} catch(QException e){
		stateTextEdit->setText(QString::fromStdString(e.getMessage()));
		return;
	}
		
	QString status;
	status = tr("Z2 axis bottom point: ") + QString::number(coarse_down_point);
	stateTextEdit->append(status);
}

//使升降台(Z轴)回到零位(通过线位控制)
void Z2_AutoFocusDialog::OnReturnOriginButton()
{
	if (stage == NULL || !stage->IsConnected()){
		stateTextEdit->setText( "Return Origin: No stage connection" );
		return;
	}

	int ret = QMessageBox::warning(this, "Warning", "Are you sure to Return Origin?", QMessageBox::Ok | QMessageBox::Cancel);
	if (ret != QMessageBox::Ok)
		return;

	moveUpButton->setEnabled(false);
	moveDownButton->setEnabled(false);
	stateTextEdit->setText( tr("Return Origin: Start") );

	motionThread->setMethod(RETURN_ORIGIN);
	motionThread->start();
}

void Z2_AutoFocusDialog::ShowState(QString state)
{
    stateTextEdit->append(state);
}

void Z2_AutoFocusDialog::DisplaySliderState()
{
    if (stage!=NULL && stage->IsConnected()){
        long current_position = long(stage->Get_CurrentPosition());
        positionLabel->setText( QString::number(current_position*Z2_STAGE::Z2_PRECISION) );
    }
}

void* Z2_AutoFocusDialog::Z2_CoarseFocusThread(void* arg)
{
	Z2_AutoFocusDialog* dialog = (Z2_AutoFocusDialog*) arg;
	dialog->Z2_CoarseFocus();
	return NULL;
}
	
void* Z2_AutoFocusDialog::Z2_FineFocusThread(void* arg)
{
	Z2_AutoFocusDialog* dialog = (Z2_AutoFocusDialog*) arg;
	dialog->Z2_FineFocus();
	return NULL;
}

void Z2_AutoFocusDialog::OnCoarseFocusButton()
{
	if (stage==NULL || !stage->IsConnected()){ return; }
	int ret = QMessageBox::warning(this, "Warning", "Are you sure to Coarse Focus?", QMessageBox::Ok | QMessageBox::Cancel);
	if (ret != QMessageBox::Ok)
		return;

	if ((coarse_up_point == AUTOFOCUS_INITIAL_POINT) || (coarse_down_point == AUTOFOCUS_INITIAL_POINT)){
		QMessageBox::critical(this, "Error", "No up and down points! Please set up and down points.");
		return;
	}

	int err,oldState;
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	//设置线程为可分离状态，当线程结束时，操作系统回收资源
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    //设置线程的取消选项
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldState);
    err = pthread_create( &coarseFocusThread, NULL, Z2_CoarseFocusThread, (void *)this);
    if (err != 0 ){
       ShowState("CoarseFocus: fail to create thread");
       return;
    }
    pthread_attr_destroy(&attr);
}

void Z2_AutoFocusDialog::OnFineFocusButton()
{
	if (stage==NULL || !stage->IsConnected()){ return; }
	int ret = QMessageBox::warning(this, "Warning", "Are you sure to Fine Focus?", QMessageBox::Ok | QMessageBox::Cancel);
	if (ret != QMessageBox::Ok)
		return;

	int err,oldState;
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	//设置线程为可分离状态，当线程结束时，操作系统回收资源
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    //设置线程的取消选项
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldState);
    err = pthread_create( &fineFocusThread, NULL, Z2_FineFocusThread, (void *)this);
    if (err != 0 ){
       ShowState("FineFocus: fail to create thread");
       return;
    }
    pthread_attr_destroy(&attr);
}

void Z2_AutoFocusDialog::Z2_CoarseFocus()
{
	double currentPoint, step, direction = 1, distance = coarse_up_point-coarse_down_point;
	double x[IMAGE_MAX], y[IMAGE_MAX], maxValue ;
	int imageNum = 0, maxIndex = 0;
	char filename[256];
	Get_CoarseTimes();

	//Motion direction
	currentPoint = stage->Get_CurrentPosition();
	if(abs(currentPoint-coarse_up_point) < abs(currentPoint-coarse_down_point)){
		direction = -Z_POSITIVE;
	}
	else{
		direction = Z_POSITIVE;
	}
	//ShowState("Coarse Focus: Start");
	cout<<"Coarse Focus: Start"<<endl;
	
	step = (distance/(coarse_motion_times-1))*direction;
	try{
		//Acquring images and calulating image evaluations
		//ShowState("Coarse Focus: Getting Images Start");
		cout<<"Coarse Focus: Getting Images Start"<<endl;
		cv::Mat src;

		for (int i=0; i<coarse_motion_times; ++i){
			currentPoint = stage->Get_CurrentPosition();
			x[imageNum] = currentPoint;

			//Get the latest image
			if (globalDalsaImage.data == NULL){
				cout<<"Fail to Coarse Focus: Dalsa image is NULL"<<endl;
				return;
			}

			read_write_locker.lockForRead();
			int rows = globalDalsaImage.rows;
		    int cols = globalDalsaImage.cols;
		    src = cv::Mat(rows,cols,CV_8UC1,(void*)globalDalsaImage.data);
            read_write_locker.unlock();

            y[imageNum] = Depth(src, "Tenengrad");
            cout<<"Pic: "<<imageNum<<", depth: "<<y[imageNum]<<endl;

			//Record the max index
			if (imageNum == 0){ maxValue = y[0]; }
			else{
				if (maxValue < y[imageNum]){
					maxValue = y[imageNum];
					maxIndex = imageNum;
				}
			}
			++imageNum;
			if (i == coarse_motion_times-1){ break; }
			stage->Move_Openloop_Unrealtime(step);
		}
		//ShowState("Coarse Focus: Getting Images Finish");
		cout<<"Coarse Focus: Getting Images Finish"<<endl;

		//Move to the desire position
		cout<<"Move to focus plane"<<endl;
		currentPoint = stage->Get_CurrentPosition();
		step = x[maxIndex] - currentPoint;
		cout<<"Go to focus plane: "<<step<<endl;
		stage->Move_Closeloop_Unrealtime(step);
	} catch (QException e){
		cout<<e.getMessage()<<endl;
		return;
	}
	//ShowState("Coarse Focus: Finish");
	cout<<"Coarse Focus: Finish"<<endl;
}

void Z2_AutoFocusDialog::Z2_FineFocus()
{
	//Fine focus range
	double currentPoint, fine_step, fine_up_point, fine_down_point, step, x[IMAGE_MAX], y[IMAGE_MAX], maxValue;
	int imageNum = 0, maxIndex = 0;
	char filename[256];

	Get_FineTimes();
	currentPoint = stage->Get_CurrentPosition();
	fine_step = 1.0*Z2_STAGE::Z2_FINE_FOCUS_STEP/Z2_STAGE::Z2_PRECISION;
	fine_down_point = currentPoint - fine_step*(fine_motion_times/2);
	fine_up_point = currentPoint + fine_step*(fine_motion_times/2);

	//ShowState("Fine Focus: Start");
	cout<<"Fine Focus: Start"<<endl;

	//Move down to the down point
	step = -Z2_STAGE::Z2_FINE_FOCUS_STEP*(fine_motion_times/2); 
	try{
		cout<<"Move down firstly"<<endl;
		stage->Move_Openloop_Unrealtime(step);
	} catch (QException e){} // Caution: Bottom switch limit!

	try{
		step = fine_step;
		//Acquring images and calulating image evaluations
		//ShowState("Fine Focus: Getting Images Start");
		cout<<"Fine Focus: Getting Images Start"<<endl;
		cv::Mat src;

		for (int i=0; i<fine_motion_times; ++i){
			currentPoint = stage->Get_CurrentPosition();
			x[imageNum] = currentPoint;

			cout<<"get image depth"<<endl;
			//Get the latest image
			if (globalDalsaImage.data == NULL){
				cout<<"Fail to Fine Focus: Dalsa image is NULL"<<endl;
				return;
			}
			
			read_write_locker.lockForRead();
			int rows = globalDalsaImage.rows;
		    int cols = globalDalsaImage.cols;
		    src = cv::Mat(rows,cols,CV_8UC1,(void*)globalDalsaImage.data);
            read_write_locker.unlock();

            y[imageNum] = Depth(src, "Tenengrad");
            cout<<"Pic: "<<imageNum<<", depth: "<<y[imageNum]<<endl;

			//Record the max index
			if (imageNum == 0){ maxValue = y[0]; }
			else{
				if (maxValue < y[imageNum]){
					maxValue = y[imageNum];
					maxIndex = imageNum;
				}
			}
			++imageNum;
			if (i == fine_motion_times-1){ break; }
			stage->Move_Openloop_Unrealtime(step);
		}
		//ShowState("Fine Focus: Getting Images Finish");
		cout<<"Fine Focus: Getting Images Finish"<<endl;

		//Move to the desire position
		currentPoint = stage->Get_CurrentPosition();
		step = x[maxIndex] - currentPoint;
		stage->Move_Closeloop_Unrealtime(step);
	} catch(QException e){
		cout<<e.getMessage()<<endl;
		return;
	}
	//ShowState("Fine Focus: Finish");
	cout<<"Fine Focus: Finish"<<endl;
}

void* Z2_AutoFocusDialog::Z2_QuickFocusThread(void* arg)
{
	Z2_AutoFocusDialog* dialog = (Z2_AutoFocusDialog*) arg;
	dialog->Z2_QuickFocus();
	return NULL;
}

void Z2_AutoFocusDialog::OnQuickFocusButton()
{
	if (stage==NULL || !stage->IsConnected()){ return; }
	int ret = QMessageBox::warning(this, "Warning", "Are you sure to Quick Focus?", QMessageBox::Ok | QMessageBox::Cancel);
	if (ret != QMessageBox::Ok)
		return;

	if (!stage->InOrigin()){
		QMessageBox::warning(this, "Warning", "Current position is not origin. Please Return Origin.");
		return;
	}

	int err,oldState;
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	//设置线程为可分离状态，当线程结束时，操作系统回收资源
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    //设置线程的取消选项
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldState);
    err = pthread_create( &quickFocusThread, NULL, Z2_QuickFocusThread, (void *)this);
    if (err != 0 ){
       ShowState("QuickFocus: fail to create thread");
       return;
    }
    pthread_attr_destroy(&attr);
}

void Z2_AutoFocusDialog::OnQuitQuickFocusButton()
{
	// terminate quick focus thread
}

void Z2_AutoFocusDialog::Z2_QuickFocus()
{
	// Check current position whether it's origin
	try{
		
		/********* Strat Coarse Focus *********/
		//ShowState("Coarse Focus: Start");
		cout<<"Coarse Focus: Start"<<endl;

		int motion_times = int(1.0*Z2_STAGE::Z2_FOCUS_REFERENCE_POSITION/Z2_STAGE::Z2_COARSE_FOCUS_STEP);
		double currentPoint, step, fine_step, maxValue, x[IMAGE_MAX], y[IMAGE_MAX];
		int imageNum = 0, maxIndex = 0;

		//Acquring images and calulating image evaluations
		//ShowState("Coarse Focus: Getting Images Start");
		step = 1.0*Z2_STAGE::Z2_COARSE_FOCUS_STEP/Z2_STAGE::Z2_PRECISION; //convert to pulse
		cv::Mat src;

		for (int i=0; i<motion_times; ++i){
			currentPoint = stage->Get_CurrentPosition();
			x[imageNum] = currentPoint;

			//Get the latest image
			if (globalDalsaImage.data == NULL){
				cout<<"Fail to Quick Focus(coarse): Dalsa image is NULL"<<endl;
				return;
			}
			
			read_write_locker.lockForRead();
			int rows = globalDalsaImage.rows;
		    int cols = globalDalsaImage.cols;
		    src = cv::Mat(rows,cols,CV_8UC1,(void*)globalDalsaImage.data);
            read_write_locker.unlock();

            y[imageNum] = Depth(src, "Tenengrad");

			//Record the max index
			if (imageNum == 0){ maxValue = y[0]; }
			else{
				if (maxValue < y[imageNum]){
					maxValue = y[imageNum];
					maxIndex = imageNum;
				}
			}
			++imageNum;
			if (i == motion_times-1){ break; }
			stage->Move_Openloop_Unrealtime(step);
		}
		//ShowState("Coarse Focus: Getting Images Start");
		//ShowState("Coarse Focus: Finish\n\n");
		cout<<"Coarse Focus: Finish"<<endl;

		//Move to the down position for fine focus
		fine_step = 1.0*Z2_STAGE::Z2_FINE_FOCUS_STEP/Z2_STAGE::Z2_PRECISION;//convert to pulse
		currentPoint = stage->Get_CurrentPosition();
		step = x[maxIndex] - currentPoint - fine_step*(Z2_STAGE::Z2_FINE_FOCUS_TIMES/2);
		stage->Move_Openloop_Unrealtime(step);
		
		/********* Strat Fine Focus *********/
		//ShowState("Fine Focus: Start");
		cout<<"Fine Focus: Start"<<endl;

		//Acquring images and calulating image evaluations
		//ShowState("Fine Focus: Getting Images Start");
		step = fine_step; //convert to pulse
		imageNum = 0, maxIndex = 0;
		for (int i=0; i<Z2_STAGE::Z2_FINE_FOCUS_TIMES; ++i){
			currentPoint = stage->Get_CurrentPosition();
			x[imageNum] = currentPoint;

			//Get the latest image
			if (globalDalsaImage.data == NULL){
				cout<<"Fail to Quick Focus(fine):   Dalsa image is NULL"<<endl;
				return;
			}

			read_write_locker.lockForRead();
			int rows = globalDalsaImage.rows;
		    int cols = globalDalsaImage.cols;
		    src = cv::Mat(rows,cols,CV_8UC1,(void*)globalDalsaImage.data);
            read_write_locker.unlock();

            y[imageNum] = Depth(src, "Tenengrad");

			//Record the max index
			if (imageNum == 0){ maxValue = y[0]; }
			else{
				if (maxValue < y[imageNum]){
					maxValue = y[imageNum];
					maxIndex = imageNum;
				}
			}
			++imageNum;
			if (i == Z2_STAGE::Z2_FINE_FOCUS_TIMES-1){ break; }
			stage->Move_Openloop_Unrealtime(step);
		}
		//ShowState("Coarse Focus: Getting Images Start");

		//Move to the focus plane
		currentPoint = stage->Get_CurrentPosition();
		step = x[maxIndex] - currentPoint;
		stage->Move_Closeloop_Unrealtime(step);

		//ShowState("Fine Focus: Finish");
		cout<<"Fine Focus: Finish"<<endl;
	} catch (QException e){
		cout<<e.getMessage()<<endl;
	}
}

