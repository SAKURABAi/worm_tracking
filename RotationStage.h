
/****************************************************************************
	RotationStage: rotation stage
****************************************************************************/
#ifndef _ROTATION_STAGE_H_
#define _ROTATION_STAGE_H_

#include "Galil.h"
#include "Stage.h"
#include "Stage_Params.h"

class RotationStage : public Stage
{
public:
	static string DEVICE_NAME;
	static string OBJECT_NAME;
	
	explicit RotationStage(Galil *controller);
	~RotationStage();

	//General control
	bool Connect();
	bool IsConnected();
	void Disconnect();

	//Motion control
	void Move_Openloop_Realtime(double distance);
	void Move_Openloop_Unrealtime(double distance);
	void Move_Closeloop_Realtime(double distance);
	void Move_Closeloop_Unrealtime(double distance);
	void Stop();
	bool ReturnOrigin();
	
	//Position acquisition
	double Get_CurrentPosition();
    void Set_CurrentPosition(double position);
    
	//Parameters settings and acquisition
	void Set_Speed(double speed);
	void Set_ACCSpeed(double acc);//set acceleration speed
	void Set_DECSpeed(double dec);//set deceleration speed
	double Get_Speed();
	double Get_ACCSpeed();
	double Get_DECSpeed();

private:
	DeviceStatus state;
	Galil *stage;
};

#endif
