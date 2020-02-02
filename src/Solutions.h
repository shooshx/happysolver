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

#ifndef __SOLUTIONS_H_INCLUDED__
#define __SOLUTIONS_H_INCLUDED__

#include "general.h"
#include "CubeAcc.h" ///< for ShapePlace
#include "MyFile.h"
#include "Pieces.h"
#include "Configuration.h"


/** \file
	Declares all classes involved with solution storage and display.
	Declares the SlvCube and Solutions which make up the solution storage
	and the SlvPainter which displays a solution.
*/

class Shape;
class PicsSet;
class PicBucket;

class PicDef;
class PicGroupDef;
class GLWidget;
class SlvCube;


class IChangeNotify {
public:
	virtual void notify(bool b) = 0;
};


class CubeDoc;

/** Solutions holds the collection of the current solutions found.
	CubeDoc holds a single instance of Solutions at all times which makes
	the current set of solutions present in the system. The user can view these
	solutions using the solutons view - ModelGlView.
	Each solution is contained in an SlvCube instance. Like SlvCube, Solutions relies
	on living in the context of the Shape the solutions belog to. This shape is also
	contained in CubeDoc.
	\see CubeDoc SlvCube
*/
class Solutions 
{

public:
	Solutions(int solveSize = -1) :slvsz(solveSize), m_bChangedFromSave(false), changedFromSave(nullptr) {}
	~Solutions() {	clear(); }
	void clear(int solveSize = -1);

	bool readFrom(MyFile *rdfl, const Shape *withShape);
	bool saveTo(MyFile *wrfl, SlvCube *slv); // slv == nullptr -> save all

	void setChangedFromSave();
	void resetChangedFromSave();
	bool getChangedFromSaved() { return m_bChangedFromSave; }

	void transform(const TTransformVec &moveTo);
    void toNewShape(const Shape* newshp);

	
	int size() const { return sv.size(); }
	SlvCube* at(int i) { return sv[i]; }
	const SlvCube* at(int i) const { return sv[i]; }
    SlvCube* release(int i) {
        auto s = sv[i];
        sv[i] = nullptr;
        return s;
    }
	
	void addBackCommon(SlvCube *tmp, bool keepOnlyOne = false);


	int slvsz; // size of a solution
	
    void setChangedNotifier(IChangeNotify* n) {
        changedFromSave = n;
    }
	
	bool filterDup(SlvCube* slv);

private:

    IChangeNotify *changedFromSave;

	vector<SlvCube*> sv;
	bool m_bChangedFromSave;

};

#endif // __SOLUTIONS_H_INCLUDED__
