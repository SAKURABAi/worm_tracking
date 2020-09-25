/****************************************************************************
    AutoFocusSetting : Auto Focus配置
****************************************************************************/
#ifndef _AUTOFOCUS_SETTING_H_
#define _AUTOFOCUS_SETTING_H_

#include "Z2_AutoFocus.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGroupBox>
#include <QtCore/QString>

class AutoFocusSetting : public QWidget
{
    Q_OBJECT
public:
    static string OBJECT_NAME;
    AutoFocusSetting(QWidget *parent=0);
    ~AutoFocusSetting();
    
    void CreateLayout();
    inline void SetTitle(const QString &title){ 
        autoFocusSettingGroup->setTitle(title); 
    }
    inline QGroupBox* Get_AutoFocusSettingGroup() const { 
        return autoFocusSettingGroup; 
    }

    void SetAutoFocusButtonEnabled(bool ok){
        autoFocusButton->setEnabled(ok);
    }

protected slots:
	void OnAutoFocusButton();

protected:

private:
    QGroupBox *autoFocusSettingGroup;
    QPushButton *autoFocusButton;
    Z2_AutoFocusDialog *z2_autoFocus;
};

#endif
