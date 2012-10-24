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

#ifndef __GENERAL_H__INCLUDED__
#define __GENERAL_H__INCLUDED__

//#include <QtGlobal>
//#include <QVector>
#include <cmath>
#include <vector>
#include <exception>

using namespace std;

/** \file
	Declares various classes and types which are used widely across the code base.
*/

enum EPlane
{
	PLANE_NONE = -1,
	YZ_PLANE = 0,
	XZ_PLANE = 1,
	XY_PLANE = 2 
};

enum EAxis
{
	AXIS_NONE = -1,
	X_AXIS = 1,
	Y_AXIS = 2,
	Z_AXIS = 3
};


typedef unsigned int uint;

#ifdef WIN32
	typedef __int64 mint64;
#else
	#include <inttypes.h>
	typedef int64_t mint64;
#endif

/// takes a number and transforms it to a quoted string such as 1,234,567.09.
/// which is much more readable for humans.
string humanCount(mint64 n);
/// \overload
/// takes a float number.
string humanCount(double n, int pers);

/// flush and burn all qt events.
/// use this when visuals need to update in mid-function before moving on.
void flushAllEvents(); 

/// boolean xor
inline bool hXor(bool a, bool b)
{
	return ((a && !b) || (!a && b));
}
/// bitwise xor
inline uint bXor(uint a, uint b)
{
	return (a ^ b);
}

/// used by several different files which don't include each other. no better place to declare this.
typedef vector<int> TTransformVec;

#define DISALLOW_COPY(className) \
	 className(const className &); \
     className &operator=(const className &)


#ifdef _DEBUG
	#define M_ASSERT(cond) do { if (!(cond)) throw std::exception(#cond); } while(0)
#else
	#define M_ASSERT(cond)
#endif


template <typename T>
inline const T &mMin(const T &a, const T &b) { if (a < b) return a; return b; }
template <typename T>
inline const T &mMax(const T &a, const T &b) { if (a < b) return b; return a; }



// profile command line VC6
// /SF ?CubeEngineProc@@YAIPAX@Z

#include "Vec.h"

#endif // __GENERAL_H__INCLUDED__
