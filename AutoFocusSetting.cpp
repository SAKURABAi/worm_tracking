#include "AutoFocusSetting.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>

string AutoFocusSetting::OBJECT_NAME = "AutoFocusSetting";

AutoFocusSetting::AutoFocusSetting(QWidget *parent):QWidget(parent)
{
    z2_autoFocus = NULL;
    z2_autoFocus = new Z2_AutoFocusDialog(this);
    
    CreateLayout();
}

AutoFocusSetting:: ~AutoFocusSetting()
{
    if (z2_autoFocus != NULL){
        delete z2_autoFocus;
        z2_autoFocus = NULL;
    }
}
 
void AutoFocusSetting::CreateLayout()
{
    autoFocusSettingGroup = new QGroupBox;
    
	autoFocusButton = new QPushButton( tr("AutoFocus") );
	QObject::connect( autoFocusButton, SIGNAL( clicked() ), this, SLOT(OnAutoFocusButton()));
   
    //set widgets property
    QHBoxLayout *autoFocusSettingLayout = new QHBoxLayout;
    autoFocusSettingLayout -> addWidget(new QLabel(tr("Z2 : Z Translation Axis for Zoom Lens")));
    autoFocusSettingLayout -> addWidget(autoFocusButton);
    autoFocusSettingLayout -> setMargin(5);
    autoFocusSettingLayout -> setSpacing(5);
    
    autoFocusSettingGroup -> setLayout(autoFocusSettingLayout);
    autoFocusSettingGroup -> setFlat(true);
}

void AutoFocusSetting::OnAutoFocusButton()
{
    if (z2_autoFocus != NULL){
        //z2_autoFocus = new Z2_AutoFocusDialog(this);
        //z2_autoFocus->show();
    //}
    //else{
        z2_autoFocus->raise();
        z2_autoFocus->show();
    }
}