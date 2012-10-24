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

#ifndef __BUILDGLWIDGET_H__INCLUDED__
#define __BUILDGLWIDGET_H__INCLUDED__

#include "GLWidget.h"
#include "BuildWorld.h"
#include "Shape.h"
#include "Mesh.h"
#include "OpenGL/Shaders.h"

/** @file
	Declares the BuildGLWidget class.
*/

class CubeDoc;
class QTimer;

/** BuildGLWidget is the interactive GLWidget used for designing the structure to solve.
	The current design displayed in this view is maintained at all times in its
	rawest form in the BuildWorld instance in CubeDoc. The method drawTargets()
	mades several passes on this structure to display it, polygon by polygon on
	the view area.

	The structure is edited by the user by double clicking an existing tile to add
	another 1x1x1 box infront of it. The editing mode is called "boxed editing". boxed
	editing was introduced in version 2.0. Earlier structured were edited in what is called
	"tiled editing" where the user adds every tile seperatly. Boxed editing is more
	intuitive to the user and faster to learn and master and can express most of the
	structures the user would ever want to build. The only exception to that is the
	class of structures which are open shapes, for instance a 3x3x0 floor. These
	shapes cannot be build using boxed editing simply because they are not made of whole
	cube boxes. Tiled editing is not supported as of Version 2.0 although the code is 
	still around.

	The user uses this view to create and edit his design. When he is satisfied with 
	the design he may click "Solve It" in order to find solution for it. Prior
	to finding solutions, the raw design data from BuildWorld needs to be transformed in to
	an instance of Shape for the solution engine to work on. This transformation is implemented
	in Shape::generate().

	When editing a structure one needs to consider the case where the user creates
	a errornous structure. such structures are:
	- D1: structures which are divided into more than one volume. the solver cannot
	handle more than one volume at a time.
	- D2: structures which are phyisically impossible to build due to limitations of the
	happy cube pieces. for instance structures which put more than 2 \link Shape::FaceDef faces \endlink 
	on a single \link Shape::SideDef side. \endlink
	- D3: structures which are physically impossible to build due to their volume properties
	for instance a large cube which have an inner hole volume which is inaccessible from outside.

	In the retired tiled editing mode the user was prohibited by the editor from creating 
	errornous structures of type D2 although he still could created structures of 
	classes D1 and D3. In boxed editing however the user can create stuructures of all
	three classes and needs to be notified about it when he does. This notification
	is achieved by generating the Shape from the sturucture with every edit the user makes.
	and keeping it in the BuildWorld::m_testShape member.
	\see BuildWorld Shape CubeDoc
*/
class BuildGLWidget :public GLWidget
{
	Q_OBJECT
public:
	BuildGLWidget(QWidget *parent, CubeDoc *document);
	virtual ~BuildGLWidget() {}

	/// keyboard events are delegated from MainWindow to this widget using this method.
	/// key delegation is needed in the case this widget is not the one in focus and we
	/// still want it to respond to ctrl presses that switch from EActStatus::ADD to EActStatus::REMOVE
	void keyEvent(QKeyEvent *event);

	/// the action a double click would cause
	enum EActStatus 
	{ 
		ADD, ///< a double click would add a box
		REMOVE,  ///< a double click would remove a box
		CANT_REMOVE, ///< a double click can't remove a box because it is the only box in the design
		EDIT_DISABLE ///< a double click would do nothing because edits are disable which solution engine is running.
	};

public slots:
	virtual void updateView(int hint);
	void setSelectYellow(bool v) { m_fSetStrtMode = v; updateGL(); }
	void setUnSetBlueMode(bool v) { m_fUnSetBlueMode = v; }
	void showAllBlue();
	void fadeTimeout();
	void enableEdit(bool v) { m_bEditEnabled = v; }
	void slvProgStatsUpdate(int hint, int data);

	// start the fade timer. we don't want it to go on if we're not in focus
	void switchIn();
	// stop the fade timer. we don't want it to go on if we're not in focus
	void switchOut();
	void changeAction(bool remove); // someone is saying its changed

signals:
	void changedAction(bool remove); // signal that I know it changed
	void changedTilesCount(int count);
	void changedTileHover(int tile, BuildGLWidget::EActStatus act); // changed the tile on which the mouse hovers, -1 is no tile

protected: 
	virtual void drawTargets(bool inChoise);

// events
	virtual void myPaintGL();

	virtual void mouseDoubleClickEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);

private:
	void tiledDblClick(int choise, QMouseEvent *event);
	void boxedDblClick(int choise, QMouseEvent *event);

	bool doMouseMove(QMouseEvent *event, bool remove);

	bool getChoiseTiles(int choise, bool remove, CoordBuild bb[6], Vec3i &g);

	//void drawTargetsPart(bool fTrans, bool fLines);
	void drawErrorCyliders();
	void checkSides();

	void reCalcBldMinMax();

	void makeBuffers();

	void initialized();

private:
	/// the document is needed for access to the current BuildWorld data
	CubeDoc *m_doc;

	bool m_bEditEnabled;
	bool m_fSetStrtMode;
	bool m_fUnSetBlueMode;

	bool m_bBoxedMode;

	bool m_bInternalBoxRemove; // internal, from ctrl
	bool m_bBoxRemove; // from the GUI button. this represents the external status, not including Ctrl button

	bool m_bLastBoxRemove; ///< used in mouseMoveEvent()
	int m_lastChoise; ///< used in mouseMoveEvent()
	Vec3i m_lastCubeChoise; ///< used in mouseMoveEvent()

	/// number of valid tiles in m_curMarkedTiles
	int m_nMarkedTiles;
	/// the tiles marked with blue or red under the mouse pointer
	CoordBuild m_curMarkedTiles[6];

	/// this timer is used to produce the slight fade effect of the transparent blue outline
	/// of a potential box to be added or the red outline of a box to be removed.
	QTimer *m_fadeTimer;

	/// when a D2 error occur the offending sides are marked by blinking cylinders
	/// to make the blinking effect we need to remember theie current alpha and
	/// the direction where that alpha was going.
	float m_errCylindrAlpha;
	float m_errCylindrAlphaDt; ///< advancing of the alpha


	bool m_bDoneUpdate; ///< optmization. avoiding multiple updates in mouseMoveEvent()

	Mesh m_realTiles;
	Mesh m_transTiles;
	Mesh m_realLines;
	Mesh m_transLines;

	Mesh m_cylinder;

	BuildProgram m_prog;
};


#endif
