
#include "TranslationStage.h"
#include <string>
#include <iostream>

string TranslationStage::DEVICE_NAME = "Translation Stage";
string TranslationStage::OBJECT_NAME = "TranslationStage";

TranslationStage::TranslationStage(Galil *controller)
{
	stage = controller;
	state = OPENED;
}

TranslationStage::~TranslationStage()
{
	Disconnect();
}

bool TranslationStage::Connect()
{
    bool success = false;
    char strCommand[32];
	try{
		stage->write("SHXY\r");
        //stage->command("SHXY","\r",":");

        sprintf(strCommand, "SPX=%ld\r", TRANSLATION_STAGE::X_SPEED);
        stage->write(string(strCommand));
        sprintf(strCommand, "SPY=%ld\r", TRANSLATION_STAGE::Y_SPEED);
        stage->write(string(strCommand));
		usleep(100000);

        success = true;
        state = CONNECTED;
	} catch (string e){
		throw QException(OBJECT_NAME, "Connect", e);
	}
    
	return success;
}

bool TranslationStage::IsConnected()
{
    return (stage!=NULL && state==CONNECTED);
}

void TranslationStage::Disconnect()
{
	stage = NULL;
	state = DISCONNECTED;
}

void TranslationStage::X_Move_Openloop_Realtime(double distance)
{
}

void TranslationStage::X_Move_Openloop_Unrealtime(double distance)
{
}

void TranslationStage::X_Move_Closeloop_Realtime(double distance)
{
	long step = long(distance);
	char strCommand[32];
	if (step == 0 || !IsConnected()){return;}

	try{
		sprintf(strCommand, "%s%ld", "IPX=", step);
		stage->command(string(strCommand), "\r", ":", true);
	}catch(string e){
		throw QException(OBJECT_NAME, "X_Move_Closeloop_Realtime", e);
	}
}

void TranslationStage::X_Move_Closeloop_Unrealtime(double distance)
{
    long step = long(distance);
    char strCommand[32];
    if (step == 0 || !IsConnected()){ return; }
    
    try{
        sprintf(strCommand, "%s%ld", "IPX=", step);
        stage->command(string(strCommand), "\r", ":", true);
        double motionTime = Get_MotionTime(TRANSLATION_STAGE::X_SPEED, step);
        usleep(long(2*motionTime));

        //Wait stage to stop
        int isBusy = 1;
        while (true){
            isBusy = stage->commandValue("MG _BGX");
            if (isBusy == 0){break;}
            usleep(TRANSLATION_STAGE::STAGE_WAITING);
        }
    }catch(string e){
    }
}

void TranslationStage::X_Stop()
{
	if (!IsConnected()){return;}

	try{
		stage->write("STX\r");
		//stage->command("STX","\r",":");
	}catch(string e){
		throw QException(OBJECT_NAME, "X_Stop", e);
	}
}

void TranslationStage::Y_Move_Openloop_Realtime(double distance)
{
}

void TranslationStage::Y_Move_Openloop_Unrealtime(double distance)
{
}

void TranslationStage::Y_Move_Closeloop_Realtime(double distance)
{
	long step = long(distance);
	char strCommand[32];
	if (step == 0 || !IsConnected()){return;}

	try{
		sprintf(strCommand, "%s%ld", "IPY=", step);
		stage->command(string(strCommand), "\r", ":", true);
	}catch(string e){
		throw QException(OBJECT_NAME, "Y_Move_Closeloop_Realtime", e);
	}
}

void TranslationStage::Y_Move_Closeloop_Unrealtime(double distance)
{
    long step = long(distance);
    char strCommand[32];
    if (step == 0 || !IsConnected()){ return; }
    
    try{
        sprintf(strCommand, "%s%ld", "IPY=", step);
        stage->command(string(strCommand), "\r", ":", true);
        double motionTime = Get_MotionTime(TRANSLATION_STAGE::Y_SPEED, step);
        usleep(long(2*motionTime));

        //Wait the stage to stop
        int isBusy = 1;
        while (true){
            isBusy = stage->commandValue("MG _BGY");
            if (isBusy == 0){break;}
            usleep(TRANSLATION_STAGE::STAGE_WAITING);
        }
    }catch(string e){
    }
}

void TranslationStage::Y_Stop()
{
	if (!IsConnected()){ return; }
	try{
		stage->write("STY\r");
		//stage->command("STY","\r",":");
	}catch(string e){
		throw QException(OBJECT_NAME, "Y_Stop", e);
    }
}

