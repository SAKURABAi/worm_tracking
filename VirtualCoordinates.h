/****************************************************************************
    DevicesVirtualCoordinates  规定了系统的虚拟坐标系，该坐标系规定：
	1、从下至上，垂直培养皿的方向为Z轴正方向
	2、在培养皿平面看，从后向前为X轴正方向（左右前后是站在目前系统前方而言的）
	3、在培养皿平面看，从左至右为Y轴正方向	
	注：虚拟坐标系中的变化矩阵均是对于正方向而言的
	
	二代系统中设备与虚拟坐标的关系：
	Dalsa CCD (X+) <-> Virtual Coordinate(X+)
	Dalsa CCD (Y+）<-> Virtual Coordinate(Y-)
	Stage (X+) <-> Virtual Coordinate(X-)
	Stage (Y+) <-> Virtual Coordinate(Y-)
	Z1(+) <-> Virtual Coordinate(Z+)
	Z2(+) <-> Virtual Coordinate(Z+)
****************************************************************************/
#ifndef _DEVICES_VIRTUAL_COORDINATES_H_
#define _DEVICES_VIRTUAL_COORDINATES_H_

#define STAGE_INTIAL_VALUE 10000000 //电机初始值

namespace VIRTUAL_COORDINATE{
	const int X_POSITIVE = 1;//虚拟坐标系X正轴
	const int Y_POSITIVE = 1;//虚拟坐标系Y正轴
	const int Z_POSITIVE = 1;//虚拟坐标系Z正轴
	
	// Calculate pulse/pixel
	const double STAGE_XY_TO_IMAGE_FACTOR = 5000/106.1914;

	// Calculate pixel/pulse
	const double IMAGE_TO_STAGE_XY_FACTOR = 1/STAGE_XY_TO_IMAGE_FACTOR;
	const double IMAGE_TO_STAGE_XY[2][2] = { -1.0000*IMAGE_TO_STAGE_XY_FACTOR, -0.0003*IMAGE_TO_STAGE_XY_FACTOR,
	                                         -0.0028*IMAGE_TO_STAGE_XY_FACTOR, 1.0000*IMAGE_TO_STAGE_XY_FACTOR };
	
	const double IMAGE_TO_STAGE_XY_DET = IMAGE_TO_STAGE_XY[0][0] * IMAGE_TO_STAGE_XY[1][1] - IMAGE_TO_STAGE_XY[0][1] * IMAGE_TO_STAGE_XY[1][0];
	const double STAGE_XY_TO_IMAGE_MATRIX[2][2] = { IMAGE_TO_STAGE_XY[1][1] / IMAGE_TO_STAGE_XY_DET, -IMAGE_TO_STAGE_XY[0][1]/IMAGE_TO_STAGE_XY_DET,
		                                           -IMAGE_TO_STAGE_XY[1][0] / IMAGE_TO_STAGE_XY_DET, IMAGE_TO_STAGE_XY[0][0] / IMAGE_TO_STAGE_XY_DET};
	//实际中的坐标系
	const int STAGE_X_POSITIVE = -X_POSITIVE;
    const int STAGE_Y_POSITIVE = -Y_POSITIVE;
    const int Z1_POSITIVE = Z_POSITIVE;
    const int Z2_POSITIVE = Z_POSITIVE;
}

#endif
