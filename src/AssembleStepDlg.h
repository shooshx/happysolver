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

#ifndef __ASSEMBLESTEPDLG_H__INCLUDED__
#define __ASSEMBLESTEPDLG_H__INCLUDED__

#include <QDialog>

/** @file
	Declares the AssembleStepDlg class.
*/

class QSlider;
class QPushButton;

/** AssembleStepDlg is the small dialog that contains a slider that controls the assembly step shown.
	All this dialog contains is the slider and the next and previos buttons.
	Normally the slider is always in the maximum notch so that all the pieces in the solution
	are visible. When the solution changes and the number of pieces with it, there is a need
	the change the slider back to the maximal place where it should be.
*/
class AssembleStepDlg :	public QDialog
{
	Q_OBJECT
public:
	AssembleStepDlg(QWidget *parent);
	virtual ~AssembleStepDlg() {}

public slots:
	void setTilesCount(int n);

private slots:
	void prev();
	void next();

signals:
	void stepChanged(int step);
	void visibilityChanged(bool showen);

private:
	QSlider *m_slider;
	QPushButton *m_next, *m_prev;

protected:
	virtual void hideEvent(QHideEvent* event) { emit visibilityChanged(false); QDialog::hideEvent(event); }
	virtual void showEvent(QShowEvent* event) { emit visibilityChanged(true); QDialog::showEvent(event); }

};

#endif //__ASSEMBLESTEPDLG_H__INCLUDED__

