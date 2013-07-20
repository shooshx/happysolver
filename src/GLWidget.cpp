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

#include <QtGui>
#include <QtOpenGL>

#include "GLWidget.h"
#include "sglu/sglu.h"

QGLFormat g_format;

// used to promote stuff to public
/** PatchQGLContext is a dummy class that promotes methods from protected to public.
	No instance of this class is ever created. It is used solely for the purpose
	of casting a normal QGLContext pointer to a pointer with the methods initialized() 
	and generateFontDisplayLists() as public methods.
*/
class PatchQGLContext :public QGLContext
{
public:
	/// although it is never instanciated, it still needs a ctor.
	/// otherwise gcc nags about it.
	PatchQGLContext() :QGLContext(QGLFormat()) {}
	void generateFontDisplayLists(const QFont & fnt, int listBase)
	{
		//QGLContext::generateFontDisplayLists(fnt, listBase);
	}
	bool initialized() const
	{
		return QGLContext::initialized();
	}
};


GLWidget::GLWidget(QWidget *parent, QGLWidget *sharefrom)
	: QGLWidget(g_format, parent, sharefrom), m_minScaleReset(0), m_cxClient(1), m_cyClient(1)
{
	m_viewState = Ortho;
	m_mouseAct = Rotate;
	m_axis = XYaxis;
	m_transformType = WorldSpace;

	m_bBackFaceCulling = true; 
	m_fUseLight = false;
	m_bSkewReset = true;

	m_zoomVal = 100;

	m_nRotSens = 25;
	m_nScaleSens = 50;
	m_nTransSens = 100;

	// continous mode control
	m_bContinuous = false;
	m_bTimerMode = false;
	m_timer = NULL;
	m_nXDelt = 0;
	m_nYDelt = 0;

	m_osf = 1.0;

	// some defaults to get things started
	aqmin = Vec3(-3, -3, -3);
	aqmax = Vec3(3, 3, 3);

	m_lightPos = Vec3(0.0f, 0.0f, 1000.0f);
	m_lightColor = Vec3(1.0f, 1.0f, 1.0f);

	m_lightAmbient = 0.2f;
	m_lightDiffuse = 0.95f;
	m_lightSpecular = 0.0f;
}

Mat4 GLWidget::transformMat() {
	return Mat4::multt(proj.cur(), model.cur());
}




void GLWidget::doBindTexture(int index, QImage img)
{
// 	makeCurrent();
// 	int id = bindTexture(img.copy(), GL_TEXTURE_2D); // QT bind, not gl
// 	// copy is ugly patch due to QT image caching bug
// 	while (index > m_textures.size())
// 		m_textures.insert(index, -1);
// 	m_textures.insert(index, id);

}

//#define GL_BGRA 0x80E1

void GLWidget::doUpdateTexture(int index, QImage img)
{
// 	makeCurrent();
// 	QImage cimg = img;
// 	int texId = m_textures[index];
// 
// 	if (!glIsTexture(texId))
// 	{
// 		qDebug("invalid texture id");
// 		return; // ha?? shouldn't happen.
// 	}
// 
// 	glBindTexture(GL_TEXTURE_2D, texId);
// 	// this is going arount QT to interface directly with the textures, causing an inversion of the image
// 	// there is currently no way to do this with QT
// 	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, cimg.width(), cimg.height(), GL_BGRA, GL_UNSIGNED_BYTE, cimg.bits());
// 
// 
// 	updateGL();
}



void GLWidget::initializeGL()
{
	checkErrors("init start");
	// specify black as clear color
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	// specify the back of the buffer as clear depth (0 closest, 1 farmost)
	glClearDepth( 1.0f );
	// enable depth testing (Enable zbuffer - hidden surface removal)
	glEnable(GL_DEPTH_TEST);
	if (m_bBackFaceCulling) {
		glEnable(GL_CULL_FACE);
	}

	glDepthFunc(GL_LEQUAL);
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(0x809D);  //GL_MULTISAMPLE (not ES!)

	BuildFont();

	DoReset();
	checkErrors("init end");

	initialized();
}


