
#ifndef _STAGE_PARAMETERS_H
#define _STAGE_PARAMETERS_H

#define AUTOFOCUS_INITIAL_POINT 10000000

struct PID
{
	//Speed parameters
	double SP;
	double AC;
	double DC;

	//PID parameters
    double KP;
    double KI;
    double KD;
};

//Translation state parameters
namespace TRANSLATION_STAGE{
	const long QUERY_DELAY = 100000; //us
	const long STAGE_WAITING = 5000; //us
	const long MAX_LIMIT_OFFSET = 250000;
	const int RETURN_ORIGIN_ERROR = 10;

	const long X_SPEED = 20000;
	const long Y_SPEED = 20000;
	const double X_PRECISION = 0.1;//光栅尺精度：0.1um/pulse
	const double Y_PRECISION = 0.1;//光栅尺精度：0.1um/pulse
	
	//PID Parameters
	const PID X_PID_INITIAL = {X_SPEED, 5120000, 5120000, 4, 0.203, 1};
	const PID Y_PID_INITIAL = {Y_SPEED, 5120000, 5120000, 4, 0.203, 1};
	//PID for 10um
	const PID X_PID_10 = {10000, 256000, 256000, 6.5, 4, 2};
	const PID Y_PID_10 = {10000, 256000, 256000, 8, 2.5, 1};
	//PID for 20um
	const PID X_PID_20 = {10000, 256000, 256000, 6.5, 2.9688, 1.75};
	const PID Y_PID_20 = {10000, 256000, 256000, 7, 2.297, 1};
	//PID for 30um
	const PID X_PID_30 = {10000, 512000, 512000, 6.5, 2.648, 0.75};
	const PID Y_PID_30 = {10000, 512000, 512000, 7, 2.0, 0.5};
	//PID for 40um
	const PID X_PID_40 = {10000, 512000, 512000, 8, 2.375, 1.5};
	const PID Y_PID_40 = {10000, 512000, 512000, 7, 1.906, 0.5};
	//PID for 50um
	const PID X_PID_50 = {20000, 1280000, 1280000, 8, 2.641, 3.75};
	const PID Y_PID_50 = {20000, 1280000, 1280000, 7, 1.703, 0.1};
	//PID for 60um
	//const PID X_PID_60 = {10000, 256000, 256000, 11, 1.703, 2.5};
	//const PID Y_PID_60 = {10000, 256000, 256000, 11, 1.703, 2.5};
}

//Rotation stage parameters
namespace ROTATION_STAGE{
	const long STAGE_WAITING = 5000; //us

	const long W_SPEED = 5000;
	const double W_PRECISION = 1.0/2000;//1 degree = 2000pulse
}

//Z2 stage parameters
namespace Z2_STAGE{
	const long STAGE_WAITING = 5000; //us
	const long Z2_SPEED = 2000;
	const long Z2_TOL = 3;
	const double Z2_PRECISION = 1.25;
	
	const double Z2_FOCUS_REFERENCE_POSITION = 4000; //um
	const int Z2_COARSE_FOCUS_STEP = 150; //um
	const int Z2_FINE_FOCUS_STEP = 50;    //um
	const int Z2_COARSE_FOCUS_TIMES = 15;
	const int Z2_FINE_FOCUS_TIMES = 9;
}

#endif
