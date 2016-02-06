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

#include "OpenGL/glGlob.h"
#include "BaseGLWidget.h"
#include <QGLWidget>



/*
#define GL_BEGIN_TEXT() {	glMatrixMode(GL_PROJECTION); glPushMatrix();glLoadIdentity(); \
                            glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity(); \
                            glDisable(GL_DEPTH_TEST); glDisable(GL_TEXTURE_2D); }
#define GL_END_TEXT() {	glEnable(GL_DEPTH_TEST); glPopMatrix(); glMatrixMode(GL_PROJECTION); \
                        glPopMatrix(); glMatrixMode(GL_MODELVIEW); }
*/

//#define ZOOM_MIN (10)
//#define ZOOM_MAX (700)
//#define ZOOM_WHEEL_FACTOR (5)




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
class GLWidget : public QGLWidget, public BaseGLWidget
{
    Q_OBJECT
public:
    /// the GLWidget constructor.
    /// alot of default values are set here. if a derived subclass wishes for a different 
    /// setting he may alter them in its constructor.
    GLWidget(QWidget *parent = nullptr, QGLWidget *sharefrom = nullptr);
    virtual ~GLWidget() {}


    void checkErrors(const char* place);

public slots:
    void rotateTimeout();
    // GUI actions
    void resetState() { 
        doReset(); 	
        updateGL(); 
    }
    void setContinuous(bool val);

    virtual void updateView(int hint) { Q_UNUSED(hint) }; // updates the opengl view according to the hints

    void call_rotate(EAxis axis, int x, int y) { 
        rotate(axis, x, y); 
    }
    void externalZoom(int v); // v between 0 and 100

    //void setUsingLight(bool use);
    //Vec3 lightPos() { return m_lightPos; }
    //void setLightPos(const Vec3& c);
    //void setLightColor(const Vec3& c);
    //void reLight() { 
    //    makeCurrent(); 
    //    updateGL(); 
    // }

signals:
    void rotated(GLWidget::EAxis axis, int x, int y);
    void callReset(); // means my reset was just called
    void zoomChanged(int v); // the view found it appropriate to change the zoom, notify interested parties

protected:
    void mglPrint(const QString &str);			// Custom GL "Print" Routine

    virtual void initializeGL();
    virtual void resizeGL(int width, int height);
    void doReset();

    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);

protected:

    virtual void rotate(EAxis axis, int x, int y);
   
    bool m_bContinuous; // the Continuous option is on. this says nothing about the current state of the timer




private:
    virtual void paintGL(); // (event) demote it to private from protected
    void callDrawTargets(); 

    void BuildFont();	///< Build Our Bitmap Font
    void KillFont();	///< Delete The Font

private:
    bool m_bTimerMode; // used for continuous rotation. if true, timer is On and rotating.
    QTimer *m_timer;   // used for continuous rotation. the timer. every GLWidget has its own timer
    int m_nXDelt, m_nYDelt; // used for continuous rotation, last mouse delta.


    int m_fontBase;

    friend class ModelGLControl;
    friend class BuildGLControl;

};

#endif // __GLWIDGET_H__INCLUDED__
