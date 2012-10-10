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

#include "PicsSet.h"

/** UsedPieces is a bit array which indicated which pieces are in use.
	It is used solely in the solution engine for the purpose of marking
	pieces that are already selected into the currently built solution
	\see Cube
*/
class UsedPieces
{	// binary array
public:
	UsedPieces(const PicsSet* picSet) : m_dt(picSet->compSize()) {
		clear();
		for(int i = 0; i < m_dt.size(); ++i) {
			m_dt[i].count = picSet->comp[i].count;
		}
	}

	int get(int i) const { return m_dt[i].use; }
	void addOne(int i) {
		++m_dt[i].use;
	}
	void subOne(int i) {
		--m_dt[i].use;
	}
	int size() const { return m_dt.size(); }

	//void multset(const int* rep, const int rpsz);
	void clear() {
		//std::fill(m_dt.begin(), m_dt.end(), 0);
		for(int i = 0; i < m_dt.size(); ++i) {
			m_dt[i].use = 0;
		}
	}
	inline bool allUsed(int i) const {
		const UseEntry& e = m_dt[i];
		return e.use >= e.count;
	}
	
private:
	struct UseEntry {
		int use;
		int count;
	};
	vector<UseEntry> m_dt;    // number of used pieces of each PicType
};

class TriedPieces {
public:
	TriedPieces() :cnt(0) {}
	void realloc(int newsize) {
		m_dt.resize(newsize);
		clear();
	}

	int size() const { return m_dt.size(); }

	bool get(int i) const { return (bool)m_dt[i]; }
	void set(int i, bool val);

	//void multset(const int* rep, const int rpsz);
	inline void clear();

	bool tryedAll() const { return cnt == m_dt.size(); }

	int cnt; // total count of tried pieces

private:
	vector<int> m_dt;
};



inline void TriedPieces::set(const int i, const bool val)
{
	if (val != (bool)m_dt[i])
	{
		m_dt[i] = (bool)val;
		if (val) 
			++cnt;
		else 
			--cnt;
	}
}

/// set a number of indexes to true, according to the list supplied.
/// this list will most likely be the PicType::rep list of
/// repeating pieces.
// inline void UsedPieces::multset(const int* list, const int lsz)
// {
// 	for(int i = 0; i < lsz; ++i)
// 	{
// 		if (!dt[list[i]])
// 		{
// 			dt[list[i]] = true;
// 			++cnt;
// 		}
// 	}
// }




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
	ShapePlace(int useSize = 0) :sc(-1), rt(0) {}

	void clear() {
		sc = -1;
		rt = 0;
		tryd.clear();
	}

	inline void mclear() {
		sc = -1;
		rt = 0;
		mtryd.clear();
	}

	void allclear() {
		sc = -1;
		rt = 0;
		mtryd.clear();
		possible.clear();
	}


	int sc;		///< which part is now in this place
	int rt;		///< what is it's rotation, index in rotation array
	TriedPieces tryd;  ///< which pieces were already tried in this place in the current iteration.
	TriedPieces mtryd; // should be the maximum size of 'possible' which is the total count or rtns in comp

	vector<TypeRef> possible;
};



#endif // __CUBEACC_H_INCLUDED__
