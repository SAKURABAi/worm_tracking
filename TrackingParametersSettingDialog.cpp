
#include "DevicePackage.h"
#include "TrackingParametersSettingDialog.h"
#include "Worm_CV/CONST_PARA.h"

#include <QtWidgets/QGroupBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtCore/QString>

TrackingParametersSettingDialog::TrackingParametersSettingDialog(TranslationStage* translationStage,QWidget *parent):QDialog(parent)
{
    //Load the last binary threshlod
    LoadBinaryThreshold();

    CreateLayout();
    //SetBinaryThresholdAvailable(false);
    
    stage = translationStage;
    OnTranslationStageConnected();
    
    //Set Current PID
    x_PID.KP = TRANSLATION_STAGE::X_PID_INITIAL.KP;
    x_PID.KI = TRANSLATION_STAGE::X_PID_INITIAL.KI;
    x_PID.KD = TRANSLATION_STAGE::X_PID_INITIAL.KD;
    y_PID.KP = TRANSLATION_STAGE::Y_PID_INITIAL.KP;
    y_PID.KI = TRANSLATION_STAGE::Y_PID_INITIAL.KI;
    y_PID.KD = TRANSLATION_STAGE::Y_PID_INITIAL.KD;
}

TrackingParametersSettingDialog::~TrackingParametersSettingDialog()
{
    if (stage!=NULL){
        stage = NULL;
    }
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

    adaptivePIDBox = new QCheckBox("Adaptive PID");

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
    binaryThresholdEdit = new QLineEdit;
    addBinaryThresholdButton = new QPushButton(tr("+1"));
    substractBinaryThresholdButton = new QPushButton(tr("-1"));
    binaryThresholdEdit->setMaximumWidth(90);
    addBinaryThresholdButton->setMaximumWidth(50);
    substractBinaryThresholdButton->setMaximumWidth(50);
    binaryThresholdEdit->setText(QString::number(BW::BINARY_THRESHOLD));
    
    //connect to slots
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
    binarySettingLayout->addLayout(binaryLayout);
    //binarySettingLayout->setContentsMargins(0,3,0,0);
    binarySettingLayout->setMargin(0);
    binarySettingLayout->setSpacing(3);
    binaryThresholdBox->setLayout(binarySettingLayout);
    
    /****** Worm Area Setting ******/
    QGroupBox *wormAreaBox = new QGroupBox(tr("Worm Area Setting"));
    wormAreaEdit = new QLineEdit;
    addWormAreaButton = new QPushButton(tr("+10%"));
    substractWormAreaButton = new QPushButton(tr("-10%"));
    wormAreaEdit->setMaximumWidth(122);
    wormAreaEdit->setMinimumWidth(122);
    addWormAreaButton->setMaximumWidth(50);
    substractWormAreaButton->setMaximumWidth(50);
    wormAreaEdit->setText(QString::number(BW::INITIAL_WORM_AREA));

    //connect to slots
    QObject::connect( wormAreaEdit, SIGNAL(returnPressed()), this, SLOT(OnWormAreaEdit()) );
    QObject::connect( addWormAreaButton, SIGNAL(clicked()), this, SLOT(OnAddWormAreaButton()) );
    QObject::connect( substractWormAreaButton, SIGNAL(clicked()), this, SLOT(OnSubstractWormAreaButton()) );
    
    QHBoxLayout *wormAreaLayout = new QHBoxLayout;
    wormAreaLayout->addWidget(new QLabel(tr(" Worm Area")));
    wormAreaLayout->addWidget(wormAreaEdit);
    wormAreaLayout->addWidget(addWormAreaButton);
    wormAreaLayout->addWidget(substractWormAreaButton);
    wormAreaLayout->setMargin(0);
    wormAreaLayout->addStretch();
    
    QVBoxLayout *wormAreaSettingLayout = new QVBoxLayout;
    wormAreaSettingLayout->addLayout(wormAreaLayout);
    //wormAreaSettingLayout->setContentsMargins(0,3,0,0);
    wormAreaSettingLayout->setMargin(0);
    wormAreaSettingLayout->setSpacing(3);
    wormAreaBox->setLayout(wormAreaSettingLayout);

    /****** PID Selection ******/
    QGroupBox *PidSelectionBox = new QGroupBox(tr("PID Selection"));
    PID_Initial_Button = new QRadioButton();
    QString pid_initial = tr("PID0: ") + Get_PID_Description(TRANSLATION_STAGE::X_PID_INITIAL,TRANSLATION_STAGE::Y_PID_INITIAL);
    PID_Initial_Button->setText(pid_initial);
    
    PID1_Button = new QRadioButton();
    QString pid1 = tr("PID1: ") + Get_PID_Description(TRANSLATION_STAGE::X_PID_10,TRANSLATION_STAGE::Y_PID_10);
    PID1_Button->setText(pid1);
    
    PID2_Button = new QRadioButton();
    QString pid2 = tr("PID2: ") + Get_PID_Description(TRANSLATION_STAGE::X_PID_20,TRANSLATION_STAGE::Y_PID_20);
    PID2_Button->setText(pid2);
    
    PID3_Button = new QRadioButton();
    QString pid3 = tr("PID3: ") + Get_PID_Description(TRANSLATION_STAGE::X_PID_30,TRANSLATION_STAGE::Y_PID_30);
    PID3_Button->setText(pid3);
    
    PID4_Button = new QRadioButton();
    QString pid4 = tr("PID4: ") + Get_PID_Description(TRANSLATION_STAGE::X_PID_40,TRANSLATION_STAGE::Y_PID_40);
    PID4_Button->setText(pid4);
    
    PID5_Button = new QRadioButton();
    QString pid5 = tr("PID5: ") + Get_PID_Description(TRANSLATION_STAGE::X_PID_50,TRANSLATION_STAGE::Y_PID_50);
    PID5_Button->setText(pid5);
    
    /*PID6_Button = new QRadioButton();
    QString pid6 = tr("PID6: ") + Get_PID_Description(TRANSLATION_STAGE::X_PID_60,TRANSLATION_STAGE::Y_PID_60);
    PID6_Button->setText(pid6);*/
    
    //connect to slots
    QObject::connect( adaptivePIDBox, SIGNAL(stateChanged(int)), this, SLOT(OnAdaptivePIDSelected(int)));
    QObject::connect( PID_Initial_Button, SIGNAL(clicked()), this, SLOT(OnPIDSelection()));
    QObject::connect( PID1_Button, SIGNAL(clicked()), this, SLOT(OnPIDSelection()) );
    QObject::connect( PID2_Button, SIGNAL(clicked()), this, SLOT(OnPIDSelection()) );
    QObject::connect( PID3_Button, SIGNAL(clicked()), this, SLOT(OnPIDSelection()) );
    QObject::connect( PID4_Button, SIGNAL(clicked()), this, SLOT(OnPIDSelection()) );
    QObject::connect( PID5_Button, SIGNAL(clicked()), this, SLOT(OnPIDSelection()) );
    //QObject::connect( PID6_Button, SIGNAL(clicked()), this, SLOT(OnPIDSelection()) );
    
    QVBoxLayout *pidLayout = new QVBoxLayout;
    pidLayout->addWidget(adaptivePIDBox);
    pidLayout->addWidget(PID_Initial_Button);
    pidLayout->addWidget(PID1_Button);
    pidLayout->addWidget(PID2_Button);
    pidLayout->addWidget(PID3_Button);
    pidLayout->addWidget(PID4_Button);
    pidLayout->addWidget(PID5_Button);
    //pidLayout->addWidget(PID6_Button);
    pidLayout->setContentsMargins(0,3,0,0);
    pidLayout->setSpacing(5);
    pidLayout->addStretch();
    
    PidSelectionBox->setLayout(pidLayout);
    
    /****** Offsets Post-Process ******/
    //QGroupBox *offsetProcessBox = new QGroupBox(tr("Offsets Post-Process"));
    	
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(refLineBox);
    mainLayout->addWidget(binaryThresholdBox);
    mainLayout->addWidget(wormAreaBox);
    mainLayout->addWidget(PidSelectionBox);
    //mainLayout->addWidget(offsetProcessBox);
    
    setMaximumWidth(320);
    setLayout(mainLayout);
    setWindowTitle(tr("Tracking Parameters Setting"));
}

