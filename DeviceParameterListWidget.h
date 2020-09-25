/****************************************************************************
	DeviceparameterlistWidget:
****************************************************************************/

#ifndef DEVICEPARAMETERLISTWIDGET_H
#define DEVICEPARAMETERLISTWIDGET_H

#include "Util.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>

#include <QtCore/QString>

class DeviceParameterListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceParameterListWidget(DeviceId deviceId,QWidget *parent = 0);
    
    inline DeviceId getDeviceId() const{
        return deviceId;
    }
    
    inline float getSpeed() const{
        return speedInput->text().toFloat();
    }
    inline float getAccSpeed() const{
        return accSpeedInput->text().toFloat();
    }
    inline float getDecSpeed() const{
        return decSpeedInput->text().toFloat();
    }
    inline float getKp() const{
        return kpInput->text().toFloat();
    }
    inline float getKi() const{
        return kiInput->text().toFloat();
    }
    inline float getKd() const{
        return kdInput->text().toFloat();
    }
    inline float getPrecision() const{
        return precisionInput->text().toFloat();
    }
    
    inline void setSpeed(float para) const{
        speedInput->setText(QString::number(para));
    }
    inline void setAccSpeed(float para) const{
        accSpeedInput->setText(QString::number(para));
    }
    inline void setDecSpeed(float para) const{
        decSpeedInput->setText(QString::number(para));
    }
    inline void setKp(float para) const{
        kpInput->setText(QString::number(para));
    }
    inline void setKi(float para) const{
        kiInput->setText(QString::number(para));
    }
    inline void setKd(float para) const{
        kdInput->setText(QString::number(para));
    }
    inline void setPrecision(float para) const{
        precisionInput->setText(QString::number(para));
    }
    
    inline void setSpeedEnabled(bool ok) const{
        speedInput->setEnabled(ok);
    }
    inline void setAccSpeedEnabled(bool ok) const{
        accSpeedInput->setEnabled(ok);
    }
    inline void setDecSpeedEnabled(bool ok) const{
        decSpeedInput->setEnabled(ok);
    }
    inline void setKiEnabled(bool ok) const{
        kiInput->setEnabled(ok);
    }
    inline void setKpEnabled(bool ok) const{
        kpInput->setEnabled(ok);
    }
    inline void setKdEnabled(bool ok) const{
        kdInput->setEnabled(ok);
    }
    
signals:
    void onSpeedChanged(DeviceId, float);
    void onAccSpeedChanged(DeviceId, float);
    void onDecSpeedChanged(DeviceId, float);
    void onKpChanged(DeviceId, float);
    void onKiChanged(DeviceId, float);
    void onKdChanged(DeviceId, float);
        
protected slots:
    void onSpeedEditedEvent();
    void onAccSpeedEditedEvent();
    void onDecSpeedEditedEvent();
    void onKpEditedEvent();
    void onKiEditedEvent();
    void onKdEditedEvent();
    
private:
    QLineEdit *speedInput;
    QLineEdit *accSpeedInput;
    QLineEdit *decSpeedInput;
    QLineEdit *kpInput;
    QLineEdit *kiInput;
    QLineEdit *kdInput;
    QLineEdit *precisionInput;

    DeviceId deviceId;
};

#endif // DEVICEPARAMETERLISTWIDGET_H
