/****************************************************************************
	Z2_AutoFocus: Z2轴AutoFocus
****************************************************************************/

#ifndef _Z2_AUTOFOCUS_H_
#define _Z2_AUTOFOCUS_H_

#include "Z2Stage.h"
#include "GlobalParameters.h"
#include "DistanceSlider.h"
#include "MotionThread.h"

#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QAction>
#include <QtCore/QThread>

#include <pthread.h>

class Z2_AutoFocusDialog : public QDialog
{
	Q_OBJECT	
public:
	static string OBJECT_NAME;

	explicit Z2_AutoFocusDialog(QWidget *parent=0, Qt::WindowFlags flags=0);
	~Z2_AutoFocusDialog();
	
public slots:
	void OnFinishMotion();
	void OnFinishReturnOrigin(int);
	
protected slots:
	void OnMoveUpButton();       //向上移动Z2轴
	void OnMoveDownButton();     //向下移动Z2轴
	void OnSetUpPointButton();   //设置上部(Z2轴)的位置
	void OnSetDownPointButton(); //设置下部(Z2轴)的位置
	void OnReturnOriginButton(); //回零
	
	void OnStopButton();
	void OnClearStateAction();
	void OnSaveStateAction();
	
	void OnCoarseFocusButton();//粗调焦
	void OnFineFocusButton();  //精确调焦
	
	void OnQuickFocusButton();
	void OnQuitQuickFocusButton();

protected:
	void CreateLayout();
	void CreateActions();
	void InitialDock();//设置窗口初始停靠位置

	void ShowState(QString);
	void DisplaySliderState();

	void Get_Z2Step();
	void Get_CoarseTimes();
	void Get_FineTimes();
	static void* Z2_CoarseFocusThread(void* arg);
	static void* Z2_FineFocusThread(void* arg);
	static void* Z2_QuickFocusThread(void* arg);
	void Z2_CoarseFocus();
	void Z2_FineFocus();
	void Z2_QuickFocus();

private:
	//coarse focus group
	QGroupBox *coarseFocusGroup;
	QSpinBox *coarseFocusTimesEdit;
	QPushButton *setUpPointButton;
	QPushButton *setDownPointButton;
	QPushButton *coarseFocusButton;
	
	//fine focus group
	QGroupBox *fineFocusGroup;
	QSpinBox *fineFocusTimesEdit;
	QPushButton *fineFocusButton;
	
	//current position group
	QGroupBox *currentPositionGroup;
	DistanceSlider *positionSlider;
	QLabel *positionLabel;

    //focus initial setting group
	QGroupBox *initialStateSetting;
	QLineEdit* z2StepEdit;
	QPushButton *moveUpButton;
	QPushButton *moveDownButton;
	QPushButton *originReturnButton;
	QPushButton *stopButton;

	//quick focus group
	QGroupBox *quickFocusGroup;
	QPushButton *quickFocusButton;
	QPushButton *quitQuickFocusButton;

	QTextEdit *stateTextEdit;
	
	//conetext menu
	QAction *saveStateAction;
	QAction *clearStateAction;
	
	//controlling parameters (unit: pulse)
	double z2_step;
	long coarse_up_point;
	long coarse_down_point;
	int coarse_motion_times;
	double coarse_focus_point;//粗调时的聚焦位置
	
	int fine_motion_times;
	double fine_focus_point;  //精调时的聚焦位置
	
	Galil* controller2;
	MotionThread* motionThread;
	Z2Stage *stage;
	pthread_t coarseFocusThread;
	pthread_t fineFocusThread;
	pthread_t quickFocusThread;
};

#endif
