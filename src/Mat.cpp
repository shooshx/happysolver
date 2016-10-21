#include "Mat.h"


Mat3 Mat4::toNormalsTrans() const {
	Mat3 v;
	v.m[0] = m[0]; v.m[1] = m[4]; v.m[2] = m[8];
	v.m[3] = m[1]; v.m[4] = m[5]; v.m[5] = m[9];
	v.m[6] = m[2]; v.m[7] = m[6]; v.m[8] = m[10];
	Mat3 i = v.invert();
	i.transpose();
	return i;
}


Mat3 Mat3::invert() const {
	const Mat3& A = *this;
	double determinant =    +A(0,0)*(A(1,1)*A(2,2)-A(2,1)*A(1,2))
		-A(0,1)*(A(1,0)*A(2,2)-A(1,2)*A(2,0))
		+A(0,2)*(A(1,0)*A(2,1)-A(1,1)*A(2,0));
	double invdet = 1/determinant;
	Mat3 result;
	result(0,0) =  (A(1,1)*A(2,2)-A(2,1)*A(1,2))*invdet;
	result(1,0) = -(A(0,1)*A(2,2)-A(0,2)*A(2,1))*invdet;
	result(2,0) =  (A(0,1)*A(1,2)-A(0,2)*A(1,1))*invdet;
	result(0,1) = -(A(1,0)*A(2,2)-A(1,2)*A(2,0))*invdet;
	result(1,1) =  (A(0,0)*A(2,2)-A(0,2)*A(2,0))*invdet;
	result(2,1) = -(A(0,0)*A(1,2)-A(1,0)*A(0,2))*invdet;
	result(0,2) =  (A(1,0)*A(2,1)-A(2,0)*A(1,1))*invdet;
	result(1,2) = -(A(0,0)*A(2,1)-A(2,0)*A(0,1))*invdet;
	result(2,2) =  (A(0,0)*A(1,1)-A(1,0)*A(0,1))*invdet;
	return result;
}