
#include "GridTracking.h"
#include "DevicePackage.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QMessageBox>

using namespace VIRTUAL_COORDINATE;

int GridWidget::DISPLAYREGION_WIDTH = 400;
int GridWidget::DISPLAYREGION_HEIGHT = 400;
string GridWidget::OBJECT_NAME = "GridWidget";

/*********************************************************
	GridWidget Object
*********************************************************/
GridWidget::GridWidget(QWidget *parent):QWidget(parent)
{
    //Initializing each image layer and fill them up transparent
    basicImage = new QImage(DISPLAYREGION_WIDTH, DISPLAYREGION_HEIGHT, QImage::Format_ARGB32_Premultiplied);
    gridImage = new QImage(DISPLAYREGION_WIDTH, DISPLAYREGION_HEIGHT, QImage::Format_ARGB32_Premultiplied);
    currentImage = new QImage(DISPLAYREGION_WIDTH, DISPLAYREGION_HEIGHT, QImage::Format_ARGB32_Premultiplied);
    gridStateImage = new QImage(DISPLAYREGION_WIDTH, DISPLAYREGION_HEIGHT, QImage::Format_ARGB32_Premultiplied);
	basicImage->fill(Qt::transparent);
	gridImage->fill(Qt::transparent);
	currentImage->fill(Qt::transparent);
	gridStateImage->fill(Qt::transparent);
	
	arrow_length = 6;
	margin = 20;
	currentRectColor = Qt::darkBlue;
	currentRectColor.setAlpha(150);
	isBlank = true;
	isReadyForMouseResponse = false;

	//Write some information into gridInfo struct
	globalGridInfo.totalWidth = DISPLAYREGION_WIDTH - arrow_length - 2*margin - 1;
    globalGridInfo.totalHeight = DISPLAYREGION_HEIGHT - arrow_length - 2*margin - 1;

	setAttribute(Qt::WA_StaticContents);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	paintBackground();
}

QSize GridWidget::sizeHint() const
{
	return QSize(DISPLAYREGION_WIDTH, DISPLAYREGION_HEIGHT);
}

void GridWidget::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);
	painter.eraseRect(this->rect());
	
	if (isBlank){
    	initializeGrid();
		painter.drawImage(0, 0, *basicImage);
        painter.drawImage(0, 0, *gridStateImage);
        painter.drawImage(0, 0, *gridImage);
        return;
	}
	painter.drawImage(0, 0, *basicImage);
    painter.drawImage(0, 0, *gridImage);
    painter.drawImage(0, 0, *currentImage);
}

void GridWidget::paintBackground()
{
	QPainter painter(basicImage);
	painter.fillRect(basicImage->rect(), Qt::transparent);
    painter.fillRect(QRect(1,1,DISPLAYREGION_WIDTH-1,DISPLAYREGION_HEIGHT-1),Qt::white);
    painter.setPen(palette().mid().color());
    painter.drawRect(QRect(0,0,DISPLAYREGION_WIDTH-1, DISPLAYREGION_HEIGHT-1));

    /*draw axis in basic image*/
    painter.setPen(Qt::black);
    int start_x = 1, start_y = 1, end_x = 1, end_y = 1;
    //paint horizontal and vertical lines
    start_x = start_x + arrow_length/2;
	start_y = start_y + arrow_length/2;
	end_x = start_x;
	end_y = DISPLAYREGION_HEIGHT - 2;
	painter.drawLine(start_x, start_y, end_x, end_y);
	end_x = DISPLAYREGION_WIDTH - 2;
	end_y = start_y;
	painter.drawLine(start_x, start_y, end_x, end_y);
	
	//draw arrows
	start_x = start_y = 1;
	start_x += arrow_length/2;
	start_y += DISPLAYREGION_HEIGHT - 2;
	end_x = start_x - arrow_length/2;
	end_y = start_y - int(arrow_length * sqrt(3) / 2);
	painter.drawLine(start_x, start_y, end_x, end_y);
	end_x = start_x + arrow_length/2;
	painter.drawLine(start_x, start_y, end_x, end_y);
	painter.drawText(start_x + 5, end_y, tr("M") );

	start_x = start_y = 1;
	start_x += DISPLAYREGION_WIDTH - 2;
	start_y += arrow_length/2;
	end_x = start_x - int(arrow_length * sqrt(3) / 2);
	end_y = start_y - arrow_length/2;
	painter.drawLine(start_x, start_y, end_x, end_y);
	end_y = start_y + arrow_length/2;
	painter.drawLine(start_x, start_y, end_x, end_y);
	painter.drawText(end_x-5, start_y + 15, tr("N") );
}

