/****************************************************************************
    DistanceSlider:
****************************************************************************/

#ifndef _DISTANCE_SLIDER_H_
#define _DISTANCE_SLIDER_H_

#include "Util.h"
#include <QtWidgets/QSlider>

class DistanceSlider : public QSlider
{
	Q_OBJECT
public:
	static string OBJECT_NAME;

	explicit DistanceSlider(Qt::Orientation orientation, QWidget *parent=0);

	inline void Set_MaxDistance(long max_value){ max_distance = max_value;}
	inline void Set_MinDistance(long min_value){ min_distance = min_value;}
	void DisplayCurrentPosition(long position);

protected:
	virtual void mouseMoveEvent ( QMouseEvent *ev );
	virtual void mousePressEvent ( QMouseEvent *ev );
	virtual void mouseReleaseEvent ( QMouseEvent *ev );

private:
	//the pritical range is [0,100]
	long max_distance;
	long min_distance;
};

#endif
