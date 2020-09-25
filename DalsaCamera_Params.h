
#ifndef _DALSA_CAMERA_PARAMS_H_
#define _DALSA_CAMERA_PARAMS_H_

#include "SapClassBasic.h"
#include "DalsaCamera.h"

#define MAX_IMAGE_BUFFERS 8
#define MAX_TEST_CONNECTION_TIMES 10
#define DALSE_SERIAL_QUERY_DELAY  500000 //500ms

enum DALSA_TRIGGERMODE{ INTERNAL, EXTERNAL };

class DalsaCamera;

typedef struct tagMY_CONTEXT
{
	UINT32 height;
	UINT32 width;
	UINT32 pixelDepth;
	UINT32 pixelFormat;
	SapBuffer *Buffers;
	void *image[MAX_IMAGE_BUFFERS];
	DalsaCamera *camera;

}MY_CONTEXT, *PMY_CONTEXT;

#endif
