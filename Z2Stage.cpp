
#include "Z2Stage.h"

string Z2Stage::DEVICE_NAME = "Z2 Stage";
string Z2Stage::OBJECT_NAME = "Z2Stage";

Z2Stage::Z2Stage(Galil *controller)
{
    stage = controller;
	state = OPENED;
}

Z2Stage::~Z2Stage()
{
	Disconnect();
}

bool Z2Stage::Connect()
{
	bool success = false;
	char strCommand[32];
	try{
		stage->write("SHZ\r");
		//stage->command("SHZ","\r",":");
		sprintf(strCommand, "SPZ=%ld", Z2_STAGE::Z2_SPEED);
		stage->command(string(strCommand));
		usleep(100000);

		success = true;
		state = CONNECTED;
	} catch (string e){
		throw QException(OBJECT_NAME, "Connect", e);
	}
	
	return success;
}

bool Z2Stage::IsConnected()
{
	return ((stage != NULL) && (state == CONNECTED));
}

void Z2Stage::Disconnect()
{
	try{
		Stop();
	} catch (QException e){}

	stage = NULL;
	state = DISCONNECTED;
}

void Z2Stage::Move_Openloop_Realtime(double pulse)
{
	char strCommand[32];
	if (long(pulse) == 0 || !IsConnected()){return;}

	try{
		sprintf(strCommand,"%s%ld","IPZ=",long(pulse));
		stage->command(string(strCommand), "\r", ":", true);
	}catch(string e){
		throw QException(OBJECT_NAME, "Move_Openloop_Unrealtime", e);
	}
}

void Z2Stage::Move_Openloop_Unrealtime(double pulse)
{
	char strCommand[32];
	if (long(pulse) == 0 || !IsConnected()){return;}

	try{
		//Query stage busy state
		while (true){
			if (stage->commandValue("MG _BGZ")==0){break;}
			usleep(Z2_STAGE::STAGE_WAITING);
		}

		//Send motion instruction
		sprintf(strCommand,"%s%ld","IPZ=",long(pulse));
		stage->command(string(strCommand), "\r", ":", true);
		double motionTime = Get_MotionTime(Z2_STAGE::Z2_SPEED, pulse);
		usleep(long(2*motionTime));

		//Wait to finish
		while (true){
			if (stage->commandValue("MG _BGZ")==0){break;}
			usleep(Z2_STAGE::STAGE_WAITING);
		}
	}catch(string e){
		throw QException(OBJECT_NAME, "Move_Openloop_Unrealtime", e);
	}
}

void Z2Stage::Move_Closeloop_Realtime(double pulse)
{
}

void Z2Stage::Move_Closeloop_Unrealtime(double pulse)
{
	long z_ideal = long(pulse), z_input = z_ideal, z_current = 0, z_start = 0;
	char strCommand[32];

	if (z_ideal == 0 || !IsConnected()){return;}

	try{
		//Query stage busy state
		while (true){
			if (stage->commandValue("MG _BGZ")==0){break;}
			usleep(Z2_STAGE::STAGE_WAITING);
		}

		//Get the initial position
		z_start = (long)stage->commandValue("MG _RPZ");

		while(true){
			//get current z2 stage position and calculate the offset between current
			//position and the ideal position, then move the offset iteratively
			sprintf(strCommand,"%s%ld","IPZ=",z_input);
			stage->command(string(strCommand), "\r", ":", true);
			double motionTime = Get_MotionTime(Z2_STAGE::Z2_SPEED, z_input);
			usleep(long(2*motionTime));

			//wait to finish motion instruction
			while (true){
				if (stage->commandValue("MG _BGZ")==0){break;}
				usleep(Z2_STAGE::STAGE_WAITING);
			}

			z_current = (long)stage->commandValue("MG _RPZ");
			z_input = z_ideal-(z_current-z_start);
			if (abs(z_input) < Z2_STAGE::Z2_TOL){break;}
		}
	}catch(string e){
		throw QException(OBJECT_NAME,"Move_Closeloop_Unrealtime",e);
	}
}

