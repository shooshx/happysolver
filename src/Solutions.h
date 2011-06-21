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
#include <QList>
#include <QObject>

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


/** SlvPainter paints an entire solution scene on the given GLWidget.
	It is responsible for arranging the various pieces objects in their
	right position and orientation according to a specific SlvCube which lives
	in the context of a specific Shape.
	An instance of SlvPainter is contained in every SlvCube created. Upon demand
	it would paint the pieces of that specific SlvCube.
	Notice, there is some preperation that should be done before the painter
	can do its job. this preperation is performed in SlvCube::genPainter()
	\see SlvCube
*/
class SlvPainter
{
public:
	// contains a pointer to its parent
	SlvPainter(const SlvCube* _scube) :scube(_scube) {}
	void paint(GLWidget* context, bool fTargets, int singleChoise, int upToStep, ELinesDraw cfgLines) const;

	bool exportToObj(QTextStream& meshFile, QTextStream& materialsFiles) const;

	bool isNull() const { return scube == NULL; }
	void setSlvCube(const SlvCube *sc) { scube = sc; }

	Coord3df qmin, qmax; // 2 opposites for bounding box

	LinesCollection m_linesIFS;
private:
	void paintPiece(int f, GLWidget* context, bool fTargets) const;
	void paintLines(const MyObject& obj, bool singleChoise, GLWidget *context, ELinesDraw cfgLines) const;

	bool exportPieceToObj(QTextStream& meshFile, QTextStream& materialsFiles, int i, uint& numVerts,
						  uint &numTexVerts, uint &numNormals, uint &numObjs) const;

	const SlvCube* scube;
};


/** SlvCube represents a single solution of a design. SlvCube does not contain any information
	about the structure of the Shape it based on and hence depends completly on living in the
	context of the Shape it was created upon. Essentially SlvCube is just a series of pieces
	layed out in the order of the faces of the Shape.
	An SlvCube \b Does contain complete information of the pieces it is made of an can live
	independantly from the PicsSet which lead to its creation.
*/
class SlvCube
{
public:
	/// SlvPiece holds the date of a single tile in a solution.
	/// An array of objects of this class, ordered by the order of the Shape tiles 
	/// make up a single solution to the shape.
	struct SlvPiece
	{
		// loaded data
		int sc; ///< the piece, from the current picdt
		int rt; ///< the rotation, from the current picdt

		// inferred data, using PicsSet
		const PicDef *sdef;
		int rtindx;
	};

	/// SlvPic holds the data of a single piece that was selected when the solution was generated.
	/// the array picdt is essentially a snapshot of the PicsSet taken when the solution was
	/// generated. It is important as a reference point to the SlvPiece data.
	struct SlvPic
	{
		int gind; // what group
		int pind; // which one in the group
	};

	/// called when the solution engine find a solution
	SlvCube(ShapePlace *plc, const PicsSet *picsc, const Shape *shape);
	/// called when opening a file
	SlvCube(const Shape *shape); 
	~SlvCube();
	
	/// prepare the painter to do its job
	void genPainter();

	bool saveTo(MyFile *wrfl, int index);
	bool readFrom(MyFile *rdfl, int _slvsz);
	void transform(const TTransformVec &moveTo);
	
	int numPieces() const { return slvsz; }
	const PicGroupDef *getPieceGrpDef(int n) const { return dt[n].sdef->mygrp; }

	int slvsz; // (size of data) keep it close. will always be equal to that of the Solutions
	SlvPiece *dt;

	int picssz;
	SlvPic *picdt; // used to construct PicsSet
	bool bConsidersSym; // the solve needs to know if it was symmetric or not to know how to build the pics. this is bad. TBD

	SlvPainter painter;
	const Shape* shape; ///< the shape this solution is based on. the shape is held by CubeDoc

private:
	void makeDt(ShapePlace *plc);

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
class Solutions : public QObject
{
	Q_OBJECT
public:
	Solutions(int solveSize = -1) :slvsz(solveSize), m_bChangedFromSave(false) {}
	~Solutions() {	clear(); }
	void clear(int solveSize = -1);

	bool readFrom(MyFile *rdfl, const Shape *withShape);
	bool saveTo(MyFile *wrfl, SlvCube *slv); // slv == NULL -> save all

	void setChangedFromSave();
	void resetChangedFromSave();
	bool getChangedFromSaved() { return m_bChangedFromSave; }

	void transform(const TTransformVec &moveTo);

	
	int size() const { return sv.size(); }
	SlvCube* at(int i) { return sv[i]; }
	const SlvCube* at(int i) const { return sv[i]; }
	
	void addBackCommon(SlvCube *tmp);

	int slvsz; // size of a solution

	typedef QList<SlvCube*> TSlvList;

signals:
	void changedFromSave(bool state);

private:

	TSlvList sv;
	bool m_bChangedFromSave;

};

#endif // __SOLUTIONS_H_INCLUDED__
