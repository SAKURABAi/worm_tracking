
/****************************************************************************
    Dalsa Camera 相机控制，例如曝光时间，帧频和roi大小等
    DALSA相机串口通信，串口为/dev/corser/x64ExpCL4x1_s0 
    相机命令如下：
    aih   area of interest - height     2~1728
    aiw   area of interest - width      512~2432
    aiy   area of interest - offset y   0~0
    aix   area of interest - offset x   0~0
    set   set exposure time             20~199925
    ssf   set sync frequency            1.000000~140.845070 [Hz]
****************************************************************************/
#ifndef _DALSA_CAMERA_H_
#define _DALSA_CAMERA_H_

#include "DalsaCamera_Params.h"
#include "GlobalParameters.h"
#include "serial.h"

#include <QtCore/QObject>

class DalsaCamera : public QObject
{
	Q_OBJECT
public:
	static string DEVICE_NAME; //device name
	static string OBJECT_NAME;

	explicit DalsaCamera(QObject *parent=0);
	~DalsaCamera();

	bool Connect();
	bool IsConnected();
    void Disconnect();
       
	inline void SetROI(UINT32 width, UINT32 height, UINT32 x_offset, UINT32 y_offset){
		RoiLeft = x_offset;
	    RoiTop = y_offset;
	    RoiWidth = width;
	    RoiHeight = height;
	}
	
	bool GetExposureTime(UINT64 &);
	void SetExposureTime(UINT64);
	bool GetFrameRate(UINT32 &);
	void SetFrameRate(UINT32);
	void SetTriggerMode(DALSA_TRIGGERMODE mode);
	bool GetTriggerMode(DALSA_TRIGGERMODE &mode);
	
signals:
   void Broadcast_Image_Recieved();
   
protected:
	void Excute();
	void ExtractROIFromImage(uchar* src, UINT32 width, UINT32 height, UINT32 x_offset, UINT32 y_offset, UINT32 rowBytes);
	static void AcqCallback(SapXferCallbackInfo *pInfo);
	bool Test_Connected();
	 
private:
    bool ValidateValue(char *valueText, int num);//验证返回值是否有效
    
	DeviceStatus deviceState;
	SerialStatus serialState;
	
	string portname;//serial port for dalsa camera
	QSerial *dalsaSerial;

	MY_CONTEXT context;
    SapAcquisition *Acq;
    SapBuffer *Buffers;
    SapTransfer *Xfer;
    SapXferPair* XferPair;

    UINT32 RoiLeft;
    UINT32 RoiTop;
    UINT32 RoiWidth;
    UINT32 RoiHeight;

    DalsaImage mDalsaImage;
    char CameraValue[64];
};

#endif
