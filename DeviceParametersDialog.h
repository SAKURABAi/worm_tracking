
#ifndef DEVICEPARAMETERSDIALOG_H
#define DEVICEPARAMETERSDIALOG_H

#include "DeviceParametersWidget.h"
#include <QtWidgets/QDialog>

class DeviceParametersWidget;

class DeviceParametersDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DeviceParametersDialog(QWidget *parent = 0);
    DeviceParametersWidget *widget();
    
private:
    DeviceParametersWidget *_widget;
};

#endif // DEVICEPARAMETERSDIALOG_H
