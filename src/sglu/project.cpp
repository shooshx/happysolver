#include "sglu.h"
#include <cmath>
// copied from mesa glu libutil/project.c

static void gluMakeIdentityd(double m[16])
{
	m[0+4*0] = 1; m[0+4*1] = 0; m[0+4*2] = 0; m[0+4*3] = 0;
	m[1+4*0] = 0; m[1+4*1] = 1; m[1+4*2] = 0; m[1+4*3] = 0;
	m[2+4*0] = 0; m[2+4*1] = 0; m[2+4*2] = 1; m[2+4*3] = 0;
	m[3+4*0] = 0; m[3+4*1] = 0; m[3+4*2] = 0; m[3+4*3] = 1;
}


#define glPi 3.14159265358979323846

void sgluPerspective(double fovy, double aspect, double zNear, double zFar, float dest[16])
{
	double m[4][4];
	double sine, cotangent, deltaZ;
	double radians = fovy / 2 * glPi / 180;

	deltaZ = zFar - zNear;
	sine = sin(radians);
	if ((deltaZ == 0) || (sine == 0) || (aspect == 0)) {
		return;
	}
	cotangent = cos(radians) / sine;

	gluMakeIdentityd(&m[0][0]);
	m[0][0] = cotangent / aspect;
	m[1][1] = cotangent;
	m[2][2] = -(zFar + zNear) / deltaZ;
	m[2][3] = -1;
	m[3][2] = -2 * zNear * zFar / deltaZ;
	m[3][3] = 0;
	//glMultMatrixd(&m[0][0]);
	for(int i = 0; i < 16; ++i) 
		dest[i] = (float)((&m[0][0])[i]);
}