void GridWidget::paintDisk()
{
    QPainter painter(gridImage);
	painter.setPen(palette().foreground().color());
    
    int circle_x = 0, circle_y = 0, radius_x = 0, radius_y = 0;
    int offset_x = arrow_length + margin + 1;
    int offset_y = arrow_length + margin + 1;
    //注 x,y在Qt坐标中
	double x = abs(globalGridInfo.rightBorder - globalGridInfo.leftBorder) * globalGridInfo.colPulseToGridDistance * 0.5;
	x += (globalGridInfo.gridCols-2)*0.5;//矩形框占据一个像素
    radius_x = int(x);
	double y = abs(globalGridInfo.topBorder - globalGridInfo.bottomBorder) * globalGridInfo.rowPulseToGridDistance * 0.5;
	y += (globalGridInfo.gridRows-2)*0.5;//矩形框占据一个像素
	radius_y = int(y);
    
    circle_x = radius_x + offset_x;
    circle_y = radius_y + offset_y;
    QPoint circle_point = QPoint(circle_x, circle_y);
    painter.drawEllipse(circle_point, radius_x, radius_y);
}

void GridWidget::paintGrid()
{
	QPainter painter(gridImage);
	painter.setPen(palette().foreground().color());
    
    //draw grids and texts
    int offset_x = arrow_length + margin + 1;
    int offset_y = arrow_length + margin + 1;
    int fullFovWidth = globalGridInfo.fullFovWidth;
    int fullFovHeight = globalGridInfo.fullFovHeight;
    int gridCols = globalGridInfo.gridCols;
    int gridRows = globalGridInfo.gridRows;
    
    int start_x = 0, start_y = 0, end_x = 0, end_y = 0;   
    for(int i = 0; i <= gridCols; ++i){
		start_x = i*(fullFovWidth + 1) + offset_x;
		start_y = offset_y;
		end_x = start_x;
		end_y = gridRows * (fullFovHeight + 1) + offset_y;
		painter.drawLine(start_x, start_y, end_x, end_y);
	}
	for(int i = 0; i <= gridRows; ++i){
		start_x = offset_x;
		start_y = i*(fullFovHeight + 1) + offset_y;
		end_x = gridCols*(fullFovWidth + 1) + offset_x;
		end_y = start_y;
		painter.drawLine(start_x, start_y, end_x, end_y);
	}
	for(int i = 0; i < gridCols; ++i){
		start_x = i*(fullFovWidth + 1) + offset_x + fullFovWidth/2 - 1;
		start_y = offset_y - 5;
		painter.drawText(start_x, start_y, QString::number(i+1));
	}
	for(int i = 0; i < gridRows; ++i){
		start_x = offset_x - 15;
		start_y = i*(fullFovHeight + 1) + offset_y + fullFovHeight/2 + 8;
		painter.drawText(start_x, start_y, QString::number(i+1) );
	}
}

void GridWidget::clearGrid()
{
    gridImage->fill(Qt::transparent);
}

void GridWidget::paintCurrentBlock(int row, int col)
{
    QPainter painter(currentImage);
    currentImage->fill(Qt::transparent);
    painter.setPen(currentRectColor);
    
    int offset_x = 0, offset_y = 0, start_x = 0, start_y = 0;
    int width = globalGridInfo.fullFovWidth;
    int height = globalGridInfo.fullFovHeight;
    offset_x = arrow_length + margin + 1;//从(1,1)处开始绘图
    offset_y = arrow_length + margin + 1;
    start_x = (col-1)*(width+1) + offset_x + 1;//网格线的宽度为1
    start_y = (row-1)*(height+1) + offset_y + 1;
    painter.fillRect(start_x, start_y, width, height, currentRectColor);
}

