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

#include <QThread>

/** \file
	Declares the SolveThread class and its related RunStats class.
*/

class PicsSet;
class EngineConf;
class Solutions;
class Shape;

/** RunStats holds basic statistics and state for the solution engine.
	Currently the only statistic is the number of piece chages the engine 
	has performed and whether the engine is in 'lucky' state.
*/
struct RunStats
{
public:
	RunStats() : tms(0), lucky(false), maxp(0) {}
	void reset() 
	{
		tms = 0;
		lucky = false;
		maxp = 0;
	}

	volatile qint64 tms; ///< number of piece changes
	volatile bool lucky; ///< luck state of the engine.
	volatile int maxp;  ///< the maximal place reached on the shape
};


/** SolveThread is the thread dedicated for the solution generation engine.
	Its main functionality is to mediate between the raw engine to the rest of the application.
	the main method, run() is the thread's entry point.
	\see Configuration
*/
class SolveThread :public QThread
{
	Q_OBJECT
public:
	SolveThread() :m_pics(NULL), m_slvs(NULL), m_shp(NULL), m_conf(NULL)
		,fExitnow(FALSE), fRunning(false)
	{}

	void setRuntime(Solutions *slvs, Shape *shp, PicsSet *pics, EngineConf* conf) 
	{ 
		m_pics = pics; 
		m_conf = conf; 
		m_slvs = slvs;
		m_shp = shp;
	}

	virtual void run();

private:
	PicsSet *m_pics;
	Solutions *m_slvs;
	Shape *m_shp;
	EngineConf *m_conf; // pointer to the m_conf in the document. copied on run()

public:
	volatile int fExitnow; // should be TRUE or FALSE
	volatile bool fRunning;
	RunStats m_stats;

signals:
	void slvProgUpdated(int hint, int data); // Solve Progress Updates. directed to the SolveDlg
	void solvePopUp(int slvind); // suddenly there is a solution. it is in slvind index
	void fullEnumNoSlv(); // a full enumeration of the solutions was made with no results

	friend class Cube; // allow it to emit signals
};


#endif
