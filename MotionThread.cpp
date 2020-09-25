
#include "MotionThread.h"

MotionThread::MotionThread(TranslationStage* stage):translationStage(stage)
{
	stageType = TRANSLAION_STAGE;
    method = NO_MOTION;
    x_distance = 0;
    y_distance = 0;
}

MotionThread::MotionThread(Stage* stage):stage(stage)
{
	stageType = SINGLE_STAGE;
	method = NO_MOTION;
	distance = 0;
}

MotionThread::~MotionThread()
{
    translationStage = NULL;
    stage = NULL;
}

void MotionThread::run()
{
	try{
		if (method == MOTION){
			stage->Move_Openloop_Unrealtime(distance);
		}
		else if (method == X_MOTION){
			translationStage->X_Move_Closeloop_Unrealtime(x_distance);
		}
		else if (method == Y_MOTION){
			translationStage->Y_Move_Closeloop_Unrealtime(y_distance);
		}
		else if (method == RETURN_ORIGIN){
			stage->ReturnOrigin();
		}
		else if (method == X_RETURN_ORIGIN){
			translationStage->X_ReturnOrigin();
		}
		else if (method == Y_RETURN_ORIGIN){
			translationStage->Y_ReturnOrigin();
		}
	} catch (QException e){
		cout<<"MotionThread: "<<e.getMessage()<<endl;
	}
	if ( (method == MOTION) || (method == X_MOTION) || (method == Y_MOTION) )
		emit FinishMotion();
	if ( (method == RETURN_ORIGIN) || (method == X_RETURN_ORIGIN) || (method == Y_RETURN_ORIGIN) )
		emit FinishReturnOrigin((int)method);
}

