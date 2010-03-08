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

#ifndef __SOLVEDLG_H__INCLUDED__
#define __SOLVEDLG_H__INCLUDED__

#include <QDialog>
#include "ui_SolveDlg.h"

/** \file
	Declares the SolveDlg class which is the small statistics dialog.
*/

#define SPEED_AVG_SIZE (5)

class CubeDoc;
class QTimer;

/** SolveDlg shows the statistics of a working solution engine.
	it displayes the data from RunStats to the user and calculates
	how many parts changes per second are observed.
	It also contains another "Solve It" button which does exactly what
	the one in BuildHelpDlg does.
*/
class SolveDlg :public QDialog
{
	Q_OBJECT
public:
	SolveDlg(QWidget *parent, CubeDoc *run);
	virtual ~SolveDlg() {}

private:
	void updateSol(int n = -1);
	void updatePart(int n = -1);
	void updateStatus();

	void start();
	void stop();

public slots:
	void statsUpdate(int hint, int data);
	void displayTimeout();

signals:
	void go();
	void visibilityChanged(bool showen);
	void resetSolves();

protected:
	virtual void hideEvent(QHideEvent* event) { emit visibilityChanged(false); QDialog::hideEvent(event); }
	virtual void showEvent(QShowEvent* event) { emit visibilityChanged(true); QDialog::showEvent(event); }

private:
	Ui::SolveDlg ui;
	CubeDoc *m_doc;

	QTimer *m_timer;
	time_t m_starttime, m_elapsedtime;

	// speed calculation
	qint64 m_last_tms;
	int m_speed[SPEED_AVG_SIZE]; // tms/sec
	int m_speedind;


};


#endif
