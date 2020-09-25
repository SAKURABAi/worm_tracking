/****************************************************************************
    ROISetting 软件中ROI模块，用于ROI大小选择以及实现鼠标响应
****************************************************************************/
#ifndef _ROI_SETTING_H_
#define _ROI_SETTING_H_

#include "DalsaCamera.h"
#include "TranslationStage.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>

class ROISetting : public QWidget
{
    Q_OBJECT
public:
    explicit ROISetting(DalsaCamera *camera=NULL,QWidget *parent=0);
    ~ROISetting();

    void CreateLayout();
    inline void SetTitle(const QString &title){
        roiSettingGroup->setTitle(title);
    }
    inline QGroupBox* GetRoiSettingGroup() const { return roiSettingGroup; }
        
signals:
    void SendState(string state);
    void StartMouseResponseSignal();

public slots:
    void Controller1_Connected();
    void Controller1_Disconnected();
    
    void FinishMouseResponse();  
    void OnMouseResponseButton();
    void SelectCenterImage();
    void OnImageWidthChanged(int);
    
protected:
    void OnROISelected();
    void SetROI();
    void FillImageSizeBox(QComboBox *box);

private:
    QGroupBox *roiSettingGroup;
	QRadioButton *fullFov;
    QRadioButton *roi_1536;
    QRadioButton *roi_1024;
    QRadioButton *roi_userDefined;
    QComboBox *imageWidthBox;
    QComboBox *imageHeightBox;

    QPushButton *mouseResponseButton;
    QPushButton *centerButton;
    
    UINT32 imageWidth;
    UINT32 imageHeight;
    
    bool isReadyForMouseResponse;
    DalsaCamera *dalsaCamera;
    TranslationStage *translationStage;
};

#endif
