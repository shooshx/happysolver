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

#ifndef __ACFFOCUSCOMBOBOX_H__INCLUDED__
#define __ACFFOCUSCOMBOBOX_H__INCLUDED__

#include <QComboBox>

/** @file
	Declares the SlvData and ActFocusComboBox classes.
*/


/** SlvData is the data object for the action in the solution number combo of ModelHelpDlg.
	It carries the index of the current selected solution and the overall number of solutions.
*/
struct SlvData
{
	SlvData() :index(0), size(0) {}
	SlvData(int i, int s) : index(i), size(s) {}

	int index; ///< current solution index
	int size; ///< overall number of solutions
};

/// SlvData needs to be detlared like this to be able to interact with QVariant
Q_DECLARE_METATYPE(SlvData); 

/**	ActFocusComboBox is the combo-box of the solution number is ModelHelpDlg.
	It is special in that it only updates its content when it get the focus. 
	This is an optimization that help avoid changing the content of the combo box
	every time a solution is found.
*/
class ActFocusComboBox :public QComboBox
{
	Q_OBJECT
public:
	ActFocusComboBox(QAction* act, QWidget *parent = NULL)
		:QComboBox(parent), m_defaultAction(NULL) 
	{
		setFocusPolicy(Qt::ClickFocus);
		setDefaultAction(act);
	}
	virtual ~ActFocusComboBox() {}

	void setDefaultAction(QAction *act);

protected:
	virtual void focusInEvent(QFocusEvent *event) { updateList(); }
	virtual void actionEvent(QActionEvent *event);

private slots:
	void setActIndex(int ind);

private:
	void updateList();
	QAction *m_defaultAction;
};

#endif // __ACFFOCUSCOMBOBOX_H__INCLUDED__
