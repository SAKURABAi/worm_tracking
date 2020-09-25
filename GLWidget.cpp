
#include "GLWidget.h"
#include "Worm_CV/CONST_PARA.h"

#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <device_launch_parameters.h>
#include <QtGui/QOpenGLContext>
#include <QtGui/QPainter>

#include <iostream>
using namespace std;

#define PI 3.1415926536f

extern "C" int pixelConvert8(uchar3* output_data, uchar* original_data, int width, int height);

GLWidget::GLWidget(QWidget* parent) : QOpenGLWidget(parent)
{
    isDrawCrossLine = false;
    readyDisplay = false;

	WindowWidth = this->width();
	WindowHeight = this->height();
	ImageWidth = DALSACEMERA::FULLIMAGE_WIDTH;
	ImageHeight = DALSACEMERA::FULLIMAGE_HEIGHT;
}

GLWidget::~GLWidget()
{
	makeCurrent();
	clearObject();
	cleanupCuda();
}

void GLWidget::initializeGL()
{
	openglF = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_2_Core>();
	openglF->initializeOpenGLFunctions();

	openglF->glViewport(0, 0, WindowWidth, WindowHeight);
	openglF->glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	openglF->glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	makeObject();
}

void GLWidget::resizeGL(int width, int height)
{
	WindowWidth = width;
	WindowHeight = height;
	openglF->glViewport(0, 0, WindowWidth, WindowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, WindowWidth, WindowHeight, 0.0f, 1.0, 1.0f);
	readyDisplay = false;
	//cout<<"ResizeGL"<<endl;
}

void GLWidget::paintGL()
{
	if (readyDisplay){
		openglF->glEnable(GL_TEXTURE_2D);
		openglF->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		openglF->glBindBuffer(GL_PIXEL_PACK_BUFFER, pixBufferObj);
		openglF->glBindTexture(GL_TEXTURE_2D, texture);
		openglF->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ImageWidth, ImageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		openglF->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

		//Texture mapping
		glBegin(GL_QUADS);
	    glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, -1.0f);
	    glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, 1.0f);
	    glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 1.0f);
	    glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, -1.0f);
	    glEnd();

	    openglF->glDisable(GL_TEXTURE_2D);
		if (isDrawCrossLine){
			float p1[2], p2[2], p3[2], p4[2];
			/*p1[0] = -1.0f;
			p1[1] = 2.0f*(ImageHeight/2 + DALSACEMERA::Y_CENTER_OFFSET)/ImageHeight - 1.0f;
			p2[0] = 1.0f;
			p2[1] = 2.0f*(ImageHeight/2 + DALSACEMERA::Y_CENTER_OFFSET)/ImageHeight - 1.0f;
			p3[0] = 2.0f*(ImageWidth/2 + DALSACEMERA::X_CENTER_OFFSET)/ImageWidth - 1.0f;
			p3[1] = -1.0f;
			p4[0] = 2.0f*(ImageWidth/2 + DALSACEMERA::X_CENTER_OFFSET)/ImageWidth - 1.0f;
			p4[1] = 1.0f;*/
			p1[0] = -1.0f;
			p1[1] = 0.0f;
			p2[0] = 1.0f;
			p2[1] = 0.0f;
			p3[0] = 0.0f;
			p3[1] = -1.0f;
			p4[0] = 0.0f;
			p4[1] = 1.0f;
			
			glBegin(GL_LINES); 
			glColor3f(0.0f,1.0f,0.0f);
			openglF->glLineWidth(6.0f);
			glVertex2f(p1[0], p1[1]);
			glVertex2f(p2[0], p2[1]);
			glVertex2f(p3[0], p3[1]);
			glVertex2f(p4[0], p4[1]);
			glEnd();
			glColor3f(1.0f,1.0f,1.0f);
		}
		if (DRAW_REFLINE_FLAG && ImageWidth == WORM::IMAGE_SIZE && ImageHeight == WORM::IMAGE_SIZE){
			if (TrackingMode == CENTROID_TRACKING){
                drawRefLine(ImageWidth/2, ImageHeight/2, vec3f(1.0f, 0.0f, 0.0f));
                drawRefLine(globalCentroid[0], globalCentroid[1], vec3f(0.0f, 1.0f, 0.0f));
            }
            else if (TrackingMode == ROI_TRACKING){ 
                drawRefLine(ImageWidth/2, ImageHeight/2, vec3f(1.0f, 0.0f, 0.0f));
                drawRefLine(globalRoi[0], globalRoi[1],  vec3f(0.0f, 1.0f, 0.0f));
            }
            else if (TrackingMode == STARING_IMAGING_TRACKING || TrackingMode == CRUISING_TRACKING){
            	double center[2] = {STARING_IMAGING::CIRCLE_CENTER[0], STARING_IMAGING::CIRCLE_CENTER[1]};
            	drawRefLine(globalOverallCenter[0], globalOverallCenter[1], vec3f(1.0f, 0.0f, 0.0f));
            	drawRectangle(globalOverallRect,vec3f(1.0f,0.0f,0.0f));
            	drawRectangle(globalOriginalOverallRect,vec3f(0.0f,0.0f,1.0f));
            	drawCircle(center, STARING_IMAGING::CIRCLE_RADIUS, vec3f(0.0f,1.0f,0.0f));
            }
            TrackingMode = NO_TRACKING;
		}
	}
} 

