
#include "DevicePackage.h"
#include "TrackingParametersSettingDialog.h"

#include <QtWidgets/QGroupBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>

#include <QtCore/QString>

TrackingParametersSettingDialog::TrackingParametersSettingDialog(QWidget *parent):QDialog(parent)
{
    //Load the last binary threshlod
    //LoadBinaryThreshold();
    CreateLayout();
}

void TrackingParametersSettingDialog::CreateLayout()
{
    /****** Reference Lines Setting ******/
    QGroupBox *refLineBox = new QGroupBox(tr("Reference Lines Setting"));
    withRefLineButton = new QCheckBox(tr("With Ref Lines"));
    withRefLineButton->setMaximumHeight(25);
    reverseROIPositionButton = new QPushButton(tr("Reverse ROI Position"));
    
    ROIPositionEdit = new QLineEdit();
    ROIPositionEdit->setMaximumWidth(60);
    ROIPositionEdit->setMinimumWidth(80);
    ROIPositionEdit->setText(QString::number(ROI_TRACKING_POSITON));

    //connect to slots
    QObject::connect( withRefLineButton, SIGNAL(stateChanged(int)), this, SLOT(OnRefLineButton(int)) );
    QObject::connect( reverseROIPositionButton, SIGNAL(clicked()), this, SLOT(OnReverseROIPositionButton()) );
    QObject::connect( ROIPositionEdit, SIGNAL(editingFinished()), this, SLOT(OnROIPositionChanched()) );

    QHBoxLayout *refLinesLayout = new QHBoxLayout;
    refLinesLayout->addWidget(withRefLineButton);
    refLinesLayout->setSpacing(5);
    refLinesLayout->addStretch();
    
    QHBoxLayout *reverseROILayout = new QHBoxLayout;
    reverseROILayout->addWidget(new QLabel("ROI Position"));
    reverseROILayout->addWidget(ROIPositionEdit);
    reverseROILayout->addWidget(reverseROIPositionButton);
    reverseROILayout->setSpacing(5);
    reverseROILayout->addStretch();

    QVBoxLayout *refLinesBoxLayout = new QVBoxLayout;
    refLinesBoxLayout->addLayout(refLinesLayout);
    refLinesBoxLayout->addLayout(reverseROILayout);
    refLinesBoxLayout->setMargin(0);
    refLinesBoxLayout->setSpacing(0);
    refLinesBoxLayout->addStretch();
    refLineBox->setLayout(refLinesBoxLayout);
    
    /****** Binary Threshold Setting ******/
    QGroupBox *binaryThresholdBox = new QGroupBox(tr("Binarization Setting"));
    adaptiveBinaryButton = new QCheckBox(tr("Adaptive Binary Threshold"));
    binaryThresholdEdit = new QLineEdit;
    binaryThresholdEdit->setMaximumWidth(90);
    addBinaryThresholdButton = new QPushButton(tr("+1"));
    addBinaryThresholdButton->setMaximumWidth(30);
    substractBinaryThresholdButton = new QPushButton(tr("-1"));
    substractBinaryThresholdButton->setMaximumWidth(30);
    binaryThresholdEdit->setText(QString::number(OPTIMAL_BINARY_THRESHOLD));
    
    //connect to slots
    QObject::connect( adaptiveBinaryButton, SIGNAL(stateChanged(int)), this, SLOT(OnAdaptiveBinaryThresholdButton(int)) );
    QObject::connect( binaryThresholdEdit, SIGNAL(returnPressed()), this, SLOT(OnBinaryThresholdEdit()) );
    QObject::connect( addBinaryThresholdButton, SIGNAL(clicked()), this, SLOT(OnAddBinaryThresholdButton()) );
    QObject::connect( substractBinaryThresholdButton, SIGNAL(clicked()), this, SLOT(OnSubstractBinaryThresholdButton()) );
    
    QHBoxLayout *binaryLayout = new QHBoxLayout;
    binaryLayout->addWidget(new QLabel(tr(" Binary Threshold")));
    binaryLayout->addWidget(binaryThresholdEdit);
    binaryLayout->addWidget(addBinaryThresholdButton);
    binaryLayout->addWidget(substractBinaryThresholdButton);
    binaryLayout->setMargin(0);
    binaryLayout->addStretch();
    
    QVBoxLayout *binarySettingLayout = new QVBoxLayout;
    binarySettingLayout->addWidget(adaptiveBinaryButton);
    binarySettingLayout->addLayout(binaryLayout);
    //binarySettingLayout->setContentsMargins(0,3,0,0);
    binarySettingLayout->setMargin(0);
    binarySettingLayout->setSpacing(0);
    
    binaryThresholdBox->setLayout(binarySettingLayout);
       
    /****** Offsets Post-Process ******/
    //QGroupBox *offsetProcessBox = new QGroupBox(tr("Offsets Post-Process"));
    	
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(refLineBox);
    mainLayout->addWidget(binaryThresholdBox);
    //mainLayout->addWidget(offsetProcessBox);
    
    setMaximumWidth(320);
    setLayout(mainLayout);
    setWindowTitle(tr("Tracking Parameters Setting"));
}