void TranslationStage::XY_Move_Closeloop_Realtime(double x_distance, double y_distance)
{
	long x_step = long(x_distance), y_step = long(y_distance);
	char strCommand[32];
	if (!IsConnected()){ return; }
    try{
    	sprintf(strCommand, "IPX=%ld;IPY=%ld",x_step,y_step);
        stage->command(string(strCommand), "\r", ":", true);
    }
    catch(string e){
        throw QException(OBJECT_NAME, "XY_Move_Closeloop_Realtime", e);
    }
}

void TranslationStage::Stop()
{
    if (!IsConnected()){ return; }
	try{
		stage->write("STX;STY\r");
		//stage->command("STX;STY","\r",":");
	}catch(string e){
		throw QException(OBJECT_NAME, "Stop", e);
    }
}

bool TranslationStage::X_ReturnOrigin()
{
    if (!IsConnected()){ return false; }

    //Slow down the speed and acceleration/deceleration speed
    char strCommand[256];
    try{
        sprintf(strCommand, "SPX=10000\r");
        stage->write(string(strCommand));
    } catch (string e){
        cout<<e<<endl;
    }
    usleep(500000);//us

    cout<<"Start x return origin"<<endl;

    //Set motion direction
    stage->write("IPX=1\r");//positive direction
    usleep(TRANSLATION_STAGE::QUERY_DELAY);

    //Start return origin
    stage->write("FIX;BGX\r");
    usleep(TRANSLATION_STAGE::QUERY_DELAY);

    //Whether return origin finishes
    int isBusy = 1;
    long offset = 0;
    while (true){
        try{
            offset = stage->commandValue("MG _RPX");//_RPX
            if (isBusy == 1 && abs(offset) > TRANSLATION_STAGE::MAX_LIMIT_OFFSET){
                stage->write("STX\r");     //stop x stage
                usleep(TRANSLATION_STAGE::QUERY_DELAY);

                stage->write("MOX\r");
                usleep(500000);//500ms

                stage->write("SHX\r");     //start x stage
                usleep(TRANSLATION_STAGE::QUERY_DELAY);

                stage->write("IPX=-1\r");  //move to the reverse direction
                usleep(TRANSLATION_STAGE::QUERY_DELAY);

                stage->write("FIX;BGX\r"); //start return origin
                usleep(1000000);//1s
                //isBusy = 1;
                //continue;
            }
            else if(isBusy == 0)
                break;

            //Get current state and offset
            isBusy = stage->commandValue("MG _BGX");
            cout<<"isBusy: "<<isBusy<<", offset: "<<offset<<endl;
            usleep(1000000);//1s
        }catch (string e){
            cout<<"X_ReturnOrigin: "<<e<<endl;
        }
    }

    //Restore the speed and acceleration/deceleration speed
    try{
        sprintf(strCommand, "SPX=%ld;SPY=%ld\r", TRANSLATION_STAGE::X_SPEED, TRANSLATION_STAGE::Y_SPEED);
        stage->write(string(strCommand));
    } catch (string e){
        cout<<e<<endl;
    }
    usleep(500000);//us
    
    cout<<"Finish x return origin"<<endl;
    return true;

    //Get current position and offset
    /*long position = stage->commandValue("MG _TPY");
    long offset = stage->commandValue("MG _RPY");
    if (abs(position) < TRANSLATIONSTAGE::RETURN_ORIGIN_ERROR && abs(offset) < TRANSLATIONSTAGE::RETURN_ORIGIN_ERROR)
        return true;
    else
        return false;*/
}

