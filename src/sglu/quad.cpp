#include "sglu.h"
#include <cmath>


typedef int GLint;
typedef float GLfloat;
typedef double GLdouble;
typedef bool GLboolean;

#define CACHE_SIZE	240

#undef	PI
#define PI 3.14159265358979323846

/* QuadricNormal */
#define GLU_SMOOTH              100000
#define GLU_FLAT                100001
#define GLU_NONE                100002

/* QuadricDrawStyle */
#define GLU_POINT               100010
#define GLU_LINE                100011
#define GLU_FILL                100012
#define GLU_SILHOUETTE          100013

/* QuadricOrientation */
#define GLU_OUTSIDE             100020
#define GLU_INSIDE              100021

#define COS cos
#define SIN sin
#define SQRT sqrt


struct GLUquadric {
	GLUquadric() :normals(GLU_SMOOTH), textureCoords(false), orientation(GLU_OUTSIDE), drawStyle(GLU_FILL)

	GLint	normals;
	GLboolean	textureCoords;
	GLint	orientation;
	GLint	drawStyle;
	//void	(GLAPIENTRY *errorCallback)( GLint );
};


void sgluCylinder(GLUquadric *qobj, GLdouble baseRadius, GLdouble topRadius, GLdouble height, GLint slices, GLint stacks)
{
	GLint i,j;
	GLfloat sinCache[CACHE_SIZE];
	GLfloat cosCache[CACHE_SIZE];
	GLfloat sinCache2[CACHE_SIZE];
	GLfloat cosCache2[CACHE_SIZE];
	GLfloat sinCache3[CACHE_SIZE];
	GLfloat cosCache3[CACHE_SIZE];
	GLfloat angle;
	GLfloat zLow, zHigh;
	GLfloat sintemp, costemp;
	GLfloat length;
	GLfloat deltaRadius;
	GLfloat zNormal;
	GLfloat xyNormalRatio;
	GLfloat radiusLow, radiusHigh;
	int needCache2, needCache3;

	if (slices >= CACHE_SIZE) slices = CACHE_SIZE-1;

	if (slices < 2 || stacks < 1 || baseRadius < 0.0 || topRadius < 0.0 || height < 0.0) {
		//gluQuadricError(qobj, GLU_INVALID_VALUE);
		return;
	}

	/* Compute length (needed for normal calculations) */
	deltaRadius = baseRadius - topRadius;
	length = SQRT(deltaRadius*deltaRadius + height*height);
	if (length == 0.0) {
		//gluQuadricError(qobj, GLU_INVALID_VALUE);
		return;
	}

	/* Cache is the vertex locations cache */
	/* Cache2 is the various normals at the vertices themselves */
	/* Cache3 is the various normals for the faces */
	needCache2 = needCache3 = 0;
	if (qobj->normals == GLU_SMOOTH) {
		needCache2 = 1;
	}

	if (qobj->normals == GLU_FLAT) {
		if (qobj->drawStyle != GLU_POINT) {
			needCache3 = 1;
		}
		if (qobj->drawStyle == GLU_LINE) {
			needCache2 = 1;
		}
	}

	zNormal = deltaRadius / length;
	xyNormalRatio = height / length;

	for (i = 0; i < slices; i++) {
		angle = 2 * PI * i / slices;
		if (needCache2) {
			if (qobj->orientation == GLU_OUTSIDE) {
				sinCache2[i] = xyNormalRatio * SIN(angle);
				cosCache2[i] = xyNormalRatio * COS(angle);
			} else {
				sinCache2[i] = -xyNormalRatio * SIN(angle);
				cosCache2[i] = -xyNormalRatio * COS(angle);
			}
		}
		sinCache[i] = SIN(angle);
		cosCache[i] = COS(angle);
	}

	if (needCache3) {
		for (i = 0; i < slices; i++) {
			angle = 2 * PI * (i-0.5) / slices;
			if (qobj->orientation == GLU_OUTSIDE) {
				sinCache3[i] = xyNormalRatio * SIN(angle);
				cosCache3[i] = xyNormalRatio * COS(angle);
			} else {
				sinCache3[i] = -xyNormalRatio * SIN(angle);
				cosCache3[i] = -xyNormalRatio * COS(angle);
			}
		}
	}

	sinCache[slices] = sinCache[0];
	cosCache[slices] = cosCache[0];
	if (needCache2) {
		sinCache2[slices] = sinCache2[0];
		cosCache2[slices] = cosCache2[0];
	}
	if (needCache3) {
		sinCache3[slices] = sinCache3[0];
		cosCache3[slices] = cosCache3[0];
	}

	switch (qobj->drawStyle) 
	{
	case GLU_FILL:
		/* Note:
		** An argument could be made for using a TRIANGLE_FAN for the end
		** of the cylinder of either radii is 0.0 (a cone).  However, a
		** TRIANGLE_FAN would not work in smooth shading mode (the common
		** case) because the normal for the apex is different for every
		** triangle (and TRIANGLE_FAN doesn't let me respecify that normal).
		** Now, my choice is GL_TRIANGLES, or leave the GL_QUAD_STRIP and
		** just let the GL trivially reject one of the two triangles of the
		** QUAD.  GL_QUAD_STRIP is probably faster, so I will leave this code
		** alone.
		*/
		for (j = 0; j < stacks; j++) 
		{
			zLow = j * height / stacks;
			zHigh = (j + 1) * height / stacks;
			radiusLow = baseRadius - deltaRadius * ((float) j / stacks);
			radiusHigh = baseRadius - deltaRadius * ((float) (j + 1) / stacks);

			glBegin(GL_QUAD_STRIP);
			for (i = 0; i <= slices; i++) {
				switch(qobj->normals) {
				case GLU_FLAT:
					glNormal3f(sinCache3[i], cosCache3[i], zNormal);
					break;
				case GLU_SMOOTH:
					glNormal3f(sinCache2[i], cosCache2[i], zNormal);
					break;
				case GLU_NONE:
				default:
					break;
				}
				if (qobj->orientation == GLU_OUTSIDE) {
					if (qobj->textureCoords) {
						glTexCoord2f(1 - (float) i / slices,
							(float) j / stacks);
					}
					glVertex3f(radiusLow * sinCache[i],
						radiusLow * cosCache[i], zLow);
					if (qobj->textureCoords) {
						glTexCoord2f(1 - (float) i / slices,
							(float) (j+1) / stacks);
					}
					glVertex3f(radiusHigh * sinCache[i],
						radiusHigh * cosCache[i], zHigh);
				} else {
					if (qobj->textureCoords) {
						glTexCoord2f(1 - (float) i / slices,
							(float) (j+1) / stacks);
					}
					glVertex3f(radiusHigh * sinCache[i],
						radiusHigh * cosCache[i], zHigh);
					if (qobj->textureCoords) {
						glTexCoord2f(1 - (float) i / slices,
							(float) j / stacks);
					}
					glVertex3f(radiusLow * sinCache[i],
						radiusLow * cosCache[i], zLow);
				}
			}
			glEnd();
		}
		break;
	}

}