void GLWidget::resizeGL(int width, int height)
{
	m_cxClient = width;
	m_cyClient = height;

	reCalcProj(true);
}



void GLWidget::reCalcProj(bool fFromScratch) // = true default
{
	if (fFromScratch)
	{
		double figX = qMax(aqmax[0] - aqmin[0], (float)m_minScaleReset),
			   figY = qMax(aqmax[1] - aqmin[1], (float)m_minScaleReset),
			   figZ = qMax(aqmax[2] - aqmin[2], (float)m_minScaleReset);

		scrScale = qMin(qMin(4/figX, 4/figY), 4/figZ)*0.7;
		realScale = qMin(qMin(m_cxClient/figX, m_cyClient/figY), m_cxClient/figZ)*0.7; // for translate

		m_AspectRatio = (GLdouble)m_cxClient/(GLdouble)m_cyClient;

		glViewport(0, 0, m_cxClient, m_cyClient);
		proj.cur().identity();
	}

	if (m_viewState == Perspective) {
		Mat4 p;
		sgluPerspective(60.0, m_AspectRatio, 0.1, 100.0, p.m );
		proj.mult(p);
		proj.translate(0.0f, 0.0f, -4.0f);
		proj.scale(scrScale, scrScale, scrScale);
	}
	else 
	{
		double windowSize = 4;	// the size of the window in GL coord system.
		if ( m_AspectRatio > 1) {	
		//	glOrtho(-windowSize*m_AspectRatio/2.0, windowSize*m_AspectRatio/2.0, -windowSize/2.0, windowSize/2.0, -windowSize*m_AspectRatio/1.0, windowSize*m_AspectRatio/1.0);
		} 
		else {
		//	glOrtho(-windowSize/2.0, windowSize/2.0, -windowSize/m_AspectRatio/2.0, windowSize/m_AspectRatio/2.0, -windowSize/m_AspectRatio/1.0, windowSize/m_AspectRatio/1.0);
		}

		//glScaled(scrScale, scrScale, scrScale);
	}

}



void GLWidget::setUsingLight(bool use) 
{ 
	if (m_fUseLight == use)
		return;
	m_fUseLight = use; 

	if (!((PatchQGLContext*)(context()))->initialized())
		return; // can happen if we go to options before going to the solution view
	makeCurrent();
	reCalcLight(); 
	updateGL();
}

void GLWidget::setLightPos(const Vec3 &c)
{
	if (m_lightPos == c)
		return;
	m_lightPos = c; 

	if (!((PatchQGLContext*)(context()))->initialized())
		return; // can happen if we go to options before going to the solution view
	makeCurrent();
	reCalcLight(); 
	updateGL();
}

void GLWidget::setLightColor(const Vec3& c)
{
	if (m_lightColor == c)
		return;
	m_lightColor = c; 

	if (!((PatchQGLContext*)(context()))->initialized())
		return; // can happen if we go to options before going to the solution view

	makeCurrent();
	reCalcLight(); 
	updateGL();

}


void GLWidget::reCalcLight()
{
}

void GLWidget::DoReset()
{
	m_osf = 1.0;
	makeCurrent();	
	m_zoomVal = 100;

	reCalcProj(true);
	reCalcLight();
	// now select the modelview matrix and clear it
	// this is the mode we do most of our calculations in
	// so we leave it as the default mode.

	model.identity();

	// this is controvercial.
	if (m_bSkewReset)
	{
		rotate(XYaxis, -20, 20);
		rotate(Zaxis, -5, 0);
	}
	
	emit zoomChanged(m_zoomVal);
	emit callReset();
} 

void GLWidget::setNewMinMax(const Vec3& min, const Vec3& max, bool scale)
{
	aqmin = min; aqmax = max;
	if (scale) {
		reCalcProj();
	}

}

double zoomFactor(double perc)
{
	if (perc <= 1)
		return perc*perc;
	else
		return pow(3, perc) - 2;
}