void GridWidget::paintCurrentPosition(int x, int y)
{
    QPainter painter(currentImage);
    currentImage->fill(Qt::transparent);
    painter.setPen(currentRectColor);
    
    //读取全局变量imageSize获得需要显示的图像大小，在转化为合适的宽，高显示
    int height = int(globalImageSize.imageHeight*globalGridInfo.rowPixelToGridDistance);
    int width = int(globalImageSize.imageWidth*globalGridInfo.colPixelToGridDistance);
    
    int offset_x = arrow_length + margin;//从(1,1)处开始绘图
    int offset_y = arrow_length + margin;
    int start_x = x - width/2 + offset_x;
    int start_y = y - height/2 + offset_y;
    painter.fillRect(x+offset_x-1, y+offset_y-1, 2, 2, Qt::red);
    painter.fillRect(start_x, start_y, width, height, currentRectColor);//画已当前点为中心的矩形
}

void GridWidget::mousePressEvent(QMouseEvent * event)
{
	if (isReadyForMouseResponse){
		int offset_x = arrow_length + margin + 1; //网格开始坐标
		int offset_y = arrow_length + margin + 1;

		QPoint currentPoint = event->pos();
		int x = currentPoint.x();
		int y = currentPoint.y();
		int col = int(ceil((x - offset_x) / (globalGridInfo.fullFovWidth+1)));
		int row = int(ceil((y - offset_y) / (globalGridInfo.fullFovHeight+1)));
		if (row<0 || row>globalGridInfo.gridRows || col<0 || col>globalGridInfo.gridCols){
			return;
		}
		row += 1;
		col += 1;
		
		this->setCursor(Qt::ArrowCursor);
		isReadyForMouseResponse = false;//关闭鼠标响应
		emit FinishMouseClickSignal(row, col);
	}
	else{
		event->ignore();
	}
}

void GridWidget::StartMouseClick()
{
    isReadyForMouseResponse = true;//开启鼠标响应
	this->setCursor(Qt::PointingHandCursor);
}

void GridWidget::getGridInfo()
{
    if (globalGridInfo.hasBorders){
        double h_distance = abs(globalGridInfo.rightBorder - globalGridInfo.leftBorder);
        double v_distance = abs(globalGridInfo.bottomBorder - globalGridInfo.topBorder);
        //cout<<"h_distance: "<<h_distance<<", v_distance: "<<v_distance<<endl;
        //cout<<"grid height distance: "<<gridInfo.fullFovImageHeight_StageDistance<<", grid width distance: "<<gridInfo.fullFovImageWidth_StageDistance<<endl;
        
        globalGridInfo.gridRows = int( round(h_distance / globalGridInfo.fullFovImageHeight_StageDistance + 0.25) );
        globalGridInfo.gridCols = int( round(v_distance / globalGridInfo.fullFovImageWidth_StageDistance + 0.25) );
        //cout<<"rows:"<<gridInfo.gridRows<<",cols:"<<gridInfo.gridCols<<endl;
        
        globalGridInfo.fullFovWidth = globalGridInfo.totalWidth / globalGridInfo.gridCols;
        globalGridInfo.fullFovHeight = globalGridInfo.totalHeight / globalGridInfo.gridRows;
        
        globalGridInfo.rowPixelToGridDistance = 1.0*globalGridInfo.fullFovHeight / globalGridInfo.fullFovImageHeight;
        globalGridInfo.rowPulseToGridDistance = 1.0*globalGridInfo.fullFovHeight / globalGridInfo.fullFovImageHeight_StageDistance;
        
        globalGridInfo.colPixelToGridDistance= 1.0*globalGridInfo.fullFovWidth / globalGridInfo.fullFovImageWidth;
        globalGridInfo.colPulseToGridDistance = 1.0*globalGridInfo.fullFovWidth / globalGridInfo.fullFovImageWidth_StageDistance;

		globalGridInfo.hasRowsAndCols = true;
        globalGridInfo.hasFullFovWidthAndHeight = true;
    }
}

