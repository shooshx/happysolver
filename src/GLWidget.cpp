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

#include "GLWidget.h"
#include <QtGui>
#include <QtOpenGL>
#include <iostream>

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
    : QGLWidget(g_format, parent, sharefrom)
{
    m_mouseAct = Rotate;

    // continous mode control
    m_bContinuous = false;
    m_bTimerMode = false;
    m_timer = nullptr;
    m_nXDelt = 0;
    m_nYDelt = 0;
    setMouseTracking(true);

    setFocusPolicy(Qt::StrongFocus); // receive the ctrl key press

}

void GLWidget::doReset() {
    makeCurrent();
    reset();
    emit zoomChanged(m_zoomVal);
    emit callReset();
}


void GLWidget::initializeGL() {
    makeCurrent();
    init();
    //BuildFont();
}

void GLWidget::paintGL() {
    makeCurrent();
    paint(false);
}

void GLWidget::resizeGL(int width, int height) {
    BaseGLWidget::resize(width, height);
}


/*
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
*/



void GLWidget::rotateTimeout()
{
    if ((!m_bTimerMode) || (!m_bContinuous))
        return; // just to be safe...
    if (QGLContext::currentContext() != context())
        return; // the current context isn't this views context

    rotate(m_axis, m_nXDelt, m_nYDelt);
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

void GLWidget::externalZoom(int v) 
{ 
    if (v == m_zoomVal)
        return;
    m_zoomVal = v; 
    updateGL(); 
    // don't emit back. no one needs this now.
}  


// MOUSE HANDLERS ///////////////////////////////////////////


void GLWidget::mousePressEvent(QMouseEvent *event)
{
    BaseGLWidget::mousePress((int)event->button(), event->x(), event->y());
    
    if ((m_bTimerMode) && (m_mouseAct == Rotate))
    {
        m_timer->stop();
        m_nXDelt = 0;
        m_nYDelt = 0;
        m_bTimerMode = false;
    }

    updateGL(); // the GLWidget mouse event doesn't have a reason to update

}


void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    BaseGLWidget::mouseRelease((int)event->button());

    if (m_bContinuous && (m_mouseAct == Rotate) && (m_nXDelt != 0 || m_nYDelt != 0))
    {
        // create it on demand.
        if (m_timer == nullptr)
        {
            m_timer = new QTimer(this);
            connect(m_timer, SIGNAL(timeout()), this, SLOT(rotateTimeout()));
        }

        m_timer->start(25);
        m_bTimerMode = true;
    }

    updateGL(); // the GLWidget mouse event doesn't have a reason to update

}

void GLWidget::rotate(EAxis axis, int x, int y) 
{
    m_nXDelt = x;
    m_nYDelt = y;
    BaseGLWidget::rotate(axis, x, y);
    emit rotated(axis, x, y);
}


void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    bool needpaint = BaseGLWidget::mouseMove((int)event->buttons(), ((event->modifiers() & Qt::ControlModifier) != 0), event->x(), event->y());
    if (needpaint) {
        updateGL();
    }
}

void GLWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    bool needpaint = BaseGLWidget::mouseDoubleClick( ((event->modifiers() & Qt::ControlModifier) != 0), event->x(), event->y() );
    if (needpaint) {
        updateGL();
    }

}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    BaseGLWidget::mouseWheelEvent(event->delta() / ZOOM_WHEEL_FACTOR);
    emit zoomChanged(m_zoomVal);
    updateGL();
}



/*
void GLWidget::scale(int xValue, int yValue) 
{
    double f = 1 + (double)(xValue + yValue) / 150.0;	// object may flip if we do it too fast
    Mat4 wmat;

    wmat = model.cur();
    model.identity();

    model.scale(f, f, f);

    model.mult(wmat);
}
*/
/*
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
    if (str == nullptr)									// If There's No Text
        return;											// Do Nothing

    glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
    glListBase(m_fontBase);					// Sets The Base Character to 32
    glCallLists(str.length(), GL_UNSIGNED_BYTE, str.toLatin1());	// Draws The Display List Text
    glPopAttrib();										// Pops The Display List Bits
}
*/


void GLWidget::checkErrors(const char* place)
{
    if (!context()->isValid()) {
        std::cout << "Error invalid context" << endl;
        return;
    }
    mglCheckErrorsC(place);
}
