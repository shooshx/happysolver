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

#ifndef __SOLVETHREAD_H__INCLUDED__
#define __SOLVETHREAD_H__INCLUDED__

#include "general.h"
#include "Cube.h"
#include <QThread>

/** \file
	Declares the SolveThread class and its related RunStats class.
*/

class PicsSet;
class EngineConf;
class Solutions;
class Shape;
class SlvCube;



/** SolveThread is the thread dedicated for the solution generation engine.
	Its main functionality is to mediate between the raw engine to the rest of the application.
	the main method, run() is the thread's entry point.
	\see Configuration
*/
class SolveThread : public QThread, public SolveContext// TBD-ios
{
	Q_OBJECT
public:
	SolveThread() {}

	virtual void run();

    virtual void notifyLastSolution(bool firstInGo) override;
    virtual void notifyFullEnum() override
    {
        emit fullEnumNoSlv();
    }

    virtual void doStart() {
        start();
    }
    virtual void doWait() {
        wait();
    }

signals:
	void slvProgUpdated(int hint, int data); // Solve Progress Updates. directed to the SolveDlg
	void solvePopUp(int slvind); // suddenly there is a solution. it is in slvind index
	void fullEnumNoSlv(); // a full enumeration of the solutions was made with no results

	friend class Cube; // allow it to emit signals
};


#endif
