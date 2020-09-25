
/****************************************************************************
	TranslationStage_Control: 
****************************************************************************/

#ifndef _TRANSLATION_STAGE_CONTROL_H_
#define _TRANSLATION_STAGE_CONTROL_H_

#include "TranslationStage.h"
#include "RotationStage.h"
#include "GlobalParameters.h"
#include "DevicePackage.h"
#include "MotionThread.h"

#include <QtCore/QObject>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QToolButton>

#define ARROW_BUTTON_WIDTH 50
#define ARROW_BUTTON_HEIGHT 50

class TranslationStageControl : public QObject
{
	Q_OBJECT
public:
	static string OBJECT_NAME;

	TranslationStageControl(QObject* parent=0);
	~TranslationStageControl();

	/****** Stage Connection Group ******/
	void Create_StageConnectionGroup();
	
	inline QGroupBox* Get_StageConnectionGroup() const {
    	return stageConnectionGroup; 
	}
	inline void Set_StageConnectionGroupTitle(const QString &title){
		stageConnectionGroup->setTitle(title);
	}

	/****** Stage Motion Group ******/
	void Create_StageMotionGroup();
	
	inline QGroupBox* Get_StageMotionGroup() const {
    	return stageMotionGroup;
	}
	inline void Set_StageMotionGroupTitle(const QString &title){
		stageMotionGroup->setTitle(title);
	}

signals:
    void SendState(string state);//向主界面发送当前状态
    void Broadcast_Controller_Connected();
    void Broadcast_Controller_Disconnected();
    void RefreshCurrentPositionSignal();

protected:    
	/****** Stage Motion Group ******/
	void BorderSettingEnabled(bool ok);
	void CreateArrowButtonsAndSetStyle();
	double Get_RotationDegree();
    double Get_MotionStep();

protected slots:
	/****** Stage Connection Group Slots******/
	void OnController_ConnectButton();
    void OnController_DisconnectButton();

    void OnReturnOrigin();
    void OnMotionFinish();
    void OnReturnOriginFinish(int method);

    /****** Stage Motion Group Slots******/
	void OnUpButton();
    void OnDownButton();
    void OnLeftButton();
    void OnRightButton();
    void OnStopButton();
    void OnClockwiseButton();
    void OnAnticlockwiseButton();

    void OnLockBorderSettingButton();
    void OnSetUpBorderButton();
    void OnSetDownBorderButton();
    void OnSetLeftBorderButton();
    void OnSetRightBorderButton();

private:
	/****** Stage Connection Group ******/
	QGroupBox *stageConnectionGroup;
    QLineEdit *controller_AddressEdit;
    QPushButton *controller_connectButton;
	QPushButton *controller_disconnectButton;

	string controller_address;
	/****** Stage Motion Group ******/
	QGroupBox *stageMotionGroup;

	QToolButton *upButton;
    QToolButton *downButton;
    QToolButton *leftButton;
    QToolButton *rightButton;
    QPushButton *stopButton;
    QDoubleSpinBox *motionStepEdit;

	QToolButton *clockwiseButton;
	QToolButton *anticlockwiseButton;
	QDoubleSpinBox *rotationDegreeEdit;

    QPushButton *borderSettingLockerButton;
    QPushButton *setUpBorderButton;
    QPushButton *setDownBorderButton;
    QPushButton *setLeftBorderButton;
    QPushButton *setRightBorderButton;
    
	long upBorder;
	long downBorder;
	long leftBorder;
	long rightBorder;
	double rotationDegree;

	MotionThread* x_MotionThread;
	MotionThread* y_MotionThread;
	TranslationStage *translationStage;
	RotationStage *rotationStage;
};

#endif
