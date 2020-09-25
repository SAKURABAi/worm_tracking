
#include "DeviceParametersDialog.h"
#include <QtWidgets/QVBoxLayout>

DeviceParametersDialog::DeviceParametersDialog(QWidget *parent) : QDialog(parent)
{
    _widget = new DeviceParametersWidget(this);
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(_widget);
    setLayout(layout);
    setWindowTitle(tr("Stage Parameters"));
}

DeviceParametersWidget * DeviceParametersDialog::widget() {
    return _widget;
}
