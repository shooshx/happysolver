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

#ifndef __MODELGLWIDGET_H__INCLUDED__
#define __MODELGLWIDGET_H__INCLUDED__

#include "GLWidget.h"
#include "Texture.h"
#include "OpenGL/Shaders.h"

/** \file
	Declares the ModelGLWidget class.
*/

class SlvPainter;
class MyObject;
class CubeDoc;
class SlvCube;

/** ModelGLWidget is the GLWidget which displays the finished solution.
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
class ModelGLWidget :public GLWidget
{
	Q_OBJECT
public:
	ModelGLWidget(QWidget *parent, CubeDoc *document);
	virtual ~ModelGLWidget() {}
	void initTex();

private:
	CubeDoc *m_doc;

	Texture texture[2];
	int m_nSingleChoise; // right click, show a single piece
	int m_nHoverChoise, m_nLastHoveChs;

	SlvProgram m_progLight;
	NoiseSlvProgram m_progNoise;
	FlatProgram m_progFlat; // for choice selection

protected:
	virtual void drawTargets(bool inChoise);

// events
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void myPaintGL();
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void initialized();
	virtual void keyPressEvent( QKeyEvent *k );

public slots:
	virtual void updateView(int hint);

signals:
	void changedHoverPiece(int piece);
	void chosenSinglePiece(int choise); // means the single piece choise was changed (may have changed to -1)

public:
	static void drawIFSPolygons(GLWidget *that, const MyObject &obj, bool fTargets);
	static void drawIFSLines(const MyObject &obj, bool singleChoise);

	/// do the actual painting the scene. This is a static method because it is used
	/// also in PieceGLWidget for drawing.
	/// \arg \c that the GL context in which to draw. (not really needed because we use display lists.)
	/// \arg \c doc the CubeDoc is not a part of the GLWidget so it needs to be sent along to know the shape
	/// \arg \c scube the actual specific solution to draw
	/// \arg \c fTargets are we drawing plain targets or actual fully textured and lit pieces.
	/// \arg \c singleChoise when drawing the whole solution this is -1 otherwise it is
	/// the index of the specific piece to draw.
	/// \arg \c upToStep up to what step to draw the solution or -1 for the entire solution.
	/// currently, -1 is never used and in the normal situation this would be the count of the
	/// tiles in the shape.
	static void paint(GLWidget* that, CubeDoc *doc, SlvCube *scube, bool fTargets, int singleChoise, int upToStep);
	

private:
	void reCalcSlvMinMax();

};


#endif
