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

#include "SolveThread.h"
#include "CubeDoc.h"
#include "Cube.h"


void SolveThread::run()
{
	setPriority(QThread::LowPriority);
	m_stats.reset();
	Cube rlcube(m_shp, m_pics, m_conf);
	fRunning = true;	

	emit slvProgUpdated(SHINT_START, 0);

	rlcube.puttgr(m_slvs, this, m_starterSlv);
	fRunning = false; // needed here because the Stop button change depends on it

	emit slvProgUpdated(SHINT_STOP, 0);

}

// when the thread stops the following signals are queued in this order:
// - SHINT_STOP : updates the status
// the last stop ensures the stats are ok
