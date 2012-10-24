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
#include "PicArr.h"
#include "OpenGL/GLTexture.h"

/** \file
	Declares all classes involved in piece decleration and storage.
	Declares the PicArr, PicDef, PicGroupDef, PicFamily and PicBucket classes.
	Declares the PicType and PicsSet classes.
*/




/// Defines the way a piece is rendered and textured
enum EDrawType 
{
	DRAW_UKNOWN = -1,
	DRAW_COLOR = 0, ///< a single uniform color (Happy Cube)
	DRAW_TEXTURE_NORM = 1, ///< a normal unblended texture. Not used nowadays
	DRAW_TEXTURE_BLEND = 2, ///< a texture blended with a background and foreground color (Marble Cube, Profi Cube)
	DRAW_TEXTURE_INDIVIDUAL_HALF = 0x14, ///< half piece uniformly colored and half with an unblended texture (Little Genius)
	DRAW_TEXTURE_INDIVIDUAL_WHOLE = 0x18,
	DRAW_FLAT = 0x100
};

inline bool isIndividual(EDrawType dt) {
	return (dt & 0x10) != 0;
}



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
	PicDef() : mygrpi(-1), indexInGroup(-1), pixmap(1, 1),
		xOffs(-1), yOffs(-1), painter(NULL), tex(NULL), nUsed(0), lastnSelected(1), nSelected(0), pathlen(0), dispRot(-1)
	{} // + 1 for the outlined line
	void reset() {
		painter = PicPainter(this);
	}

	void makeBoundingPath();

	void setSelected(int n) const { nSelected = n; }
	void addSelected(int n) const { nSelected += n; }
	int getSelected() const { return nSelected; }

	// to which group do I belong
	const PicGroupDef *mygrp() const; 
	int mygrpi; // index of the group this piece is part of
	int indexInGroup;  // index of this piece in the group it is part of
	PicArr v;
	QPixmap pixmap;

	int xOffs, yOffs; // in case of TEXTURE_INDIVIDUAL_***. the x,y offsets of the texture
	PicPainter painter;
	Texture *tex; // Pic specific texture or NULL

	mutable int nUsed;
	mutable int lastnSelected; // used for the repressing of the button. to return to the last value.

	shared_ptr<PicDisp> disp;
	int dispRot;

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

private:
	//DISALLOW_COPY(PicDef);

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
	: tex(NULL), baseTex(NULL), drawtype(DRAW_UKNOWN), r(1.0), g(1.0), b(1.0),   
	  exR(0), exG(0), exB(0), sideTex(NULL), sideTexX(0), sideTexY(0), blackness(BLACK_NOT), gtex(NULL)
	{}

	QImage blendImage(); // produce an image from the texture, and the colors in blend mode

	PicDef& getPic(int myi);
	const PicDef& getPic(int myi) const;

	int numPics() const { return picsi.size(); }

	vector<int> picsi; // indices of this group pics in the bucket
	GlTexture *gtex;

	Texture *tex;  // the texture used
	Texture *baseTex; // the basic texture, before modulation

	string name;
	EDrawType drawtype; // could be that according to the type there is a texture but tex is NULL

	float r,g,b;
	float exR, exG, exB;
	Texture *sideTex;
	int sideTexX, sideTexY;

	EBlackness blackness; // means that the color is a dark one. use white lines with this group

	bool isTexExist() const { return ((drawtype == DRAW_TEXTURE_NORM) || (drawtype == DRAW_TEXTURE_BLEND) || isIndividual(drawtype)); }
	
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

	string name;
	int startIndex; // index of the first cube (picgroup) in the family in the groups array
	int numGroups; // number of groups in this family;
	int onResetSetCount; // number of instances of this family upon reset
	string iconFilename;

	mutable int nSetsSelected; // number of selected sets of this family. updated for gui
	mutable int nSelected; // number of pieces selected. updated for gui.
};

class DisplayConf;
class GLWidget;


int rotationAdd(int base, int defRot);
int rotationSub(int x, int defRot);

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
	bool loadXML(const string& xmlname);
	Texture* newTexture(QImage img, bool emitb = true);

	static void createSingleton();
	static const PicBucket& instance() { return *g_instance; }
	static PicBucket& mutableInstance() { return *g_instance; }

	void makeBitmapList(); 
	void buildMeshes(const DisplayConf& dpc, bool showStop, GLWidget* listContext);

	int selectedCount() const;

	const PicDef& getPic(int gind, int pind) const { return pdefs[getPicInd(gind, pind)]; }
	int getPicInd(int gind, int pind) const { return grps[gind].picsi[pind]; }
	void getGP(int picInd, int* gind, int* pind) const {
		*gind = pdefs[picInd].mygrpi;
		*pind = pdefs[picInd].indexInGroup;
	}

	void setSwapEndians(int v) { nSwapEndians = v; }
	void updateSwapEndian(int v);

	//int numDefs() const { return defs.size(); }
	void setToFamResetSel(); ///< reset the selected count to the reset number from the config
	void distinctMeshes();

public:
	int sumPics; ///< how many cubes, how many pics in total
	vector<PicGroupDef> grps; ///< (defs) group definitions, inside them the piece definitions
	vector<PicDef> pdefs;

	QList<Texture*> texs;
	vector<GlTexture*> gtexs;

	vector<PicFamily> families;

	vector<shared_ptr<PicDisp>> meshes;

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
	Q_DISABLE_COPY(PicBucket);

	static PicBucket *g_instance; // can't be a real instance because it needs to be created after the QApplication
	int nSwapEndians; // the numerical representation of the octet order normal is 1234

};


#endif // __PIECES_H_INCLUDED__
