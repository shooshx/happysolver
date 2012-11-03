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

#ifndef __SPACE3D_H__INCLUDED__
#define __SPACE3D_H__INCLUDED__

#include "general.h"

/** \file
	Defines the templated Space3D.
	Defines BoundedBlock which is used as a template argument to Space3D and 
	BoundedBlockSpace3D which is the specific Space3D instancisation with additional
	methods.
*/


/** Space3D is a generic three dimentional array.
	It provided optimized access to the elements in it.
	it is used mostly in the shape generation.
	it is \e not yet used in the main solution engine in Cube
	\see Shape
*/
template<class T>
class Space3D  
{
public:
	Space3D() :szx(-1), szy(-1), szz(-1), szxTy(-1), data(NULL) {};
	Space3D(int _szx, int _szy, int _szz)
		:szx(_szx), szy(_szy), szz(_szz), szxTy(szx * szy), data(new T[szxTy * szz]) {}
	~Space3D()
	{
		delete[] data;
	}
	void create(int _szx, int _szy, int _szz)
	{
		const_cast<int&>(szx) = _szx; 
		const_cast<int&>(szy) = _szy; 
		const_cast<int&>(szz) = _szz; 
		szxTy = szx * szy;
		data = new T[szxTy * szz];
	}
	bool isCreated() const { return (szx != -1); }

	void clear(const T& val = T())
	{
		int sz = szxTy * szz;
		for (int i = 0; i < sz; ++i)
			data[i] = val;
	}

	T& axx(int x, int y, int z) { return data[x + (szx * y) + (szxTy * z)]; }
	T& axx(Vec3i c) { return data[c.x + (szx * c.y) + (szxTy * c.z)]; }
	T& axx(Vec3i c, int factor) { return data[(c.x/factor) + (szx * (c.y/factor)) + (szxTy * (c.z/factor))]; }

	const T& axx(int x, int y, int z) const { return data[x + (szx * y) + (szxTy * z)]; }
	const T& axx(Vec3i c) const { return data[c.x + (szx * c.y) + (szxTy * c.z)]; }
	const T& axx(Vec3i c, int factor) const { return data[(c.x/factor) + (szx * (c.y/factor)) + (szxTy * (c.z/factor))]; }

	const T& axxRepeat(int x, int y, int z) const {
		return axx(x % szx, y % szy, z % szz);
	}

	T* ptr() { return data; }

	const int szx, szy, szz;

private:
	
	int szxTy; ///< x times y. - used optimization.

	T *data;

};

/** BoundedBlock contains the walls existance data of a single voxel.
	A voxel is a 3D pixel. this class represents the walls of a voxel, specifically,
	which walls are blocked and which aren't. It is used in the calculation
	of the volume of a shape.
	\see BoundedBlockSpace3D
*/
struct BoundedBlock
{
	// initialized with 1s, filled with 0s
	BoundedBlock() :fill(1), left(false), right(false), up(false), down(false), front(false), back(false) {}
	BoundedBlock(int v) :fill(v), left(false), right(false), up(false), down(false), front(false), back(false) {}
	// walls that exist
	int fill;
	bool left, right; ///< X axis
	bool up, down; ///< Y axis
	bool front, back; ///< Z axis
};

/** BoundedBlockSpace3D is a 3D space of BoundedBlock elements.
	This class adds some additional functionality to Space3D<BoundedBlock>.
	It is used in the calculation of the volume of the shape.
	\see Space3D BoundedBlock
*/
class BoundedBlockSpace3D : public Space3D<BoundedBlock>
{
public:
	BoundedBlockSpace3D(int _szx, int _szy, int _szz)
		:Space3D<BoundedBlock>(_szx, _szy, _szz) {}

	int FloodFill(int x, int y, int z);
	void ErectWalls(int dim, Vec3i front, Vec3i back);
};


#endif // __SPACE3D_H__INCLUDED__
