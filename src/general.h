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

#include <QtGlobal>
#include <QVector>
#include <cmath>

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

/** Coord3d is a general purpose three dimentional vertor of ints.
	It is used in various places where there is no need for a float vector
	such as in the processes of BuildWorld
*/
struct Coord3d
{
public:
	Coord3d(int dx = 0, int dy = 0, int dz = 0) :x(dx), y(dy), z(dz) {}
	Coord3d(const Coord3d& src) :x(src.x), y(src.y), z(src.z) {};

	int x, y, z;

	Coord3d& operator+=(const Coord3d &a) { x += a.x; y += a.y; z += a.z; return *this; }
	Coord3d& operator-=(const Coord3d &a) { x -= a.x; y -= a.y; z -= a.z; return *this; }

};

inline bool operator==(const Coord3d &a, const Coord3d &b)
{
	return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
}
inline bool operator!=(const Coord3d &a, const Coord3d &b)
{
	return ((a.x != b.x) || (a.y != b.y) || (a.z != b.z));
}
inline Coord3d operator+(const Coord3d &a, const Coord3d &b)
{
	return Coord3d(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline Coord3d operator-(const Coord3d &a, const Coord3d &b)
{
	return Coord3d(a.x - b.x, a.y - b.y, a.z - b.z);
}

#define EPSILON (1e-5)

/** Coord3df is a general purpose three dimentional vector of floats.
	It is used mostly for purposes related to openGL coordinates system
*/
struct Coord3df
{
	Coord3df() { v[0] = 0.0; v[1] = 0.0; v[2] = 0.0; }
	Coord3df(float x, float y, float z) { v[0] = x; v[1] = y; v[2] = z; }
	/// initialize from a Coord3d of ints.
	explicit Coord3df(const Coord3d& a) { v[0] = (float)a.x; v[1] = (float)a.y; v[2] = (float)a.z; }
	void clear() { v[0] = 0.0; v[1] = 0.0; v[2] = 0.0; }

	Coord3df& operator=(const Coord3d &a)
	{
		v[0] = a.x; v[1] = a.y; v[2] = a.z;
		return *this;
	}
	Coord3df& operator+=(const Coord3df& a)
	{
		v[0] += a.v[0]; v[1] += a.v[1]; v[2] += a.v[2];
		return *this;
	}
	Coord3df& operator/=(float s)
	{
		v[0] /= s; v[1] /= s; v[2] /= s;
		return *this;
	}
	Coord3df& operator*=(float s)
	{
		v[0] *= s; v[1] *= s; v[2] *= s;
		return *this;
	}

	float &operator[](int row) { Q_ASSERT(row < 3); return v[row]; }
	const float &operator[](int row) const { Q_ASSERT(row < 3); return v[row]; }
	float const* ptr() const { return &v[0]; }

	friend Coord3df operator+(const Coord3df &a, const Coord3df &b);
	friend Coord3df operator/(const Coord3df &a, float s);
	friend Coord3df operator*(const Coord3df &a, float s);
	friend Coord3df operator*(float s, const Coord3df &a);
	friend bool operator==(const Coord3df& a, const Coord3df& b);

	void pmin(const Coord3df &a)
	{
		v[0] = qMin(v[0], a[0]);
		v[1] = qMin(v[1], a[1]);
		v[2] = qMin(v[2], a[2]);
	}

	void pmax(const Coord3df &a)
	{
		v[0] = qMax(v[0], a[0]);
		v[1] = qMax(v[1], a[1]);
		v[2] = qMax(v[2], a[2]);
	}
	void unitize()
	{
		double len = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
		Q_ASSERT(len != 0.0);
		v[0] /= len; v[1] /= len; v[2] /= len;
	}

	bool isNear(const Coord3df& vert) const
	{
		return (abs(vert.x - x) < EPSILON) && 
			   (abs(vert.y - y) < EPSILON) &&
			   (abs(vert.z - z) < EPSILON);
	}


	union {
		float v[3];
		struct {
			float x,y,z;
		};
	};
};

inline Coord3df operator+(const Coord3df &a, const Coord3df &b)
{
	return Coord3df(a.v[0] + b.v[0], a.v[1] + b.v[1], a.v[2] + b.v[2]);
}
inline Coord3df operator/(const Coord3df &a, float s)
{
	return Coord3df(a.v[0] / s, a.v[1] / s, a.v[2] / s);
}
inline Coord3df operator*(const Coord3df &a, float s)
{
	return Coord3df(a.v[0] * s, a.v[1] * s, a.v[2] * s);
}
inline Coord3df operator*(float s, const Coord3df &a)
{
	return Coord3df(a.v[0] * s, a.v[1] * s, a.v[2] * s);
}
inline bool operator==(const Coord3df& a, const Coord3df& b)
{
	return ((a.v[0] == b.v[0]) && (a.v[1] == b.v[1]) && (a.v[2] == b.v[2]));
}

struct Coord2df
{
public:

	Coord2df() : x(0.0f), y(0.0f) {}
	Coord2df(float nx, float ny) : x(nx), y(ny) {}


	bool isNear(const Coord2df& vert) const
	{
		return (abs(vert.x - x) < EPSILON) &&
			   (abs(vert.y - y) < EPSILON);
	}

	float x, y;

};



/// takes a number and transforms it to a quoted string such as 1,234,567.09.
/// which is much more readable for humans.
QString humanCount(qint64 n);
/// \overload
/// takes a float number.
QString humanCount(double n, int pers);

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
	return (a & ~b) | (~a & b);
}

/// used by several different files which don't include each other. no better place to declare this.
typedef QVector<int> TTransformVec;

// profile command line VC6
// /SF ?CubeEngineProc@@YAIPAX@Z

#endif // __GENERAL_H__INCLUDED__
