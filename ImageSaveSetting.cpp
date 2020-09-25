#include "ImageSaveSetting.h"
#include "GlobalParameters.h"

#include <QtWidgets/QLabel>
#include <QtCore/QLatin1Char>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QMessageBox>

#include <time.h>
#include <opencv2/highgui/highgui.hpp>

string ImageSaveSetting::OBJECT_NAME = "ImageSaveSetting";

ImageSaveSetting::ImageSaveSetting(QWidget *parent):QWidget(parent)
{
    imageFormat = tr( "tiff" );
    CreateLayout();
}

void ImageSaveSetting::CreateLayout()
{
    imageSaveSettingGroup = new QGroupBox;
    
	QLabel *titleLabel = new QLabel( tr("Front Title") );
	imageFontTitleEdit = new QLineEdit;

	time_t t = time(NULL);
	struct tm *local_time = gmtime(&t);
	imageFrontTitle = tr("Image") + QString("%1").arg(local_time->tm_year + 1900,4,10,QLatin1Char('0')) + 
	                        QString("%1").arg(local_time->tm_mon + 1,2,10,QLatin1Char('0'))+
	                        QString("%1").arg(local_time->tm_mday,2,10,QLatin1Char('0'));
	imageFontTitleEdit->setText( imageFrontTitle );
	
	imageFormatCombo = new QComboBox;
	imageFormatCombo->addItem( tr(".tiff") );
	imageFormatCombo->addItem( tr(".bmp") );
	imageFormatCombo->addItem( tr(".bin") );
	
	imagePathEdit = new QLineEdit;
	imagePathEdit->setReadOnly( true );
	imagePathEdit->setEnabled( false );
	
	imageSavePathButton = new QPushButton( tr("Select Path") );
	//saveImageButton = new QPushButton( tr("Save") );
	//saveImageButton->setMaximumWidth(60);
	
	QObject::connect( imageFontTitleEdit, SIGNAL( editingFinished() ), this, SLOT( OnImageTitleChanged() ));
	QObject::connect( imageFormatCombo, SIGNAL( activated(int) ), this, SLOT( OnImageFormatChanged() ));
	QObject::connect( imageSavePathButton, SIGNAL( clicked() ), this, SLOT( OnImageSavePathButton() ));
	//QObject::connect( saveImageButton, SIGNAL( clicked() ), this, SLOT( OnSaveImageButton()) );
	
    //add layout
    QHBoxLayout * hBoxLayout1 = new QHBoxLayout;
	hBoxLayout1->addWidget( titleLabel );
	hBoxLayout1->addWidget( imageFontTitleEdit );
	hBoxLayout1->addWidget( imageFormatCombo );
    hBoxLayout1->setMargin(0);
    hBoxLayout1->setSpacing(5);

	QHBoxLayout * hBoxLayout2 = new QHBoxLayout;
	hBoxLayout2->addWidget( imageSavePathButton );
    hBoxLayout2->addWidget( imagePathEdit );
    //hBoxLayout2->addWidget( saveImageButton );
    hBoxLayout2->setMargin(0);
    hBoxLayout2->setSpacing(5);
    
	QVBoxLayout * vBoxLayout = new QVBoxLayout;
	vBoxLayout->addLayout( hBoxLayout1 );
	vBoxLayout->addLayout( hBoxLayout2 );
	vBoxLayout->setMargin(5);
    vBoxLayout->setSpacing(5);
    
	imageSaveSettingGroup->setLayout(vBoxLayout);
}

void ImageSaveSetting::OnImageTitleChanged()
{
    imageFrontTitle = imageFontTitleEdit->text();
}

void ImageSaveSetting::OnImageFormatChanged()
{
	int index = imageFormatCombo->currentIndex();
	imageFormat = imageFormatCombo->itemText(index);
}

void ImageSaveSetting::OnImageSavePathButton()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Save Image"),
	                 imagePathEdit->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	imagePathEdit->setText(dir);
	imageSavePath = imagePathEdit->text();
	
	if ( imageSavePath.isEmpty() ){
		imageSavePath = tr("./image");
	}
	OnImageTitleChanged();
	OnImageFormatChanged();
}

void ImageSaveSetting::OnSaveImageButton()
{
	static UINT64 count = 0;
	QString separator = tr("/");//文件分隔符

	if (imageSavePath.isEmpty()){
		QMessageBox::critical(NULL, "Warning", "Empty image save path");
		return;
	}
    QString filename = imageSavePath + separator + imageFrontTitle+"_"+QString::number(count)+imageFormat;
    
    read_write_locker.lockForRead();
    if (globalDalsaImage.data!=NULL){
    	int height = globalDalsaImage.rows;
    	int width = globalDalsaImage.cols;
    	cv::Mat src(height,width,CV_8UC1,(void*)globalDalsaImage.data);
        cv::imwrite(filename.toStdString(), src);
        ++count;
    }
    read_write_locker.unlock();
}

