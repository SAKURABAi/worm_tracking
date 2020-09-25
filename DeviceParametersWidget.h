#ifndef DEVICEPARAMETERSWIDGET_H
#define DEVICEPARAMETERSWIDGET_H

#include "DeviceParameterListWidget.h"
#include "Galil.h"
#include "TranslationStage.h"
#include "RotationStage.h"
#include "Z2Stage.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QToolBox>

#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QList>

class QLabel;
class QTextEdit;
class QPushButton;
class QWidget;
class QVBoxLayout;

class DeviceParametersWidget : public QWidget
{
    Q_OBJECT

public:
    DeviceParametersWidget(QWidget *parent = 0);
    ~DeviceParametersWidget();
        
protected:
    void drawLayout();
    void ListConnectSignals(DeviceParameterListWidget *widget);    
    void Initialize_Stage_Parameters();
        
private slots:
    void onSpeedChangedEvent(DeviceId deviceId, float para);
    void onAccSpeedChangedEvent(DeviceId deviceId, float para);
    void onDecSpeedChangedEvent(DeviceId deviceId, float para);
    void onKpChangedEvent(DeviceId deviceId, float para);
    void onKiChangedEvent(DeviceId deviceId, float para);
    void onKdChangedEvent(DeviceId deviceId, float para);
    
private:
    QToolBox *deviceBox;

    DeviceParameterListWidget *rotationStageList;
    DeviceParameterListWidget *translationStageList;
    DeviceParameterListWidget *z1StageList;
    DeviceParameterListWidget *z2StageList;
    
    Galil *controller;
    TranslationStage *translationStage;
    RotationStage *rotationStage;
    Z2Stage *z2Stage;
};

#endif // DEVICEPARAMETERSWIDGET_H
