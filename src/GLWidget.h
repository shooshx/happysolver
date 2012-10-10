// Happy Cube Solver - Building and designing models for the Happy cube puzzles
// Copyright (C) 1995-2006 Shy Shalom, shooshX@gmail.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef __GLWIDGET_H__INCLUDED__
#define __GLWIDGET_H__INCLUDED__

#include <QGLWidget>
#include "general.h"
#ifdef Q_WS_MAC
#include <OpenGL/glu.h>
#endif

/** \file
	Declares the GLWidget class which serves as a parent class to all OpenGL widgets.
*/


#define GL_BEGIN_TEXT() {	glMatrixMode(GL_PROJECTION); glPushMatrix();glLoadIdentity(); \
							glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity(); \
							glDisable(GL_DEPTH_TEST); glDisable(GL_TEXTURE_2D); }
#define GL_END_TEXT() {	glEnable(GL_DEPTH_TEST); glPopMatrix(); glMatrixMode(GL_PROJECTION); \
						glPopMatrix(); glMatrixMode(GL_MODELVIEW); }

//#define ZOOM_MIN (10)
//#define ZOOM_MAX (700)
//#define ZOOM_WHEEL_FACTOR (5)

#define ZOOM_MIN (50)
#define ZOOM_MAX (300)
#define ZOOM_WHEEL_FACTOR (13)

class QActionGroup;

/** GLWidget serves as a further abstraction of OpenGL from QGLWidget.
	It implements several key functionalities all the QGLWidgets I'm using share in common.
	these unctionalities are:
	- react to mouse drag operations and perform specific actions such as
	rotation, translation and scaling of the current scene. Also, mainatain
	the current selected state of these operations.
	- create the viewing volume in orthographic or perspective mode
	- create lighting settings
	- implement selection of objects
	- implement global zoom control and use the mouse wheel to control it.
	- implement text printing on the viewport.
	GLWidget is designed to be derived from. a deriving class needs only implemet
	a small number of methods to control only what it need to control (mostly painting and
	target drawing) and can focus on implementing its own specific functionality.
*/
class GLWidget : public QGLWidget
{
	Q_OBJECT
public:
	/// the GLWidget constructor.
	/// alot of default values are set here. if a derived subclass wishes for a different 
	/// setting he may alter them in its constructor.
	GLWidget(QWidget *parent = NULL, QGLWidget *sharefrom = NULL);
	virtual ~GLWidget() {}

	enum EViewState { Ortho, Perspective };
	enum EMouseAction { Rotate, Translate, Scale };
	enum EAxis { Xaxis, Yaxis, Zaxis, XYaxis, XZaxis, YZaxis };
	enum ETransformType { WorldSpace, ObjectSpace };

	bool isUsingLight() { return m_fUseLight; }

	EViewState currentViewState() const { return m_viewState; } 
	EMouseAction currentMouseAction() const { return m_mouseAct; }
	EAxis currentAxis() const { return m_axis; }
	ETransformType currentTransformType() const { return m_transformType; }

	void checkErrors(const char* place);
 
public slots:
	void rotateTimeout();
	// GUI actions
	void resetState() { DoReset(); 	updateGL(); }
	void setContinuous(bool val);
	void setViewFrustrum(QAction *act);
	void setViewFrustrum(GLWidget::EViewState newState);
	void setAction(QAction *act);
	void setAxis(QAction *act);
	void setSpace(QAction *act);
	void setCullFace(bool b);

	virtual void updateView(int hint) { Q_UNUSED(hint) }; // updates the opengl view according to the hints
	void doBindTexture(int index, QImage img);
	void doUpdateTexture(int index, QImage img);

	void rotate(EAxis axis, int x, int y);
	void zoom(int v); // v between 0 and 100

	void setUsingLight(bool use);
	Vec3 lightPos() { return m_lightPos; }
	void setLightPos(const Vec3& c);
	void setLightColor(const Vec3& c);
	void reLight() { makeCurrent(); reCalcLight(); updateGL(); }

signals:
	void rotated(GLWidget::EAxis axis, int x, int y);
	void callReset(); // means my reset was just called
	void zoomChanged(int v); // the view found it appropriate to change the zoom, notify interested parties

protected:
	
	/// this is a call back to whoever subclasses this widget and calls DoChoise
	/// the default implementation does nothing.
	/// \arg \c inChoise a helper argument for user specified functionality. 
	///	GLWidget always calls this method with inChoise == true.
	///	if the user calls this function on his own he may want to do so with inChoise == false
	/// and do slightly differnt things, such us use more color and textures.
	virtual void drawTargets(bool inChoise) { Q_UNUSED(inChoise) } 
		
	/// perform actual selection of target.
	int DoChoise(int chX, int chY);
	void DoReset();
	void setNewMinMax(const Vec3& min, const Vec3& max, bool scale); 

	void mglPrint(const QString &str);			// Custom GL "Print" Routine

	virtual void myPaintGL();
	virtual void initialized() {} // called after the initialization is done for further gl-related init

// events
	virtual void initializeGL();
	virtual void resizeGL(int width, int height);

	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void wheelEvent(QWheelEvent *event);

protected:

	double getScrDiv() const { return realScale * (m_zoomVal / 100.0); }

	const int m_minScaleReset;  //a view can set this member for to limit the maximal size of an object at reset. default is 0 for no effect
	
	bool m_bBackFaceCulling;		// is Back Face Culling enabled
	bool m_bContinuous; // the Continuous option is on. this says nothing about the current state of the timer

	int m_nRotSens, m_nScaleSens, m_nTransSens;

	Vec3 aqmin, aqmax; // bbox for everything, floats as an optinimazation, no conversion 
	QPoint m_lastPos;					// hold last mouse x,y coord

	EViewState m_viewState;
	EMouseAction m_mouseAct;
	EAxis m_axis;
	ETransformType m_transformType;
	int m_zoomVal;

	bool m_fUseLight;
	Vec3 m_lightPos, m_lightColor;
	float m_lightAmbient, m_lightDiffuse, m_lightSpecular;
	bool m_bSkewReset; // should the reset be slighty rotated for better view or not

public: 
	// this needs to be public for the PicPainter to be able to access it.
	QList<int> m_textures;

private:
	virtual void paintGL(); // (event) demote it to private from protected
	void callDrawTargets(); 

	bool SetupViewingFrustum(void);
	bool SetupViewingOrthoConstAspect(void);

	// continuos rotate - not really used.
	int m_nXDelt, m_nYDelt; // used for continuous rotation, last mouse delta.
	bool m_bTimerMode; // used for continuous rotation. if true, timer is On and rotating.
	QTimer *m_timer;   // used for continuous rotation. the timer. every GLWidget has its own timer

	int m_fontBase;

	int m_cxClient;			///< hold the windows width
	int m_cyClient;			///< hold the windows height
	double m_AspectRatio;		///< hold the fixed Aspect Ration
	double scrScale, realScale;
	double m_osf; // moving scale
	
	void reCalcProj(bool fFromScratch = true);
	void reCalcLight();

	void ActMouseMove(EMouseAction action, int dx, int dy);
	void translate(int xValue, int yValue);
	void scale(int xValue, int yValue);
	// rotate is a slot

	void BuildFont();	///< Build Our Bitmap Font
	void KillFont();	///< Delete The Font

};

#endif // __GLWIDGET_H__INCLUDED__
