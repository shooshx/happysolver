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

#ifndef __BUILDDIMENSION_H__INCLUDED__
#define __BUILDDIMENSION_H__INCLUDED__

#include "general.h" //Coord3d
#include "Space3D.h"
#include "Shape.h"

/** @file
    Declares the BuildWorld, BuildDimension, SqrLimits and CoordBuild classes.
    BuildWorld is used by CubeDoc to hold the state of the design editor
    BuildWorld uses BuildDimension internally to hold the data and SqrLimits to
    hold the boundaries of the current structure.
    this file also defines various \#define constacts such as FACE_NORM and FACE_STRT
    that are used as the tile values in the array of BuildDimension and various macros
    such as GET_TYPE() and GET_VAL() that are used to make sense of these values.
*/



#define FACE_NORM		0x00000201  ///< normal white
#define FACE_STRT		0x00000202  ///< starting, yellow
#define FACE_NORM_SELR  0x00020201  ///< show for selected to be removed
#define FACE_STRT_SELR  0x00020202  

#define FACE_TRANS		0x00000101  ///< blue ones, possible to put there a tile
#define FACE_TRANS_NONE 0x00010101  ///< should be blue, but not shown.

#define FACE_DONT_TRANS	0x00000102  ///< impossible to put there

#define FACE_TRANS_SEL  0x00000103  ///< shown where its possible to put a new cube

#define TYPE_REAL		0x00000200  ///< real tiles
#define TYPE_VIR		0x00000100  ///< virtual tiles

#define SHOW_REOMOVE    0x00020000  ///< can happen only to REAL tiles
#define SHOW_DONT		0x00010000  ///< can happen only to TRANS tiles
#define SHOW_DO			0x00000000

#define GET_TYPE(face) ((face) & 0x0000FF00) 
#define GET_VAL(face)  ((face) & 0x0000FFFF)
#define GET_SHOW(face) ((face) & 0x00FF0000)
#define GET_VAL_SHOW(face) ((face) & 0x00FFFFFF)

// last 8 bits are for intensity. intensity is for fading tiles in. range - 1 to 8.
#define GET_INTENSITY(face) (((face) >> 24) & 0xFF)
#define SET_INTENSITY(face, in) ((face & 0x00FFFFFF) | ((in) << 24))

#define BUILD_START_CUBE Vec3i(24, 24, 24)

class MyFile;

/** BuildDimension contains the raw data of a single dimention of tiles in BuildWorld.
    A single dimention is either the X, Y or Z axis. This class holds the tiles
    data for all the tiles which are prependicular to a specific axis.
    These tiles come in BUILD_SIZE parallel pages, each page is a matrix of 
    BUILD_SIZE x BUILD_SIZE tiles.
    \see BuildWorld
*/
class BuildDimension
{
public:
    /// BuildPage is a single page in the dimention.
    class BuildPage
    {
    public:
        BuildPage();
        int fc[BUILD_SIZE][BUILD_SIZE];
    };
    BuildDimension() {};
    BuildPage pages[BUILD_SIZE];
};





/** BuildWorld holds the raw data from the design editor. The data is represented in
    its rawest and most direct form. Every polygon the user sees in the design view
    Translateds to datum in this class.
    From this data the Shape is later generated for the solution engine to work on.
    The data is organized in three BuildDimension structures, each for every axis.
    every such BuildDimension contains the tiles which are prependicular to that axis.
    an instance of this class is contained by CubeDoc as the current state of the design
    editor.
    \see BuildDimension CubeDoc
*/
class BuildWorld
{
public:

    /// WorldLimits contains the actual limits of BuildWorld in its 3 dimentions.
    /// it contains three instances of SqrLimits, one for every dimention.
    struct WorldLimits
    {
        SqrLimits d[3];
        SqrLimits& operator[](int i) { return d[i]; }
        const SqrLimits& operator[](int i) const { return d[i]; }
        void Inverse(int size = BUILD_SIZE) { for(int i = 0; i < 3; ++i) d[i].Inverse(size); }
        void Init(int size = BUILD_SIZE) { for(int i = 0; i < 3; ++i) d[i].Init(size); }
    };


