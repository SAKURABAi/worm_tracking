
#include "ROISetting.h"
#include "GlobalParameters.h"
#include "DevicePackage.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>

ROISetting::ROISetting(DalsaCamera *camera, QWidget *parent):QWidget(parent),dalsaCamera(camera)
{
    translationStage = NULL;
	isReadyForMouseResponse = false;

	CreateLayout();
    fullFov->setChecked(true);
}

ROISetting::~ROISetting()
{
	translationStage = NULL;
}

void ROISetting::Controller1_Connected()
{
    try{
        translationStage = new TranslationStage(controller1);
        translationStage -> Connect();
    }catch (string e){
        emit SendState(e);
    }catch (QException e){
        emit SendState(e.getMessage());
    }
    cout<<"ROISetting connect stage"<<endl;
}

void ROISetting::Controller1_Disconnected()
{
	if (translationStage!=NULL){
    	delete translationStage;
    	translationStage = NULL;
	}
}

void ROISetting::CreateLayout()
{
	roiSettingGroup = new QGroupBox;
    
    //create widgets
    QLabel *label = new QLabel( tr("x") );
	fullFov = new QRadioButton(QString::number(DALSACEMERA::FULLIMAGE_WIDTH) + tr("x") + QString::number(DALSACEMERA::FULLIMAGE_HEIGHT));
    roi_1536 = new QRadioButton( tr("1536x1536") );
    roi_1024 = new  QRadioButton( tr("1024x1024") );
    roi_userDefined = new QRadioButton;
        
	centerButton = new QPushButton(tr("Center"));
	mouseResponseButton = new QPushButton(tr("Mouse Response"));
    
    //Set widgets property
    imageWidthBox = new QComboBox;
    imageHeightBox = new QComboBox;
    imageWidthBox->setMinimumWidth(48);
    imageWidthBox->setMaximumWidth(48);
    imageHeightBox->setMinimumWidth(48);
	imageHeightBox->setMaximumWidth(48);
	FillImageSizeBox(imageWidthBox);
    FillImageSizeBox(imageHeightBox);
    
    //Connect signals and slots
	QObject::connect(mouseResponseButton, SIGNAL(clicked()), this, SLOT(OnMouseResponseButton()));
	QObject::connect(centerButton, SIGNAL(clicked()), this, SLOT(SelectCenterImage()));
	QObject::connect( imageWidthBox, SIGNAL( currentIndexChanged(int) ), this, SLOT( OnImageWidthChanged(int) ) );
	
	QHBoxLayout *imageSizeEdit_Layout = new QHBoxLayout;
	imageSizeEdit_Layout->addWidget(roi_userDefined);
	imageSizeEdit_Layout->addWidget(imageWidthBox);
	imageSizeEdit_Layout->addWidget(label);
	imageSizeEdit_Layout->addWidget(imageHeightBox);
	imageSizeEdit_Layout->setMargin(0);
	imageSizeEdit_Layout->setSpacing(0);
	//imageSizeEdit_Layout->insertSpacing(1,-8);
	//imageSizeEdit_Layout->addStretch();

	QHBoxLayout *roiLayout = new QHBoxLayout;
	roiLayout->addWidget(fullFov);
	roiLayout->addWidget(roi_1536);
	roiLayout->addWidget(roi_1024);
	roiLayout->addLayout(imageSizeEdit_Layout);
	roiLayout->setMargin(0);
	roiLayout->setSpacing(0);

	QHBoxLayout *controlLayout = new QHBoxLayout;
	controlLayout->addWidget(centerButton);
	controlLayout->addWidget(mouseResponseButton);
	controlLayout->setMargin(0);
	controlLayout->setSpacing(40);

	QVBoxLayout *vLayout = new QVBoxLayout;
	vLayout->addLayout(roiLayout);
	vLayout->addLayout(controlLayout);
	vLayout->setContentsMargins(0,5,0,0);
	vLayout->setSpacing(5);

	roiSettingGroup->setLayout(vLayout);
}

void ROISetting::FillImageSizeBox(QComboBox *box)
{
    box->addItem( tr("512") );
    box->addItem( tr("256") );
    box->addItem( tr("128") );
    box->addItem( tr("64") );
    box->addItem( tr("32") );
    box->addItem( tr("16") );
}

void ROISetting::OnROISelected()
{
	if (fullFov->isChecked()){
		imageWidth = DALSACEMERA::FULLIMAGE_WIDTH;
		imageHeight = DALSACEMERA::FULLIMAGE_HEIGHT;
		roi_1536->setChecked(false);
		roi_1024->setChecked(false);
		roi_userDefined->setChecked(false);
	}
	if (roi_1536->isChecked()){
		imageWidth = imageHeight = 1536;
		fullFov->setChecked(false);
		roi_1024->setChecked(false);
		roi_userDefined->setChecked(false);
	}
	else if (roi_1024->isChecked()){
		imageWidth = imageHeight = 1024;
		fullFov->setChecked(false);
		roi_1536->setChecked(false);
		roi_userDefined->setChecked(false);
	}
	else if (roi_userDefined->isChecked()){
		imageWidth = (imageWidthBox->currentText()).toInt();
        imageHeight = (imageHeightBox->currentText()).toInt();
        fullFov->setChecked(false);
		roi_1536->setChecked(false);
		roi_1024->setChecked(false);
	}

	//Override expected roi image size to global variable
	globalImageSize.roiWidth = imageWidth;
	globalImageSize.roiHeight = imageHeight;
}

void ROISetting::OnImageWidthChanged(int index)
{
    imageWidth = (imageWidthBox->itemText(index)).toInt();
	imageHeightBox->setCurrentIndex(index);
	imageHeight = imageWidth;
}

