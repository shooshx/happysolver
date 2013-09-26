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

#ifndef __PICPAINTER_H_INCLUDED__
#define __PICPAINTER_H_INCLUDED__

#include "MyObject.h"
#include "MyFile.h"
#include "Configuration.h"
#include "Mat.h"
#include "Mesh.h"
#include "PicArr.h"

/** \file
    Declares the PicPainter class used for configuration editing.
*/

class MyObject;
class PicDef;
class BaseGLWidget;
class DisplayConf;
class ObjExport;


// represents a piece shape which may be in multiple piece definitions
class PicDisp
{
public:
    /// create the polygon mesh and the display list.
    void init(const DisplayConf &dpc);

    static MyAllocator& getAllocator() { return g_smoothAllocator; }

    void generateStraightShape(const DisplayConf& dpc, MyObject& obj) const;
    void placeSidePolygon(MyObject& obj, int b, bool is1, int x, int y) const;

    bool uncub(int x, int y) const;

    enum EPlaceType { PLACE_NORM, PLACE_X, PLACE_Y };
    void PlaceInto(int pntn, Vec3 *shpp, Vec3 *pnti1, Vec3 *pnti2, EPlaceType type) const;

    TPicBits bits() { return m_arr.getBits(); } 
    /// BuildFrame is the datum of the internal lookup table used while creating
    /// the piece initial polygon mesh.
    struct BuildFrame
    {
        struct { int x, y; } pnt;
        struct { int x, y; } ln[2], bot[2];
        bool corner; // if true then (if 1, take both, if 0 take none)
        struct {
            unsigned int in1, in0;
        } pold; // up and down
        struct {
            unsigned int next0, prev0;
        } sds; // left and right

    };
    static const BuildFrame build[17];

public:
    Mesh m_mesh;
    PicArr m_arr;

private:
    static MyAllocator g_smoothAllocator;
};



/** PicPainter creates the display list of the polygon mesh of a single piece in 
    the 3D soultion view. It is the heart of the 3D engine and where most of the 
    3D drawing logic is performed.
    An instance of this class is contained in the PicDef instance of the piece
    it is responsible for. It is called into creating the display list from 
    PicBucket::buildMeshes() and into actual drawing by SlvPainter::paint()
    \see SlvPainter MyAllocator MyObject
*/
class PicPainter
{
public:
    PicPainter(PicDef* _pdef) :m_pdef(_pdef) {}
    
    /// do the actual painting
    /// \arg \c bTargets draw for targets, no colors.
    /// \arg invertTex true if in individual textures, we need to invert the X tex coordinates so that the texture 
    ///      be on the correct side
    void paint(bool bTargets, const Vec3& name, BaseGLWidget *context, bool invertTex) const; 

    bool exportToObj(ObjExport& oe, const Mat4& fMatrix) const;

private:
    bool realExportToObj(ObjExport& oe, const Mat4& fMatrix) const;

private:
    PicDef *m_pdef;
    DisplayConf m_displayConf;

};


#endif
