
#include "DeviceParametersWidget.h"

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QSpacerItem>

#include <QtCore/QListIterator>

DeviceParametersWidget::DeviceParametersWidget(QWidget *parent) : QWidget(parent)
{
    controller = NULL;
    translationStage = NULL;
    rotationStage = NULL;
    z2Stage = NULL;
    
    drawLayout();
    try{
        //create controllers for stage devices
        controller = new Galil("192.168.0.5");
        controller->timeout_ms = 500;
        usleep(500000);//sleep for 500ms
        
        rotationStage = new RotationStage(controller);
        rotationStage->Connect();
        
        translationStage = new TranslationStage(controller);
        translationStage->Connect();
                
        z2Stage = new Z2Stage(controller);
        z2Stage->Connect();
        
        Initialize_Stage_Parameters();//初始化参数
    } catch(string e){
        QMessageBox::critical(NULL, "Warning", QString::fromStdString(e));
    } catch(QException e){
        QMessageBox::critical(NULL, "Warning", QString::fromStdString(e.getMessage()));
    }
}

DeviceParametersWidget::~DeviceParametersWidget()
{
    if (z2Stage!=NULL){
        z2Stage->Stop();
        delete z2Stage;
        z2Stage = NULL;
    }
    if (rotationStage!=NULL){
        rotationStage->Stop();
        delete rotationStage;
        rotationStage = NULL;
    }
    if (translationStage!=NULL){
        translationStage->Stop();
        delete translationStage;
        translationStage = NULL;
    }
    if (controller!=NULL){
        delete controller;
        controller = NULL;
    }
}

void DeviceParametersWidget::drawLayout() 
{
    rotationStageList = new DeviceParameterListWidget(DEVICE_CRTLR1_ROT);
    translationStageList = new DeviceParameterListWidget(DEVICE_CRTLR1_TRS);
    z2StageList = new DeviceParameterListWidget(DEVICE_CRTLR2_Z);
    
    //connect signals to relative slot
    ListConnectSignals(rotationStageList);
    ListConnectSignals(translationStageList);
    ListConnectSignals(z2StageList);
    
    deviceBox = new QToolBox;
    deviceBox->addItem(rotationStageList,tr("Rotation Stage"));
    deviceBox->addItem(translationStageList,tr("Translation Stage"));
    deviceBox->addItem(z2StageList,tr("Z2 Stage"));
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(deviceBox);
    
    setLayout(mainLayout);
    setFixedWidth(300);
}

void DeviceParametersWidget::ListConnectSignals(DeviceParameterListWidget *widget) 
{
    switch (widget->getDeviceId()){
        case DEVICE_CRTLR1_ROT:
            rotationStageList->setPrecision(ROTATION_STAGE::W_PRECISION);
            break;
        case DEVICE_CRTLR1_TRS:
            translationStageList->setPrecision(TRANSLATION_STAGE::X_PRECISION);
            break;
        case DEVICE_CRTLR2_Z:
            z2StageList->setPrecision(Z2_STAGE::Z2_PRECISION);
            break;
    }
    
    connect(widget, SIGNAL(onSpeedChanged(DeviceId,float)), this, SLOT(onSpeedChangedEvent(DeviceId,float)));
    connect(widget, SIGNAL(onAccSpeedChanged(DeviceId,float)), this, SLOT(onAccSpeedChangedEvent(DeviceId,float)));
    connect(widget, SIGNAL(onDecSpeedChanged(DeviceId,float)), this, SLOT(onDecSpeedChangedEvent(DeviceId,float)));
    connect(widget, SIGNAL(onKpChanged(DeviceId,float)), this, SLOT(onKpChangedEvent(DeviceId,float)));
    connect(widget, SIGNAL(onKiChanged(DeviceId,float)), this, SLOT(onKiChangedEvent(DeviceId,float)));
    connect(widget, SIGNAL(onKdChanged(DeviceId,float)), this, SLOT(onKdChangedEvent(DeviceId,float)));
}

void DeviceParametersWidget::onSpeedChangedEvent(DeviceId deviceId, float para) 
{
   switch (deviceId){
        case DEVICE_CRTLR1_ROT:
            rotationStage->Set_Speed(long(para));
            break;
        case DEVICE_CRTLR1_TRS:
            translationStage->X_Set_Speed(long(para));
            translationStage->Y_Set_Speed(long(para));
            break;
        case DEVICE_CRTLR2_Z:
            z2Stage->Set_Speed(long(para));
            break;
    }
}