bool TranslationStage::Y_ReturnOrigin()
{
    if (!IsConnected()){ return false; }

    //Slow down the speed and acceleration/deceleration speed
    char strCommand[256];
    try{
        sprintf(strCommand, "SPY=10000\r");
        stage->write(string(strCommand));
    } catch (string e){
        cout<<e<<endl;
    }
    usleep(500000);//us

    cout<<"Start y return origin"<<endl;

    //Set motion direction
    stage->write("IPY=1\r");//positive direction
    usleep(TRANSLATION_STAGE::QUERY_DELAY);

    //Start return origin
    stage->write("FIY;BGY\r");
    usleep(TRANSLATION_STAGE::QUERY_DELAY);

    //Whether return origin finishes
    int isBusy = 1;
    long offset = 0;
    while (true){
        try{
            offset = stage->commandValue("MG _RPY");//_RPY
            if (isBusy == 1 && abs(offset) > TRANSLATION_STAGE::MAX_LIMIT_OFFSET){
                stage->write("STY\r");     //stop x stage
                usleep(TRANSLATION_STAGE::QUERY_DELAY);

                stage->write("MOY\r");
                usleep(500000);//500ms

                stage->write("SHY\r");     //start x stage
                usleep(TRANSLATION_STAGE::QUERY_DELAY);

                stage->write("IPY=-1\r");  //move to the reverse direction
                usleep(TRANSLATION_STAGE::QUERY_DELAY);

                stage->write("FIY;BGY\r"); //start return origin
                usleep(1000000);//1s
                //isBusy = 1;
                //continue;
            }
            else if(isBusy == 0)
                break;

            //Get current state and offset
            isBusy = stage->commandValue("MG _BGY");
            cout<<"isBusy: "<<isBusy<<", offset: "<<offset<<endl;
            usleep(1000000);//1s
        }catch (string e){
            cout<<"Y_ReturnOrigin: "<<e<<endl;
        }
    }

    //Restore the speed and acceleration/deceleration speed
    try{
        sprintf(strCommand, "SPX=%ld;SPY=%ld\r", TRANSLATION_STAGE::X_SPEED, TRANSLATION_STAGE::Y_SPEED);
        stage->write(string(strCommand));
    } catch (string e){
        cout<<e<<endl;
    }
    usleep(500000);//us

    cout<<"Finish y return origin"<<endl;
    return true;

    //Get current position and offset
    /*long position = stage->commandValue("MG _TPY");
    long offset = stage->commandValue("MG _RPY");
    if (abs(position) < TRANSLATIONSTAGE::RETURN_ORIGIN_ERROR && abs(offset) < TRANSLATIONSTAGE::RETURN_ORIGIN_ERROR)
        return true;
    else
        return false;*/
}

double TranslationStage::X_GetCurrentPosition()
{
	if (!IsConnected()){ 
    	throw QException(OBJECT_NAME, "X_GetCurrentPosition", "No Stage Connection");
	}
	else{
        try{
            return (stage->commandValue("MG _TPX"));
        }catch(string e){
            throw QException(OBJECT_NAME, "X_GetCurrentPosition", e);
        }
    }
}

double TranslationStage::Y_GetCurrentPosition()
{
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Y_GetCurrentPosition", "No Stage Connection");
	}
	else{
        try{
            return (stage->commandValue("MG _TPY"));
        }catch(string e){
            throw QException(OBJECT_NAME, "Y_GetCurrentPosition", e);
        }
    }
}

void TranslationStage::X_SetCurrentPosition(long position)
{
    char strCommand[32];
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "X_SetCurrentPosition", "No Stage Connection");
	}
	else{
    	try{
        	sprintf(strCommand,"DPX=%ld",position);
    		stage->command(strCommand, "\r", ":", true);
    	}catch (string e){
        	throw QException(OBJECT_NAME, "X_SetCurrentPosition", e);
    	}
	}
}

void TranslationStage::Y_SetCurrentPosition(long position)
{
    char strCommand[32];
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Y_SetCurrentPosition", "No Stage Connection");
	}
	else{
    	try{
        	sprintf(strCommand,"DPY=%ld",position);
    		stage->command(strCommand, "\r", ":", true);
    	}catch (string e){
        	throw QException(OBJECT_NAME, "Y_SetCurrentPosition", e);
    	}
	}
}
    
//Parameters gettings and settings
void TranslationStage::X_Set_Speed(long speed)
{
	char strCommand[32];
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "X_Set_Speed", "No Stage Connection");
	}
	else{
    	try{
    		sprintf(strCommand,"%s%ld","SPX=",speed);
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch(string e){
    		throw QException(OBJECT_NAME, "X_Set_Speed", e);
    	}
	}
}

long TranslationStage::X_Get_Speed()
{
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "X_Get_Speed", "No Stage Connection");
	}
	else{
    	try{
    		return stage->commandValue("MG _SPX");
    	}catch (string e){
    		throw QException(OBJECT_NAME, "X_Get_Speed", e);
    	}
	}
}

//get acceleration speed
long TranslationStage::X_Get_ACCSpeed()
{
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "X_Get_ACCSpeed", "No Stage Connection");
	}
	else{
    	try{
    		return stage->commandValue("MG _ACX");
    	}catch (string e){
    		throw QException(OBJECT_NAME, "X_Get_ACCSpeed", e);
    	}
	}
}