void gluPartialDisk(GLUquadric *qobj, GLdouble innerRadius, GLdouble outerRadius, GLint slices, GLint loops, GLdouble startAngle, GLdouble sweepAngle)
{
	GLint i,j;
	GLfloat sinCache[CACHE_SIZE];
	GLfloat cosCache[CACHE_SIZE];
	GLfloat angle;
	GLfloat sintemp, costemp;
	GLfloat deltaRadius;
	GLfloat radiusLow, radiusHigh;
	GLfloat texLow = 0.0, texHigh = 0.0;
	GLfloat angleOffset;
	GLint slices2;
	GLint finish;

	if (slices >= CACHE_SIZE) slices = CACHE_SIZE-1;
	if (slices < 2 || loops < 1 || outerRadius <= 0.0 || innerRadius < 0.0 ||
		innerRadius > outerRadius) {
			gluQuadricError(qobj, GLU_INVALID_VALUE);
			return;
	}

	if (sweepAngle < -360.0) sweepAngle = 360.0;
	if (sweepAngle > 360.0) sweepAngle = 360.0;
	if (sweepAngle < 0) {
		startAngle += sweepAngle;
		sweepAngle = -sweepAngle;
	}

	if (sweepAngle == 360.0) {
		slices2 = slices;
	} else {
		slices2 = slices + 1;
	}

	/* Compute length (needed for normal calculations) */
	deltaRadius = outerRadius - innerRadius;

	/* Cache is the vertex locations cache */

	angleOffset = startAngle / 180.0 * PI;
	for (i = 0; i <= slices; i++) {
		angle = angleOffset + ((PI * sweepAngle) / 180.0) * i / slices;
		sinCache[i] = SIN(angle);
		cosCache[i] = COS(angle);
	}

	if (sweepAngle == 360.0) {
		sinCache[slices] = sinCache[0];
		cosCache[slices] = cosCache[0];
	}

	switch(qobj->normals) {
	case GLU_FLAT:
	case GLU_SMOOTH:
		if (qobj->orientation == GLU_OUTSIDE) {
			glNormal3f(0.0, 0.0, 1.0);
		} else {
			glNormal3f(0.0, 0.0, -1.0);
		}
		break;
	default:
	case GLU_NONE:
		break;
	}

	switch (qobj->drawStyle) {
	case GLU_FILL:
		if (innerRadius == 0.0) {
			finish = loops - 1;
			/* Triangle strip for inner polygons */
			glBegin(GL_TRIANGLE_FAN);
			if (qobj->textureCoords) {
				glTexCoord2f(0.5, 0.5);
			}
			glVertex3f(0.0, 0.0, 0.0);
			radiusLow = outerRadius -
				deltaRadius * ((float) (loops-1) / loops);
			if (qobj->textureCoords) {
				texLow = radiusLow / outerRadius / 2;
			}

			if (qobj->orientation == GLU_OUTSIDE) {
				for (i = slices; i >= 0; i--) {
					if (qobj->textureCoords) {
						glTexCoord2f(texLow * sinCache[i] + 0.5,
							texLow * cosCache[i] + 0.5);
					}
					glVertex3f(radiusLow * sinCache[i],
						radiusLow * cosCache[i], 0.0);
				}
			} else {
				for (i = 0; i <= slices; i++) {
					if (qobj->textureCoords) {
						glTexCoord2f(texLow * sinCache[i] + 0.5,
							texLow * cosCache[i] + 0.5);
					}
					glVertex3f(radiusLow * sinCache[i],
						radiusLow * cosCache[i], 0.0);
				}
			}
			glEnd();
		} else {
			finish = loops;
		}
		for (j = 0; j < finish; j++) {
			radiusLow = outerRadius - deltaRadius * ((float) j / loops);
			radiusHigh = outerRadius - deltaRadius * ((float) (j + 1) / loops);
			if (qobj->textureCoords) {
				texLow = radiusLow / outerRadius / 2;
				texHigh = radiusHigh / outerRadius / 2;
			}

			glBegin(GL_QUAD_STRIP);
			for (i = 0; i <= slices; i++) {
				if (qobj->orientation == GLU_OUTSIDE) {
					if (qobj->textureCoords) {
						glTexCoord2f(texLow * sinCache[i] + 0.5,
							texLow * cosCache[i] + 0.5);
					}
					glVertex3f(radiusLow * sinCache[i],
						radiusLow * cosCache[i], 0.0);

					if (qobj->textureCoords) {
						glTexCoord2f(texHigh * sinCache[i] + 0.5,
							texHigh * cosCache[i] + 0.5);
					}
					glVertex3f(radiusHigh * sinCache[i],
						radiusHigh * cosCache[i], 0.0);
				} else {
					if (qobj->textureCoords) {
						glTexCoord2f(texHigh * sinCache[i] + 0.5,
							texHigh * cosCache[i] + 0.5);
					}
					glVertex3f(radiusHigh * sinCache[i],
						radiusHigh * cosCache[i], 0.0);

					if (qobj->textureCoords) {
						glTexCoord2f(texLow * sinCache[i] + 0.5,
							texLow * cosCache[i] + 0.5);
					}
					glVertex3f(radiusLow * sinCache[i],
						radiusLow * cosCache[i], 0.0);
				}
			}
			glEnd();
		}
		break;
	case GLU_POINT:
		glBegin(GL_POINTS);
		for (i = 0; i < slices2; i++) {
			sintemp = sinCache[i];
			costemp = cosCache[i];
			for (j = 0; j <= loops; j++) {
				radiusLow = outerRadius - deltaRadius * ((float) j / loops);

				if (qobj->textureCoords) {
					texLow = radiusLow / outerRadius / 2;

					glTexCoord2f(texLow * sinCache[i] + 0.5,
						texLow * cosCache[i] + 0.5);
				}
				glVertex3f(radiusLow * sintemp, radiusLow * costemp, 0.0);
			}
		}
		glEnd();
		break;
	case GLU_LINE:
		if (innerRadius == outerRadius) {
			glBegin(GL_LINE_STRIP);

			for (i = 0; i <= slices; i++) {
				if (qobj->textureCoords) {
					glTexCoord2f(sinCache[i] / 2 + 0.5,
						cosCache[i] / 2 + 0.5);
				}
				glVertex3f(innerRadius * sinCache[i],
					innerRadius * cosCache[i], 0.0);
			}
			glEnd();
			break;
		}
		for (j = 0; j <= loops; j++) {
			radiusLow = outerRadius - deltaRadius * ((float) j / loops);
			if (qobj->textureCoords) {
				texLow = radiusLow / outerRadius / 2;
			}

			glBegin(GL_LINE_STRIP);
			for (i = 0; i <= slices; i++) {
				if (qobj->textureCoords) {
					glTexCoord2f(texLow * sinCache[i] + 0.5,
						texLow * cosCache[i] + 0.5);
				}
				glVertex3f(radiusLow * sinCache[i],
					radiusLow * cosCache[i], 0.0);
			}
			glEnd();
		}
		for (i=0; i < slices2; i++) {
			sintemp = sinCache[i];
			costemp = cosCache[i];
			glBegin(GL_LINE_STRIP);
			for (j = 0; j <= loops; j++) {
				radiusLow = outerRadius - deltaRadius * ((float) j / loops);
				if (qobj->textureCoords) {
					texLow = radiusLow / outerRadius / 2;
				}

				if (qobj->textureCoords) {
					glTexCoord2f(texLow * sinCache[i] + 0.5,
						texLow * cosCache[i] + 0.5);
				}
				glVertex3f(radiusLow * sintemp, radiusLow * costemp, 0.0);
			}
			glEnd();
		}
		break;
	case GLU_SILHOUETTE:
		if (sweepAngle < 360.0) {
			for (i = 0; i <= slices; i+= slices) {
				sintemp = sinCache[i];
				costemp = cosCache[i];
				glBegin(GL_LINE_STRIP);
				for (j = 0; j <= loops; j++) {
					radiusLow = outerRadius - deltaRadius * ((float) j / loops);

					if (qobj->textureCoords) {
						texLow = radiusLow / outerRadius / 2;
						glTexCoord2f(texLow * sinCache[i] + 0.5,
							texLow * cosCache[i] + 0.5);
					}
					glVertex3f(radiusLow * sintemp, radiusLow * costemp, 0.0);
				}
				glEnd();
			}
		}
		for (j = 0; j <= loops; j += loops) {
			radiusLow = outerRadius - deltaRadius * ((float) j / loops);
			if (qobj->textureCoords) {
				texLow = radiusLow / outerRadius / 2;
			}

			glBegin(GL_LINE_STRIP);
			for (i = 0; i <= slices; i++) {
				if (qobj->textureCoords) {
					glTexCoord2f(texLow * sinCache[i] + 0.5,
						texLow * cosCache[i] + 0.5);
				}
				glVertex3f(radiusLow * sinCache[i],
					radiusLow * cosCache[i], 0.0);
			}
			glEnd();
			if (innerRadius == outerRadius) break;
		}
		break;
	default:
		break;
	}
}


void gluDisk(GLUquadric *qobj, GLdouble innerRadius, GLdouble outerRadius, GLint slices, GLint loops)
{
	gluPartialDisk(qobj, innerRadius, outerRadius, slices, loops, 0.0, 360.0);
}