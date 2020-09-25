/****************************************************************************
	Z2Stage: Z2 stage
****************************************************************************/

#ifndef _Z2Stage_H_
#define _Z2Stage_H_

#include "Galil.h"
#include "Stage.h"
#include "Stage_Params.h"
#include "Util.h"
#include "VirtualCoordinates.h"
using namespace VIRTUAL_COORDINATE;

class Z2Stage : public Stage
{
public:
	explicit Z2Stage(Galil *controller);
	~Z2Stage();

	static string DEVICE_NAME;
	static string OBJECT_NAME;

	//General control
	bool Connect();
	bool IsConnected();
	void Disconnect();

	//Motion
	void Move_Openloop_Realtime(double distance);
	void Move_Openloop_Unrealtime(double distance);
	void Move_Closeloop_Realtime(double distance);
	void Move_Closeloop_Unrealtime(double distance);
	void Stop();
	bool ReturnOrigin();
	bool InOrigin();

	//Position
	double Get_CurrentPosition();
	void Set_CurrentPosition(double position);
	
	//Parameters
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