//set acceleration speed
void TranslationStage::X_Set_ACCSpeed(long acc)
{
    char strCommand[32];
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "X_Set_ACCSpeed", "No Stage Connection");
	}
	else{
    	try{
    		sprintf(strCommand,"%s%ld","ACX=",acc);
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch (string e){
    		throw QException(OBJECT_NAME, "X_Set_ACCSpeed", e);
    	}
	}
}

//get deceleration speed
long TranslationStage::X_Get_DECSpeed()
{
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "X_Get_DECSpeed", "No Stage Connection");
	}
	else{
    	try{
    		return stage->commandValue("MG _DCX");
    	}catch (string e){
    		throw QException(OBJECT_NAME, "X_Get_DECSpeed", e);
    	}
	}
}

//set deceleration speed
void TranslationStage::X_Set_DECSpeed(long dec)
{
    char strCommand[32];
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "X_Set_DECSpeed", "No Stage Connection");
	}
	else{
    	try{
    		sprintf(strCommand,"%s%ld","DCX=",dec);
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch (string e){
    		throw QException(OBJECT_NAME, "X_Set_DECSpeed", e);
    	}
	}
}

void TranslationStage::X_Set_PID(PID x_pid)
{
    char strCommand[128];
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "X_Set_PID", "No Stage Connection");
	}
	else{
    	try{
            //set speed
            sprintf(strCommand,"SPX=%ld;ACX=%ld;DCX=%ld", long(x_pid.SP), long(x_pid.AC), long(x_pid.DC));
            stage->command(string(strCommand), "\r", ":", true);

            //set pid
    		sprintf(strCommand,"KPX=%f;KIX=%f;KDX=%f",x_pid.KP,x_pid.KI,x_pid.KD);
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch (string e){
    		throw QException(OBJECT_NAME, "X_Set_PID", e);
    	}
	}
}

double TranslationStage::X_Get_KP()
{
   if (!IsConnected()){
    	throw QException(OBJECT_NAME, "X_Get_KP", "No Stage Connection");
	}
	else{
    	try{
    		return stage->commandValue("MG _KPX");
    	}catch (string e){
    		throw QException(OBJECT_NAME, "X_Get_KP", e);
    	}
	}
}

void TranslationStage::X_Set_KP(double kp)
{
    char strCommand[32];
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "X_Set_KP", "No Stage Connection");
	}
	else{
    	try{
    		sprintf(strCommand,"%s%f","KPX=",kp);
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch (string e){
    		throw QException(OBJECT_NAME, "X_Set_KP", e);
    	}
	}
}

double TranslationStage::X_Get_KI()
{
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "X_Get_KI", "No Stage Connection");
	}
	else{
    	try{
    		return stage->commandValue("MG _KIX");
    	}catch (string e){
    		throw QException(OBJECT_NAME, "X_Get_KI", e);
    	}
	}
}

void TranslationStage::X_Set_KI(double ki)
{
    char strCommand[32];
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "X_Set_KI", "No Stage Connection");
	}
	else{
    	try{
    		sprintf(strCommand,"%s%f","KIX=",ki);
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch (string e){
    		throw QException(OBJECT_NAME, "X_Set_KI", e);
    	}
	}
}

double TranslationStage::X_Get_KD()
{
  if (!IsConnected()){
    	throw QException(OBJECT_NAME, "X_Get_KD", "No Stage Connection");
	}
	else{
    	try{
    		return stage->commandValue("MG _KDX");
    	}catch (string e){
    		throw QException(OBJECT_NAME, "X_Get_KD", e);
    	}
	}
}

void TranslationStage::X_Set_KD(double kd)
{
    char strCommand[32];
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "X_Set_KD", "No Stage Connection");
	}
	else{
    	try{
    		sprintf(strCommand,"%s%f","KDX=",kd);
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch (string e){
    		throw QException(OBJECT_NAME, "X_Set_KD", e);
    	}
	}
}
	
void TranslationStage::Y_Set_Speed(long speed)
{
	char strCommand[32];
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Y_Set_Speed", "No Stage Connection");
	}
	else{
    	try{
    		sprintf(strCommand,"%s%ld","SPY=",speed);
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch(string e){
    		throw QException(OBJECT_NAME, "Y_Set_Speed", e);
        }
	}
}

long TranslationStage::Y_Get_Speed()
{
	if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Y_Get_Speed", "No Stage Connection");
	}
	else{
    	try{
    		return stage->commandValue("MG _SPY");
    	}catch (string e){
    		throw QException(OBJECT_NAME, "Y_Get_Speed", e);
    	}
	}
}