void TrackingParametersSettingDialog::OnRefLineButton(int state)
{
    if (state == Qt::Unchecked){
        DRAW_REFLINE_FLAG = false;
        withRefLineButton->setChecked(false);
    }
    else if(state == Qt::Checked){
        DRAW_REFLINE_FLAG = true;
        withRefLineButton->setChecked(true);
    }
}

void TrackingParametersSettingDialog::OnAdaptiveBinaryThresholdButton(int state)
{
    if (state == Qt::Unchecked){
        ADAPTIVE_BINARY_FLAG = false;
        adaptiveBinaryButton->setChecked(false);
    }
    else if(state == Qt::Checked){
        ADAPTIVE_BINARY_FLAG = true;
        adaptiveBinaryButton->setChecked(true);
    }
}

void TrackingParametersSettingDialog::OnBinaryThresholdEdit()
{
    OPTIMAL_BINARY_THRESHOLD = (binaryThresholdEdit->text()).toInt();
    SaveBinaryThreshold();
}

void TrackingParametersSettingDialog::OnAddBinaryThresholdButton()
{
    OPTIMAL_BINARY_THRESHOLD = OPTIMAL_BINARY_THRESHOLD + 1;
    binaryThresholdEdit -> setText(QString::number(OPTIMAL_BINARY_THRESHOLD));
    SaveBinaryThreshold();
}

void TrackingParametersSettingDialog::OnSubstractBinaryThresholdButton()
{
    OPTIMAL_BINARY_THRESHOLD = OPTIMAL_BINARY_THRESHOLD - 1;
    binaryThresholdEdit -> setText(QString::number(OPTIMAL_BINARY_THRESHOLD));
    SaveBinaryThreshold();
}

void TrackingParametersSettingDialog::OnReverseROIPositionButton()
{
    ROI_TRACKING_POSITON = 1.0 - ROI_TRACKING_POSITON;
}

void TrackingParametersSettingDialog::OnROIPositionChanched()
{
    double currentROI = ROIPositionEdit->text().toDouble();
    if (currentROI < 0 || currentROI > 1.0){
        QMessageBox::critical(this, "Error", "Invalid ROI Position");
    }
    ROI_TRACKING_POSITON = currentROI;
}

bool TrackingParametersSettingDialog::LoadBinaryThreshold()
 {
    FILE *file;
    char filename[256];
    int binaryThreshold;

    sprintf(filename, "config/binaryThreshold.cfg");
    file = fopen(filename, "rt");
    if(file == NULL){
        printf("%s", "Load Binary Threshold: cannot open the file");
        return false;
    }
    if (fscanf(file, "%d\n", &binaryThreshold) > 0){
        OPTIMAL_BINARY_THRESHOLD = binaryThreshold;
    }

    fclose(file);
    return true;
 }

bool TrackingParametersSettingDialog::SaveBinaryThreshold()
{
    FILE *file;
    char filename[256];

    sprintf(filename, "config/binaryThreshold.cfg");
    file = fopen(filename, "wt");
    if(file == NULL){
        printf("%s", "Save Binary Threshold: cannot open the file");
        return false;
    }
    fprintf(file, "%d\n", OPTIMAL_BINARY_THRESHOLD);

    fclose(file);
    return true;
}