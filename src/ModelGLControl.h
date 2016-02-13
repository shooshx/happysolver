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

#ifndef __ModelGLControl_H__INCLUDED__
#define __ModelGLControl_H__INCLUDED__

#include "GLWidget.h"
#include "ModelControlBase.h"



class SlvPainter;
class MyObject;
class CubeDoc;
class SlvCube;

/** ModelGLControl is the GLWidget which displays the finished solution.
    At any time only one solution can be visible. The user can control the
    looks of the solution pieces from the options dialog under the display tab
    While viewing a solution the user can hover the mouse pointer over the
    solution pieces. This would cause the individual piece to show in the
    solution sidebar - ModelHelpDlg. The user can also right click a piece
    and that would cause it to appear on its own, effectively switching roles
    between the view in the sidebar and the main view.
    The paint() static method does the actual drawing by calling the SlvPainter of the
    current selected solution from CubeDoc. It is called by the virtual fucntion drawTarget()
    which in turn gets called by myPaintGL(), the entrypoint for drawing the actual scene.
    The SlvPainter of the specific solution selected is the one who eventually draws 
    the actual 3D scene.
    \see PieceGLWidget SlvPainter
*/
class ModelGLControl : public QObject, public ModelControlBase
{
    Q_OBJECT
public:
    ModelGLControl(GLWidget *gl, CubeDoc *document);
    virtual ~ModelGLControl() {}

    // delegated from MainWindow
    void keyEvent(QKeyEvent *event);

protected:
    virtual void emitChosenPiece(int p) {
        emit chosenSinglePiece(p);
    }
    virtual void emitHoverPiece(int p) {
        emit changedHoverPiece(p);
    }

private:
    GLWidget *m_gl;
    QTimer *m_fadeTimer;

public slots:
    virtual void updateView(int hint);

    bool fadeTimeout();

signals:
    void changedHoverPiece(int piece);
    void chosenSinglePiece(int choise); // means the single piece choise was changed (may have changed to -1)


};


#endif