void TrackingParametersSettingDialog::OnTranslationStageConnected()
{
    try{
        /*if (controller1 == NULL){
            QMessageBox::critical(NULL,"Warning"," Motorized translation stage not connected");
            return;
        }
        stage = new TranslationStage(controller1); //controller1 must be allocated!*/
        if (stage == NULL || !stage->IsConnected()){
            QMessageBox::critical(NULL,"Warning"," Motorized translation stage not connected");
            return;
        }
        stage->Connect();
    }catch (string e){
        QMessageBox::critical(NULL,"Warning",QString::fromStdString(e));
    }catch (QException e){
        QMessageBox::critical(NULL,"Warning",QString::fromStdString(e.getMessage()));
    }
}

void TrackingParametersSettingDialog::OnTranslationStageDisconnected()
{
    if (stage!=NULL){
    	delete stage;
    	stage = NULL;
	}
}
    
void TrackingParametersSettingDialog::SetCurrentPID(PID x_pid, PID y_pid)
{
    if (stage!=NULL && stage->IsConnected()){
        try{
            stage->X_Set_PID(x_pid);
            stage->Y_Set_PID(y_pid);
            cout <<"Set translation stage PID successfully"<<endl;
        } catch(QException e){
            cout<<e.getMessage()<<endl;
            return;
        }
    
        //Set Current PID
        x_PID.SP = x_pid.SP;
        x_PID.AC = x_pid.AC;
        x_PID.DC = x_pid.DC;
        x_PID.KP = x_pid.KP;
        x_PID.KI = x_pid.KI;
        x_PID.KD = x_pid.KD;

        y_PID.SP = y_pid.SP;
        y_PID.AC = y_pid.AC;
        y_PID.DC = y_pid.DC;
        y_PID.KP = y_pid.KP;
        y_PID.KI = y_pid.KI;
        y_PID.KD = y_pid.KD;
    }
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

void TrackingParametersSettingDialog::SetBinaryThresholdAvailable(bool available)
{
    binaryThresholdEdit->setEnabled(available);
    addBinaryThresholdButton->setEnabled(available);
    substractBinaryThresholdButton->setEnabled(available);
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
    //SaveBinaryThreshold();
    BW::BINARY_THRESHOLD = OPTIMAL_BINARY_THRESHOLD;
}

void TrackingParametersSettingDialog::OnSubstractBinaryThresholdButton()
{
    OPTIMAL_BINARY_THRESHOLD = OPTIMAL_BINARY_THRESHOLD - 1;
    binaryThresholdEdit -> setText(QString::number(OPTIMAL_BINARY_THRESHOLD));
    //SaveBinaryThreshold();
    BW::BINARY_THRESHOLD = OPTIMAL_BINARY_THRESHOLD;
}

void TrackingParametersSettingDialog::OnWormAreaEdit()
{
    BW::INITIAL_WORM_AREA = (wormAreaEdit->text()).toDouble();
    emit UpdateWormAreaSignal();
}

void TrackingParametersSettingDialog::OnAddWormAreaButton()
{
    BW::INITIAL_WORM_AREA = BW::INITIAL_WORM_AREA*( 1 + 0.1);
    wormAreaEdit -> setText(QString::number(BW::INITIAL_WORM_AREA));
    emit UpdateWormAreaSignal();
}

void TrackingParametersSettingDialog::OnSubstractWormAreaButton()
{
    BW::INITIAL_WORM_AREA = BW::INITIAL_WORM_AREA*( 1 - 0.1);
    wormAreaEdit -> setText(QString::number(BW::INITIAL_WORM_AREA));
    emit UpdateWormAreaSignal();
}

void TrackingParametersSettingDialog::OnPIDSelection()
{
    if (PID_Initial_Button->isChecked())
    {
        PID_Initial_Button->setChecked(true);
        PID1_Button->setChecked(false);
        PID2_Button->setChecked(false);
        PID3_Button->setChecked(false);
        PID4_Button->setChecked(false);
        PID5_Button->setChecked(false);
        //PID6_Button->setChecked(false);
        
        SetCurrentPID(TRANSLATION_STAGE::X_PID_INITIAL,TRANSLATION_STAGE::Y_PID_INITIAL);//Set Current PID
    }
    else if (PID1_Button->isChecked())
    {
        PID_Initial_Button->setChecked(false);
        PID1_Button->setChecked(true);
        PID2_Button->setChecked(false);
        PID3_Button->setChecked(false);
        PID4_Button->setChecked(false);
        PID5_Button->setChecked(false);
        //PID6_Button->setChecked(false);
        
        SetCurrentPID(TRANSLATION_STAGE::X_PID_10,TRANSLATION_STAGE::Y_PID_10);//Set Current PID
    }
    else if (PID2_Button->isChecked())
    {
        PID_Initial_Button->setChecked(false);
        PID1_Button->setChecked(false);
        PID2_Button->setChecked(true);
        PID3_Button->setChecked(false);
        PID4_Button->setChecked(false);
        PID5_Button->setChecked(false);
        //PID6_Button->setChecked(false);
        
        SetCurrentPID(TRANSLATION_STAGE::X_PID_20,TRANSLATION_STAGE::Y_PID_20);//Set Current PID
    }
    else if (PID3_Button->isChecked())
    {
        PID_Initial_Button->setChecked(false);
        PID1_Button->setChecked(false);
        PID2_Button->setChecked(false);
        PID3_Button->setChecked(true);
        PID4_Button->setChecked(false);
        PID5_Button->setChecked(false);
        //PID6_Button->setChecked(false);
        
        SetCurrentPID(TRANSLATION_STAGE::X_PID_30,TRANSLATION_STAGE::Y_PID_30);//Set Current PID
    }
    else if (PID4_Button->isChecked())
    {
        PID_Initial_Button->setChecked(false);
        PID1_Button->setChecked(false);
        PID2_Button->setChecked(false);
        PID3_Button->setChecked(false);
        PID4_Button->setChecked(true);
        PID5_Button->setChecked(false);
        //PID6_Button->setChecked(false);
        
        SetCurrentPID(TRANSLATION_STAGE::X_PID_40,TRANSLATION_STAGE::Y_PID_40);//Set Current PID
    }
    else if (PID5_Button->isChecked())
    {
        PID_Initial_Button->setChecked(false);
        PID1_Button->setChecked(false);
        PID2_Button->setChecked(false);
        PID3_Button->setChecked(false);
        PID4_Button->setChecked(false);
        PID5_Button->setChecked(true);
        //PID6_Button->setChecked(false);
        
        SetCurrentPID(TRANSLATION_STAGE::X_PID_50,TRANSLATION_STAGE::Y_PID_50);//Set Current PID
    }
    /*else if (PID6_Button->isChecked())
    {
        PID_Initial_Button->setChecked(false);
        PID1_Button->setChecked(false);
        PID2_Button->setChecked(false);
        PID3_Button->setChecked(false);
        PID4_Button->setChecked(false);
        PID5_Button->setChecked(false);
        PID6_Button->setChecked(true);
        
        SetCurrentPID(TRANSLATION_STAGE::X_PID_60,TRANSLATION_STAGE::Y_PID_60);//Set Current PID
    }*/
}

void TrackingParametersSettingDialog::OnReverseROIPositionButton()
{
    BACKBONE_REVERSE = !BACKBONE_REVERSE;
}

void TrackingParametersSettingDialog::OnROIPositionChanched()
{
    double currentROI = ROIPositionEdit->text().toDouble();
    if (currentROI < 0 || currentROI > 1.0){
        QMessageBox::critical(this, "Error", "Invalid ROI Position");
    }
    ROI_TRACKING_POSITON = currentROI;
}

void TrackingParametersSettingDialog::OnAdaptivePIDSelected(int state)
{
    if (adaptivePIDBox->isChecked()){
        ADAPTIVE_PID = true;
        PID_Initial_Button->setEnabled(false);
        PID1_Button->setEnabled(false);
        PID2_Button->setEnabled(false);
        PID3_Button->setEnabled(false);
        PID4_Button->setEnabled(false);
        PID5_Button->setEnabled(false);
        //PID6_Button->setEnabled(false);

        //Excuting Adaptive PID program in stage ROM (Program name: AUTO)
        stage->WriteCommand("XQ#WORM\r");
    }
    else{
        ADAPTIVE_PID = false;
        PID_Initial_Button->setEnabled(true);
        PID1_Button->setEnabled(true);
        PID2_Button->setEnabled(true);
        PID3_Button->setEnabled(true);
        PID4_Button->setEnabled(true);
        PID5_Button->setEnabled(true);
        //PID6_Button->setEnabled(true); 

        //Excuting Exit Adaptive PID program in stage ROM (Program name: EXIT)
        stage->WriteCommand("XQ#EXIT\r");

        //Set current PID to Initial PID !!!
        //SetCurrentPID(TRANSLATION_STAGE::X_PID_INITIAL,TRANSLATION_STAGE::Y_PID_INITIAL);
    }
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