void GLWidget::makeObject()
{
	makeCurrent();
	openglF->glEnable(GL_TEXTURE_2D);
	openglF->glGenTextures(1, &texture);
	openglF->glBindTexture(GL_TEXTURE_2D, texture);
	openglF->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	openglF->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	openglF->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//GL_CLAMP_TO_EDGE
	openglF->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//GL_CLAMP_TO_EDGE
	openglF->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ImageWidth, ImageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	//create pixel buffer object
	size_t numBytes = 3 * ImageWidth * ImageHeight * sizeof(uchar);
	openglF->glGenBuffers(1, &pixBufferObj);
	openglF->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixBufferObj);
	openglF->glBufferData(GL_PIXEL_UNPACK_BUFFER, numBytes, NULL, GL_DYNAMIC_DRAW);
	//openglF->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

void GLWidget::clearObject()
{
	makeCurrent();
	openglF->glDeleteTextures(1, &texture);
	openglF->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	openglF->glDeleteBuffers(1, &pixBufferObj);
	doneCurrent();
}

void GLWidget::cleanupCuda()
{
	cudaDeviceReset();
}

void GLWidget::drawRefLine(int x, int y, vec3f color)
{
	makeCurrent();

	y = ImageHeight - y;
	int length = 9;
	if ((x - length) < 0 || (x + length) > ImageWidth) { return; }
	if ((y - length) < 0 || (y + length) > ImageHeight) { return; }

	float p1[2], p2[2], p3[2], p4[2];
	p1[0] = 2.0f*(x-length)/ImageWidth - 1.0f;
	p1[1] = 2.0f*y/ImageHeight - 1.0f;
	p2[0] = 2.0f*(x+length)/ImageWidth - 1.0f;
	p2[1] = 2.0f*y/ImageHeight - 1.0f;
	p3[0] = 2.0f*x/ImageWidth - 1.0f;
	p3[1] = 2.0f*(y-length)/ImageHeight - 1.0f;
	p4[0] = 2.0f*x/ImageWidth - 1.0f;
	p4[1] = 2.0f*(y+length)/ImageHeight - 1.0f;
	
	glBegin(GL_LINES); 
	glColor3f(color.r, color.g, color.b);
	glVertex2f(p1[0], p1[1]);
	glVertex2f(p2[0], p2[1]);
	glVertex2f(p3[0], p3[1]);
	glVertex2f(p4[0], p4[1]);
	glEnd();
	glColor3f(1.0f,1.0f,1.0f);
}

void GLWidget::drawRectangle(double *wormRegionRect, vec3f color)
{
	makeCurrent();
	double min_x = wormRegionRect[0];
	double max_x = wormRegionRect[1];
	double min_y = ImageHeight - wormRegionRect[2];
	double max_y = ImageHeight - wormRegionRect[3];

	float p1[2], p2[2], p3[2], p4[2];
	p1[0] = 2.0f*min_x/ImageWidth - 1.0f;
	p1[1] = 2.0f*min_y/ImageHeight - 1.0f;
	p2[0] = 2.0f*max_x/ImageWidth - 1.0f;
	p2[1] = 2.0f*min_y/ImageHeight - 1.0f;
	p3[0] = 2.0f*min_x/ImageWidth - 1.0f;
	p3[1] = 2.0f*max_y/ImageHeight - 1.0f;
	p4[0] = 2.0f*max_x/ImageWidth - 1.0f;
	p4[1] = 2.0f*max_y/ImageHeight - 1.0f;

	glBegin(GL_LINE_STRIP); 
	glColor3f(color.r, color.g, color.b);
	glVertex2f(p1[0], p1[1]);
	glVertex2f(p2[0], p2[1]);
	glVertex2f(p4[0], p4[1]);
	glVertex2f(p3[0], p3[1]);
	glVertex2f(p1[0], p1[1]);
	glEnd();
	glColor3f(1.0f,1.0f,1.0f);
}

