
#ifndef _GLWIDGET_H_
#define _GLWIDGET_H_

#include "GlobalParameters.h"
#include <driver_types.h>
#include <QtWidgets/QOpenGLWidget>
#include <QtGui/QOpenGLFunctions_3_2_Core>

struct vec3f{
	vec3f(float r, float g, float b):r(r),g(g),b(b){}
	float r, g, b;
};

class GLWidget : public QOpenGLWidget
{
	Q_OBJECT
public:
	explicit GLWidget(QWidget* parent = 0);
	~GLWidget();

	void ShowImage(void* image_data, int width, int height);
	inline void SetIsDrawCrossLine(bool ok){
		isDrawCrossLine = ok;
	}

protected:
	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;
	void resizeGL(int width, int height) Q_DECL_OVERRIDE;

private:
	void makeObject();
	void clearObject();
	void cleanupCuda();
	void drawRefLine(int x, int y, vec3f color);
	void drawRectangle(double *globalWormRegionRect, vec3f color);
	void drawCircle(double *center, double radius, vec3f color);

	bool isDrawCrossLine;
	bool readyDisplay;

	//display window property
	int ImageWidth, ImageHeight;
	int WindowWidth, WindowHeight;

	struct cudaGraphicsResource* cuda_pbo_resource;
	QOpenGLFunctions_3_2_Core* openglF;
	GLuint texture;
	GLuint pixBufferObj;
};

#endif //_MYGLWIDGET_H_