void GLWidget::paintGL()
{
	makeCurrent();
	checkErrors("cur");
	model.push();
	double zv = zoomFactor(m_zoomVal / 100.0);
	//double zv = m_zoomVal / 100.0;
	model.scale(zv, zv, zv);

	model.translate(-(aqmax[0] + aqmin[0])/2, -(aqmax[1] + aqmin[1])/2, -(aqmax[2] + aqmin[2])/2);
	// a change here needs also a change in callDrawTargets!!
	checkErrors("x20");
	myPaintGL();
	model.pop();
	checkErrors("paint");
}

void GLWidget::callDrawTargets()
{
	model.push();
	double zv = zoomFactor(m_zoomVal / 100.0);
	//double zv = 2.1, m_zoomVal / 100.0;
	model.scale(zv, zv, zv);

	model.translate(-(aqmax[0] + aqmin[0])/2, -(aqmax[1] + aqmin[1])/2, -(aqmax[2] + aqmin[2])/2);

	drawTargets(true);	// the virtual function

	model.pop();
}


void GLWidget::myPaintGL()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// 	glBegin(GL_QUADS);
// 	glNormal3d(0, 0, -1);
// 	glVertex3d(0, 0, 0);
// 	glVertex3d(0, 1, 0);
// 	glVertex3d(1, 1, 0);
// 	glVertex3d(1, 0, 0);
// 
// 	glNormal3d(0, -1, 0);
// 	glVertex3d(0, 0, 0);
// 	glVertex3d(0, 0, 1);
// 	glVertex3d(1, 0, 1);
// 	glVertex3d(1, 0, 0);
// 	glEnd();
}

void GLWidget::setCullFace(bool b) 
{ 
	m_bBackFaceCulling = b; 
	if (m_bBackFaceCulling)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
	updateGL(); 
}

void GLWidget::setViewFrustrum(QAction *act) 
{ 
	setViewFrustrum((EViewState)act->data().toInt()); 
}

void GLWidget::setViewFrustrum(GLWidget::EViewState newState)
{
	if (m_viewState == newState)
		return; // nothing to do.
	m_viewState = newState;
	reCalcProj();
	reCalcLight();
	updateGL(); // Invalidates the window, posts WM_PAINT
}



void GLWidget::rotateTimeout()
{
	if ((!m_bTimerMode) || (!m_bContinuous))
		return; // just to be safe...
	if (QGLContext::currentContext() != context())
		return; // the current context isn't this views context

	ActMouseMove(Rotate, m_nXDelt, m_nYDelt);
	updateGL();

}

void GLWidget::setContinuous(bool val)
{ 
	m_bContinuous = val; 
	if (m_bTimerMode)
	{
		m_timer->stop();
		m_bTimerMode = false;
	}
}

void GLWidget::zoom(int v) 
{ 
	if (v == m_zoomVal)
		return;
	m_zoomVal = v; 
	reCalcLight();
	updateGL(); 
	// don't emit back. no one needs this now.
}  


// MOUSE HANDLERS ///////////////////////////////////////////

void sgluPickMatrix(double x, double y, double deltax, double deltay, int viewport[4], MatStack& mat)
{
	if (deltax <= 0 || deltay <= 0) { 
		return;
	}

	mat.translate((viewport[2] - 2 * (x - viewport[0])) / deltax, (viewport[3] - 2 * (y - viewport[1])) / deltay, 0);
	mat.scale(viewport[2] / deltax, viewport[3] / deltay, 1.0);
}


int GLWidget::doChoise(int chX, int chY)
{
	int	viewport[4] = {0, 0, m_cxClient, m_cyClient};
	makeCurrent();

	proj.push();
	proj.identity();
	
	// This Creates A Matrix That Will Zoom Up To A Small Portion Of The Screen, Where The Mouse Is.
	//gluPickMatrix((GLdouble)chX, (GLdouble)(viewport[3]-chY), 1.0f, 1.0f, viewport);
	sgluPickMatrix((double)chX, (double)(viewport[3]-chY), 1.0f, 1.0f, viewport, proj);
	
	reCalcProj(false);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	callDrawTargets();
		
	proj.pop();

 	int choose = -1;
	uint buf[10] = {0};

	//glReadPixels(viewport[2] / 2, viewport[3] / 2, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, buf);
	glReadPixels(0, 0, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, buf);
	//printf("%X\n", buf[0]);
	choose = buf[0] & 0xffffff;
	if (choose == 0)
		return -1;

	return choose;

}


