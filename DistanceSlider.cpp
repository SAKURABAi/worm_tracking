
#include "DistanceSlider.h"

string DistanceSlider::OBJECT_NAME = "DistanceSlider";

DistanceSlider::DistanceSlider(Qt::Orientation orientation, QWidget *parent):QSlider(orientation, parent)
{
	setMaximum(100);
	setMinimum(0);
}

void DistanceSlider::mouseMoveEvent ( QMouseEvent *ev ){}
void DistanceSlider::mousePressEvent ( QMouseEvent *ev ){}
void DistanceSlider::mouseReleaseEvent ( QMouseEvent *ev ){}

void DistanceSlider::DisplayCurrentPosition(long position)
{
	int value = 0;
	if (position<min_distance){
		value = 0;
	}
	else if(position>max_distance){
		value = 100;
	}
	else{
		value = int(position*100/(max_distance-min_distance));
	}
	setValue(value);
}

