
#include "TranslationStage.h"
#include "Stage.h"
#include <QtCore/QThread>

#ifndef _TRANSLATION_STAGE_MOTION_THREAD_H_
#define _TRANSLATION_STAGE_MOTION_THREAD_H_

enum MOTION_METHOD{ NO_MOTION, MOTION, X_MOTION, Y_MOTION, RETURN_ORIGIN, X_RETURN_ORIGIN, Y_RETURN_ORIGIN};
enum STAGE_TYPE {NO_STAGE, TRANSLAION_STAGE, SINGLE_STAGE};

class MotionThread : public QThread
{
    Q_OBJECT
public:
	explicit MotionThread(TranslationStage*);
	explicit MotionThread(Stage*);
	~MotionThread();

	inline void setMethod( MOTION_METHOD m){ method = m; }
	inline void setDistance(double d) { distance = d; }
	inline void setXDistance(double x_d){ x_distance = x_d; }
	inline void setYDistance(double y_d){ y_distance = y_d; }

	inline void setDescription(QString& str){ description = str; }
	inline QString& getDescription() { return description; }

	inline MOTION_METHOD getMethod(){ return method; }
	inline STAGE_TYPE getStageType() {return stageType; }

signals:
	void FinishMotion();
	void FinishReturnOrigin(int);

protected:
	virtual void run();

private:
	TranslationStage* translationStage;
	Stage* stage;
	
	STAGE_TYPE stageType;
	MOTION_METHOD method;
	double distance;
	double x_distance;
	double y_distance;
	QString description;
};

#endif //_TRANSLATION_STAGE_MOTION_THREAD_H_