void GLWidget::mousePressEvent(QMouseEvent *event)
{
	m_lastPos = event->pos();
	
	if ((m_bTimerMode) && (m_mouseAct == Rotate))
	{
		m_timer->stop();
		m_nXDelt = 0;
		m_nYDelt = 0;
		m_bTimerMode = false;
	}
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (m_bContinuous && (m_mouseAct == Rotate) && (m_nXDelt != 0) || (m_nYDelt != 0))
	{
		// create it on demand.
		if (m_timer == NULL)
		{
			m_timer = new QTimer(this);
			connect(m_timer, SIGNAL(timeout()), this, SLOT(rotateTimeout()));
		}

		m_timer->start(25);
		m_bTimerMode = true;
	}
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
	makeCurrent();

	if (event->buttons() != 0)
	{
		int dx = event->x() - m_lastPos.x();
		int dy = event->y() - m_lastPos.y();

		if (m_mouseAct == Rotate)
		{		
			m_nXDelt = dx;
			m_nYDelt = dy;
		}
		ActMouseMove(m_mouseAct, dx, dy);
		m_lastPos = event->pos();

		updateGL();					// redraw scene
	}
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
	m_zoomVal += event->delta() / ZOOM_WHEEL_FACTOR;
	m_zoomVal = qMin(qMax(m_zoomVal, ZOOM_MIN), ZOOM_MAX);
	emit zoomChanged(m_zoomVal);
	updateGL();
}

void GLWidget::ActMouseMove(EMouseAction action, int dx, int dy)
{
	switch(action) 
	{			// act based on the current action
	case Rotate:    rotate(m_axis, dx, dy); break;
	case Translate: translate(dx, dy); break;
	case Scale:     scale(dx, dy); break;
	}
	// do not call updateGL, called does this
}


void GLWidget::rotate(EAxis axis, int x, int y) 
{
	Mat4 wmat;
	double fx = (double)x, fy = (double)y;

	if (m_transformType == WorldSpace) {
		wmat = model.cur();
		model.identity();
	}
	else {// objectspace, translate it to 0,0,0
		model.translate((aqmax[0] + aqmin[0])/2, (aqmax[1] + aqmin[1])/2, (aqmax[2] + aqmin[2])/2);
	}

	switch (axis)
	{
	case Xaxis:	model.rotate(fx, 1, 0, 0); break;
	case Yaxis:	model.rotate(fx, 0, 1, 0); break;
	case Zaxis:	model.rotate(fx, 0, 0, 1); break;
	case XYaxis: model.rotate(fx, 0, 1, 0); model.rotate(fy, 1, 0, 0); break;
	case XZaxis: model.rotate(fx, 0, 0, 1); model.rotate(fy, 1, 0, 0); break;
	case YZaxis: model.rotate(fx, 0, 1, 0); model.rotate(fy, 0, 0, 1); break;
	}

	if (m_transformType == WorldSpace) {
		model.mult(wmat);
	}
	else { // translate it back
		model.translate(-(aqmax[0] + aqmin[0])/2, -(aqmax[1] + aqmin[1])/2, -(aqmax[2] + aqmin[2])/2);
	}

	emit rotated(axis, x, y);
}