bool GridWidget::loadGrid()
{
    FILE *file;
    char filename[256];
    int flag;

	//Load grid info, there are up, down, left anf right border position corresponding to translational stage
	sprintf(filename, "config/grid.cfg");
	file = fopen(filename, "rt");
	if(file == NULL){
    	printf("%s", "Load Grid : cannot open the file");
		return false;
	}
	fscanf(file, "%d\r\n", &flag);
	globalGridInfo.hasBorders = flag>=1 ? true : false;
	if (globalGridInfo.hasBorders){
    	fscanf(file, "%ld\r\n", &globalGridInfo.topBorder);
    	fscanf(file, "%ld\r\n", &globalGridInfo.bottomBorder);
    	fscanf(file, "%ld\r\n", &globalGridInfo.leftBorder);
    	fscanf(file, "%ld\r\n", &globalGridInfo.rightBorder);
	}
	fclose(file);
	return true;
}

void GridWidget::initializeGrid()
{
	if (!loadGrid()){
		globalGridInfo.hasBorders = false;
	}
    if (globalGridInfo.hasBorders){        
        getGridInfo();
        isBlank = false;
        paintGrid();
        paintDisk();
    }
}

/*********************************************************
	GridTracking Object
*********************************************************/
string GridTracking::OBJECT_NAME = "GridTracking";

GridTracking::GridTracking(QWidget* parent):QObject(parent)
{
    translationStage = NULL;
	current_row = current_col = 0;
	refreshGridState = REFRESH_POSITION;
	
	Create_FullFovSearchGridGroup();
	Create_GridGroup();
	
	//Register signal and relative slots
	QObject::connect(this, SIGNAL(StartMouseClickSignal()), grid, SLOT(StartMouseClick()));
	QObject::connect(grid, SIGNAL(FinishMouseClickSignal(int,int)), this, SLOT(FinishMouseClick(int,int)));
}

GridTracking::~GridTracking()
{
	Controller1_Disconnected();
}

void GridTracking::Controller1_Connected()
{
    try{
        translationStage = new TranslationStage(controller1);
        translationStage->Connect();
    }catch (string e){
        emit SendState(e);
    }catch (QException e){
        emit SendState(e.getMessage());
    }
    cout<<"GridTracking connect stage"<<endl;
}

void GridTracking::Controller1_Disconnected()
{
	if (translationStage!=NULL){
    	delete translationStage;
    	translationStage = NULL;
	}
}
    
//Create FullFov Search Grid Groupbox
void GridTracking::Create_FullFovSearchGridGroup()
{
    fullFovSearchGridGroup = new QGroupBox;    
    goNearBlock = new QPushButton( tr("Go Near Block") );
    mouseClickButton = new QPushButton( tr("Click Block") );

    //set properties
    goNearBlock->setFixedWidth(120);
    mouseClickButton->setFixedWidth(120);

    //connect signals and slots
    QObject::connect( goNearBlock, SIGNAL( clicked() ), this, SLOT( OnGoNearBlockButton() ) );
    QObject::connect( mouseClickButton, SIGNAL( clicked() ), this, SLOT( OnMouseClickButton() ) );
    
    CreateArrowButtonsAndSetStyle();
    QGridLayout *gridLayout = new QGridLayout;//layout for arrow buttons
    gridLayout->addWidget(upButton, 0, 1);
    gridLayout->addWidget(leftButton, 1, 0);
    gridLayout->addWidget(rightButton, 1, 2);
    gridLayout->addWidget(downButton, 2, 1);
    gridLayout->setMargin(0);
    gridLayout->setSpacing(0);
    
    QVBoxLayout* gridButtonsLayout = new QVBoxLayout;
    gridButtonsLayout->addWidget(goNearBlock);
    gridButtonsLayout->addWidget(mouseClickButton);
    
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addLayout(gridLayout);
    hLayout->addLayout(gridButtonsLayout);
    hLayout->setContentsMargins(0,5,0,0);
    hLayout->setSpacing(0);
    
    fullFovSearchGridGroup->setLayout(hLayout);
}

