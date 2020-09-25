/****************************************************************************
    ImageSaveSetting 图像保存设置
****************************************************************************/
#ifndef _SINGLE_IMAGE_SAVE_H_
#define _SINGLE_IMAGE_SAVE_H_

#include "Util.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGroupBox>

#include <QtCore/QString>

class ImageSaveSetting : public QWidget
{
    Q_OBJECT
public:
    static string OBJECT_NAME;
    explicit ImageSaveSetting(QWidget *parent=0);

    void CreateLayout();   
    inline QGroupBox* Get_ImageSaveSettingGroup() const { 
        return imageSaveSettingGroup; 
    }
    inline void SetTitle(const QString &title){
        imageSaveSettingGroup->setTitle(title);
    }
      
protected slots:
	void OnImageTitleChanged();
	void OnImageFormatChanged();
	void OnImageSavePathButton();
	void OnSaveImageButton();
	
protected:

private:
    QGroupBox *imageSaveSettingGroup;
    QLineEdit *imageFontTitleEdit;
    QComboBox *imageFormatCombo;
    QLineEdit *imagePathEdit;
    QPushButton *imageSavePathButton;
    //QPushButton *saveImageButton;

    QString imageFormat;
    QString imageFrontTitle;
    QString imageSavePath;
};

#endif