void GLWidget::drawCircle(double *center, double radius, vec3f color)
{
	makeCurrent();

	int num = 300;
	float p0[2],p[2];

	glBegin(GL_LINE_STRIP); 
	glColor3f(color.r, color.g, color.b);
	for (int i=0; i<num; ++i){
		p[0] = 2.0f*(center[0] + radius*cos(2*PI*i/num))/ImageWidth - 1.0f;
		p[1] = 2.0f*(ImageHeight - (center[1] + radius*sin(2*PI*i/num)))/ImageHeight - 1.0f;
		if (i==0){ p0[0] = p[0]; p0[1] = p[1]; }
		glVertex2f(p[0], p[1]);
	}
	glVertex2f(p0[0], p0[1]);
	glEnd();
	glColor3f(1.0f,1.0f,1.0f);
}

void GLWidget::ShowImage(void* image_data, int width, int height)
{
	if (image_data == NULL){ return; }	
	readyDisplay = true;

	if (ImageWidth != width || ImageHeight != height){
		//update the texture and pixel buffer object
		ImageWidth = width;
		ImageHeight = height;
		clearObject();
		makeObject();
	}
	//cout<<"Display image, width: "<<width<<", height: "<<height<<endl;
	
	//Allocate the pixel buffer object
	size_t numBytes = 3*ImageWidth*ImageHeight, originalDataSize = sizeof(uchar)*ImageWidth*ImageHeight;;
	uchar3* outputData = NULL;
	uchar* originalData = reinterpret_cast<uchar*>(image_data), *devData = NULL;

	openglF->glBufferData(GL_PIXEL_UNPACK_BUFFER, numBytes, NULL, GL_DYNAMIC_DRAW);

	//register the PBO and map the PBO to the CUDA
	cudaError error = cudaGraphicsGLRegisterBuffer(&cuda_pbo_resource, pixBufferObj, cudaGraphicsMapFlagsWriteDiscard);//register the PBO to the CUDA
	if (error != cudaSuccess){
		//QMessageBox::critical(NULL, "Warning", "Fail to register PBO to the CUDA");
		cout<<"Fail to register PBO to the CUDA"<<endl;
		return;
	}

	error = cudaGraphicsMapResources(1, &cuda_pbo_resource, 0);
	if (error != cudaSuccess){
		//QMessageBox::critical(NULL, "Warning", "Fail to map resources");
		cout<<"Fail to map resources"<<endl;
		return;
	}

	error = cudaGraphicsResourceGetMappedPointer((void**)(&outputData), &numBytes, cuda_pbo_resource);
	if (error != cudaSuccess){
		//QMessageBox::critical(NULL, "Warning", "Fail to get mapped pointer");
		cout<<"Fail to get mapped pointer"<<endl;
		return;
	}

	error = cudaMalloc((void**)&devData, originalDataSize);
	if (error != cudaSuccess){
		//QMessageBox::critical(NULL, "Warning", "Fail to malloc memory in the device");
		cout<<"Fail to malloc memory in the device"<<endl;
		return;
	}

	error = cudaMemcpy(devData, originalData, originalDataSize, cudaMemcpyHostToDevice);
	if (error != cudaSuccess){
		//QMessageBox::critical(NULL, "Warning", "Fail to memcpy data from host to device");
		cout<<"Fail to memcpy data from host to device"<<endl;
		return;
	}

	//launch kernel function to display image
	pixelConvert8(outputData, devData, width, height);
	error = cudaFree(devData);
	if (error != cudaSuccess){
		//QMessageBox::critical(NULL, "Warning", "Fail to free memory");
		cout<<"Fail to free memory"<<endl;
		return;
	}

	//delete the map and unregister the pbo
	error = cudaGraphicsUnmapResources(1, &cuda_pbo_resource, 0);
	if (error != cudaSuccess){
		//QMessageBox::critical(NULL, "Warning", "Fail to unmap resource");
		cout<<"Fail to unmap resource"<<endl;
		return;
	}

	error = cudaGraphicsUnregisterResource(cuda_pbo_resource);
	if (error != cudaSuccess){
		//QMessageBox::critical(NULL, "Warning", "Fail to unregister PBO from CUDA");
		cout<<"Fail to unregister PBO from CUDA"<<endl;
		return;
	}

	update();
}