void GridTracking::CreateArrowButtonsAndSetStyle()
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
    
    QObject::connect( upButton, SIGNAL( clicked() ), this, SLOT( OnUpButton() ) );
    QObject::connect( downButton, SIGNAL( clicked() ), this, SLOT( OnDownButton() ) );
    QObject::connect( leftButton, SIGNAL( clicked() ), this, SLOT( OnLeftButton() ) );
    QObject::connect( rightButton, SIGNAL( clicked() ), this, SLOT( OnRightButton() ) );
}

void GridTracking::Create_GridGroup()
{
	gridGroup = new QGroupBox;
	grid = new GridWidget();
	refreshGridButton = new QPushButton( tr("Refresh Grid") );
    saveGridButton = new QPushButton( tr("Save Grid") );
    returnOriginButton = new QPushButton( tr("Return Origin") );
    returnOriginButton->setMinimumHeight(60);
    
    QGridLayout *buttonsLayout = new QGridLayout;
    buttonsLayout->addWidget(refreshGridButton, 0, 0, 1, 1);
    buttonsLayout->addWidget(saveGridButton, 1, 0, 1, 1);
    buttonsLayout->addWidget(returnOriginButton, 0, 1, 2, 1);
    buttonsLayout->setContentsMargins(25,3,15,0);
    buttonsLayout->setVerticalSpacing(10);
    buttonsLayout->setHorizontalSpacing(60);

    //connect buttons to relative slots
    QObject::connect( refreshGridButton, SIGNAL( clicked() ), this, SLOT( OnRefreshGridButton() ) );
    QObject::connect( saveGridButton, SIGNAL( clicked() ), this, SLOT( OnSaveGridButton() ) );
    QObject::connect( returnOriginButton, SIGNAL( clicked() ), this, SLOT( OnReturnOriginButton() ) );
    
    QVBoxLayout *gridLayout = new QVBoxLayout;
    gridLayout->addWidget(grid);
    gridLayout->addLayout(buttonsLayout);
    gridLayout->addStretch();
    gridLayout->setContentsMargins(5,5,5,0);
    gridLayout->setSpacing(5);
    gridGroup->setLayout(gridLayout);
}

void GridTracking::OnUpButton()
{
	if (translationStage==NULL || !translationStage->IsConnected()){
		emit SendState(OBJECT_NAME + ": Translation Stage No Connection");
		return;
	}
	if (refreshGridState == REFRESH_BLOCK){
		if (!globalGridInfo.hasRowsAndCols){
			emit SendState(OBJECT_NAME + ": No Grid Information");
			return;
		}
		if (current_row == 1){ return; } else { --current_row; }
	}
	/*虚拟坐标系Dalsa CCD(X+) <-> Stage(X-), Dalsa CCD(Y+) <-> Stage(Y+)，
      当前位置为图像中心点出的位置，向上移动即对应Y轴向负方向运动*/
	double distance = globalGridInfo.fullFovImageHeight_StageDistance*(-STAGE_Y_POSITIVE);
	translationStage->Y_Move_Closeloop_Realtime(distance);
	
	refreshGrid();//刷新当前位置
}

void GridTracking::OnDownButton()
{
	if (translationStage==NULL || !translationStage->IsConnected()){
		emit SendState(OBJECT_NAME + ": Translation Stage No Connection");
		return;
	}
	if (refreshGridState == REFRESH_BLOCK){
		if (!globalGridInfo.hasRowsAndCols){
			emit SendState(OBJECT_NAME + ": No Grid Information");
			return;
		}
		if (current_row == globalGridInfo.gridRows){ return; } else { ++current_row; }
	}
	/*虚拟坐标系Dalsa CCD(X+) <-> Stage(X-), Dalsa CCD(Y+) <-> Stage(Y+)，
      当前位置为图像中心点出的位置，向下移动即对应Y轴向正方向运动*/
    double distance = globalGridInfo.fullFovImageHeight_StageDistance*(STAGE_Y_POSITIVE);
    translationStage->Y_Move_Closeloop_Realtime(distance);
    
    refreshGrid();//刷新当前位置
}

