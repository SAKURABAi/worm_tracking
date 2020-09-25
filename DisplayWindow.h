/****************************************************************************
    DisplayWindow 用于显示图像以及响应鼠标选择线虫
****************************************************************************/
#ifndef _DISPLAY_WINDOW_H_
#define _DISPLAY_WINDOW_H_

#include "GlobalParameters.h"
#include "GLWidget.h"

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#include <QtWidgets/QLabel>
#include <QtGui/QResizeEvent>
#include <QtGui/QMouseEvent>
#include <QtCore/QString>

#define DISPLAY_WIDTH  800
#define DISPLAY_HEIGHT 600
class DisplayWindow : public QMainWindow
{
    Q_OBJECT
public:
    DisplayWindow(bool statusBarEnabled, QWidget *parent=0);

    void ShowImage(void* image_data, int width, int height);
	void SetContextMenuEnabled(bool ok);
    void SetMouseMoveEnabled(bool ok);
	inline void SetTitle(const QString &title){
    	setWindowTitle(title);
	}
	
signals:
    void FinishMouseResponseSignal();
    
public slots:
    void StartMouseResponse();
    void OnCrossLineAction();
    void ShowContextMenu();
    
protected:  
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseMoveEvent(QMouseEvent * event);
    virtual void resizeEvent(QResizeEvent * event);
    
private:
    GLWidget *glWidget;//显示图像

    QMenu *contextMenu;
    bool isStartMouseResponse;//是否开启鼠标响应
    bool isDrawCrossLine;
    bool isMouseMoveEnabled;
    bool contextMenuEnabled;
    bool statusBarEnabled;
    int DisplayWidth, DisplayHeight;
    int LastImageWidth, LastImageHeight;
    DalsaImage displayImageData;
};

#endif
