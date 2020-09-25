
#ifndef _TRACKING_PARAMETERS_SETTING_DIALOG_H
#define _TRACKING_PARAMETERS_SETTING_DIALOG_H

#include "GlobalParameters.h"
#include <QtWidgets/QDialog>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QLineEdit>

class TrackingParametersSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TrackingParametersSettingDialog(QWidget *parent=0); 

protected:
    void CreateLayout();
    bool LoadBinaryThreshold();
    bool SaveBinaryThreshold();

protected slots:
    void OnROIPositionChanched();
    void OnRefLineButton(int state);
    void OnReverseROIPositionButton();

    void OnAdaptiveBinaryThresholdButton(int state);
    void OnBinaryThresholdEdit();
    void OnAddBinaryThresholdButton();
    void OnSubstractBinaryThresholdButton();

private:
    //Reference Line Setting
    QCheckBox *withRefLineButton;
    QPushButton *reverseROIPositionButton;
    QLineEdit *ROIPositionEdit;
    
    //Binary Threshold Setting
    QLineEdit *binaryThresholdEdit;
    QPushButton *addBinaryThresholdButton;
    QPushButton *substractBinaryThresholdButton;
    QCheckBox *adaptiveBinaryButton;
};

#endif //_TRACKING_PARAMETERS_SETTING_DIALOG_H