void DeviceParametersWidget::onAccSpeedChangedEvent(DeviceId deviceId, float para) 
{
   switch (deviceId){
        case DEVICE_CRTLR1_ROT:
            rotationStage->Set_ACCSpeed(long(para));
            break;
        case DEVICE_CRTLR1_TRS:
            translationStage->X_Set_ACCSpeed(long(para));
            translationStage->Y_Set_ACCSpeed(long(para));
            break;
        case DEVICE_CRTLR2_Z:
            z2Stage->Set_ACCSpeed(long(para));
            break;
    }
}

void DeviceParametersWidget::onDecSpeedChangedEvent(DeviceId deviceId, float para) 
{
    switch (deviceId){
        case DEVICE_CRTLR1_ROT:
            rotationStage->Set_DECSpeed(long(para));
            break;
        case DEVICE_CRTLR1_TRS:
            translationStage->X_Set_DECSpeed(long(para));
            translationStage->Y_Set_DECSpeed(long(para));
            break;
        case DEVICE_CRTLR2_Z:
            z2Stage->Set_DECSpeed(long(para));
            break;
    }
}

void DeviceParametersWidget::onKpChangedEvent(DeviceId deviceId, float para) 
{
    switch (deviceId){
        case DEVICE_CRTLR1_ROT:
            break;
        case DEVICE_CRTLR1_TRS:
            translationStage->X_Set_KP(para);
            translationStage->Y_Set_KP(para);
            break;
        case DEVICE_CRTLR2_Z:
            break;
    }
}

void DeviceParametersWidget::onKiChangedEvent(DeviceId deviceId, float para) 
{
   switch (deviceId){
        case DEVICE_CRTLR1_ROT:
            break;
        case DEVICE_CRTLR1_TRS:
            translationStage->X_Set_KI(para);
            translationStage->Y_Set_KI(para);
            break;
        case DEVICE_CRTLR2_Z:
            break;
    }
}

void DeviceParametersWidget::onKdChangedEvent(DeviceId deviceId, float para) 
{
   switch (deviceId){
        case DEVICE_CRTLR1_ROT:
            break;
        case DEVICE_CRTLR1_TRS:
            translationStage->X_Set_KD(para);
            translationStage->Y_Set_KD(para);
            break;
        case DEVICE_CRTLR2_Z:
            break;
    }
}

void DeviceParametersWidget::Initialize_Stage_Parameters()
{
    long speed = 0, accSpeed = 0, decSpeed = 0;
    double ki = 0, kp = 0, kd = 0;
    
    //Get RotationStage Parameters
    speed = rotationStage->Get_Speed();
    accSpeed = rotationStage->Get_ACCSpeed();
    decSpeed = rotationStage->Get_DECSpeed();
    rotationStageList -> setSpeed(speed);
    rotationStageList -> setAccSpeed(accSpeed);
    rotationStageList -> setDecSpeed(decSpeed);
    rotationStageList -> setKiEnabled(false);
    rotationStageList -> setKpEnabled(false);
    rotationStageList -> setKdEnabled(false);
    
    //Get TranslationStage Parameters
    speed = translationStage->X_Get_Speed();
    accSpeed = translationStage->X_Get_ACCSpeed();
    decSpeed = translationStage->X_Get_DECSpeed();
    ki = translationStage->X_Get_KI();
    kp = translationStage->X_Get_KP();
    kd = translationStage->X_Get_KD();
    translationStageList -> setSpeed(speed);
    translationStageList -> setAccSpeed(accSpeed);
    translationStageList -> setDecSpeed(decSpeed);
    translationStageList -> setKi(ki);
    translationStageList -> setKp(kp);
    translationStageList -> setKd(kd);
        
    //Get Z2Stage Parameters
    speed = z2Stage->Get_Speed();
    accSpeed = z2Stage->Get_ACCSpeed();
    decSpeed = z2Stage->Get_DECSpeed();
    z2StageList -> setSpeed(speed);
    z2StageList -> setAccSpeed(accSpeed);
    z2StageList -> setDecSpeed(decSpeed);
    z2StageList -> setKiEnabled(false);
    z2StageList -> setKpEnabled(false);
    z2StageList -> setKdEnabled(false);
}
