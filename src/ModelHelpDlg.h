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

#ifndef __MODELHELPDLG_H__INCLUDED__
#define __MODELHELPDLG_H__INCLUDED__

#include "GLWidget.h"
#include "DataWidgets.h"

/** \file
	Declares the ModelHelpDlg class and OpenGl view inside it: PieceGLWidget.
*/


class CubeDoc;
class QSlider;
class QSplitter;

/** PieceGLWidget is the small OpenGL widget in the solution sidebar which shows 
	the current piece. The piece viewed is the last piece hovered by the mouse 
	in the main ModelGLControl. This view is mostly for convenience so that the user 
	would be able to see the exact shape and orientation of a possibly obscured 
	piece in the solution. When the user right clicks a piece in the main solution
	view the solution view and the piece view switch places so that the single 
	piece is displayed in the main view and the whole solution is displayed in the
	piece view. The object in the piece view can be freely rotated on its own
	but it will reset its rotation when the there is a rotation in the main view
	or when a different piece is selected.
	This class uses the static ModelGLControl::paint() to do all its 3D display.
	\see ModelGLControl ModelHelpDlg
*/
class PieceGLWidget : public GLWidget
{
	Q_OBJECT
public:
	PieceGLWidget(QWidget *parent, CubeDoc *doc, QGLWidget* shareFrom);
	virtual ~PieceGLWidget() {}
	virtual void myPaintGL();

	void setSelectedPiece(int p);
	void setSelectAll(int piece);
	virtual void mouseMoveEvent(QMouseEvent *event);

public slots:
	void externRotate(GLWidget::EAxis axis, int x, int y);
	void clear();

private:
	CubeDoc *m_doc;
	int m_lastSelectedPiece; // last before we move to selectAll. needed if we won't have a new one on release
	int m_selectedPiece;
	bool m_bSelectAll; // means we should display it all

	/// true when the user rotated the model in this view independendtly from the rotation 
	/// of the main solution view.
	bool m_bHasPushedMatrix;

};

class QLabel;
class QPushButton;
class MainWindow;
class ActionPushButton;

/** ModelHelpDlg is the solution view sidebar displayed next to ModelGLControl.
	the solution sidebar contains the following widgets:
	- a small 3D piece view with the currently selected piece which shows its
	orientation in 3D space (m_pieceView)
	- a 2D piece view that identifies the piece using a bitmap from the detailed
	piece selection. (m_picImage)
	- (bottom) the solution browser which enables the user to browse within the
	currently found solutions. this browser is also available as a toolbar which 
	is off by default.
	- a zoom sliderer for the ModelGLControl
	- a button for showing the step-by-step dialog

	the botder betweent the 3D piece view and the rest of the sidebar is a split
	window border which can be moved if the user wants to resize the piece view.
	\see PieceGLWidget
*/
class ModelHelpDlg :public SizedWidget
{
	Q_OBJECT
public:
	ModelHelpDlg(QWidget *parent, MainWindow *main, CubeDoc *doc, QGLWidget *shareFrom);
	virtual ~ModelHelpDlg() {}

	//virtual QSize minimumSizeHint () const;
	PieceGLWidget *getPieceView() { return m_pieceView; }

	virtual QSize minimumSizeHint() const;

public slots:
	/// the currently selected (hovered) piece was changed
	void changeViewPiece(int piece);

	/// the current selection state was changed. possibly
	/// there's a need to switch from single piece viewing to
	/// whole solution viewing.
	void changedExternViewPiece(int piece); 

	void clear(); ///< sets the current piece to nothing (loaded a new solution)
	void updateView(int lHint);
	void statsUpdate(int hint, int data);
	void updateZoom(int v);

signals:
	void zoomChanged(int v);

protected:
	void resizeEvent(QResizeEvent *event);

private:
	void updatePixmapLabel(int piece);
	
	MainWindow *m_main;

	PieceGLWidget *m_pieceView;
	QLabel *m_picImage;
	CubeDoc *m_doc;
	QSlider *m_zoomSlider;
	QLabel *m_animLabel;
	QMovie *m_cubeAnim;
	ActionPushButton *m_goBot;
	QSplitter *m_splitter;

};


#endif
