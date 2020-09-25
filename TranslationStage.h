/****************************************************************************
TranslationStage 描述平移台的属性以及相关控制;
                 平移台具有二轴独立运动的能力,程序中分别对轴(X,Y)控制;
                 二轴均使用闭环运动(已由底层控制器完成),因而不需要程序实现;

                 注:程序中两轴的运动量均为脉冲,而非实际距离,因此在实际操作
                    时需要将其转化为对应的脉冲.
****************************************************************************/
#ifndef _TRANSLATION_STAGE_H
#define _TRANSLATION_STAGE_H

#include "Galil.h"
#include "QException.h"
#include "Util.h"
#include "Stage_Params.h"
#include "VirtualCoordinates.h"

using namespace VIRTUAL_COORDINATE;

class TranslationStage
{
public:
	explicit TranslationStage(Galil *controller);
	~TranslationStage();

	static string DEVICE_NAME; //device name
	static string OBJECT_NAME; //object name

	//General control
	bool Connect();
	bool IsConnected();
	void Disconnect();

	//Motion control
	void X_Move_Openloop_Realtime(double distance);
	void X_Move_Openloop_Unrealtime(double distance);
	void X_Move_Closeloop_Realtime(double distance);
	void X_Move_Closeloop_Unrealtime(double distance);
	void X_Stop();

	void Y_Move_Openloop_Realtime(double distance);
	void Y_Move_Openloop_Unrealtime(double distance);
	void Y_Move_Closeloop_Realtime(double distance);
	void Y_Move_Closeloop_Unrealtime(double distance);
	void Y_Stop();

	void XY_Move_Closeloop_Realtime(double x_distance, double y_distance);
	void Stop();
	bool X_ReturnOrigin();
	bool Y_ReturnOrigin();
	
	//Position acquisition
	double X_GetCurrentPosition();
	void X_SetCurrentPosition(long position);

	double Y_GetCurrentPosition();
    void Y_SetCurrentPosition(long position);
    
	//Parameters gettings and settings
	void X_Set_Speed(long speed);
	long X_Get_Speed();
	long X_Get_ACCSpeed();//get acceleration speed
	void X_Set_ACCSpeed(long acc);//set acceleration speed
	long X_Get_DECSpeed();//get deceleration speed
	void X_Set_DECSpeed(long dec);//set deceleration speed
	
	void X_Set_PID(PID x_pid);
	double X_Get_KI();
	void X_Set_KI(double ki);
	double X_Get_KP();
	void X_Set_KP(double kp);
	double X_Get_KD();
	void X_Set_KD(double kd);
	
	void Y_Set_Speed(long speed);
	long Y_Get_Speed();
    long Y_Get_ACCSpeed();//get acceleration speed
	void Y_Set_ACCSpeed(long acc);//set acceleration speed
	long Y_Get_DECSpeed();//get deceleration speed
	void Y_Set_DECSpeed(long dec);//set deceleration speed
	
	void Y_Set_PID(PID y_pid);
    double Y_Get_KI();
	void Y_Set_KI(double ki);
	double Y_Get_KP();
	void Y_Set_KP(double kp);
	double Y_Get_KD();
	void Y_Set_KD(double kd);
	
	//Special method(not recommended)
	void WriteCommand(string);

protected:

private:
	DeviceStatus state;
	Galil *stage;
};

#endif
