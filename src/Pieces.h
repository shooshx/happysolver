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

#ifndef __PIECES_H_INCLUDED__
#define __PIECES_H_INCLUDED__

#include "general.h"
#include "Texture.h"
#include "PicPainter.h"

/** \file
	Declares all classes involved in piece decleration and storage.
	Declares the PicArr, PicDef, PicGroupDef, PicFamily and PicBucket classes.
	Declares the PicType and PicsSet classes.
*/


#define TEX_X 64
#define TEX_Y 64

/** PicArr is the most basic definition of the layout a piece.
	it defines a single 5x5 piece which can be turned, reversed, copied, changed and accessed
*/
class PicArr
{
public:
	PicArr() :turned(false), rtnindx(-1) {} // array uninitialized
 
	void turn();
	void revY();
	void copyTo(PicArr &dest) const;
	bool equalTo(PicArr &dest, bool bSym) const;

	int& set(int x, int y) { return v[x * 5 + y]; }
	int axx(int x, int y) const { return v[(x << 2)+x + y]; }

	int v[25];
	bool turned; // is this rtn turned up-side-down or not. (used in genIFS)
	int rtnindx;	 // also used in getIFS, original rotation index
};

/// Defines the way a piece is rendered and textured
enum EDrawType 
{
	DRAW_COLOR = 0, ///< a single uniform color (Happy Cube)
	DRAW_TEXTURE_NORM, ///< a normal unblended texture. Not used nowadays
	DRAW_TEXTURE_BLEND, ///< a texture blended with a background and foreground color (Marble Cube, Profi Cube)
	DRAW_TEXTURE_INDIVIDUAL_HALF, // half piece uniformly colored and half with an unblended texture (Little Genius)
	DRAW_MAX
};



class PicGroupDef;

 
///	A path is the outline of a piece.
///	maximum theoretical number of path sections is 36 + 1 for last connecting to the first
#define MAX_PATH_LEN 37 

/** PicDef holds a complete definition of a single piece.
	Contains all the information one needs to know about a piece
	instances of this object are held by PicBucket
	\see PicGroupDef PicBucket
*/
class PicDef
{
public:
	PicDef() : mygrp(NULL), pixmap(1, 1), 
		xOffs(-1), yOffs(-1), painter(this), tex(NULL), nUsed(0), lastnSelected(1), nSelected(0), pathlen(0)
	{} // + 1 for the outlined line

	void makeBoundingPath();

	void setSelected(int n) const { nSelected = n; }
	void addSelected(int n) const { nSelected += n; }
	int getSelected() const { return nSelected; }

	PicGroupDef *mygrp; // to which group do I belong
	PicArr v;
	QPixmap pixmap;

	int xOffs, yOffs; // in case of TEXTURE_INDIVIDUAL_***. the x,y offsets of the texture
	PicPainter painter;
	Texture *tex; // Pic specific texture or NULL

	mutable int nUsed;
	mutable int lastnSelected; // used for the repressing of the button. to return to the last value.

private:
	mutable int nSelected;

public:
	/// NAPathCoord is the same as PathPoint but without any constructors.
	/// a no-constructor version of this struct is needed for the static initializer
	struct NAPathCoord // non-aggregate
	{
		int x, y;
	};

	/// PathCoord is a 2D point used in the representation of the bounding path in PicDef.
	struct PathCoord
	{ 
		PathCoord() :x(-1), y(-1) {}
		PathCoord(const NAPathCoord &p) :x(p.x), y(p.y) {}
		PathCoord(int _x, int _y) :x(_x), y(_y) {}
		int x, y;
	};

	/// PathPoint is the datum of the pathway table used to build the bounding path of a PicDef.
	struct PathPoint
	{
		struct { int x, y; } pnt;
		struct 
		{ 
			bool both; // true means: (cell is 1 - put both, cell is 0 - put none). 
					   // false means:(cell is 1 - put 1, cell is 0 - put 2)
			NAPathCoord l1, l2;
		} frm;
		// this is the line that would pass in the next prependicular if current is 1 and next is 0
		struct { int sx, sy, ex, ey; } prp;
	};

	int pathlen;
	PathCoord path[MAX_PATH_LEN]; // bounding path - includes the last point which equals the first
};

/**	The blackness properties of a pieces color.
	controls wether it will be surrounded by white or black lines
	\see PicGroupDef
*/
enum EBlackness 
{ 
	BLACK_NOT = 0, 
	BLACK_BOTH = 1, 
	BLACK_ONE = 2 
};

/**	PicGroupDef is a collection of similar looking pieces.
	A "PicGroup" is a group of pieces that generally has the same look
	currently all groups are of 6 pieces which make a single cube which has a unique
	apperance.
	All the pieces in a PicGroup usually share the same color or texture
	this class holds everything there is to know about this pieces group
	instances of this object are held by PicBucket
	\see PicDef PicBucket
*/
class PicGroupDef 
{
public:
	PicGroupDef() 
	: tex(NULL), baseTex(NULL), drawtype(DRAW_MAX), r(1.0), g(1.0), b(1.0),   
	  exR(0), exG(0), exB(0), sideTex(NULL), sideTexX(0), sideTexY(0), blackness(BLACK_NOT)
	{}

	QImage blendImage(); // produce an image from the texture, and the colors in blend mode

	int numPics() const { return pics.size(); }
	QVector<PicDef> pics;
	Texture *tex;  // the texture used
	Texture *baseTex; // the basic texture, before modulation

