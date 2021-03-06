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
#include "PicPainter.h"
#include "PicArr.h"
#include "ImgBuf.h"
#include "PicsSet.h"

#ifdef QT_CORE_LIB
#include <QPixmap>
#endif

#include <memory>
using namespace std;

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
    DRAW_TEXTURE_IMAGE = 0x14, ///< half piece uniformly colored and half with an unblended texture (Little Genius)
    //DRAW_TEXTURE_INDIVIDUAL_WHOLE = 0x18,
    DRAW_TEXTURE_MARBLE = 4,
    DRAW_FLAT = 0x100
};



class PicGroupDef;
class GlTexture;
 
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
    PicDef()
#ifdef QT_CORE_LIB
        :pixmap(1, 1)
#endif
    {
    } 


    void makeBoundingPath();

    void setSelected(int n) const { 
        nSelected = n; 
    }
    void addSelected(int n) const {
        nSelected += n; 
    }
    int getSelected() const { 
        return nSelected; 
    }

    // to which group do I belong
    const PicGroupDef *mygrp() const; 


    int mygrpi = -1; // index of the group this piece is part of
    int indexInGroup = -1;  // index of this piece in the group it is part of, for saving in a solution
    PicArr v;
    PicArr defRtns[8];

#ifdef QT_CORE_LIB
    QPixmap pixmap;
#endif

    ImgBuf *tex = nullptr; // Pic specific texture or nullptr

    int xOffs = 0, yOffs = 0; // in case of TEXTURE_INDIVIDUAL_***. the x,y offsets of the texture
    float texX = 0, texY = 0, texScaleX = 1.0f, texScaleY = 1.0f; // in texture coordinates, range [0,1] for opengl

    mutable int nUsed = 0;
    mutable int lastnSelected = 1; // used for the repressing of the button. to return to the last value.

    PicDisp* disp = nullptr;
    int dispRot = -1;  // rotation of this def compared to the mesh pointed in disp

    int indInAllComp = -1; // index of unified shape in the Bucket's allComp
    int defRot = -1; // rotation of this def to the def in allComp  (same as dispRot for now)

private:
    mutable int nSelected = 0;

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

    int pathlen = 0;
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
    BLACK_NOT = 0, // border of this piece should not be white line
    BLACK_BOTH = 1, // border of this piece should be white only if both pieces on the line are black
    BLACK_ONE = 2  // border of this piece should be white even if only one of the pieces is black
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
    : drawtype(DRAW_UKNOWN), color(1.0f, 1.0f, 1.0f),   
      exColor(0.0f, 0.0f, 0.0f), blackness(BLACK_NOT), gtex(nullptr)
    {}

    ImgBuf* blendImage(ImgBuf* baseTex); // produce an image from the texture, and the colors in blend mode

    PicDef& getPic(int myi); // myi-[0,5] index of the piece we want
    const PicDef& getPic(int myi) const;

    int numPics() const { return picsi.size(); }

    bool isIndividual() const {
        return (drawtype & 0x10) != 0;
    }

    vector<int> picsi; // indices of this 6 group pics in the bucket
    shared_ptr<GlTexture> gtex; // might be shared with other pieces
 
    ImgBuf* tex;  // the texture used

    string name;
    EDrawType drawtype; // could be that according to the type there is a texture but tex is nullptr

    Vec3 color;
    Vec3 exColor;

    EBlackness blackness; // means that the color is a dark one. use white lines with this group
    bool twoColor = false; // relevant only in drawtype texture

    // data from the editor from which other data of the cube is derived from
    struct EditorData {
        string piecesFrame; // pieces array derive from this - 9 byte string
        Vec2i imageOffset;   // texture coordinates derive from this, in pixels
        float imageZoom = 1.0;
        string backHex, frontHex, blackSelect;
        int rotate = 0; // 0,1,2,3 * 90 degrees  -rotate the image - only for texture drawType
        string url; // url for the image download in case of texture drawType, otherwise, empty string
        bool fromEditor = true; // false if it's a preset piece 
    };

    EditorData editorData;