void GridTracking::OnLeftButton()
{
	if (translationStage==NULL || !translationStage->IsConnected()){
		emit SendState(OBJECT_NAME + ": Translation Stage No Connection");
		return;
	}
	if (refreshGridState == REFRESH_BLOCK){
		if (!globalGridInfo.hasRowsAndCols){
			emit SendState(OBJECT_NAME + ": No Grid Information");
			return;
		}
		if (current_col == 1){ return; } else { --current_col; }
	}
	/*虚拟坐标系Dalsa CCD(X+) <-> Stage(X-), Dalsa CCD(Y+) <-> Stage(Y+)，
      当前位置为图像中心点出的位置，向左移动即对应X轴向正方向运动*/
    double distance = globalGridInfo.fullFovImageWidth_StageDistance*(STAGE_X_POSITIVE);
	translationStage->X_Move_Closeloop_Realtime(distance);
	
	refreshGrid();//刷新当前位置
}

void GridTracking::OnRightButton()
{
	if (translationStage==NULL || !translationStage->IsConnected()){
		emit SendState(OBJECT_NAME + ": Translation Stage No Connection");
		return;
	}
	if (refreshGridState == REFRESH_BLOCK){
		if (!globalGridInfo.hasRowsAndCols){
			emit SendState(OBJECT_NAME + ": No Grid Information");
			return;
		}
		if (current_col == globalGridInfo.gridCols){ return; } else { ++current_col; }
	}
	/*虚拟坐标系Dalsa CCD(X+) <-> Stage(X-), Dalsa CCD(Y+) <-> Stage(Y+)，
      当前位置为图像中心点出的位置，向右移动即对应X轴向负方向运动*/
    double distance = globalGridInfo.fullFovImageWidth_StageDistance*(-STAGE_X_POSITIVE);
    translationStage->X_Move_Closeloop_Realtime(distance);
    
    refreshGrid();//刷新当前位置
}

void GridTracking::OnGoNearBlockButton()
{
	if (translationStage==NULL || !translationStage->IsConnected()){
		return;
	}
	refreshGridState = REFRESH_BLOCK;
	
	double x_pulse = 0, y_pulse = 0;
	FindNearBlockAndMotionSteps(x_pulse,y_pulse);
	translationStage->XY_Move_Closeloop_Realtime(x_pulse, y_pulse);
	
	grid->paintCurrentBlock(current_row, current_col);
	grid->update();
}

void GridTracking::OnMouseClickButton()
{
	mouseClickButton->setEnabled(false);
	emit StartMouseClickSignal();
}

void GridTracking::FinishMouseClick(int row, int col)
{
    mouseClickButton->setEnabled(true);
    if (translationStage==NULL || !translationStage->IsConnected()){
        return;
    }
    try {
        //虚拟坐标系Dalsa CCD(X+) <-> Stage(X-), Dalsa CCD(Y+) <-> Stage(Y+)，
        //网格向上运动对应于图像的向上运动(Dalsa CCD(Y-))，向右运动对应于图像的向右运动(Dalsa CCD(X+))
        //cout<<"current row:"<<current_row<<", current col:"<<current_col<<endl;
        //cout <<"row :"<<row<<", col :"<<col<<endl;
        double x_distance = (-STAGE_X_POSITIVE)*(1.0*col-1.0*current_col)*globalGridInfo.fullFovImageWidth_StageDistance;
        double y_distance = (STAGE_Y_POSITIVE)*(1.0*row-1.0*current_row)*globalGridInfo.fullFovImageHeight_StageDistance;
        translationStage->XY_Move_Closeloop_Realtime(x_distance,y_distance);
        
        //Refresh Block
        current_row = row;
        current_col = col;
        refreshGridState = REFRESH_BLOCK;
        grid->paintCurrentBlock(current_row, current_col);
    	grid->update();
    } catch (QException e){
        cout<<e.getMessage()<<endl;
    }
}

