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

#ifndef __MYOBJECT_H__INCLUDED__
#define __MYOBJECT_H__INCLUDED__

#include "MyPolygon.h"
#include "MemoryMgmt.h"
#include <set>
#include <list>
#include <unordered_set>
using namespace std;

/** \file
    Declares the MyObject and LinesCollection classes.
*/

class PicGroupDef;
struct MyAllocator;
class Mesh;

/** MyObject is a mesh of polygons which make out a single piece in the 3D solution engine.
    The polygons and points are actually saved in an Indexed Face Set data structure.
    There is a main MyPoint repository and the polygons reference the points in there.
    MyObject instances are used only for creating the piece display list. after
    the list is created it is saved in the Piece's PicPainter member and the MyObject
    instance is discarded.
    \see PicPainter MyPolygon MyPoint MyLine
*/
class MyObject // should be called MyMesh
{
public:
    MyObject(MyAllocator* alloc = nullptr) 
        :poly(nullptr), lines(nullptr), points(nullptr), nPolys(0), nLines(0), nPoints(0), 
         nakedLinesColor(0.0),  m_alloc(alloc)
    {}

    ~MyObject()
    {
        delete[] poly;
        delete[] lines;
        delete[] points;
    }
    
    MyPolygon** poly; // array sized nPoly of pointers to polygons
    MyLine *lines;   // array sized nLines
    MyPoint** points; // array sized numPoint of pointers to the points

    int nPolys, nLines, nPoints;

    float nakedLinesColor; // color of the lines when the object is on its own (not in grpdef)

    void addPoly(Vec3 *inplst, TexAnchor *ancs = nullptr, Texture *tex = nullptr); //copy needed vertices, add poly
    void addPoly(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, bool flip = false, bool addNormal=false);
    void addLine(Vec3 *inp1, Vec3 *inp2, double inR, double inG, double inB, ELineType type);
    void setNakedLineColor(float color) { nakedLinesColor = color; }
    void vectorify();
    void clacNormals(bool vtxNormals);
    void clacNormalsExceptTouched();

    /// perform the subdivision algorithm over the current mesh, producing a mesh
    /// that has 4 times as many polygons.
    /// \arg smooth should the subdivision be averaged or flat (no averaging)
    void subdivide(bool smooth);

    void toMesh(Mesh& mesh, bool quads);
    
private:
    ///////////////////// structures for startup
    MyPoint* CopyCheckPoint(const Vec3 *p);

    typedef list<MyPolygon*> TPolyList;
    typedef list<MyLine> TLineList;
    typedef list<MyPoint*> TPointsList;
    TPolyList plylst; 
    TLineList lnlst;
    TPointsList pntlst; // points can come either from the list or the map.

    MyAllocator *m_alloc; // used to allocate points, polygons and halfedges

    /// MyPointWrapper is a shallow wrapper for a MyPoint pointer.
    /// it is used for the comparison of points in order to avoid having to write 
    /// an operator=(MyPoint*, MyPoint*) which would be bad.
    struct MyPointWrapper 
    {
        explicit MyPointWrapper(MyPoint *p) :ptr(p) {}
        void detach(MyAllocator *m_alloc) const;
        mutable MyPoint *ptr; 
        // this is a workaround the constness of the set. we know the hash code is not
        // going to change due to the detach
    };

    struct MpHash {
        size_t operator()(const MyObject::MyPointWrapper &pnt) const {
            return pnt.ptr->hash();
        }
    };
    unordered_set<MyPointWrapper, MpHash> m_tmppoints; // used in startup to accumilate points. 

    friend bool operator==(const MyObject::MyPointWrapper &p1, const MyObject::MyPointWrapper &p2);

private:
    // add a point directly to the points repository, without duplicacy check.
    inline void basicAddPoint(MyPoint *pnt);
    // add a polygon made of points in the repository, without duplicacy checks.
    inline void basicAddPoly(MyPoint *inparr[], TexAnchor *ancs = nullptr);

    typedef vector<HalfEdge*> THalfEdgeList;
    bool buildHalfEdges(THalfEdgeList& lst);

};



#endif // __SHAPEIFS_H__INCLUDED__
