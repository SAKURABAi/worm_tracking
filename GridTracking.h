/****************************************************************************
	GridTracking: 
****************************************************************************/

#ifndef _GRID_TRACKING_H_
#define _GRID_TRACKING_H_

#include "GlobalParameters.h"
#include "TranslationStage.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QPaintEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

#include <QtCore/QString>

#define ARROW_BUTTON_WIDTH 50
#define ARROW_BUTTON_HEIGHT 50
enum RefreshGridState{REFRESH_BLOCK, REFRESH_POSITION};

/*********************************************************
	GridWidget Object
*********************************************************/
class GridWidget : public QWidget
{
	Q_OBJECT
public:
	static int DISPLAYREGION_WIDTH;
	static int DISPLAYREGION_HEIGHT;
	static string OBJECT_NAME;

	explicit GridWidget(QWidget *parent=0);
	QSize sizeHint() const;

	inline void setBlank(bool ok){ isBlank = ok; }

	void paintDisk();
	void paintGrid();
	void clearGrid();
	void paintCurrentBlock(int row, int col);
	void paintCurrentPosition(int x, int y);
	void getGridInfo();
	bool loadGrid();
	void initializeGrid();
	
signals:
    void FinishMouseClickSignal(int row, int col);

public slots:
    void StartMouseClick();
    
protected:
	void paintBackground();
	void paintEvent(QPaintEvent * event);
    virtual void mousePressEvent(QMouseEvent * event);
    
private:
	QImage *currentImage;//动态位置
	QImage *basicImage;
    QImage *gridImage;   
    QImage *gridStateImage;

	UINT32 arrow_length;
	UINT32 margin;
    bool isBlank;
    bool isReadyForMouseResponse;
    QColor currentRectColor;
};

/*********************************************************
	GridTracking Object
*********************************************************/
class GridTracking : public QObject
{
	Q_OBJECT
public:
	static string OBJECT_NAME;

	explicit GridTracking(QWidget* parent=0);
	~GridTracking();

	//Create FullFov Search Grid Groupbox
	void Create_FullFovSearchGridGroup();
	
	inline QGroupBox *Get_FullFovSearchGridGroup() const {
    	return fullFovSearchGridGroup;
	}
	inline void Set_FullFovSearchGridGroupTitle(const QString &title){
    	fullFovSearchGridGroup->setTitle(title);
    }

	//Grid GroupBox
	void Create_GridGroup();
	
	inline QGroupBox *Get_GridGroup() const {
    	return gridGroup;
	}
	inline void Set_GridGroupTitle(const QString &title){
		gridGroup->setTitle(title);
	}

signals:
    void StartMouseClickSignal();
    void ReturnOriginSignal();
	void SendState(string state);//send signal to main window

public slots:
    void refreshCurrentPosition();
    
    void FinishMouseClick(int row, int col);
    void Controller1_Connected();
    void Controller1_Disconnected();
    
protected:
	void CreateArrowButtonsAndSetStyle();
	
	void refreshGrid();
	bool HaveBordersInGridInfo();
	void FindCurrentPosition();
	void FindNearBlockAndMotionSteps(double &x,double &y);
	
protected slots:
	//FullFOV Search Grid Group slots
    void OnUpButton();
    void OnDownButton();
    void OnLeftButton();
    void OnRightButton();
    void OnGoNearBlockButton();
    void OnMouseClickButton();

	//Refresh and Save Drid slots
    void OnRefreshGridButton();
    void OnSaveGridButton();   
    void OnReturnOriginButton();
    
private:
	//FullFOV Search Grid Group
	QGroupBox *fullFovSearchGridGroup;

	QToolButton *upButton;
    QToolButton *downButton;
    QToolButton *leftButton;
    QToolButton *rightButton;
    QPushButton *goNearBlock;
    QPushButton *mouseClickButton;

	//Divide and Display Grid
	QGroupBox *gridGroup;
	GridWidget *grid;
    QPushButton *refreshGridButton;
    QPushButton *saveGridButton;  
    QPushButton* returnOriginButton;
    
	//Translation stage object
	TranslationStage *translationStage;

	RefreshGridState refreshGridState;
	int current_row;//Current Grid state
	int current_col;
	long current_x; //Current stage position
	long current_y;
};

#endif