// 	bool isTexExist() const { 
// 		return ((drawtype == DRAW_TEXTURE_NORM) || (drawtype == DRAW_TEXTURE_BLEND) || (drawtype == DRAW_TEXTURE_MARBLE) || isIndividual(drawtype)); 
// 	}
    
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
    string name;
    int startIndex = -1; // index of the first cube (picgroup) in the family in the groups array
    int numGroups = -1; // number of groups in this family;
    int onResetSetCount = 0; // number of instances of this family upon reset
    string iconFilename;
    string ctrlId; // for html

    mutable int nSetsSelected = 0; // number of selected sets of this family. updated for gui
    mutable int nSelected = 0; // number of pieces selected. updated for gui.
};

class DisplayConf;



int rotationAdd(int base, int defRot);
int rotationSub(int x, int defRot);

class ProgressCallback {
public:
    virtual void init(int maxv) = 0;
    // return false if need to cancel
    virtual bool setValue(int v) = 0;

};

/**	PicBucket is the main repository where all the data about all the pieces resides.
    PicBucket is a singleton object which is created at startup
    according to the piece definition XML
    it contains all the data know about all the pieces the application is aware 
    of, even if some of these pieces are not currently being used.
    \see PicGroupDef PicDef
*/
class PicBucket 
{
public:
    /// load the main configuration xml. this is one of the first things that
    /// ever happen in the application
    bool loadXML(const char* xmlname);
    ImgBuf* newTexture(ImgBuf* img, bool in3d);

    static PicBucket& createSingleton();
    static const PicBucket& instance() { return *g_instance; }
    static PicBucket& mutableInstance() { return *g_instance; }

    void makeBitmapList(); 
    void buildMeshes(const DisplayConf& dpc, ProgressCallback* prog);
    bool loadMeshes(const string& filename);
    bool loadUnified(const char* filename);
    bool loadUnifiedJs();


    int selectedCount() const;

    const PicDef& getPic(int gind, int pind) const { 
        return pdefs[getPicInd(gind, pind)]; 
    }
    int getPicInd(int gind, int pind) const { 
        return grps[gind].picsi[pind]; 
    }
    void getGP(int picInd, int* gind, int* pind) const {
        *gind = pdefs[picInd].mygrpi;
        *pind = pdefs[picInd].indexInGroup;
    }

    void setSwapEndians(int v) { nSwapEndians = v; }
    void updateSwapEndian(int v);

    //int numDefs() const { return defs.size(); }
    void setToFamResetSel(); ///< reset the selected count to the reset number from the config
    void distinctMeshes(ProgressCallback* progress = nullptr);

    void makeAllComp();

    static void buildAllMeshes();

    // returns the grpi (userful if given -1)
    int updateGrp(int grpi, PicArr arrs[6], bool reCompress = true);
    void doReCompress();

public:
    int sumPics; ///< how many cubes, how many pics in total
    vector<PicGroupDef> grps; ///< (defs) group definitions, inside them the piece definitions
    vector<PicDef> pdefs;

    vector<ImgBuf*> texs;
    vector<shared_ptr<GlTexture>> gtexs; // textures loaded with the inital xml

    vector<PicFamily> families;

    map<TPicBits, std::unique_ptr<PicDisp> > m_meshes; // same size as allComp

    vector<PicType> allComp; // (compressed) distict pieces from pdefs with references to pdefs in addedInds

private:
    /// private ctor, this is a singleton
    PicBucket() : sumPics(-1), nSwapEndians(1234) 
    { }
    /// private unimplemented copy semantics
    DISALLOW_COPY(PicBucket);

    static PicBucket *g_instance; // can't be a real instance because it needs to be created after the QApplication
    int nSwapEndians; // the numerical representation of the octet order normal is 1234

};


#endif // __PIECES_H_INCLUDED__
