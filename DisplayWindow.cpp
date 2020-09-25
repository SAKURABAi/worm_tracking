
#include "DisplayWindow.h"
#include <QtWidgets/QStatusBar>
#include <QtGui/QColor>
#include <QtCore/QRect>
#include <QtCore/QPoint>

DisplayWindow::DisplayWindow(bool statusBarEnabled, QWidget *parent) : QMainWindow(parent)
{
    this->statusBarEnabled = statusBarEnabled;
    displayImageData.data = NULL;
    isStartMouseResponse = false;
    isDrawCrossLine = false;
    isMouseMoveEnabled = false;

    LastImageWidth = 0;
    LastImageHeight = 0;

    DisplayWidth = DISPLAY_WIDTH;
    DisplayHeight = DISPLAY_HEIGHT;
    globalImageSize.displayWidth = DisplayWidth;
    globalImageSize.displayHeight = DisplayHeight;
	
	contextMenu = NULL;
    glWidget = new GLWidget(this);
    if (statusBarEnabled){
        QStatusBar* statusBar = this->statusBar();
        statusBar->showMessage("(NA, NA), Data: NA");
    }

    setCentralWidget(glWidget);
    resize(DisplayWidth, DisplayHeight);
}

void DisplayWindow::SetMouseMoveEnabled(bool ok){
    isMouseMoveEnabled = ok;
    if (isMouseMoveEnabled && glWidget!=NULL){
        glWidget->setMouseTracking(true);
        this->setMouseTracking(true);
    }
}

void DisplayWindow::OnCrossLineAction()
{
    if (isDrawCrossLine){
        isDrawCrossLine = false;
    }else{
        isDrawCrossLine = true;
    }
    glWidget->SetIsDrawCrossLine(isDrawCrossLine);
}

void DisplayWindow::SetContextMenuEnabled(bool ok)
{
    contextMenuEnabled = ok;
    if (ok){
        setContextMenuPolicy(Qt::CustomContextMenu);
        QObject::connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
                         this, SLOT(ShowContextMenu()));
    }
}

void DisplayWindow::ShowContextMenu()
{
    if (contextMenuEnabled){
        if (contextMenu != NULL){
            delete contextMenu;
            contextMenu = NULL;
        }
        contextMenu = new QMenu(this);
        QAction *crosslineAction;
        
        if (isDrawCrossLine){
            crosslineAction = contextMenu->addAction( tr("Clear CrossLine") );
        }
        else{
            crosslineAction = contextMenu->addAction( tr("Draw CrossLine") );
        }
        QObject::connect( crosslineAction, SIGNAL( triggered(bool) ), this, SLOT( OnCrossLineAction() ) );
        contextMenu->exec(QCursor::pos());
    }
}

void DisplayWindow::ShowImage(void* image_data, int width, int height)
{
    if (image_data == NULL){ 
        cout<<"DisplayWindow: DisplayImage is NULL"<<endl;
        return; 
    }
    
    // Update display image data
    displayImageData.rows = height;
    displayImageData.cols = width;
    displayImageData.data = (uchar*)image_data;

    if (LastImageWidth != width || LastImageHeight != height){
        int display_width = DisplayWidth, display_height = DisplayHeight;
        int xDisplayOffset = 0, yDisplayOffset = 0;
        
        if (width*DisplayHeight > DisplayWidth*height){
            display_width = DisplayWidth;
    		display_height = DisplayWidth * height/width;
    		xDisplayOffset = 0;
    		yDisplayOffset = (DisplayHeight - display_height)/2; //实际图像在显示窗口中的偏移量
    	}
    	else{
    		display_width = width*DisplayHeight/height;
    		display_height = DisplayHeight;
    		xDisplayOffset = (DisplayWidth - display_width)/2; //实际图像在显示窗口中的偏移量
    		yDisplayOffset = 0;
    	}
    	//cout<<"image height: "<<image_height<<", image width: "<<image_width<<endl;
        //glWidget->resize(display_width,display_height);
        if ((LastImageWidth <= 512 && LastImageHeight <= 512) &&
            (width == DALSACEMERA::FULLIMAGE_WIDTH && height == DALSACEMERA::FULLIMAGE_HEIGHT)){
            glWidget->ShowImage(image_data, width, height);
            setContentsMargins(xDisplayOffset, yDisplayOffset, xDisplayOffset, yDisplayOffset);
        }
        else{
            setContentsMargins(xDisplayOffset, yDisplayOffset, xDisplayOffset, yDisplayOffset);
            glWidget->ShowImage(image_data, width, height);
        }

        LastImageHeight = height;
        LastImageWidth = width;
    }
    else{
        glWidget->ShowImage(image_data, width, height);
    }
}

void DisplayWindow::StartMouseResponse()
{
    isStartMouseResponse = true;
    glWidget->setCursor(Qt::PointingHandCursor);
}

void DisplayWindow::mousePressEvent(QMouseEvent * event)
{
    if (isStartMouseResponse){
        QPoint point = event->pos();
               
        //获取需要设置的roi大小，来自与全局变量imageSize
        globalImageSize.x_position = point.x();
        globalImageSize.y_position = point.y();
        //cout << "x_position：" << imageSize.x_position << ", y_position：" << imageSize.y_position << endl;
        
        isStartMouseResponse = false;
        glWidget->setCursor(Qt::ArrowCursor);
        emit FinishMouseResponseSignal();//send signal to roiSetting
    }
    else {
        event->ignore();
    }
}

void DisplayWindow::mouseMoveEvent(QMouseEvent * event)
{
    if (isMouseMoveEnabled && !IS_TRACKING && !isStartMouseResponse){
        QPoint point = event->pos();
        QPoint imagePoint = glWidget->mapFromParent(point);
        //cout<<"image point x:"<<imagePoint.x()<<", image point y:"<<imagePoint.y()<<endl;
        //QRect glWidgetRect = glWidget->frameGeometry();
        //cout<<"1: glwidget width:"<<glWidgetRect.width()<<", glwidget height:"<<glWidgetRect.height()<<endl;
        //cout<<"2: glwidget width:"<<glWidget->width()<<", glwidget height:"<<glWidget->height()<<endl;
        int glWidgetWidth = glWidget->width();
        int glWidgetHeight = glWidget->height();
        int x_index = int((1.0*imagePoint.x()/glWidgetWidth)*displayImageData.cols);
        int y_index = int((1.0*imagePoint.y()/glWidgetHeight)*displayImageData.rows);

        x_index = (x_index >= displayImageData.cols ? (displayImageData.cols) : x_index);
        y_index = (y_index >= displayImageData.rows ? (displayImageData.rows) : y_index);

        // Get current image data at the point
        if (displayImageData.data != NULL){
            int data = displayImageData.data[y_index*displayImageData.cols + x_index];
            // Display corresponding image coordinates and data
            QString status = tr("(") + QString::number(x_index) + tr(", ") + QString::number(y_index) +
            tr("), Data: ") + QString::number(data);
            if (statusBarEnabled ){
                statusBar()->showMessage(status);
            }
        }
    }
    else{
        event->ignore();
    }
}

void DisplayWindow::resizeEvent(QResizeEvent *event)
{
    QSize size = event->size();
    int height = size.height();
    int width = size.width();
    DisplayWidth = width;
    DisplayHeight = height;
	
	event->accept();
}