	QString name;
	EDrawType drawtype; // could be that according to the type there is a texture but tex is NULL

	float r,g,b;
	float exR, exG, exB;
	Texture *sideTex;
	int sideTexX, sideTexY;

	EBlackness blackness; // means that the color is a dark one. use white lines with this group

	bool isTexExist() { return ((drawtype == DRAW_TEXTURE_NORM) || (drawtype == DRAW_TEXTURE_BLEND) || (drawtype == DRAW_TEXTURE_INDIVIDUAL_HALF)); }
	
};


/**	PicFamily is a collection of the pieces of the same family.
	Pieces groups divide into several families such as the Happy Cube family and the Marble Cube
	family. each family contains 6 cubes.
	this class does not hold the pieces of the groups. it only holds the indexes of
	where it is possible to find them in the PicBucket data structures.
	It does not hold any meaningful role in the engine and is used only for
	GUI purposes
	instances of this object are reside in PicBucket
	\see PicBucket
*/
struct PicFamily
{
	PicFamily() :startIndex(-1), numGroups(-1), onResetSetCount(0), nSetsSelected(0), nSelected(0) {}

	QString name;
	int startIndex; // index of the first cube in the family in the groups array
	int numGroups; // number of groups in this family;
	int onResetSetCount; // number of instances of this family upon reset
	QString iconFilename;

	mutable int nSetsSelected; // number of selected sets of this family. updated for gui
	mutable int nSelected; // number of pieces selected. updated for gui.
};

class DisplayConf;
class GLWidget;

/**	PicBucket is the main repository where all the data about all the pieces resides.
	PicBucket is a singleton object which is created at startup
	according to the piece definition XML
	it contains all the data know about all the pieces the application is aware 
	of, even if some of these pieces are not currently being used.
	\see PicGroupDef PicDef
*/
class PicBucket : public QObject
{
	Q_OBJECT
public:
	/// load the main configuration xml. this is one of the first things that
	/// ever happen in the application
	bool loadXML(const QString& xmlname);
	Texture* newTexture(QImage img, bool emitb = true);

	static void createSingleton();
	static const PicBucket& instance() { return *g_instance; }
	static PicBucket& mutableInstance() { return *g_instance; }

	void makeBitmapList(); 
	void buildMeshes(const DisplayConf& dpc, bool showStop, GLWidget* listContext);

	int selectedCount() const;
	const PicDef& getPic(int gind, int pind) const { return defs[gind].pics[pind]; }

	void setSwapEndians(int v) { nSwapEndians = v; }
	void updateSwapEndian(int v);

	int numDefs() const { return defs.size(); }
	void setToFamResetSel(); ///< reset the selected count to the reset number from the config

	int sumPics; ///< how many cubes, how many pics in total
	QVector<PicGroupDef> defs; ///< group definitions, inside them the piece definitions

	QList<Texture*> texs;

	QVector<PicFamily> families;

public slots:
	void updateTexture(int gind); // some parameter changed in this gind, recalc the texture, send updates

signals:
	void boundTexture(int index, QImage img); // all interested glwidgets should bind this texture to this index.
	void updateTexture(int index, QImage img);

private:
	/// private ctor, this is a singleton
	PicBucket() : sumPics(-1), nSwapEndians(1234) 
	{ }
	/// private unimplemented copy semantics
	PicBucket(const PicBucket&);
	PicBucket& operator=(const PicBucket&);

	static PicBucket *g_instance; // can't be a real instance because it needs to be created after the QApplication
	int nSwapEndians; // the numerical representation of the octet order normal is 1234

};

/**	PicType represents a piece when it is a part of an active piece set.
	when a piece is selected to take part in a construction and the 
	construction engine begins, it is loaded into a PicType instance
	as a PicType the piece has some more information which has to do 
	with piece repetition and rotation
	\see PicSet
*/
class PicType
{	// hold one piece
public:
	
	PicType() :repnum(0), rep(NULL), rtnnum(0), gind(-1), pind(-1), thedef(NULL), thegrp(NULL) {}
	~PicType() { delete[] rep; }

	/// load a piece from the bucket to this instance
	/// \arg bCsym the what-to-do-with-Asymmetric-pieces option
	void load(int gind, int pind, bool bCsym);

	int repnum;		///< repetition number
	int *rep;		///< repetitioning pics in the pic array TBD-move to QVerctor
	
	int rtnnum;
	PicArr rtns[8];

	int gind, pind; ///< used solely for interaction with the solutions in SlvCube::SlvCube
	const PicDef *thedef; ///< who is it in the PicBucket
	const PicGroupDef *thegrp; ///< who is it in the PicBucket
};

class SlvCube;

/** PicsSet holds all the pieces of an active solving session.
	When the used hits "Solve It" a PicsSet is constructed with the currently selected
	pieces and the solution engine is ran using it.
*/
class PicsSet
{	
public:
	/// this ctor takes the selection from the Solution
	PicsSet(const SlvCube *scube); 

	///	this ctor takes the selection from the bucket
	///	with bSym == true, there are more rtns for every pic.
	PicsSet(bool bSym); 	
						
	int size() const { return pics.size(); }

	/// create the rotated version of all pieces in the set
	void makereps();

	bool bConsiderSymetric;
	QVector<PicType> pics;
};


#endif // __PIECES_H_INCLUDED__
