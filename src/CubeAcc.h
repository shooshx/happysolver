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

#ifndef __CUBEACC_H_INCLUDED__
#define __CUBEACC_H_INCLUDED__

/** \file
	Declares various accessory classes used by the solution engine in Cube.
	Declares the UsedPieces and ShapePlace classes and their inline methods.
*/


/** UsedPieces is a bit array which indicated which pieces are in use.
	It is used solely in the solution engine for the purpose of marking
	pieces that are already selected into the currently built solution
	\see Cube
*/
class UsedPieces
{	// binary array
public:
	UsedPieces(int size_);
	~UsedPieces();
	void realloc(int newsize);

	bool axx(const int n);
	void set(const int n, const bool val);
	void multset(const int* rep, const int rpsz);
	void clear();
	
	static inline bool allones(UsedPieces& first, UsedPieces& second);

	int cnt, size;
private:
	
	bool *dt;

};

inline UsedPieces::UsedPieces(int size_): cnt(0), size(size_), dt(NULL)
{
	if (size == 0)
		return;
	dt = new bool[size];
	clear();
}

inline void UsedPieces::realloc(int newsize)
{
	delete[] dt;
	size = newsize;
	dt = new bool[size];
	clear();
}

inline UsedPieces::~UsedPieces()
{
	delete[] dt;
}

inline bool UsedPieces::axx(const int n)
{
	return dt[n];
}

inline void UsedPieces::set(const int n, const bool val)
{
	if (val != dt[n])
	{
		dt[n] = val;
		if (val) ++cnt;
		else --cnt;
	}
}

/// set a number of indexes to true, according to the list supplied.
/// this list will most likely be the PicType::rep list of
/// repeating pieces.
inline void UsedPieces::multset(const int* list, const int lsz)
{
	for(int i = 0; i < lsz; ++i)
	{
		if (!dt[list[i]])
		{
			dt[list[i]] = true;
			++cnt;
		}
	}
}

inline void UsedPieces::clear()
{
	for(int i = 0; i < size; ++i)
	{
		dt[i] = false;
	}
	cnt = 0;
}

/// check if the is an index in which both lists have a value of false.
inline bool UsedPieces::allones(UsedPieces& first, UsedPieces& second)
{
	Q_ASSERT(first.size == second.size);
	
	for (int i = 0; i < first.size; ++i)
	{
		if ((!first.axx(i)) && (!second.axx(i))) // both are 0;
			return false;
	}
	return true;
}


///////////////////////////////////////////////////////////////////////

/** ShapePlace represents a single tile in the Shape to be built.
	it is used solely in the processes of the solution engine.
	At all times the solution engine in Cube maintains an array of ShapePlace
	objects, one for every tile in the design. The order of this array
	is the exact order of the tiles in the generated Shape.
	This array is essentially the current state of the solution engine.
	it holds the information about which piece goes where and in what orientation.
	In addition to that, every ShapePlace object holds the data about what pieces
	were tried and failed in this place when this information is relevant.
	This simple structure is the heart of the solution engine.
	When a solution is found, a copy of the current ShapePlace array is made into
	an array of SlvCube::SlvPiece objects which resides in SlvCube.
	\see Cube
*/
class ShapePlace
{
public:
	ShapePlace(int useSize = 0) :sc(-1), rt(0), tryd(useSize) {}
	void realloc(int newsize); // needed since default ctor int he array receives 0 as size
	void clear();

	int sc;		///< which part is now in this place
	int rt;		///< what is it's rotation, index in rotation array
	UsedPieces tryd;  ///< which pieces were already tried in this place in the current iteration.
};

inline void ShapePlace::clear()
{
	sc = -1;
	rt = 0;
	tryd.clear();
}

inline void ShapePlace::realloc(int newsize)
{
	tryd.realloc(newsize);
}

#endif // __CUBEACC_H_INCLUDED__
