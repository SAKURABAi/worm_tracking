
#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
using namespace std;

#define NETWORK_PORT "eth2" 
#define _CUDA_LAUNCH_FAILURE -1
#define _CUDA_LAUNCH_SUCCESS  1

typedef unsigned long UINT64;
typedef unsigned int  UINT32;
typedef unsigned char UCHAR;
typedef unsigned char uchar;

//status for devices
enum DeviceStatus{OPENED, CONNECTED, DISCONNECTED};

//stage device id
enum DeviceId {
    DEVICE_CRTLR1_ROT,
    DEVICE_CRTLR1_TRS,
    DEVICE_CRTLR2_Z
};

inline double Get_MotionTime(double speed, double distance){
	return (1.0e6*fabs(distance)/speed); //us
}

#endif