void GridTracking::OnRefreshGridButton()
{
    if (HaveBordersInGridInfo()){
        grid->clearGrid();
        globalGridInfo.hasBorders = true;
        grid->getGridInfo();
        grid->paintDisk();
        grid->paintGrid();
        grid->update();
    }
}

bool GridTracking::HaveBordersInGridInfo()
{
    if (globalGridInfo.topBorder == STAGE_INTIAL_VALUE){
        emit SendState("No Top Border");
        return false;
    }
    if (globalGridInfo.bottomBorder == STAGE_INTIAL_VALUE){
        emit SendState("No Bottom Border");
        return false;
    }
    if (globalGridInfo.leftBorder == STAGE_INTIAL_VALUE){
        emit SendState("No Left Border");
        return false;
    }
    if (globalGridInfo.rightBorder == STAGE_INTIAL_VALUE){
        emit SendState("No Right Border");
        return false;
    }
    return true;
}

void GridTracking::OnSaveGridButton()
{
    if (HaveBordersInGridInfo()){
        globalGridInfo.hasBorders = true;
        char filename[256];//注意数组越界
    	FILE *file;
	
    	//Save grid info, there are up, down, left anf right border position corresponding to translational stage
    	sprintf(filename, "config/grid.cfg");
    	file = fopen(filename, "wt");//文本文件
    	if(file == NULL){//如果文件打开失败，显示错误并退出
    		emit SendState("Save Grid : cannot open the file");
    		return;
    	}
    	fprintf(file, "%d\n", globalGridInfo.hasBorders);
    	fprintf(file, "%ld\n", globalGridInfo.topBorder);
    	fprintf(file, "%ld\n", globalGridInfo.bottomBorder);
    	fprintf(file, "%ld\n", globalGridInfo.leftBorder);
    	fprintf(file, "%ld\n", globalGridInfo.rightBorder);
    	fclose(file);
    }
    else {
        emit SendState("Save Grid : fail to save grid");
    }
}

void GridTracking::OnReturnOriginButton()
{
    if (translationStage==NULL || !translationStage->IsConnected()){
        return;
    }
    int ret = QMessageBox::warning(NULL, "Warning", "Are you sure to Return Origin?", QMessageBox::Ok | QMessageBox::Cancel);
    if (ret == QMessageBox::Cancel)
        return;

   emit ReturnOriginSignal(); 
}

//刷新网格，根据当前刷新类别不同，自动选择不同的刷新方式
void GridTracking::refreshGrid()
{
    if (refreshGridState == REFRESH_BLOCK){
        grid->paintCurrentBlock(current_row, current_col);
        grid->update();
    }
    else if(refreshGridState == REFRESH_POSITION){
        refreshCurrentPosition();
    }
}

void GridTracking::refreshCurrentPosition()
{
    if (translationStage==NULL || !translationStage->IsConnected()){
        return;
    }
    
    refreshGridState = REFRESH_POSITION;
	double x_currentPosition = translationStage->X_GetCurrentPosition();
	double y_currentPosition = translationStage->Y_GetCurrentPosition();
	/*
    	虚拟坐标系Dalsa CCD(X+) <-> Stage(Y+), Dalsa CCD(Y+) <-> Stage(-X)
		判断此时处于那个网格， 其中
		bottomBorder记录了培养皿的下边缘位置，实际为平移台的上边界
		leftBorder记录了培养皿的左边缘位置，实际为平移台的右边界
		平移台X轴运动导致图像竖直方向运动，Y轴运动导致图像水平方向运动
	*/
	current_row = int(abs(globalGridInfo.topBorder-y_currentPosition)/globalGridInfo.fullFovImageHeight_StageDistance);
	current_row += 1;//从1开始计数
	current_col = int(abs(globalGridInfo.leftBorder-x_currentPosition)/globalGridInfo.fullFovImageWidth_StageDistance);
	current_col += 1;
	
	//判断此时处于网格的位置， 不考虑边界的情况,平移台X轴在水平方向，Y轴在竖直方向
	current_x = int(abs(x_currentPosition-globalGridInfo.leftBorder)*globalGridInfo.colPulseToGridDistance);
	current_y = int(abs(y_currentPosition-globalGridInfo.topBorder)*globalGridInfo.rowPulseToGridDistance);
	current_x += current_col;//考虑网格线
	current_y += current_row;
	
	grid->paintCurrentPosition(current_x,current_y);
    grid->update();
}

