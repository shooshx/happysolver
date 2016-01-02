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

#ifndef __PICSSELECTWIDGET_H__INCLUDED__
#define __PICSSELECTWIDGET_H__INCLUDED__

#include <QWidget>

/** \file
	Declares the PicsSelectWidget class used for advanced pieces selection.
*/

class CubeDoc;
class DataPushButton;
class DataSpinBox;
class QToolBar;
class QAction;
class QLabel;
class QVBoxLayout;
class QPushButton;
class DataSpinBox;
class QTabWidget;


/** PicsSelectWidget is the detailed piece selection window.
	It appears when the used presses the "advanced" button in the piece
	selection widget in the design view. it allows the user to select pieces
	for the solution in a move fine grained manner, down to the single piece.
	Every family of pieces has its own tab in the PicsSelectWidget. In the
	family tab the user can see all the pieces of the family and select them
	individually.
	Additionally, when viewing a solution, the widget displays how many instances
	of every piece participate in it.
	This widget contains quite alot of labels, buttons and spinboxes which are cross
	referenced. in a naive implementation a change to one of them is likely to cause 
	a cascading change which repeats itself many times and causes significat delay in the GUI.
	The widgets in this class are carefully tuned and optimized to avoid these
	cascading changes and minimize the delay.
*/
class PicsSelectWidget : public QWidget
{
	Q_OBJECT
public:
	PicsSelectWidget(QWidget *parent, CubeDoc *document);
	virtual ~PicsSelectWidget() {}
	virtual QSize minimumSizeHint() const { return QSize(0, 0); }

public slots:
	// verbs in past tense refer to a thing that happened and needs appropriate reaction or update
	// verbs in present refer ro something that needs to be done promptly.
	void continueCreate();
	void pressedPicButton(int data, bool checked); ///< a pic button was pressed
	void changedNumBox(int data, int num); ///< a spinbox value changed
	void pressedGlobalAll();
	void pressedGlobalNone();
	void changeTab(int data);
	void changedTabBox(int count); ///< tab changed, react
	void changeFamBox(int count, int data); // data is Zero based number of family
	void setBuildTilesCount(int n); // from the build view
	void changeToResetValues();

	void updateView(int hint);
	void slvProgStatsUpdate(int hint, int data);
	void enableEdit(bool v);


signals:
	void changedPieceCount(int num);
	void switchToDesign();

private slots:
	void updateSetsSpinBox(int index);
	void changedGrpBox(int val, int data);

private:
	void updateCount();
	void updatePic(int g, int p, int num);
	void updateSolNum(int g, int p, int num);
	void updateSolveText(int first, int second); // the solution label 
	void updateFamSelected();
	void updateNumLabel();
	int calcGrpSelected(int g);

	void globalSignalBlock(bool block);	
	void famSignalBlock(bool block);

	/// PicCtrl holds the widgets relevant for a \link PicDef single piece \endlink in PicsSelectWidget.
	struct PicCtrl
	{
		PicCtrl() :num(nullptr), bot(nullptr), snum(nullptr), settledNum(-1) {}

		DataSpinBox *num;
		DataPushButton *bot;
		QLabel *snum; ///< this QLabel has special color set by QPalette

		int settledNum; ///< the number of pics that's going to be set in the widgets. used to avoid some recursion
	};

	/// GroupCtrl holds the widgets relevant for a \link PicGroupDef group of pieces. \endlink
	struct GroupCtrl
	{
		QList<PicCtrl> pics;
		DataSpinBox *num;
	};

	QVector<GroupCtrl> m_groups;

private:
	CubeDoc *m_doc;
	bool m_bFinishedCreate;
	QVBoxLayout *m_layout;
	QPalette *m_slvPalette, *m_countPalette;

	QPushButton *m_selectAllBot;
	QPushButton *m_selectNoneBot;
	QLabel *m_solveLabel;
	QLabel *m_numLabel;
	DataSpinBox *m_selectCurrent;
	QTabWidget *m_tabs;

	int m_currentGlobalCount; ///< optimization for UpdateCount
	int m_currentBuildTilesCount;

	bool m_bIncomingSignalsBlocked; // optimization
	int m_nGblocked, m_nFblocked;
};


#endif
