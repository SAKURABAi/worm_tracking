
#include "RotationStage.h"

string RotationStage::DEVICE_NAME = "Rotation Stage";
string RotationStage::OBJECT_NAME = "RotationStage";

RotationStage::RotationStage(Galil *controller)
{
	stage = controller;
	state = OPENED;
}

RotationStage::~RotationStage()
{
	Disconnect();
}

/******* general control *******/
bool RotationStage::Connect()
{
	bool success = false;
	char strCommand[32];
	try{
		stage->write("SHW\r");
		//stage->command("SHW","\r",":");
		sprintf(strCommand, "SPW=%ld", ROTATION_STAGE::W_SPEED);
		stage->command(string(strCommand));
		usleep(100000);

		success = true;
		state = CONNECTED;
	} catch (string e){
		throw QException(OBJECT_NAME, "Connect", e);
	}
	
	return success;
}

bool RotationStage::IsConnected()
{
	return ((stage != NULL) && (state == CONNECTED));
}

void RotationStage::Disconnect()
{
	try{
		Stop();
	} catch (QException e){}

	stage = NULL;
	state = DISCONNECTED;
}

/******* motion control *******/
void RotationStage::Move_Openloop_Realtime(double distance)
{
	long step = long(distance);
	char strCommand[32];
	if (step == 0 || !IsConnected()){return;}

	try{
		sprintf(strCommand,"%s%ld","IPW=",step);
		stage->command(string(strCommand), "\r", ":", true);
	}catch(string e){
		throw QException(OBJECT_NAME, "Move_Openloop_Realtime", e);
	}
}

void RotationStage::Move_Openloop_Unrealtime(double distance)
{
	char strCommand[32];
	if (long(distance) == 0 || !IsConnected()){return;}

	try{
		//Query stage busy state
		while (true){
			if (stage->commandValue("MG _BGW")==0){ break; }
			usleep(ROTATION_STAGE::STAGE_WAITING);
		}
		//Send motion instruction
		sprintf(strCommand,"%s%ld","IPW=", long(distance));
		stage->command(string(strCommand), "\r", ":", true);
		double motionTime = Get_MotionTime(ROTATION_STAGE::W_SPEED, distance);
		usleep(long(2*motionTime));

		//Wait to finish
		while (true){
			if (stage->commandValue("MG _BGW")==0){break;}
			usleep(ROTATION_STAGE::STAGE_WAITING);
		}
	}catch(string e){
		throw QException(OBJECT_NAME, "Move_Openloop_Unrealtime", e);
	}
}

void RotationStage::Move_Closeloop_Realtime(double distance)
{
}

void RotationStage::Move_Closeloop_Unrealtime(double distance)
{
}

void RotationStage::Stop()
{
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Stop", "No Stage Connection");
	}
	else{
    	try{
    		stage->write("STW;\r");
    	}catch(string e){
    		throw QException(OBJECT_NAME, "Stop", e);
    	}
	}
}

bool RotationStage::ReturnOrigin()
{
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "ReturnOrigin", "No Stage Connection");
    	return false;
	}
	return true;
}

/******* position acquisition *******/
double RotationStage::Get_CurrentPosition()
{
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Get_CurrentPosition", "No Stage Connection");
	}
	else{
        try{
            return stage->commandValue("MG _RPW");
        }
        catch(string e){
            throw QException(OBJECT_NAME, "Get_CurrentPosition", e);
        }
    }
}

void RotationStage::Set_CurrentPosition(double position)
{
    char strCommand[32];
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Set_CurrentPosition", "No Stage Connection");
	}
	else{
    	try{
        	sprintf(strCommand,"DPW=%ld",long(position));
    		stage->command(strCommand, "\r", ":", true);
    	}catch (string e){
        	throw QException(OBJECT_NAME, "Set_CurrentPosition", e);
    	}
	}
}

/******* parameters settings and acquisition *******/
void RotationStage::Set_Speed(double speed)
{
	char strCommand[32];
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Set_Speed", "No Stage Connection");
	}
	else{
    	try{
    		sprintf(strCommand,"%s%ld","SPW=",long(speed));
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch(string e){
    		throw QException(OBJECT_NAME, "Set_Speed", e);
    	}
	}
}

//set acceleration speed
void RotationStage::Set_ACCSpeed(double acc)
{
	char strCommand[32];
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Set_ACCSpeed", "No Stage Connection");
	}
	else{
    	try{
    		sprintf(strCommand,"%s%ld","ACW=",long(acc));
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch(string e){
    		throw QException(OBJECT_NAME, "Set_ACCSpeed", e);
    	}
	}
}

//set deceleration speed
void RotationStage::Set_DECSpeed(double dec)
{
	char strCommand[32];
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Set_DECSpeed", "No Stage Connection");
	}
	else{
    	try{
    		sprintf(strCommand,"%s%ld","DCW=",long(dec));
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch(string e){
    		throw QException(OBJECT_NAME, "Set_DECSpeed", e);
    	}
	}
}

double RotationStage::Get_Speed()
{
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Get_Speed", "No Stage Connection");
	}
	else{
    	try{
    		return stage->commandValue("MG _SPW");
    	}catch (string e){
    		throw QException(OBJECT_NAME, "Get_Speed", e);
    	}
	}
}

double RotationStage::Get_ACCSpeed()
{
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Get_ACCSpeed", "No Stage Connection");
	}
	else{
    	try{
    		return stage->commandValue("MG _ACW");
    	}catch (string e){
    		throw QException(OBJECT_NAME, "Get_ACCSpeed", e);
    	}
	}
}

double RotationStage::Get_DECSpeed()
{
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Get_DECSpeed", "No Stage Connection");
	}
	else{
    	try{
    		return stage->commandValue("MG _DCW");
    	}catch (string e){
    		throw QException(OBJECT_NAME, "Get_DECSpeed", e);
    	}
	}
}