//get acceleration speed
long TranslationStage::Y_Get_ACCSpeed()
{
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Y_Get_ACCSpeed", "No Stage Connection");
	}
	else{
    	try{
    		return stage->commandValue("MG _ACY");
    	}catch (string e){
    		throw QException(OBJECT_NAME, "Y_Get_ACCSpeed", e);
    	}
	}
}

//set acceleration speed
void TranslationStage::Y_Set_ACCSpeed(long acc)
{
    char strCommand[32];
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Y_Set_ACCSpeed", "No Stage Connection");
	}
	else{
    	try{
    		sprintf(strCommand,"%s%ld","ACY=",acc);
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch (string e){
    		throw QException(OBJECT_NAME, "Y_Set_ACCSpeed", e);
    	}
	}
}

//get deceleration speed
long TranslationStage::Y_Get_DECSpeed()
{
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Y_Get_DECSpeed", "No Stage Connection");
	}
	else{
    	try{
    		return stage->commandValue("MG _DCY");
    	}catch (string e){
    		throw QException(OBJECT_NAME, "Y_Get_DECSpeed", e);
    	}
	}
}

//set deceleration speed
void TranslationStage::Y_Set_DECSpeed(long dec)
{
    char strCommand[32];
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Y_Set_DECSpeed", "No Stage Connection");
	}
	else{
    	try{
    		sprintf(strCommand,"%s%ld","DCY=",dec);
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch (string e){
    		throw QException(OBJECT_NAME, "Y_Set_DECSpeed", e);
    	}
	}
}

void TranslationStage::Y_Set_PID(PID y_pid)
{
    char strCommand[128];
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Y_Set_PID", "No Stage Connection");
	}
	else{
    	try{
            //set speed
            sprintf(strCommand,"SPY=%ld;ACY=%ld;DCY=%ld", long(y_pid.SP), long(y_pid.AC), long(y_pid.DC));
            stage->command(string(strCommand), "\r", ":", true);

            //set pid
    		sprintf(strCommand,"KPY=%f;KIY=%f;KDY=%f",y_pid.KP,y_pid.KI,y_pid.KD);
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch (string e){
    		throw QException(OBJECT_NAME, "Y_Set_PID", e);
    	}
	}
}

double TranslationStage::Y_Get_KP()
{
   if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Y_Get_KP", "No Stage Connection");
	}
	else{
    	try{
    		return stage->commandValue("MG _KPY");
    	}catch (string e){
    		throw QException(OBJECT_NAME, "Y_Get_KP", e);
    	}
	}
}

void TranslationStage::Y_Set_KP(double kp)
{
    char strCommand[32];
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Y_Set_KP", "No Stage Connection");
	}
	else{
    	try{
    		sprintf(strCommand,"%s%f","KPY=",kp);
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch (string e){
    		throw QException(OBJECT_NAME, "Y_Set_KP", e);
    	}
	}
}

double TranslationStage::Y_Get_KI()
{
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Y_Get_KI", "No Stage Connection");
	}
	else{
    	try{
    		return stage->commandValue("MG _KIY");
    	}catch (string e){
    		throw QException(OBJECT_NAME, "Y_Get_KI", e);
    	}
	}
}

void TranslationStage::Y_Set_KI(double ki)
{
    char strCommand[32];
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Y_Set_KI", "No Stage Connection");
	}
	else{
    	try{
    		sprintf(strCommand,"%s%f","KIY=", ki);
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch (string e){
    		throw QException(OBJECT_NAME, "Y_Set_KI", e);
    	}
	}
}

double TranslationStage::Y_Get_KD()
{
  if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Y_Get_KD", "No Stage Connection");
	}
	else{
    	try{
    		return stage->commandValue("MG _KDY");
    	}catch (string e){
    		throw QException(OBJECT_NAME, "Y_Get_KD", e);
    	}
	}
}

void TranslationStage::Y_Set_KD(double kd)
{
    char strCommand[32];
    if (!IsConnected()){
    	throw QException(OBJECT_NAME, "Y_Set_KD", "No Stage Connection");
	}
	else{
    	try{
    		sprintf(strCommand,"%s%f","KDY=",kd);
    		stage->command(string(strCommand), "\r", ":", true);
    	}catch (string e){
    		throw QException(OBJECT_NAME, "Y_Set_KD", e);
    	}
	}
}

void TranslationStage::WriteCommand(string strCommand)
{
    try{
        cout<<strCommand<<endl;
        stage->write(strCommand);
    }catch(std::string e){
        cout<<e<<endl;
    }
}