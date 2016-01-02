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

#ifndef __SHAPE_H_INCLUDED__
#define __SHAPE_H_INCLUDED__

#include "general.h"
#include "MyFile.h"
#include "Space3D.h"
#include "PicArr.h"
#include "Mat.h"

#include <list>

class BuildWorld;
struct SqrLimits;
struct BoundedBlock;
class MatStack;
struct BNode;
template<class T> class Space3D;

/** \file
    Declares the Shape class.
*/

/// return result of Shape::generate()
enum EGenResult
{
    GEN_RESULT_OK = 0,   ///< everything went as expected
    GEN_RESULT_NO_START, ///< no yellow start tile was found
    GEN_RESULT_NOT_CONNECT,  ///< shape is made of several unconnected volumes
    GEN_RESULT_ILLEGAL_SIDE, ///< there is a side with more the two faces agaist it
    GEN_RESULT_UNKNOWN = 0xFF  ///< should not occur...
};

/** Shape holds the definition of a structure design which can be built using happy cube pieces.
    The basic foundations of the design are the faces it is made of
    A "face" is a placeholder for a single piece in a specific orientation
    from the faces data, further data about the sides and corners is extracted.
    A shape is usually generated from a BuildWorld which is the direct representation
    of what is designed in the GUI.
    Shape is used in the solution engine in Cube as the blue print over which the solution is build.
    It is also used to aid user interaction in BuildWorld
    \see Cube BuildWorld
*/
class Shape
{
public:
    
    Shape()	:size(-1, -1, -1), fcn(0), sdn(0), cnn(0), sdnError(0),
        rotfirst(false), faces(nullptr), sides(nullptr), corners(nullptr), errorSides(nullptr), volume(-1)
    {}
    
    ~Shape()
    {
        deallocate();
    }

    /// fill the Shape with content generated from given BuildWorld.
    /// \returns A result code according to success or failure of the operation.
    EGenResult generate(const BuildWorld *build); 

    int locateFaceHardWay(EPlane ldr, Vec3i lex) const;	// locate a face, if not existing, return -1;
    bool createTrasformTo(const Shape *news, TTransformVec& movedTo, bool* trivialTransform);
    void deallocate();
    bool saveTo(MyFile *wrfl);
    bool loadFrom(MyFile *rdfl);

    enum EFacing { FACING_OUT = 0, FACING_IN, FACING_UNKNOWN };
    enum EAngleType { ANG_ZERO, ANG_PLUS, ANG_MINUS, ANG_180MINUS, ANG_180PLUS };
    
    // for every neighbor of a face, holds, what's the transformation to get to it from the current face
    struct NeiTransform {
        NeiTransform() {}
        NeiTransform(int px, int py, EAngleType angleType, int rx, int ry, int rz, bool _flip=false, bool _flipDiag=false) 
            : planeMove(px, py), angleType(angleType), rotAxis(rx, ry, rz), flip(_flip), flipDiag(_flipDiag) {}
        Vec2i planeMove; // same plane as the piece
        //int rotAngle;
        EAngleType angleType;
        Vec3i rotAxis;
        bool flip, flipDiag;

    };

    /** FaceDef is the definion of a face, A face is a placeholder for a 5x5 happy cube piece.
        A face is defined by its point of origin #ex and its orientation #dr
        it also links to its corners, sides and neibours
    */
    struct FaceDef
    {
        FaceDef() :dr(PLANE_NONE), facing(FACING_UNKNOWN) {}
        FaceDef(EPlane setdr, Vec3i setex) :dr(setdr), ex(setex), facing(FACING_UNKNOWN) {}

        EPlane dr;		///< directions of the face	
        Vec3i ex;		///< start point of the face

        EFacing facing;
        int index; // in the shape faces array
        TPicBits fmask;

        // corners and sides of this face, reverse neibours
        int corners[4]; 
        int sides[4];
        int nei[4]; // indexes of face neibours of this face (used in generate)
        NeiTransform neiTrans[4];

        FaceDef& operator=(const FaceDef& src)
        {
            if (&src == this) return *this;
            dr = src.dr; ex = src.ex; facing = src.facing;
            return *this;
        }

        // should only be used offline
        Vec3i size() const
        {
            switch (dr)
            {
            case XY_PLANE: return Vec3i(5, 5, 1);
            case XZ_PLANE: return Vec3i(5, 1, 5);
            case YZ_PLANE: return Vec3i(1, 5, 5);
            default: break;
            }
            return Vec3i(-1, -1, -1); // shouldn't happen.
        }
    };