void GridTracking::FindCurrentPosition()
{
    double x_currentPosition, y_currentPosition;
    if (translationStage==NULL || !translationStage->IsConnected()){
        return;
    }
    x_currentPosition = translationStage->X_GetCurrentPosition();
    y_currentPosition = translationStage->Y_GetCurrentPosition();
    //cout << "x_point : " << x_currentPosition << ", y_point :" << y_currentPosition << endl;
    
    //获取当前处于那个网格
    current_row = int(ceil(abs(1.0*globalGridInfo.topBorder-1.0*y_currentPosition)/globalGridInfo.fullFovImageHeight_StageDistance));
    current_col = int(ceil(abs(1.0*globalGridInfo.leftBorder-1.0*x_currentPosition)/globalGridInfo.fullFovImageWidth_StageDistance));
    //cout << "CurrentRow:" << current_row <<", CurrentCol:" << current_col << endl;
    
    //获取当前处于网格中的位置，注需要考虑网格线的宽度信息（与网格关系密切）
    current_x = int( abs(1.0*globalGridInfo.leftBorder-1.0*x_currentPosition)*globalGridInfo.colPulseToGridDistance );
    current_y = int( abs(1.0*globalGridInfo.topBorder-1.0*y_currentPosition)*globalGridInfo.rowPulseToGridDistance );
    //考虑网格线
    current_x += current_col;
    current_y += current_row;
}

void GridTracking::FindNearBlockAndMotionSteps(double &x_step,double &y_step)
{
    y_step = 0;
    x_step = 0;
    
    //获取当前x,y方向的位置  
    double x_currentPosition, y_currentPosition;
    if (translationStage==NULL || !translationStage->IsConnected()){
        return;
    }
    x_currentPosition = translationStage->X_GetCurrentPosition();
    y_currentPosition = translationStage->Y_GetCurrentPosition();
    //cout << "x_point : " << x_currentPosition << ", y_point :" << y_currentPosition << endl;
    
	//判断此时处于那个网格， 其中：
	//topBorder记录了培养皿的下边缘位置，实际为平移台的上边界，leftBorder记录了培养皿的左边缘位置，实际为平移台的左边界，
	//current_row, current_col从1开始计数。
	current_row = int(abs(1.0*globalGridInfo.topBorder-1.0*y_currentPosition)/globalGridInfo.fullFovImageHeight_StageDistance);
	current_row += 1;
	current_col = int(abs(1.0*globalGridInfo.leftBorder-1.0*x_currentPosition)/globalGridInfo.fullFovImageWidth_StageDistance);
	current_col += 1;
	//cout << "CurrentRow:" << current_row <<", CurrentCol:" << current_col << endl;
	
    //获取当前处于那个网格位置
    y_step = globalGridInfo.topBorder+(STAGE_Y_POSITIVE)*(1.0*current_row-0.5)*globalGridInfo.fullFovImageHeight_StageDistance-1.0*y_currentPosition;
    x_step = globalGridInfo.leftBorder+(-STAGE_X_POSITIVE)*(1.0*current_col-0.5)*globalGridInfo.fullFovImageWidth_StageDistance-1.0*x_currentPosition;
    //考虑方向，计算最终的运动量
    y_step *= (-STAGE_Y_POSITIVE);
    x_step *= (-STAGE_X_POSITIVE);
}
