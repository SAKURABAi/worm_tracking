
#include "PolyFitting.h"

template <typename T>
PolyFitting<T>::PolyFitting(const T *data_x, const T *data_y, const int data_size, int n)
{
	size = data_size;
	x = new T[size];
	y = new T[size];
	
	if (n>MAX_POLY_DEGREE || n<1){ 
    	this->n = MAX_POLY_DEGREE; 
	}
	else{ 
    	this->n = n; 
	}
	
	p[0] = p[1] = p[2] = 0.0;
	for (int i=0;i<size;++i){
    	x[i] = data_x[i];
    	y[i] = data_y[i];
	}
}

template <typename T>
PolyFitting<T>::~PolyFitting()
{
    delete x;
    delete y;
}

template <typename T>
void PolyFitting<T>::Fit(double *coff)
{
	if(n==1){
		PolyFit_Linear();
		coff[0] = p[0];
		coff[1] = p[1];
	}
	else{
		PolyFit_Quadratic();
		coff[0] = p[0];
		coff[1] = p[1];
		coff[2] = p[2];
	}	
}

//最小二乘拟合直线
/*
	假设函数形式为y=a*x+b,已知x1,x2,...,xn,y1,y2,...,yn,利用最小二乘计算参数a,b
*/
template <typename T>
void PolyFitting<T>::PolyFit_Linear()
{
	double A[3],B[2];
	A[0] = A[1] = B[0] = B[1] = 0;
	A[2] = 1.0*size;

	for (int i=0;i<size;++i){
		A[0] += 1.0*x[i]*x[i];
		A[1] += 1.0*x[i];
		B[0] += 1.0*x[i]*y[i];
		B[1] += 1.0*y[i];
	}
	p[0] = (A[2]*B[0]-A[1]*B[1])/(A[2]*A[0]-A[1]*A[1]+1.0e-10);
	p[1] = (A[0]*B[1]-A[1]*B[0])/(A[2]*A[0]-A[1]*A[1]+1.0e-10);
}

//最小二乘拟合抛物线
/*
	假设函数形式为y=a*x^2+b*x+c,已知x1,x2,...,xn,y1,y2,...,yn,利用最小二乘计算参数a,b,c
*/
template <typename T>
void PolyFitting<T>::PolyFit_Quadratic()
{
	double A[5] = { 0.0, 0.0, 0.0, 0.0, 0.0 }, B[3] = { 0.0, 0.0, 0.0 }, temp = 0, det = 0;
	A[2] = n;
	for (int i = 0; i<size; ++i){
		temp = 1.0 * x[i] * x[i];
		A[0] += temp;
		A[1] += 1.0 * x[i];
		A[3] += temp *x[i];
		A[4] += temp *temp;
		B[0] += 1.0 * y[i];
		B[1] += 1.0 * y[i] * x[i];
		B[2] += temp * y[i];
	}
	det = 2 * A[0] * A[1] * A[3] + A[0] * A[2] * A[4] - A[0] * A[0] * A[0] - A[1] * A[1] * A[4] - A[2] * A[3] * A[3];
	p[0] = B[2] * (A[0] * A[2] - A[1] * A[1]) + B[1] * (A[0] * A[1] - A[2] * A[3]) + B[0] * (A[1] * A[3] - A[0] * A[0]);
	p[0] = p[0] / (det + 1.0e-10);
	p[1] = B[2] * (A[0] * A[1] - A[2] * A[3]) + B[1] * (A[2] * A[4] - A[0] * A[0]) + B[0] * (A[0] * A[3] - A[1] * A[4]);
	p[1] = p[1] / (det + 1.0e-10);
	p[2] = B[2] * (A[1] * A[3] - A[0] * A[0]) + B[1] * (A[0] * A[3] - A[1] * A[4]) + B[0] * (A[0] * A[4] - A[3] * A[3]);
	p[2] = p[2] / (det + 1.0e-10);
}
