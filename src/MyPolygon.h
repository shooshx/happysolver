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

#ifndef __POLYGON_H__INCLUDED__
#define __POLYGON_H__INCLUDED__

#include "MyPoint.h"
#include "general.h"

class Texture;

/** \file
	Declares the MyPolygon class and the TexAnchor which relate to it.
*/

/**	TexAnchor is a simple 2D coordinate which specifies an anchor in a texture.
	Values of x and y of a texture anchor are usually between 0 to 1,
	(0,0) is the lower left corner and (1,1) is the upper right corner.
	This class is used in various texture related tasks in MyPolygon
*/
class TexAnchor
{
public:
	TexAnchor(double _x = 0.0, double _y = 0.0) :x(_x), y(_y) {}

	float x, y; // the mapping on the texture
};


/**	MyPolygon represents a single polygon to the 3D solution display engine.
	objects of this class aren't really used in the normal rendering of solutions.
	They are only used in the creation of the pieces display lists. Actual
	rendering is performed with these display lists.
	instances of this class are usually managed with in a Pool of objects
	\see PicPainter MyObject MyPoint
*/
class MyPolygon  
{
public:

	MyPolygon(MyPoint* inVtx[], TexAnchor *inAncs)
	 : he(NULL), f(NULL)
	{	// point are shallow and anchors are deep copied
		init(inVtx, inAncs);
		++g_ctorCount;
	}

	MyPolygon() { ++g_ctorCount; } 
	~MyPolygon() { ++g_dtorCount; }

	// two versions, to help the branch prediction :)
	void init(MyPoint* inVtx[], TexAnchor *inAncs)
	{
		for(int i = 0; i < 4; ++i)
		{
			vtx[i] = inVtx[i];
			texAncs[i] = inAncs[i];
		}
	}

	void init(TexAnchor *inAncs, Texture *_tex)
	{
		for(int i = 0; i < 4; ++i)
		{
			texAncs[i] = inAncs[i];
		}
	}

	/// calculate the normal of this polygon which is the vector pependicular to it.
	/// this is done with some math using the polygon's vertices.
	/// if none of the vertices for a polygon that is not a line an asserion is asserted.
	/// That assertion means there is a bug somewhere.
	void calcNorm();

	MyPoint *vtx[4]; ///< A vector list of the polygons points in their order
	
	Vec3 center; ///< The normal of polygon

	TexAnchor texAncs[4]; ///< Texture anchors by the order of 0,0 - 1,0 - 1,1 - 0,1
	//Texture *tex; 
	// relevant only to half-textured pieces per-polygon texture, use sort for this to be efficient
	// NULL to disable texturing

	// TBD- move away from here?
	HalfEdge *he;	
	MyPoint *f; // the center of polygon point.

	static int g_ctorCount, g_dtorCount; ///< keep bookmarking of creation and deletion for debug

};



#endif // __POLYGON_H__INCLUDED__