void GLWidget::translate(int xValue, int yValue) 
{
	double fx = ((double)xValue / realScale);
	double fy = ((double)yValue / realScale);
	Mat4 wmat;

	if (m_transformType == ObjectSpace) 
	{
		fx /= m_osf;
		fy /= m_osf;
	}

	if (m_transformType == WorldSpace)
	{
		wmat = model.cur();
		model.identity();
	}

	switch (m_axis)
	{
	case Xaxis: model.translate(fx, 0, 0); break;
	case Yaxis: model.translate(0, fx, 0); break;
	case Zaxis: model.translate(0, 0, fx); break;
	case XYaxis: model.translate(fx, 0, 0); model.translate(0, -fy, 0); break;
	case XZaxis: model.translate(fx, 0, 0); model.translate(0, 0, fy); break;
	case YZaxis: model.translate(0, 0, fx); model.translate(0, -fy, 0); break;
	}

	if (m_transformType == WorldSpace) {
		model.mult(wmat);
	}
}

void GLWidget::scale(int xValue, int yValue) 
{
	double f = 1 + (double)(xValue + yValue) / 150.0;	// object may flip if we do it too fast
	Mat4 wmat;

	if (m_transformType == WorldSpace)	// damn right it's needed
	{
		wmat = model.cur();
		model.identity();
	}

	model.scale(f, f, f);

	m_osf = m_osf * f;
	if (m_transformType == WorldSpace) {
		model.mult(wmat);
	}
}



void GLWidget::BuildFont()								// Build Our Bitmap Font
{
	QFont font("Courier New", 16, 75);										// Windows Font ID
	//font.setBold(true);
	
	m_fontBase = glGenLists(256);								// Storage For 96 Characters
	((PatchQGLContext*)(context()))->generateFontDisplayLists(font, m_fontBase);
}

void GLWidget::KillFont()	//TBD-call this?
{
	glDeleteLists(m_fontBase, 256);							// Delete All 256 Characters
}

void GLWidget::mglPrint(const QString &str)			// Custom GL "Print" Routine
{
	if (str == NULL)									// If There's No Text
		return;											// Do Nothing

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(m_fontBase);					// Sets The Base Character to 32
	glCallLists(str.length(), GL_UNSIGNED_BYTE, str.toLatin1());	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
}



const char* errorText(uint code)
{
	switch(code)
	{
	case GL_NO_ERROR: return "GL_NO_ERROR";
	case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
	case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
	case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
	default: return "-Unknown Error Code-";
	}
}


void mglCheckErrors(const char* place = NULL)
{
	QString s;
	GLenum code;
	int count = 0;
	while ((code = glGetError()) != GL_NO_ERROR && count++ < 10)
		s += QString("  0x%1: %2\n").arg(code, 0, 16).arg(errorText(code));
	if (!s.isNull())
	{
		QString ps =  "\n";
		if (place != NULL)
			ps = QString(place) + "\n";
		s.sprintf("GLError: %s%s", ps.toLatin1().data(), s.toLatin1().data());
		int ret = QMessageBox::critical(NULL, "GLError", s, "Continue", "Exit", "Break");
#ifdef Q_WS_WIN
		if (ret == 1)
			TerminateProcess(GetCurrentProcess(), 1);
		if (ret == 2)
			__asm int 3;
#else
		if (ret == 1 || ret == 2)
			exit(1);
#endif
	}
}
void mglCheckErrors(const string& s)
{
	mglCheckErrors(s.c_str());
}


void mglCheckErrorsC(const char* place = NULL)
{
	QString s;
	GLenum code;
	int count = 0;
	while ((code = glGetError()) != GL_NO_ERROR && count++ < 10)
		s += QString("  0x%1: %2\n").arg(code, 0, 16).arg(errorText(code));
	if (!s.isNull())
	{
		QString ps =  "\n";
		if (place != NULL)
			ps = QString(place) + "\n";
		printf("GLError: %s%s", ps.toLatin1().data(), s.toLatin1().data());
	}
}
void mglCheckErrorsC(const string& s)
{
	mglCheckErrorsC(s.c_str());
}

void GLWidget::checkErrors(const char* place)
{
	if (!context()->isValid()) {
		QMessageBox::critical(NULL, "Error", "invalid context");
		return;
	}
	mglCheckErrorsC(place);
}
