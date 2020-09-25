
/****************************************************************************
    TrackingSystem : 追踪系统
****************************************************************************/
#ifndef _TRACKING_SYSTEM_H_
#define _TRACKING_SYSTEM_H_

#include "AboutDialog.h"
#include "AutoFocusSetting.h"
#include "GridTracking.h"
#include "ROISetting.h"
#include "DeviceParametersDialog.h"
#include "ImageSaveSetting.h"
#include "TrackingControl.h"
#include "TranslationStageControl.h"
#include "DisplayWindow.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QFrame>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenuBar>

#include <QtGui/QCloseEvent>
    
class TrackingWindow : public QMainWindow
{
    Q_OBJECT
public:
	static string OBJECT_NAME;

	explicit TrackingWindow(QWidget *parent=0, Qt::WindowFlags flags=0);
	~TrackingWindow();
	
    void DockDisplayWindows();//定位Displayindow和Analyseindow的初始位置
    
public slots:
	void ShowState(string state){
		stateDisplayEdit->append(QString::fromStdString(state));
	}
    void SetStageParameters();
    void DisplayImage();
    void AnalyseImage();
    
protected:
    void CreateLayout();
	QFrame* CreateLeftPanel();
    QFrame* CreateRightpanel();
	void CreateMenus();
	virtual void closeEvent(QCloseEvent *event);
	
protected slots:

private:    
	QMenu *fileMenu;
    QMenu *settingMenu;
    QMenu *helpMenu;
    QAction *exitAction;
    QAction *aboutAction;
    QAction *configAction;
    QAction *stageParamsAction;
    
    DisplayWindow *displayWindow;
    DisplayWindow *analyseWindow;
    QFrame *leftPanel;
    QFrame *rightPanel;
    
    AutoFocusSetting *autoFocusSetting;
    ImageSaveSetting *imageSaveSetting;
    TranslationStageControl *translationStageControl;
    GridTracking *gridTracking;
    ROISetting *roiSetting;
    TrackingControl* trackingControl;
    DeviceParametersDialog *stageParamsDialog;
    
    QTextEdit *stateDisplayEdit;
    
    //Devices: Dalsa Camera   
    DalsaCamera *dalsaCamera;
};

#endif
