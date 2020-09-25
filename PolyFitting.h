/****************************************************************************
	使用最小二乘拟合一次及二次多项式
****************************************************************************/
#ifndef _POLY_FITTING_H_
#define _POLY_FITTING_H_

#define MAX_POLY_DEGREE 2

template <typename T>
class PolyFitting
{
public:
	explicit PolyFitting(const T *data_x, const T *data_y, const int data_size, int n=1);
	~PolyFitting();
	void Fit(double *coffs);

protected:
	void PolyFit_Linear();
	void PolyFit_Quadratic();

private:
	T *x;
	T *y;
	int n;//degree of polynomial
	int size;
	double p[MAX_POLY_DEGREE+1];
};

#endif