void Z2Stage::Stop()
{
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Stop", "No Stage Connection");
	}
	else{
    	try{
    		stage->write("STZ\r");//控制器急停指令
    	}catch(string e){
    		throw QException(OBJECT_NAME,"Stop",e);
    	}
	}
}

bool Z2Stage::InOrigin()
{
	if (!IsConnected()){ return false; }

	try{
		int isOrigin = (int)stage->commandValue("MG _LRZ");
		return (isOrigin == 0);
	} catch(string e){
		throw QException(OBJECT_NAME,"InOrigin",e);
		return false;
	}
}

bool Z2Stage::ReturnOrigin()
{
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "ReturnOrigin", "No Stage Connection");
	}
	
	int isOrigin = 1;
	try{
		while (true){
			isOrigin = (int)stage->commandValue("MG _LRZ"); //Check reverse switch

			//Reach reverse switch
			if (isOrigin == 0){ 
				break;
			}

			stage->command("IPZ=-1000");
			double motionTime = Get_MotionTime(Z2_STAGE::Z2_SPEED, 1000);
			usleep(long(2.0*motionTime));
		}
	}catch(string e){
		throw QException(OBJECT_NAME,"ReturnOrigin",e);
	}
	
	return true;
}

double Z2Stage::Get_CurrentPosition()
{
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Get_CurrentPosition", "No Stage Connection");
	}
	else{
        try{
            return stage->commandValue("MG _RPZ");//由于没有使用光栅尺，因此使用辅助编码器的数度获取当前位置。因此需要使用DP，DE指令设置位置
        }
        catch(string e){
            throw QException(OBJECT_NAME, "Get_CurrentPosition", e);
        }
    }
}

void Z2Stage::Set_CurrentPosition(double position)
{
    char strCommand[32];
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Set_CurrentPosition", "No Stage Connection");
	}
	else{
    	try{
        	sprintf(strCommand,"DPZ=%ld",long(position));
    		stage->command(strCommand, "\r", ":", true);
    	}catch (string e){
        	throw QException(OBJECT_NAME, "Set_CurrentPosition", e);
    	}
	}
}

void Z2Stage::Set_Speed(double speed)
{
	char strCommand[32];
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Set_Speed", "No Stage Connection");
	}
	else{
    	try{
    		sprintf(strCommand,"%s%ld","SPZ=",long(speed));
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch(string e){
    		throw QException(OBJECT_NAME, "Set_Speed", e);
    	}
	}
}

//set acceleration speed
void Z2Stage::Set_ACCSpeed(double acc)
{
	char strCommand[32];
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Set_ACCSpeed", "No Stage Connection");
	}
	else{
    	try{
    		sprintf(strCommand,"%s%ld","ACZ=",long(acc));
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch(string e){
    		throw QException(OBJECT_NAME, "Set_ACCSpeed", e);
    	}
	}
}

//set deceleration speed
void Z2Stage::Set_DECSpeed(double dec)
{
	char strCommand[32];
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Set_DECSpeed", "No Stage Connection");
	}
	else{
    	try{
    		sprintf(strCommand,"%s%ld","DCZ=",long(dec));
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch(string e){
    		throw QException(OBJECT_NAME, "Set_DECSpeed", e);
    	}
	}
}

double Z2Stage::Get_Speed()
{
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Get_Speed", "No Stage Connection");
	}
	else{
    	try{
    		return stage->commandValue("MG _SPZ");
    	}catch (string e){
    		throw QException(OBJECT_NAME, "Get_Speed", e);
    	}
	}
}

double Z2Stage::Get_ACCSpeed()
{
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Get_ACCSpeed", "No Stage Connection");
	}
	else{
    	try{
    		return stage->commandValue("MG _ACZ");
    	}catch (string e){
    		throw QException(OBJECT_NAME, "Get_Speed", e);
    	}
	}
}

double Z2Stage::Get_DECSpeed()
{
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Get_DECSpeed", "No Stage Connection");
	}
	else{
    	try{
    		return stage->commandValue("MG _DCZ");
    	}catch (string e){
    		throw QException(OBJECT_NAME, "Get_Speed", e);
    	}
	}
}
