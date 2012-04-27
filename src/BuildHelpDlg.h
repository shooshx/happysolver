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

#ifndef __BUILDHELPDLG_H__INCLUDED__
#define __BUILDHELPDLG_H__INCLUDED__

#include "DataWidgets.h"
#include "CubeDoc.h" // for EWarnMsg

/** @file
	Declares the BuildHelpDlg class.
*/

class QPushButton;
class QButtonGroup;
class QLabel;
class DataSpinBox;
class DataPushButton;
class QSlider;
class QMovie;
class MainWindow;
class QGroupBox;
class ActionPushButton;

/** BuildHelpDlg is the design sidebar widget displayed next to BuildGLWidget.
	The design sidebar contains the following widgets:
	- Add and remove buttons which control the double click action in the design view.
	- Global piece family selection and an "advanced" button to open the detailed
	piece selection window - PicsSelectWidget.
	- Counters of the currently selected tiles and tiles needed for the structures.
	- A textual frame with the current message for the user, possibly for error messages
	- A big "Solve It!" button
	- zoom control for the BuildGLWidget

	The design sidebar is designed to be as much intuitive and self explanatory as 
	possible. It is expected that the user would follow it top to bottom. First, add and
	remove piece from the structure. Second, select pieces for the solutions. Third,
	resolve any errors that might occur and if there are none, press "Solve It".
	The Global piece family selection has strong relation to the detailed piece selection.
	It directly affects the selection in it and is directly affected by selection
	made in it.
	\see BuildGLWidget PicsSelectWidget
*/
class BuildHelpDlg :public SizedWidget
{
	Q_OBJECT
public:
	BuildHelpDlg(QWidget* parent, MainWindow *main, CubeDoc *doc);
	virtual ~BuildHelpDlg() {}

	virtual QSize minimumSizeHint() const;

public slots:
	void changeAction(bool remove);
	void setTilesCount(int count);
	void setPieceCount(int count);
	void completePicsWidgets();
	void updateZoom(int v);

private slots:
	void actionPressed(int id);
	void statsUpdate(int hint, int data);

signals:
	void changedAction(bool remove); // Add or Remove
	void changedPiecesSelect();  // number of tiles in the structure
	void changedFamBox(int data, int count);
	void switchToPicsTab(int family = -1);
	void zoomChanged(int d); // delta

private:
	void updateFamCount(); // there are new numbers for the fam spinboxen
	void updateWarnMsgs();
	void updatePieceCount();
	void enableEditWidgets(bool v);
	void setActionLabel(bool enabled);

	CubeDoc *m_doc;
	MainWindow *m_main;
	
	QSlider *m_zoomSlider;
	QPushButton *m_addBot, *m_removeBot;
	QButtonGroup *m_buildBots;
	QLabel *m_actHelpLabel;
	QLabel *m_buildTilesLabel;
	QLabel *m_selPieces;
	QLabel *m_warnLabel;
	QWidget *m_picSel; // container for pics widgets
	QMovie *m_cubeAnim;
	QLabel *m_animLabel;
	ActionPushButton *m_goBot;

	QFont m_boldf; // bold font
	QString m_picText, m_picPre, m_tilesText; // the two parts of the piece count text

	/// FamCtrl aggragates the widgets of a single piece family in BuildHelpDlg.
	struct FamCtrl
	{
		DataSpinBox *num;
		//DataPushButton *custom; // button removed
		QLabel *count; // pieces count
		QLabel *name; // special color set by QPalette
		QLabel *icon;
	};

	QList<FamCtrl> m_fams;
	int m_curPressedId; // id of the currently pressed action button (add or remove)
};


#endif