    BuildWorld() 
        :size(BUILD_SIZE, BUILD_SIZE, BUILD_SIZE),
         m_space(BUILD_SIZE, BUILD_SIZE, BUILD_SIZE), 
         nFaces(0), fClosed(false), fChangedFromGen(true), fChangedFromSave(false),
         m_testResult(GEN_RESULT_UNKNOWN), m_bTested(false)
    {
        initializeNew(true);
    }

    void setBox(const Vec3i s);

    bool set(int dim, int page, int x, int y, int set);
    bool set(const CoordBuild& c, int set);
    int get(int dim, int page, int x, int y) const
    {
        if ((x >= 0) && (x < BUILD_SIZE) && (y >= 0) && (y < BUILD_SIZE) && (page >= 0) && (page < BUILD_SIZE) && (dim >= 0) && (dim < 3))
            return dm[dim].pages[page].fc[x][y];
        return 0;
    }

    int get(const CoordBuild &c) const
    {
        if ((c.x >= 0) && (c.x < BUILD_SIZE) && (c.y >= 0) && (c.y < BUILD_SIZE) && (c.page >= 0) && (c.page < BUILD_SIZE) && (c.dim >= 0) && (c.dim < 3))
            return dm[c.dim].pages[c.page].fc[c.x][c.y];
        return 0;
    }

    int tilesCount() const { return nFaces; }

    void initializeNew(bool boxed);
    void doTransparent();
    void reClacLimits();

    bool loadFrom(MyFile *rdfl);
    bool saveTo(MyFile *wrfl);

    bool search(int face, int changeTo = -1, bool onlyShow = false, bool doret = true);

    void justChanged() { fChangedFromGen = true; fChangedFromSave = true; m_bTested = false; }
    void justGen() { fChangedFromGen = false; }
    void justSave() { fChangedFromSave = false; }
    bool getChangedFromGen() const { return fChangedFromGen; }
    bool getChangedFromSave() const { return fChangedFromSave; }


    EGenResult testShape();
    EGenResult getTestResult() { return (m_bTested?m_testResult:GEN_RESULT_UNKNOWN); }
    const Shape& getTestShape() { return m_testShape; } // needed for inspection of the sides

    void setUnTest() { m_bTested = false; }

    static void get3dCoords(CoordBuild s, Vec3i &g1, Vec3i &g2);
    static void getBuildCoords(Vec3i g, CoordBuild b[6]);
    int getTestShapeFcInd(CoordBuild s) const;

    void unGenerate(const Shape *shp);

    enum ECleanMethod 
    {
        CLEAN_CLEAR, ///< wipe it all
        CLEAN_TRANS, ///< clean only the transperant tiles, maitain show status
        CLEAN_TRANS_SHOW ///< clean transperant and show status
    };

    void clean(ECleanMethod meth); 
    void getAllNei(const CoordBuild& in, CoordBuild out[12]); // need to be a pointer to an array of 12 places

public:
    Vec3i size; //in faces, so far, should be x=y=z, so far should be constant == BUILD_SIZE;
    BoundedBlockSpace3D m_space;
    int nFaces;
    bool fClosed;

    WorldLimits m_limits;

    /// the bounds of the last generate called with this BuildWorld
    /// page plays the z axis
    mutable SqrLimits m_gen_bounds; 

private:

    /// create m_space
    void bootstrapSpace();
    void justInvalidatedTest() { m_bTested = false; }

    BuildDimension dm[3];

    bool fChangedFromGen;
    bool fChangedFromSave;

    Shape m_testShape; ///< this shape is used to test if the build is legal.
    EGenResult m_testResult; ///< valid only when m_bTested == true
    bool m_bTested;
    
public:
    /// TransSqr is the datum of the transp internal lookup table used doTransparent()
    struct TransSqr
    {
        int plotDim;
        int prm[3][2];
    };
                 

};

#endif // __BUILDDIMENSION_H__INCLUDED__
