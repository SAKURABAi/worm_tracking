
#ifndef _TRACKING_PARAMETERS_SETTING_DIALOG_H
#define _TRACKING_PARAMETERS_SETTING_DIALOG_H

#include "GlobalParameters.h"
#include "TranslationStage.h"

#include <QtWidgets/QDialog>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QLineEdit>

class TrackingParametersSettingDialog : public QDialog
{
    Q_OBJECT

signals:
    void UpdateWormAreaSignal();

public:
    explicit TrackingParametersSettingDialog(TranslationStage* translationStage, QWidget *parent=0);
    ~TrackingParametersSettingDialog();

    void OnTranslationStageConnected();
    void OnTranslationStageDisconnected();

    inline void SetAdaptivePIDState(bool ok){
        if (ok){
            adaptivePIDBox->setCheckState(Qt::Checked);
        }
        else{
            adaptivePIDBox->setCheckState(Qt::Unchecked);
        }
    }

protected:
    void CreateLayout();
    
    void SetCurrentPID(PID x_pid, PID y_pid);
    inline QString Get_PID_Description(PID x_pid, PID y_pid){
        return ("X(" + QString::number(x_pid.KP) + ", " + QString::number(x_pid.KI) + ", " + QString::number(x_pid.KD)+"), Y("+ QString::number(y_pid.KP) + ", " +
                    QString::number(y_pid.KI) + ", " + QString::number(y_pid.KD) + ")");
    }
    bool LoadBinaryThreshold();
    bool SaveBinaryThreshold();
    void SetBinaryThresholdAvailable(bool available);

protected slots:
    void OnROIPositionChanched();
    void OnRefLineButton(int state);
    void OnReverseROIPositionButton();

    void OnBinaryThresholdEdit();
    void OnAddBinaryThresholdButton();
    void OnSubstractBinaryThresholdButton();
     
    void OnWormAreaEdit();
    void OnAddWormAreaButton();
    void OnSubstractWormAreaButton();

    void OnPIDSelection();
    void OnAdaptivePIDSelected(int);

private:
    //Reference Line Setting
    QCheckBox *withRefLineButton;
    QPushButton *reverseROIPositionButton;
    QLineEdit *ROIPositionEdit;
    
    //Binary Threshold Setting
    QLineEdit *binaryThresholdEdit;
    QPushButton *addBinaryThresholdButton;
    QPushButton *substractBinaryThresholdButton;
    
    QLineEdit *wormAreaEdit;
    QPushButton *addWormAreaButton;
    QPushButton *substractWormAreaButton;

    //PID Setting
    QCheckBox *adaptivePIDBox;
    QRadioButton *PID_Initial_Button;
    QRadioButton *PID1_Button;
    QRadioButton *PID2_Button;
    QRadioButton *PID3_Button;
    QRadioButton *PID4_Button;
    QRadioButton *PID5_Button;
    //QRadioButton *PID6_Button;
    
    //Offset Post-Process
    TranslationStage *stage;
    PID x_PID;
    PID y_PID;
};

#endif //_TRACKING_PARAMETERS_SETTING_DIALOG_H
