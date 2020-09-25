######################################################################
# Automatically generated by qmake (3.0) Thu Jul 14 13:20:00 2016
######################################################################

TEMPLATE = app
TARGET = WormTracking
INCLUDEPATH += .

# Input
HEADERS += AboutDialog.h \
           AutoFocusSetting.h \
           DalsaCamera.h \
           DalsaCamera_Params.h \
           DevicePackage.h \
           DeviceParameterListWidget.h \
           DeviceParametersDialog.h \
           DeviceParametersWidget.h \
           DisplayWindow.h \
           DistanceSlider.h \
           GlobalParameters.h \
           GLWidget.h \
           GridTracking.h \
           ImageSaveSetting.h \
           MotionThread.h \
           PolyFitting.h \
           QException.h \
           ROISetting.h \
           RotationStage.h \
           serial.h \
           Stage.h \
           Stage_Params.h \
           ThreadPackage.h \
           TrackingControl.h \
           TrackingParametersSettingDialog.h \
           TranslationStage.h \
           TranslationStageControl.h \
           Util.h \
           VirtualCoordinates.h \
           WormCentroid.h \
           WormTracking.h \
           Z2_AutoFocus.h \
           Z2Stage.h \
           Worm_CV/Backbone.h \
           Worm_CV/Candidate_Points.h \
           Worm_CV/CONST_PARA.h \
           Worm_CV/Graph.h \
           Worm_CV/Graph_Builder.h \
           Worm_CV/Graph_Prune.h \
           Worm_CV/Graph_Structure.h \
           Worm_CV/Root_Smooth.h \
           Worm_CV/Search_Backbone.h \
           Worm_CV/stdafx.h
SOURCES += AboutDialog.cpp \
           AutoFocusSetting.cpp \
           DalsaCamera.cpp \
           depth.cpp \
           DeviceParameterListWidget.cpp \
           DeviceParametersDialog.cpp \
           DeviceParametersWidget.cpp \
           DisplayWindow.cpp \
           DistanceSlider.cpp \
           GLWidget.cpp \
           GridTracking.cpp \
           ImageSaveSetting.cpp \
           main.cpp \
           MotionThread.cpp \
           PolyFitting.cpp \
           QException.cpp \
           ROISetting.cpp \
           RotationStage.cpp \
           serial.cpp \
           ThreadPackage.cpp \
           TrackingControl.cpp \
           TrackingParametersSettingDialog.cpp \
           TranslationStage.cpp \
           TranslationStageControl.cpp \
           WormCentroid.cpp \
           WormTracking.cpp \
           Z2_AutoFocus.cpp \
           Z2Stage.cpp \
           Worm_CV/Backbone.cpp \
           Worm_CV/Candidate_Points.cpp \
           Worm_CV/Candidate_Points_Detect.cpp \
           Worm_CV/Graph.cpp \
           Worm_CV/Graph_Builder.cpp \
           Worm_CV/Graph_Prune.cpp \
           Worm_CV/Graph_Structure.cpp \
           Worm_CV/Root_Smooth.cpp \
           Worm_CV/Search_Backbone.cpp \
           pixelConvert.cu

QT += gui core widgets opengl