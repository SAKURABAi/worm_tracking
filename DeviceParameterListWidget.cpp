
#include "DeviceParameterListWidget.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>

DeviceParameterListWidget::DeviceParameterListWidget(DeviceId deviceId, QWidget *parent)
 : QWidget(parent)
{
    this->deviceId = deviceId;
        
    QLabel *speedLabel = new QLabel("Speed", this);
    speedLabel->setFixedWidth(130);
    speedInput = new QLineEdit(this);
    speedInput->setFixedWidth(100);
    
    QLabel *accSpeedLabel = new QLabel("Acce", this);
    accSpeedLabel->setFixedWidth(130);
    accSpeedInput = new QLineEdit(this);
    accSpeedInput->setFixedWidth(100);
    
    QLabel *decSpeedLabel = new QLabel("Dece", this);
    decSpeedLabel->setFixedWidth(130);
    decSpeedInput = new QLineEdit(this);
    decSpeedInput->setFixedWidth(100);
    
    QLabel *kpLabel = new QLabel("KP", this);
    kpLabel->setFixedWidth(130);
    kpInput = new QLineEdit(this);
    kpInput->setFixedWidth(100);
    
    QLabel *kiLabel = new QLabel("KI", this);
    kiLabel->setFixedWidth(130);
    kiInput = new QLineEdit(this);
    kiInput->setFixedWidth(100);
    
    QLabel *kdLabel = new QLabel("KD", this);
    kdLabel->setFixedWidth(130);
    kdInput = new QLineEdit(this);
    kdInput->setFixedWidth(100);
    
    QLabel *precisionLabel = NULL;
    switch(deviceId){
        case DEVICE_CRTLR1_ROT:
            precisionLabel = new QLabel("Unit = degree/pulse", this); break;
        case DEVICE_CRTLR1_TRS:
            precisionLabel = new QLabel("Unit = um/pulse", this); break;
        case DEVICE_CRTLR2_Z:
            precisionLabel = new QLabel("Unit = um/pulse", this); break;
    }
    precisionLabel->setFixedWidth(130);
    precisionInput = new QLineEdit(this);
    precisionInput->setFixedWidth(100);
    precisionInput->setReadOnly(true);
        
    connect(speedInput, SIGNAL(returnPressed()), this, SLOT(onSpeedEditedEvent()));
    connect(accSpeedInput, SIGNAL(returnPressed()), this, SLOT(onAccSpeedEditedEvent()));
    connect(decSpeedInput, SIGNAL(returnPressed()), this, SLOT(onDecSpeedEditedEvent()));
    connect(kpInput, SIGNAL(returnPressed()), this, SLOT(onKpEditedEvent()));
    connect(kiInput, SIGNAL(returnPressed()), this, SLOT(onKiEditedEvent()));
    connect(kdInput, SIGNAL(returnPressed()), this, SLOT(onKdEditedEvent()));
    
    QGridLayout *mLayout = new QGridLayout;
    mLayout->addWidget(speedLabel,0,0);
    mLayout->addWidget(speedInput,0,1);
    mLayout->addWidget(accSpeedLabel,1,0);
    mLayout->addWidget(accSpeedInput,1,1);
    mLayout->addWidget(decSpeedLabel,2,0);
    mLayout->addWidget(decSpeedInput,2,1);
    mLayout->addWidget(kpLabel,3,0);
    mLayout->addWidget(kpInput,3,1);
    mLayout->addWidget(kiLabel,4,0);
    mLayout->addWidget(kiInput,4,1);
    mLayout->addWidget(kdLabel,5,0);
    mLayout->addWidget(kdInput,5,1);
    mLayout->addWidget(precisionLabel,6,0);
    mLayout->addWidget(precisionInput,6,1);
    mLayout->setHorizontalSpacing(5);
    
    setLayout(mLayout);
}

void DeviceParameterListWidget::onSpeedEditedEvent() 
{
    QString para = speedInput->text();
    emit onSpeedChanged(deviceId, para.toFloat());
}

void DeviceParameterListWidget::onAccSpeedEditedEvent() 
{
    QString para = accSpeedInput->text();
    emit onAccSpeedChanged(deviceId, para.toFloat());
}

void DeviceParameterListWidget::onDecSpeedEditedEvent() 
{
    QString para = decSpeedInput->text();
    emit onDecSpeedChanged(deviceId, para.toFloat());
}

void DeviceParameterListWidget::onKpEditedEvent() 
{
    QString para = kpInput->text();
    emit onKpChanged(deviceId, para.toFloat());
}

void DeviceParameterListWidget::onKiEditedEvent() 
{
    QString para = kiInput->text();
    emit onKiChanged(deviceId, para.toFloat());
}

void DeviceParameterListWidget::onKdEditedEvent() 
{
    QString para = kdInput->text();
    emit onKdChanged(deviceId, para.toFloat());
}
