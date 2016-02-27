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
    UsedPieces(const PicsSet* picSet) 
        : m_dt( (picSet != nullptr) ? picSet->compRef.size() : 0) // will be null in Cube::genLinesIFS
    {
		clear();
		for(int i = 0; i < m_dt.size(); ++i) {
			m_dt[i].count = picSet->compRef[i].count();
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
	vector<UseEntry> m_dt;    // number of used pieces of each PicType. index is the same as the comp array
};



class BitVector
{
public:
    BitVector()
    {}
    ~BitVector() {
        delete[] m_d;
    }
    BitVector(const BitVector& o) : m_bitsz(o.m_bitsz), m_intsz(o.m_intsz) 
    {
        m_d = new int[m_intsz];
        memcpy(m_d, o.m_d, m_intsz * 32);
    }
    void operator=(const BitVector&) = delete;

    void resize(int szbits) {
        delete m_d;
        m_bitsz = szbits;
        m_intsz = (szbits + 31) / 32;
        m_d = new int[m_intsz];
        clear();
    }
    bool get(int i) const {
        int p = i / 32;
        int n = i % 32;
        return ((m_d[p] >> n) & 1) == 1;
    }
    void set(int i, bool v) {
        int p = i / 32;
        int n = i % 32;
        int bit = 1 << n;
        if (v)
            m_d[p] |= bit;
        else
            m_d[p] &= ~bit;
    }
    void clear() {
        memset(m_d, 0, m_intsz * 4);
    }
    int size() const { 
        return m_bitsz;
    }

private:
    int m_intsz = 0;
    int m_bitsz = 0;
    int* m_d = nullptr;

};


#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))

//#define EMSCRIPTEN

class TriedPieces 
{
public:
	TriedPieces() :cnt(0) {}
	void realloc(int newsize) {
       // int sz = ROUND_UP(newsize, 4);
       // cout << "alloc-sz=" << sz << endl;
        m_dt.resize(newsize);
		clear();
	}

	int size() const { 
        return m_dt.size(); 
    }

#ifdef EMSCRIPTEN
    bool get(int i) const {
        return m_dt.get(i);
    }
    void set(int i, bool val) {
        if (val != m_dt.get(i))
        {
            m_dt.set(i, (bool)val);
            if (val)
                ++cnt;
            else
                --cnt;
        }
    }
#else
	bool get(int i) const { 
        return (bool)m_dt[i]; 
    }
	void set(int i, bool val) {
        if (val != (bool)m_dt[i])
        {
            m_dt[i] = (bool)val;
            if (val)
                ++cnt;
            else
                --cnt;
        }
    }
#endif
	//void multset(const int* rep, const int rpsz);

    inline void clear()
    {
        cnt = 0;
        if (m_dt.size() == 0)
            return;
#ifdef EMSCRIPTEN
        m_dt.clear();
#else
        memset(&m_dt[0], 0, m_dt.size() * sizeof(m_dt[0]));
#endif
    }

	bool tryedAll() const { return cnt == m_dt.size(); }

	int cnt; // total count of tried pieces

private:
#ifdef EMSCRIPTEN
    BitVector m_dt;
#else
    vector<int> m_dt;
#endif
    

};




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

	/*void clear() {
		sc = -1;
		rt = 0;
		tryd.clear();
	}*/

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


	int sc;		///< which part is now in this place, from compRef
	int rt;		///< what is it's rotation, index in rotation array
	//TriedPieces tryd;  ///< which pieces were already tried in this place in the current iteration.
	TriedPieces mtryd; // should be the maximum size of 'possible' which is the total count or rtns in comp
                       // a bit for every TypeRef in possibilities to indicate if we tryed it

    int start_sc = -1; // if sc == -2, the piece there is a starter piece that was not in our comp, this is its pdef index
    int start_rt = -1;

    int start_compsc = -1; // if its a set we returned to, this is the compRef sc and rti 
    int start_rti = -1;

	vector<TypeRef> possible;
};



#endif // __CUBEACC_H_INCLUDED__
