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

#ifndef __POINT_H__INCLUDED__
#define __POINT_H__INCLUDED__

#include "general.h"

class MyPoint;
class MyPolygon;

/** \file
	Declares the MyPoint class and the HalfEdge and MyLine classes which relate to it.
*/

/** HalfEdge objects are used in the process of polygon subdivision in MyObject::subdivide().
	For every subdivision iteration performed, a complete half-edge model is generated
	for the polygon mesh. a half-edge is a tupple of an edge of a polygon and one of
	the vertices it touces.
	\see MyPolygon MyObject::subdivide()
*/
class HalfEdge
{
public:
	HalfEdge() :point(nullptr), pair(nullptr), poly(nullptr), next(nullptr), edgePoint(nullptr) {}
	HalfEdge(MyPolygon *_poly, MyPoint *_pnt, HalfEdge *_next)
		:point(_pnt), pair(nullptr), poly(_poly), next(_next), edgePoint(nullptr) {}
	void init(MyPolygon *_poly, MyPoint *_pnt, HalfEdge *_next)
	{ point = _pnt; pair = nullptr; poly = _poly; next = _next; edgePoint = nullptr; }

	MyPoint* point; ///< point of origin of the half edge.
	HalfEdge* pair; ///< matchin half-edge sibling
	MyPolygon* poly; ///< the polygon this half edge is part of
	HalfEdge* next; ///< the next half edge in the polygon circumference.

	/// the new point generated in this edge in the subdivision process
	/// this point is shared between the two edges which make the polygon edge.
	MyPoint* edgePoint; 
};

/** MyPoint represents a point in 3D space with some additional information relevat to it.
	objects of this class are used along side with MyPolygon and MyLine in the process
	of creating the pieces display lists and in the direct rendering of lines. They are
	used solely with in the 3D solution display engine.
	instances of this class are usually managed with in a Pool of objects
	\see PicPainter MyObject MyPolygon MyLine
*/
class MyPoint  
{
public:
	MyPoint(float inX = 0.0, float inY = 0.0, float inZ = 0.0)
		:p(inX, inY, inZ), he(nullptr), touched(false) { ++g_ctorCount; }
	MyPoint(const MyPoint& a)
		:p(a.p), he(nullptr), touched(false) { ++g_ctorCount; }

	/// explicit conversion from Coord3df. we don't want to convert it to MyPoint 
	/// by accident
	explicit MyPoint(const Vec3& c) 
		:p(c), he(nullptr), touched(false) { ++g_ctorCount; }

	~MyPoint() { ++g_dtorCount; }
	
	void clear() { p.clear(); n.clear(); }
	void setp(const Vec3 &c) { p = c; }
	uint hash() const { const uint *up = reinterpret_cast<const uint*>(p.ptr()); return (bXor(up[0], bXor(up[1] >> 8, up[2] >> 16))); }

	MyPoint &operator+=(const MyPoint& a) { p += a.p; return *this; }
	MyPoint &operator/=(float s) { p /= s; return *this; }
	MyPoint &operator*=(float s) { p *= s; return *this; }
	MyPoint &operator=(const MyPoint& a) { p = a.p; return *this; }

	Vec3 p; ///< actual coordinate value
	Vec3 n; ///< normal of this point.

	// temporarys
	HalfEdge *he; ///< HalfEdge originating from this point. - used in MyObject::subdivide()
    /// was this point adjusted in the current subdiv iteration? - used in MyObject::subdivide()
    /// in no-subdiv meshes, did this point have a set normal? or do we need to calc it (for flat faces)
	bool touched; 
                   

	// managment
	static int g_ctorCount, g_dtorCount; ///< keep bookmarking of creation and deletion for debug
};

inline MyPoint operator+(const MyPoint& a, const MyPoint& b) { MyPoint r(a); r += b; return r; }
inline MyPoint operator/(const MyPoint& a, float s) { MyPoint r(a); r /= s; return r; }
inline MyPoint operator*(float s, const MyPoint& a) { MyPoint r(a); r *= s; return r; }
inline MyPoint operator*(const MyPoint& a, float s) { MyPoint r(a); r *= s; return r; }
inline bool operator==(const MyPoint &p1, const MyPoint &p2) { return (p1.p == p2.p); }




/** MyLine is a single line drawned using a specific color between two MyPoint instance.
	lines are used in the 3D solution display engine to mark the edges of pieces in 
	a clear manner. MyLine objects are not part of MyObject instances. they are drawn
	seperatly using LinesCollection.
	\see MyPoint
*/
class MyLine  
{
public:
	Vec3 color;
	MyPoint *p1, *p2; 

	ELineType type;

	MyLine(MyPoint *_p1, MyPoint *_p2, float clR, float clG, float clB, ELineType _type)
	: color(clR, clG, clB), p1(_p1), p2(_p2), type(_type)	{}
	MyLine() {} ///< empty ctor, needed for it to be possible to create arrays of MyLine
};

#endif // __POINT_H__INCLUDED__