    /** SideDef is the definition of a side, a border between two faces.
        Every two faces connect in a "side" a side is defined by its point
        of origin and the axis along which it goes
    */
    struct SideDef
    {
        SideDef() :dr(AXIS_NONE) { nei[0] = -1; nei[1] = -1; }
        SideDef(EAxis setdr, Vec3i setex, int *setnei)
            :dr(setdr), ex(setex)
        {
            nei[0] = setnei[0];
            nei[1] = setnei[1];
        }
        EAxis dr;		///< direction of the side
        Vec3i ex;		///< start point of the side
        int nei[2];		///< indexes of the faces bordering with this side. may be two or one (empty is -1)
    };

    /**	CornerDef is the definition of a corner, a point meeting place of several faces.
        Every face has 4 corners, every side has two corners. corners are the point
        edges of a structure but can also be in the middle of a flat meeting of 4 faces
        A corder is defined only by the point it occupies
    */
    struct CornerDef
    {
        CornerDef() :numnei(0) { for(int i = 0; i < 6; ++i) nei[i] = -1; }
        CornerDef(Vec3i setex, int *setnei)
            :ex(setex), numnei(0)
        {
            for(int i = 0; i < 6; ++i)
            {
                if ((nei[i] = setnei[i]) != -1) numnei++;
            }
        }		
        Vec3i ex;		///< start point
        int numnei;
        int nei[6];		///< can have any number of 1,2,3,4,5,6 neibours. (empty is -1)
    };

    vector<Vec3> testQuads;

private:

    /// TransType holds the data of a transformation from and index to an index.
    /// it is used in the Shape generation procedure for the purpose of rearranging
    /// the faces of the Shape.
    struct TransType
    {
    public:
        TransType() :to(-1), who(-1) {}
        int to,who;	// to where did it go, who came here
    };

    void readAxis(const BuildWorld *build, int iss, int jss, int pgss, EPlane planedr, list<FaceDef> &flst, int *reqfirst, SqrLimits &bound);
    int checkSide(EAxis ldr, int x, int y, int z, list<SideDef> &slst, list<SideDef> &slstError);
    int checkCorner(int x, int y, int z, list<CornerDef> &clst);

    void copyFace(Shape::FaceDef src, Shape::FaceDef *dest); 

    void makeReverseNei(); // returns true if succesfull
    void makeVolumeAndFacing();
    void makePieceCheckBits();
    void make_sides_facenei() ;
    int locateFace(EPlane ldr, Vec3i lex) const;	// locate a face, if not existing, return -1;

    void faceNei(int whos, int fnei[4]);	// return a face's neibours
    int faceNeiFirst(int whos, TransType trans[]);
    int faceNeiFirstOpt(int whos, TransType trans[]);
    EGenResult reArrangeFacesDFS(FaceDef faces[], FaceDef revis[], TransType trans[]);
    EGenResult reArrangeFacesBFS(FaceDef faces[], FaceDef revis[], TransType trans[]);

    // stuff related to neighbor transformations
    void makeNeiTransforms();
    void checkAddQuad(const Mat4& m);
    template<typename FAdd>
    void checkNeiTranDFS(MatStack& m, int fi, vector<int>& pass, int lvl, float ang, FAdd& addPiece);
    void transformFromFaceToNei(MatStack& m, int fi, int ni, float baseAng) const; // baseAng is in [0,90]
    template<typename FAdd>
    void checkNeiTranBFS(MatStack& m, float ang, const FAdd& addPiece) const;
    template<typename FAdd>
    void runBFSTree(MatStack& m, const BNode& node, float ang, FAdd& addPiece) const;
    void startNeiTransform(MatStack& m) const;

public:
    void makeTransformsMatrics(float angle, vector<Mat4>& mats) const;

    
public:
    Vec3i size;	///< size in basic units

    int fcn;		///< number of faces
    int sdn;		///< number of sides (edges)
    int cnn;		///< number of corners
    int sdnError;  ///< number of Error sides. if there are error sides, "sides" contains the errors and sdn = 0
        
    bool rotfirst;	///< true if first piece should be rotated

    FaceDef *faces;		///< array of faces
    SideDef *sides;		///< array of sides
    CornerDef *corners; ///< array of corners
    SideDef *errorSides; ///< array of error sides.

private:
    int volume; ///< volume == 0 means it's an open shape. volume == -1 means it was not calculated

    // used for optimization of the generate process
    // an array of 3 Space3D, each for every plane
    // contains the numbers of faces present in the build.
    Space3D<int> m_opt_facesLoc[3];


public:
    /// SideFind is the datum of the normSide lookup table used in Shape::checkSide().
    /// this lookup table is used for optimization of Shape::locateFace()
    struct SideFind 
    {
        EPlane pln;
        int x,y,z;
    };

};


#endif // __SHAPE_H_INCLUDED__