void ROISetting::SelectCenterImage()
{
    OnROISelected();
	//如果roi大小与当前图像大小一致则返回
	if (globalImageSize.imageWidth == globalImageSize.roiWidth && globalImageSize.imageHeight == globalImageSize.roiHeight){
		return;
	}
    SetROI();
}

void ROISetting::SetROI()
{
    int x_offset = 0, y_offset = 0;
    if (imageWidth < DALSACEMERA::FULLIMAGE_WIDTH && imageHeight < DALSACEMERA::FULLIMAGE_HEIGHT){
    	x_offset = int((DALSACEMERA::FULLIMAGE_WIDTH - imageWidth)/2 + DALSACEMERA::X_CENTER_OFFSET);
    	y_offset = int((DALSACEMERA::FULLIMAGE_HEIGHT - imageHeight)/2 + DALSACEMERA::Y_CENTER_OFFSET);
	}
	else{
    	x_offset = int((DALSACEMERA::FULLIMAGE_WIDTH - imageWidth)/2);
    	y_offset = int((DALSACEMERA::FULLIMAGE_HEIGHT - imageHeight)/2);
	}
	dalsaCamera->SetROI(imageWidth, imageHeight, x_offset, y_offset);

	//Update current image size
	globalImageSize.imageWidth = globalImageSize.roiWidth;
	globalImageSize.imageHeight = globalImageSize.roiHeight;
}

void ROISetting::OnMouseResponseButton()
{
    OnROISelected();
	isReadyForMouseResponse = true;
	mouseResponseButton->setEnabled(false);
	emit StartMouseResponseSignal();//send signal to imageviewer
}

void ROISetting::FinishMouseResponse()
{
	mouseResponseButton->setEnabled(true);

	int xDisplayPosition = 0, yDisplayPosition = 0;//鼠标实际在图像中的位置
	int displayWidth = globalImageSize.displayWidth, displayHeight = globalImageSize.displayHeight;
	double x_ratio = 0, y_ratio = 0;
	
	//Move current image to the center of display window
	if(globalImageSize.imageWidth*globalImageSize.displayHeight > globalImageSize.displayWidth*globalImageSize.imageHeight){
    	//no x offset
    	int display_height = globalImageSize.displayWidth*globalImageSize.imageHeight/globalImageSize.imageWidth;
		int yDisplayOffset = (globalImageSize.displayHeight - display_height)/2; //实际图像在显示窗口中的偏移量
		
		displayWidth = globalImageSize.displayWidth;
		displayHeight = display_height;
		xDisplayPosition = globalImageSize.x_position;
		yDisplayPosition = globalImageSize.y_position - yDisplayOffset;
	}
	else{
    	//no y offset
    	int display_width = globalImageSize.imageWidth*globalImageSize.displayHeight/globalImageSize.imageHeight;
		int xDisplayOffset = (globalImageSize.displayWidth - display_width)/2; //实际图像在显示窗口中的偏移量

		displayWidth = display_width;
		displayHeight = globalImageSize.displayHeight;
		xDisplayPosition = globalImageSize.x_position-xDisplayOffset;
		yDisplayPosition = globalImageSize.y_position;
	}
	
	x_ratio = 1.0*globalImageSize.imageWidth/displayWidth;  //水平方向上显示区域的像素对应到平移台的比例
	y_ratio = 1.0*globalImageSize.imageHeight/displayHeight;//竖直方向上显示区域的像素对应到平移台的比例
	
	double x_image_motion = 0, y_image_motion = 0;
	if (globalImageSize.imageWidth == DALSACEMERA::FULLIMAGE_WIDTH && globalImageSize.imageHeight == DALSACEMERA::FULLIMAGE_HEIGHT){
    	// Adding offset to make the object to be the center of Andor image and the offsets
    	// don't change the offset between behavior imaging optical axis and neruon imaging optical axis.
    	// So croping full area image for acquring ROI image is neccessary!
    	x_image_motion = (displayWidth/2-xDisplayPosition)*x_ratio + DALSACEMERA::X_CENTER_OFFSET;
    	y_image_motion = (displayHeight/2-yDisplayPosition)*y_ratio + DALSACEMERA::Y_CENTER_OFFSET;
	} else {
    	x_image_motion = (displayWidth/2-xDisplayPosition)*x_ratio;
    	y_image_motion = (displayHeight/2-yDisplayPosition)*y_ratio;
	}
	double x_motion = STAGE_XY_TO_IMAGE_MATRIX[0][0]*x_image_motion+STAGE_XY_TO_IMAGE_MATRIX[0][1]*y_image_motion;
	double y_motion = STAGE_XY_TO_IMAGE_MATRIX[1][0]*x_image_motion+STAGE_XY_TO_IMAGE_MATRIX[1][1]*y_image_motion;
	x_motion = x_motion * (-STAGE_X_POSITIVE);
	y_motion = y_motion * (-STAGE_Y_POSITIVE);
	
	try{
		if(translationStage!=NULL && translationStage->IsConnected()){
			cout<<"x motion: "<<x_motion<<", y motion: "<<y_motion<<endl;
			translationStage->XY_Move_Closeloop_Realtime(x_motion, y_motion);
		}
		else{
	    	cout <<"No Transation Stage Connection"<<endl;
	    	return;
		}
	} catch(QException e){
		cout<<e.getMessage()<<endl;
	}
	
	imageWidth = globalImageSize.roiWidth;
	imageHeight = globalImageSize.roiHeight;

	//如果图像大小没有变化，则不需要改变相机的ROI大小
	if (globalImageSize.imageWidth==globalImageSize.roiWidth && globalImageSize.imageHeight==globalImageSize.roiHeight){
		return;
	}
	SetROI();
}
