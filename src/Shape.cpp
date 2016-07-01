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


#include "Shape.h"
#include "general.h"
#include "Space3D.h"
#include "BuildWorld.h"
#include "MatStack.h"

#include <stdio.h>
#include <cmath>
#include <iostream>
#include <iomanip>
using namespace std;


#define HEADER_SHAPE "Shape Definition"
#define VAL_SHAPE_FACENUM "FaceNum"
#define VAL_SHAPE_SIDENUM "SideNum"
#define VAL_SHAPE_CORNERNUM "CornerNum"
#define VAL_SHAPE_SIZE "Size"
#define VAL_SHAPE_COMPLEX_SIZE "ComplexSize"
#define VAL_SHAPE_ROTF "Rotate First"
#define VAL_SHAPE_FACES "Faces"
#define VAL_SHAPE_SIDES "Sides"
#define VAL_SHAPE_CORNERS "Corners"



bool Shape::loadFrom(MyFile *rdfl)
{
    int i, j, rttmp, nne;

    deallocate();

    if (rdfl->getState() != STATE_OPEN_READ)
        return false;


    if (!rdfl->seekHeader(HEADER_SHAPE)) return false;

    if (!rdfl->seekValue(VAL_SHAPE_FACENUM)) return false;
    rdfl->readNums(1, &fcn);
    if (!rdfl->seekValue(VAL_SHAPE_SIDENUM)) return false;
    rdfl->readNums(1, &sdn);
    if (!rdfl->seekValue(VAL_SHAPE_CORNERNUM)) return false;
    rdfl->readNums(1, &cnn);

    if (!rdfl->seekValue(VAL_SHAPE_SIZE)) return false;
    rdfl->readNums(3, &size.x, &size.y, &size.z);		


    if (rdfl->seekValue(VAL_SHAPE_ROTF))
    { // optional
        rdfl->readNums(1, &rttmp);
        rotfirst = rttmp?true:false;
    }
    else
    {
        rotfirst = false;
    }

    if (!rdfl->seekValue(VAL_SHAPE_FACES)) return false;
    faces = new FaceDef[fcn];
    
    for(i = 0; i < fcn; ++i)
    {
        if (rdfl->readNums(4, &faces[i].dr, &faces[i].ex.x, &faces[i].ex.y, &faces[i].ex.z) < 4)
        {
            deallocate();
            return false;
        }
    }

    if (!rdfl->seekValue(VAL_SHAPE_SIDES)) 
    {
        deallocate();
        return false;
    }
    sides.resize(sdn);

    for(i = 0; i < sdn; ++i)
    {
        if (rdfl->readNums(6 ,&sides[i].dr, &sides[i].ex.x, &sides[i].ex.y, &sides[i].ex.z, 
                                            &sides[i].nei[0], &sides[i].nei[1]) < 6)
        {
            deallocate();
            return false;
        }

    }

    if (!rdfl->seekValue(VAL_SHAPE_CORNERS)) 
    {
        deallocate();
        return false;
    }
    corners.resize(cnn);
    for(i = 0; i < cnn; ++i)
    {
        nne = 0;
        if (rdfl->readNums(3, &corners[i].ex.x, &corners[i].ex.y, &corners[i].ex.z) < 3)
        {
            deallocate();
            return false;
        }
        for(j = 0; j < 6 ; ++j)
        {
            if (rdfl->readNums(1, &corners[i].nei[j]) < 1)
            {
                deallocate();
                return false;
            }
            if (corners[i].nei[j] != -1) nne++;
        }
        corners[i].numnei = nne;
    }

    makeReverseNei();
    makeVolumeAndFacing();

    return true;
}



void Shape::deallocate()
{
    if (faces != nullptr) delete[] faces;
    faces = nullptr;
    sides.clear();
    corners.clear();
    errorSides.clear();
    fcn = 0; sdn = 0; cnn = 0;

}


bool Shape::saveTo(MyFile *wrfl)
{
    int i, j;

    if (wrfl->getState() != STATE_OPEN_WRITE) 
        return false;

    wrfl->writeHeader(HEADER_SHAPE);

    wrfl->writeValue(VAL_SHAPE_FACENUM, false); wrfl->writeNums(1, true, fcn);
    wrfl->writeValue(VAL_SHAPE_SIDENUM, false); wrfl->writeNums(1, true, sdn);
    wrfl->writeValue(VAL_SHAPE_CORNERNUM, false); wrfl->writeNums(1, true, cnn);

    wrfl->writeValue(VAL_SHAPE_SIZE, false);
    wrfl->writeNums(3, true, size.x, size.y, size.z);
    
    wrfl->writeValue(VAL_SHAPE_ROTF, false);
    wrfl->writeNums(1, true, rotfirst?1:0);
    wrfl->writeNums(0, true);

    wrfl->writeValue(VAL_SHAPE_FACES, true);
    for(i = 0; i < fcn; ++i)
    {
        wrfl->writeNums(4, true, faces[i].dr, faces[i].ex.x, faces[i].ex.y, faces[i].ex.z); 
    }

    wrfl->writeNums(0, true);

    wrfl->writeValue(VAL_SHAPE_SIDES, true);
    for(i = 0; i < sdn; ++i)
    {
        wrfl->writeNums(6, true, sides[i].dr, sides[i].ex.x, sides[i].ex.y, sides[i].ex.z, sides[i].nei[0], sides[i].nei[1]); 
    }
    wrfl->writeNums(0, true);

    wrfl->writeValue(VAL_SHAPE_CORNERS, true);
    for(i = 0; i < cnn; ++i)
    {
        wrfl->writeNums(3, false, corners[i].ex.x, corners[i].ex.y, corners[i].ex.z);
        for(j = 0; j < 6; ++j)
        {
            wrfl->writeNums(1, false, corners[i].nei[j]);
        }
        wrfl->writeNums(0, true);
    }

    return true;
}


static Shape::SideFind normSide[3][4] = 
{
    {{XY_PLANE, 0, 0, 0}, {XZ_PLANE, 0, 0, 0}, {XY_PLANE, 0, -4, 0}, {XZ_PLANE, 0, 0, -4}},
    {{XY_PLANE, 0, 0, 0}, {YZ_PLANE, 0, 0, 0}, {XY_PLANE, -4, 0, 0}, {YZ_PLANE, 0, 0, -4}},
    {{XZ_PLANE, 0, 0, 0}, {YZ_PLANE, 0, 0, 0}, {XZ_PLANE, -4, 0, 0}, {YZ_PLANE, 0, -4, 0}}
};

// get the fc index of the face in the generated test shape
int Shape::getShapeFcInd(CoordBuild s) const
{
    Vec3i g1, gtmp;
    BuildWorld::get3dCoords(s, g1, gtmp);
    g1.x = g1.x * 4 - buildBounds.minx;
    g1.y = g1.y * 4 - buildBounds.miny;
    g1.z = g1.z * 4 - buildBounds.minpage;
    //return locateFace((EPlane)s.dim, g1);
    return locateFaceHardWay((EPlane)s.dim, g1);
}

/// locate face when m_opt_facesLoc is invalidated (after generate completes) 
int Shape::locateFaceHardWay(EPlane ldr, Vec3i lex) const
{
    for (int i = 0; i<fcn; ++i)
    {
        if ((faces[i].dr == ldr) && (faces[i].ex.x == lex.x) && (faces[i].ex.y == lex.y) && (faces[i].ex.z == lex.z))
            return i;
    }
    return -1;
}


/////////////////////////////// GENERATOR /////////////////////////////////////

int Shape::locateFace(EPlane ldr, Vec3i lex) const
{
    if ((lex.x < 0) || (lex.y < 0) || (lex.z < 0) || (lex.x >= size.x) || (lex.y >= size.y) || (lex.z >= size.z)) 
        return -1;
    return m_opt_facesLoc[ldr].axx(lex, 4);
}


bool Shape::checkSide(EAxis ldr, int x, int y, int z, vector<SideDef> &slst, vector<SideDef>& slstError)
{	// fails, return 0, if there are more than 2 faces to a side
    int nec = 0, nei[4] ,i, fcl;			
    
    for(i=0; i<4; ++i)
    {
        if ((fcl = locateFace(normSide[ldr-1][i].pln, 
                              Vec3i(x + normSide[ldr-1][i].x, 
                                      y + normSide[ldr-1][i].y, 
                                      z + normSide[ldr-1][i].z))) != -1) 
            nei[nec++] = fcl;
    }
                
    if (nec == 2)
    {
        slst.push_back(SideDef(ldr, Vec3i(x, y, z), nei));
        ++sdn;
    }
    else if (nec > 2) 
    {
        // error side ex should in the scene coordinates, adding bounds
        slstError.push_back(SideDef(ldr, Vec3i(x, y, z) + buildBounds.getMin(), nei));
        return false;	// found a side with more than 2 faces
    }
    return true;
}

bool Shape::checkCorner(int x, int y, int z, vector<CornerDef> &clst)
{
    int nec = 0, i, fcl, nei[12] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    EPlane lpln[12];
    Vec3i lex[12];

    lpln[0] = XY_PLANE;	lex[0] = Vec3i(x, y, z);
    lpln[1] = XY_PLANE;	lex[1] = Vec3i(x-4, y, z);
    lpln[2] = XY_PLANE;	lex[2] = Vec3i(x, y-4, z);
    lpln[3] = XY_PLANE; lex[3] = Vec3i(x-4, y-4, z);
    
    lpln[4] = XZ_PLANE;	lex[4] = Vec3i(x, y, z);
    lpln[5] = XZ_PLANE;	lex[5] = Vec3i(x, y, z-4);
    lpln[6] = XZ_PLANE;	lex[6] = Vec3i(x-4, y, z);
    lpln[7] = XZ_PLANE;	lex[7] = Vec3i(x-4, y, z-4);

    lpln[8] = YZ_PLANE;	lex[8] = Vec3i(x, y, z);
    lpln[9] = YZ_PLANE;	lex[9] = Vec3i(x, y, z-4);
    lpln[10] = YZ_PLANE; lex[10] = Vec3i(x, y-4, z);
    lpln[11] = YZ_PLANE; lex[11] = Vec3i(x, y-4, z-4);

    for(i = 0; i < 12; ++i)
    {
        if ((fcl = locateFace(lpln[i], lex[i])) != -1) 
            nei[nec++] = fcl;
    }
    // limiting conditions to when it can be 3, 4, 5, 6	
    if ((nec >= 3) && (nec <= 6))
    {
        for(i=nec; i<12; ++i)
            nei[i] = -1;
        clst.push_back(CornerDef(Vec3i(x, y, z), nei));

        ++cnn;
    }
    else
    {
        if (nec > 6) 
            return false;	// found a corner with more than 6 faces
    }
    return true;
}



CoordBuild Shape::fcToBuildCoord(int fc) const
{
    M_ASSERT(fc < fcn);
    return scdrToBuildCoord(faces[fc].ex, faces[fc].dr);
}

// se unGenerate
CoordBuild Shape::scdrToBuildCoord(const Vec3i sc, int dr) const
{
    Vec3i v = (sc + Vec3i(buildBounds.minx, buildBounds.miny, buildBounds.minpage)) / 4;
    CoordBuild r;
    r.dim = dr;
    switch (dr)
    {
    case YZ_PLANE: r.page = v.x; r.x = v.y; r.y = v.z; break;
    case XZ_PLANE: r.x = v.x; r.page = v.y; r.y = v.z; break;
    case XY_PLANE: r.x = v.x; r.y = v.y; r.page = v.z; break;
    }
    return r;
}


void Shape::readAxis(const BuildWorld *build, int iss, int jss, int pgss, EPlane planedr, list<FaceDef> &flst, int *reqfirst, SqrLimits &bound)
{ 
    int i, j, page, x = -1 , y = -1, z = -1, tmp;
    for(page = 0; page < pgss; ++page)
    {
        for(i = 0; i < iss; ++i)//x
        {
            for(j = 0; j < jss; ++j)//y
            {
                tmp = build->get(planedr, page, i, j);
                if (GET_TYPE(tmp) == TYPE_REAL)
                {
                    switch (planedr)
                    {
                    case YZ_PLANE: x = page; y = i; z = j; break;
                    case XZ_PLANE: x = i; y = page; z = j; break;
                    case XY_PLANE: x = i; y = j; z = page; break;
                    }
                    x *= 4; y *= 4; z *= 4;

                    if (GET_VAL(tmp) == FACE_STRT) 
                        *reqfirst = fcn;
                    flst.push_front(FaceDef(planedr, Vec3i(x, y, z)));
                    fcn++;

                    bound.MaxMin(z, x, y); // page plays z;
                }
            }
        }
    }
}

/*
int Shape::faceNeiFirst(int whos, TransType trans[])
{
    for(int i = 0; i < 4; ++i)
    {
        if (trans[faces[whos].nei[i]].to == -1)
            return faces[whos].nei[i];
    }
    return -1;
}
*/

// create a translation of face neighbors according to the order of their importance.
// neighbors which have more neighbors themselves have higher priority
int Shape::faceNeiFirstOpt(int whos, vector<TransType>& trans)
{
    int win = -1, wrank = 0;
    bool wInTemp = false; // winner
    int i, j;
    for(i = 0; i < 4; ++i)
    {
        int cand = faces[whos].nei[i]; // candidte
        if (m_genTemplate) {
            bool inTemp = m_genTemplate->contains(faces[cand].ex + buildBounds.getMin(), faces[cand].dr, faces[cand].facing);
            if (!inTemp)
                continue;
        }

        if (trans[cand].to == -1)
        {
            int rank = 0;
            for(j = 0; j < 4; ++j)
            {
                if (trans[faces[cand].nei[j]].to != -1) // occupied
                    ++rank;
            }
            if (rank > wrank)
            {
                win = cand;
                wrank = rank;
            }
        }
    }
    return win;
}


EGenResult Shape::reArrangeFacesDFS(FaceDef faces[], FaceDef revis[], vector<TransType>& trans)
{
    int revi = 0;
    int addi = 0;
  startDfs:
    do
    {	// assume the whole shape is coonected... or not
        int next = faceNeiFirstOpt(trans[revi].who, trans);
        if (next != -1)
        {
            ++addi;
            revi = addi;
            revis[addi] = faces[next];
            
            trans[next].to = revi;
            trans[revi].who = next;
        }
        else
            --revi;
    }
    while (revi > 0);

    if (addi + 1 < fcn) // addi points to the last index added, this is 1 less the the size
    {
        if (m_genTemplate == nullptr)
            return GEN_RESULT_NOT_CONNECT;
        m_genTemplate = nullptr;
        revi = addi;
        goto startDfs; // restart the DFS, now without the template
    }
    return GEN_RESULT_OK;
}

void Shape::initSizeAndBounds(const SqrLimits& bounds)
{
    buildBounds = bounds;
    //cout << "BOUNDS=" << bounds.minx << "," << bounds.miny << "," << bounds.minpage << endl;
    // fix it all according to the discovered bounds
    size.x = bounds.maxx - bounds.minx + 4 + 1; // need the +4 if we have a piece in the side with no cover
    size.y = bounds.maxy - bounds.miny + 4 + 1;
    size.z = bounds.maxpage - bounds.minpage + 4 + 1; // page plays z
}


EGenResult Shape::readWorld(const BuildWorld *build, int* reqf)
{
    *reqf = -1;
    if (m_orderTemplate == nullptr)
    {
        Vec3i fcSize(build->size);
        SqrLimits bounds; //page plays z
        bounds.Inverse(BUILD_SIZE * 4);

        ///////// BUILD FACES ///
        list<FaceDef> flst;

        readAxis(build, fcSize.y, fcSize.z, fcSize.x, YZ_PLANE, flst, reqf, bounds);
        readAxis(build, fcSize.x, fcSize.z, fcSize.y, XZ_PLANE, flst, reqf, bounds);
        readAxis(build, fcSize.x, fcSize.y, fcSize.z, XY_PLANE, flst, reqf, bounds);
        build->m_gen_bounds = bounds; // save the bounds of the last generate
        initSizeAndBounds(bounds);

        faces = new FaceDef[fcn];
        auto faceit = flst.begin();
        for (int i = 0; i < fcn; ++i)
        {	// convert the linked list to an array, NORMAIZE the coodrinates to the size of the array, and dispose of the list
            M_ASSERT(faceit != flst.end());
            faces[i].dr = faceit->dr;
            faces[i].ex = Vec3i(faceit->ex.x - bounds.minx, faceit->ex.y - bounds.miny, faceit->ex.z - bounds.minpage);
            ++faceit;
        }
        flst.clear();
        // first some sanity check needed for generating the face sequences later
        if (*reqf == -1)
            return GEN_RESULT_NO_START; // no start found
        *reqf = fcn - *reqf - 1; // order is upsidedown (list to vector)

    }
    else { // have m_orderTemplate
        faces = m_orderTemplate->faceDefs;
        fcn = m_orderTemplate->faces.size();
        initSizeAndBounds(m_orderTemplate->bounds);
        *reqf = 0;
    }

    for (int i = 0; i < fcn; ++i) {
        m_opt_facesLoc[faces[i].dr].axx(faces[i].ex, 4) = i; // build face optimizing array
    }

    return GEN_RESULT_OK;
}

EGenResult Shape::buildSidesAndCorners()
{
    bool illegalCorners = false; // sanity check for corners
    bool illegalSides = false;
    for (int x = 0; x < size.x; x += 4)
    {
        for (int y = 0; y < size.y; y += 4)
        {
            for (int z = 0; z < size.z; z += 4)
            {
                // look for sides in the all axis
                // need a seperate if for each one so none will get skipped
                if (!checkSide(X_AXIS, x, y, z, sides, errorSides))
                    illegalSides = true;
                if (!checkSide(Y_AXIS, x, y, z, sides, errorSides))
                    illegalSides = true;
                if (!checkSide(Z_AXIS, x, y, z, sides, errorSides))
                    illegalSides = true;

                if (!(checkCorner(x, y, z, corners)))
                    illegalCorners = true;
            }
        }
    }

    //	t3 = GetTickCount();
    // this shouldn't happen since it should have been discovered as illegal sides
    // illegal conrners can't happen without illegal sides
    if (illegalCorners && (!illegalSides))
        return GEN_RESULT_UNKNOWN;

    if (illegalSides)
    {
        sdn = 0; cnn = 0;
        return GEN_RESULT_ILLEGAL_SIDE;
    }
    return GEN_RESULT_OK;
}

EGenResult Shape::orderFacesInSequence(int reqf)
{
    // to help the face travesal transform routine, set the face-sides information temporatly
    // needed so faceNei() could work properly in O(1)
    make_sides_facenei();

    // prepare ground for transform
    FaceDef *revis = new FaceDef[fcn];
    vector<TransType> trans(fcn);

    revis[0] = faces[reqf];
    trans[reqf].to = 0;
    trans[0].who = reqf;

    // TBD- not randomised?

    // call the transform
    EGenResult aret = reArrangeFacesDFS(faces, revis, trans);
    if (aret != GEN_RESULT_OK) {
        delete[] faces;
        return aret;
    }

    // cleanup and transform for the sides and corners

    for (int i = 0; i < sdn; ++i)
    {
        sides[i].nei[0] = trans[sides[i].nei[0]].to;
        sides[i].nei[1] = trans[sides[i].nei[1]].to;
    }

    for (int i = 0; i < cnn; ++i)
    {
        for (int j = 0; j < 6; ++j)
        {
            if (corners[i].nei[j] != -1)
                corners[i].nei[j] = trans[corners[i].nei[j]].to;
        }
    }

    delete[] faces;
    faces = revis;
    return GEN_RESULT_OK;
}

EGenResult Shape::generate(const BuildWorld *build)
{
    for (int i = 0; i < 3; ++i)
    {
        if (!m_opt_facesLoc[i].isCreated())
            m_opt_facesLoc[i].create(BUILD_SIZE, BUILD_SIZE, BUILD_SIZE);
        m_opt_facesLoc[i].clear(-1); // clear face optimizing array
    }

    deallocate();

    rotfirst = false; // ####### change
    fcn = 0;
    sdn = 0;
    cnn = 0;

    int reqf = -1; // index of the start piece in the list
    EGenResult gret;

    gret = readWorld(build, &reqf);
    if (gret != GEN_RESULT_OK)
        return gret;



    gret = buildSidesAndCorners();
    if (gret != GEN_RESULT_OK)
        return gret;

    makeVolumeAndFacing(); // need this now because the template is indexed with EFacing
    
    if (m_orderTemplate == nullptr) {
        // transform faces order for a building sequence (so far the order was what was read from the world
        // unless we have an order from a loaded shape
        gret = orderFacesInSequence(reqf);
        if (gret != GEN_RESULT_OK)
            return gret;
    }

    // some additional tasks
    makeReverseNei();

    makePieceCheckBits();
    makeNeiTransforms();

    return GEN_RESULT_OK;
}


// make the check-bits mask for every face, according to its relative place in the placing order
// pieces that are placed before a piece are added to the mask
void Shape::makePieceCheckBits()
{
    for(int i = 0; i < fcn; ++i) {
        faces[i].index = i;
    }
    // by this point, the faces are ordered according to the frameX-frameY order
    for(int i = 0; i < fcn; ++i) 
    {
        FaceDef& f = faces[i];
        TPicBits fmask = 0;
        // does each neighbor face exist when we put this piece?
        // low - neighbors with index lower than me
        bool neiLow[] = { false, false, false, false };
        
        // check sides
        TPicBits faceNeiMasker = 0x000E;
        for (int j = 0; j < 4; ++j) {
            int neiInd = f.nei[j];
            neiLow[j] = (neiInd != -1 && faces[neiInd].index < f.index);
            if (neiLow[j])
                fmask |= faceNeiMasker;
            faceNeiMasker <<= 4;
        }

        // check corners
        faceNeiMasker = 0x0001;
        //int oldmask = fmask;
        for (int j = 0; j < 4; ++j) {
            if (f.corners[j] == -1)
                continue;
            const CornerDef& corner = corners[f.corners[j]];
            int countLow = 0;
            for(int cni = 0; cni < 6; ++cni) {
                int neiInd = corner.nei[cni];
                bool isLow = (neiInd != -1 && faces[neiInd].index <= f.index);
                if (isLow)
                    ++countLow;
            }
            if (countLow ==  corner.numnei)
                fmask |= faceNeiMasker;

            //if (neiLow[(j-1+4)%4] && neiLow[j])
            //    oldmask |= faceNeiMasker;

            faceNeiMasker <<= 4;
        }


        f.fmask = fmask;

        //printf("%3d  mask=%04x\n", i, fmask);
    }
    //printf("\n");
}


// for each face, save if it's facing outside or inside, for example
//    +--In--+
// Out|    In|
//    +-Out--+
void Shape::makeVolumeAndFacing()
{
    // space of cubes. the pieces are thin sheets between the cubes
    BoundedBlockSpace3D space(size.x/4 + 2, size.y/4 + 2, size.z/4 + 2); // pad with one layer from every direction
    int i, x, y, z, d;
    for (i = 0; i < fcn; ++i)
    {	// the coordinates of the two adjucent cubes (in single cube space)
        x = faces[i].ex.x/4 + 1; y = faces[i].ex.y/4 + 1; z = faces[i].ex.z/4 + 1; d = faces[i].dr;
        
        Vec3i front(x, y, z);
        Vec3i back((d == YZ_PLANE)?(x - 1):x, (d == XZ_PLANE)?(y - 1):y, (d == XY_PLANE)?(z - 1):z);
        space.ErectWalls(d, front, back);
    }
    
    volume = (space.szx * space.szy * space.szz) - space.FloodFill(0, 0, 0);


    for (i = 0; i < fcn; ++i)
    {
        faces[i].facing = (space.axx(faces[i].ex.x/4 + 1, faces[i].ex.y/4 + 1, faces[i].ex.z/4 + 1).fill)?FACING_IN:FACING_OUT;
        if (faces[i].dr == XY_PLANE) faces[i].facing = (EFacing)(1 - faces[i].facing); // switch it's direction.
        // I have no idea why that is like that.
    }

}


void Shape::make_sides_facenei()
{
    vector<int> counters(fcn, 0);

    for (int i = 0; i < fcn; ++i) 
    {
        for (int j = 0; j < 4; ++j)
        { // they can stay -1 even after all if the shape is open
            faces[i].sides[j] = -1;
        }
    }

    int fc;
    for(int i = 0; i < sdn; ++i)
    {
        fc = sides[i].nei[0];
        faces[fc].sides[counters[fc]++] = i;
        fc = sides[i].nei[1];
        faces[fc].sides[counters[fc]++] = i;
    }

    for (int i = 0; i < fcn; ++i) 
        faceNei(i, faces[i].nei);

}


// this function generally makes additional decoration to the shape other than
// what is generated or loaded from file
void Shape::makeReverseNei()
{
    int i, j, fc;

    // build face neibours according to the reverse nei
    // we rearranged the faces since the last time this was done
    make_sides_facenei();

    vector<int> counters(fcn, 0);

    for (i = 0; i < fcn; ++i) 
    {
        for (j = 0; j < 4; ++j)
        { // they can stay -1 even after all if the shape is open
            faces[i].corners[j] = -1;
        }
    }

    for (i = 0; i < cnn; ++i)
    {
        for (j = 0; j < 6; ++j)
        {
            fc = corners[i].nei[j]; 
            if (fc != -1)
                faces[fc].corners[counters[fc]++] = i;
        }
    }


    ///////////// order them now ////////////////
    for (fc = 0; fc < fcn; ++fc)
    {
        int trans[4] = {-1,-1,-1,-1};
        FaceDef &cfc = faces[fc];
        // order the sides in order accorsing to frameX and frameY
        for (j = 0; j < 4; ++j)
        {
            int curside = cfc.sides[j];
            if (curside == -1) 
                continue;
            bool strtAtface = (sides[curside].ex ==  cfc.ex);
            int sidedr = sides[curside].dr;
            bool firstDir = ( ((cfc.dr == XY_PLANE) && (sidedr == X_AXIS)) ||
                              ((cfc.dr == YZ_PLANE) && (sidedr == Z_AXIS)) ||
                              ((cfc.dr == XZ_PLANE) && (sidedr == X_AXIS)) );

            if (strtAtface && firstDir) trans[0] = curside;
            else if (strtAtface) trans[3] = curside;
            else if (firstDir) trans[2] = curside;
            else trans[1] = curside;
        }

        for (j = 0; j < 4; ++j)
        {
            cfc.sides[j] = trans[j];
            trans[j] = -1;
        }

        // rearrange face nei according to the new sides order
        faceNei(fc, faces[fc].nei);
        
        // order corners
        for (j = 0; j < 4; ++j)
        {
            int curcorn = cfc.corners[j];
            if (curcorn == -1)
                continue;
            Vec3i &cex = corners[curcorn].ex;
            if (cex == cfc.ex) trans[0] = curcorn;
            else if (cex == sides[cfc.sides[1]].ex) trans[1] = curcorn;
            else if (cex == sides[cfc.sides[2]].ex) trans[3] = curcorn;
            else trans[2] = curcorn;
        }

        for (j = 0; j < 4; ++j)
            cfc.corners[j] = trans[j];

    }

}


// BFS
/*
void Shape::faceNei(int whos, int fnei[4])
{
    int nec=0, i;
    for(i = 0; i < 4; ++i)
    {
        const SideDef &sd = sides[faces[whos].sides[i]];
        if (sd.nei[0] == whos)
        {
            fnei[nec++] = sd.nei[1];
        }
        else if (sd.nei[1] ==  whos) // needed because it can be not one of them
        {
            fnei[nec++] = sd.nei[0];
        }
    }
    for(i = nec; i < 4; ++i)
    {
        fnei[i] = -1;
    }
}*/

// set the face nei according to the order of the sides
void Shape::faceNei(int whos, int fnei[4])
{
    for(int i = 0; i < 4; ++i)
    {
        const SideDef &sd = sides[faces[whos].sides[i]];
        if (sd.nei[0] == whos)
        {
            fnei[i] = sd.nei[1];
        }
        else if (sd.nei[1] ==  whos) // needed because it can be not one of them
        {
            fnei[i] = sd.nei[0];
        }
        else
        {
            fnei[i] = -1;
        }
    }

}

/*
EGenResult Shape::reArrangeFacesBFS(FaceDef faces[], FaceDef revis[], TransType trans[])
{
    int fnei[4], revi = 1, j;
    for(int revpnt = 0; revpnt < fcn; ++revpnt)
    {	// assume the whole shape is coonected... or not
        faceNei(trans[revpnt].who, fnei);
        j = 0;
        while ((j < 4) && (fnei[j] != -1))
        {	
            if (trans[fnei[j]].to == -1)
            {
                revis[revi] = faces[fnei[j]];
                
                trans[fnei[j]].to = revi;
                trans[revi].who = fnei[j];
                ++revi;
            }
            ++j;
        }
    }	

    if (revi != fcn)
    {	// shape is not connected (???)
        return GEN_RESULT_NOT_CONNECT;
    }
    return GEN_RESULT_OK;
}
*/

bool Shape::createTrasformTo(const Shape *news, TTransformVec& movedTo, bool* trivialTransform)
{
    if (fcn != news->fcn)
        return false;
    // calc the offset of the two shapes from the first piece which must be in the same place in both models
    if (faces[0].dr != news->faces[0].dr)
        return false; // first face is not the same orientation
    Vec3i offset(faces[0].ex - news->faces[0].ex); 

    *trivialTransform = true;
    std::fill(movedTo.begin(), movedTo.end(), -1);
    for (int i = 0; i < fcn; ++i)
    { // go over my faces
        for(int j = 0; j < fcn; ++j)
        { // search in new shape faces
            if ((news->faces[j].ex + offset == faces[i].ex) && (news->faces[j].dr == faces[i].dr))
            {
                movedTo[i] = j;
                if (i != j)
                    *trivialTransform = false;
                break;
            }
        }
        if (movedTo[i] == -1)
            return false;
    }
    return true;
}


//---------------------------------------------------------------------------------------------------------

ostream& operator<<(ostream& o, const Vec3& v) {
    o << std::fixed << setw(1) << v.x << "," << v.y << "," << v.z;
    return o;
}

const char* drName(int d) {
    switch(d) {
    case 0: return "YZ_PLANE";
    case 1: return "XZ_PLANE";
    case 2: return "XY_PLANE";
    }
    return "UNKNOWN";
}

//int g_count = 0;

/*
void Shape::checkAddQuad(const Mat4& m) 
{
    testQuads.push_back( m.transformVec(Vec3(0.1,0.1,0.0)) );
    testQuads.push_back( m.transformVec(Vec3(3.9,0.1,0.0)) );
    testQuads.push_back( m.transformVec(Vec3(3.9,3.9,0.0)) );
    testQuads.push_back( m.transformVec(Vec3(0.1,3.9,0.0)) );
    if (false)
        testQuads.push_back( Vec3(1,1,1));
    else
        testQuads.push_back( Vec3(0.3,0.3,1));
}
*/

int g_testAngle[3] = {90,90,90};
bool doPrint = false;

template<typename T, int N>
bool itemIn(T v, T (&arr)[N]) {
    for(int i = 0; i < N; ++i) 
        if (v == arr[i])
            return true;
    return false;
}

#define SQRT_2_INV (0.70710678118654752440084436210485)

// transform m from face fi[0-fcn) to neighbour ni[0-3] of fi
void Shape::transformFromFaceToNei(MatStack& m, int fi, int ni, float baseAng) const
{
    NeiTransform& t = faces[fi].neiTrans[ni];

    m.translate(t.planeMove.x, t.planeMove.y, 0);

    if (t.angleType != ANG_ZERO) {
        float sa = 0;
        switch (t.angleType) {
        case ANG_PLUS:     sa = baseAng; break;
        case ANG_MINUS:    sa = -baseAng; break;
        case ANG_180MINUS: sa = 180-baseAng; break;
        case ANG_180PLUS:  sa = 180+baseAng; break;
        }
        m.rotate(sa, t.rotAxis.x, t.rotAxis.y, t.rotAxis.z);
    }

    if (t.flip) {
        m.translate(2, 2, 0);
        m.rotate(180, t.rotAxis.x, t.rotAxis.y, t.rotAxis.z);
        m.translate(-2, -2, 0);
    }
    if (t.flipDiag) {
        m.translate(2, 2, 0);
        m.rotate(180, SQRT_2_INV,SQRT_2_INV,0);
        m.translate(-2, -2, 0);
    }
}

template<typename FAdd>
void Shape::checkNeiTranDFS(MatStack& m, int fi, vector<int>& pass, int lvl, float ang, FAdd& addPiece) 
{
    FaceDef& f = faces[fi]; 

    if (pass[fi] > 0)
        return;
    pass[fi]++;

    addPiece(m.cur(), fi);

    for (int ni = 0; ni < 4; ++ni) 
    {
        int n = f.nei[ni];
        if (n == -1)
            continue;
        m.push();

        transformFromFaceToNei(m, fi, ni, ang);

        checkNeiTranDFS(m, f.nei[ni], pass, lvl + 1, ang, addPiece);
        m.pop();
    }
}


struct BNode {
    BNode(int _fi, int _ni) : fi(_fi), ni(_ni) {}
    int fi, ni;
    list<BNode> c;
};

template<typename FAdd>
void Shape::runBFSTree(MatStack& m, const BNode& node, float ang, FAdd& addPiece) const
{
    addPiece(m.cur(), node.fi);
    for(const BNode& next : node.c)  {
        m.push();

        transformFromFaceToNei(m, node.fi, next.ni, ang);

        runBFSTree(m, next, ang, addPiece);
        m.pop();
    }
}

template<typename FAdd>
void Shape::checkNeiTranBFS(MatStack& m, float ang, const FAdd& addPiece) const
{
    BNode root(0, -1);
    
    vector<int> passed(fcn);
    list<BNode*> fq;
    fq.push_back(&root);
    passed[root.fi]++;

    while (!fq.empty())
    {
        BNode* cur = fq.front();
        int fi = cur->fi;
        fq.pop_front();
        FaceDef& f = faces[fi]; 

        for(int ni = 0; ni < 4; ++ni) {        
            int n = f.nei[ni];
            if (n == -1)
                continue;
            if (passed[n] > 0)
                continue;
            passed[n]++;
            cur->c.push_back(BNode(n, ni));
            fq.push_back( &cur->c.back() );
        }
    }

    runBFSTree(m, root, ang, addPiece);

}



// YZ_PLANE(0) XZ_PLANE(1) XY_PLANE(2)
void Shape::makeNeiTransforms() 
{
    //cout << "NEI-TRANSFORM **********************" << endl;
    for (int fi = 0; fi < fcn; ++fi) 
    {
        FaceDef& f = faces[fi];
        EAngleType sa = ANG_ZERO;
        //cout << fi << ": " << f.dr << endl;
        for (int ni = 0; ni < 4; ++ni) 
        {
            if (f.nei[ni] == -1)
                continue;
            FaceDef& nf = faces[f.nei[ni]];
            Vec3i diff = nf.ex - f.ex;
            //cout << "  " << diff.x << "," << diff.y << "," << diff.z << " : " << nf.dr << endl;
            NeiTransform nt;
            bool flip = false, flipDiag = false;
            if (f.dr == XY_PLANE) { // V
                if (nf.dr == XY_PLANE) {
                    nt = NeiTransform(diff.x, diff.y, ANG_ZERO, 0,0,0); // 0,-4,0|4,0,0|0,4,0|-4,0,0
                }
                else if (nf.dr == XZ_PLANE) { // 3->5
                    if (diff.y == 0) {
                        if (diff.z == 0)
                            sa = ANG_180PLUS; //180+ang; // V
                        else {
                            sa = ANG_180MINUS; //180-ang; // V
                            flip = true;
                        }
                    }
                    else
                        if (diff.z == 0) 
                            sa = ANG_MINUS; //-ang; // V
                        else {
                            sa = ANG_PLUS; //ang; //V
                            flip = true;
                        }
                    nt = NeiTransform(diff.x, diff.y, sa, 1,0,0, flip); // VV XZ_PLANE: 0,0,0|0,4,0|0,0,-4|0,4,-4 
                }
                else if (nf.dr == YZ_PLANE) {
                     if (diff.x == 0) {
                        if (diff.z != 0) {
                            sa = ANG_180PLUS; //180+ang; // V
                            flip = true;
                        }
                        else 
                            sa = ANG_180MINUS; //180-ang; // V
                     }
                     else
                        if (diff.z != 0){
                            sa = ANG_MINUS; //-ang; // V
                            flip = true;
                        }
                        else
                            sa = ANG_PLUS; //ang; // V
                    nt = NeiTransform(diff.x, diff.y, sa, 0,1,0, flip); // VV YZ_PLANE: 4,0,0|0,0,0|4,0,-4|0,0,-4
                }
            }
            // -----------------------------------
            else if (f.dr == XZ_PLANE) {
                if (nf.dr == XZ_PLANE) {
                    nt = NeiTransform(diff.x, diff.z, ANG_ZERO, 0,0,0); //  //0,0,-4|4,0,0|0,0,4|-4,0,0
                }
                else if (nf.dr == XY_PLANE) { // 0,-4,0|0,-4,4|0,0,0|0,0,4
                    if (diff.z == 0) {
                        if (diff.y == 0) {
                            sa = ANG_180MINUS; //180-ang; // V
                        }
                        else {
                            sa = ANG_180PLUS; //180+ang;
                            flip = true; // V
                        }
                    }
                    else {
                        if (diff.y == 0) 
                            sa = ANG_PLUS; //ang;  //V 2->3
                        else {
                            sa = ANG_MINUS; //-ang; //V 5->3
                            flip = true;
                        }
                    }
                    nt = NeiTransform(0, diff.z, sa, 1,0,0, flip); 
                }
                else if (nf.dr == YZ_PLANE) {
                    if (diff.x == 0) {
                        if (diff.y == 0) 
                            sa = ANG_180PLUS; //180+ang; // V 5->4
                        else {
                            sa = ANG_180MINUS; //180-ang; // V
                            flip = true;
                        }
                    }
                    else {
                        if (diff.y == 0) 
                            sa = ANG_MINUS; //-ang;  // V 5->1
                        else {
                            sa = ANG_PLUS; //ang;  // V
                            flip = true;
                        }
                    }
                    nt = NeiTransform(diff.x, diff.z, sa, 0,1,0, flip, true);
                }
            }
            else if (f.dr == YZ_PLANE) {
                if (nf.dr == YZ_PLANE) {
                    nt = NeiTransform(diff.z, diff.y, ANG_ZERO, 0,0,0); //0,-4,0|*0,0,4|0,4,0|0,0,-4                   
                }
                else if (nf.dr == XY_PLANE) { // 1->3
                    if (diff.x == 0) {
                        if (diff.z == 0) 
                            sa = ANG_180PLUS; //180+ang; // V 4->0
                        else 
                            sa = ANG_MINUS; //-ang; // V 4->3
                    }
                    else {
                        if (diff.z == 0) { 
                            sa = ANG_180MINUS; //180-ang;  // V1->0
                            flip = true;
                        }
                        else {
                            sa = ANG_PLUS; //ang;  //  V 1->3
                            flip = true;
                        }
                    }
                    nt = NeiTransform(diff.z, diff.y, sa, 0,1,0, flip);
                }
                else if (nf.dr == XZ_PLANE) {
                    if (diff.y == 0) {
                        if (diff.x == 0) 
                            sa = ANG_180MINUS; //180-ang; // V 4->5
                        else {
                            sa = ANG_180PLUS; //180+ang;
                            flip = true;
                        }
                    }
                    else {
                        if (diff.x == 0)  
                            sa = ANG_PLUS; //ang; // V 4->2
                        else {
                            sa = ANG_MINUS; //-ang; 
                            flip = true;
                        }
                    }
                    nt = NeiTransform(diff.z, diff.y, sa, 1,0,0, flip, true);
                }
            }
            f.neiTrans[ni] = nt;
        }
    }

    // check
    //g_count = 0;

    // move to the first face

    //int ang = g_testAngle[0];
    //checkNeiTranDFS(m, start, pass, 0, ang, checkAddQuad);

    //g_count = 0;
    //testQuads.clear();

    MatStack m;
    startNeiTransform(m);
//    checkNeiTranBFS(m, ang, [this](const Mat4& m, int){ this->checkAddQuad(m); } ); // test martrics
    //m.translate(0,0,-4);
    //checkNeiTran(m, 3, pass, 0);

    //getchar();
    //exit(1);
}

void Shape::startNeiTransform(MatStack& m) const {
    int start = 0;
    m.identity();
    FaceDef& face = faces[start];    	
    // transformation to the center of the build
    Vec3i startEx = face.ex + buildBounds.getMin() - (BUILD_START_CUBE * 4); // this is where the build start its first cube
    m.translate(startEx.x, startEx.y, startEx.z);
    m.scale(1,1,-1); // opengl Z axis is inside out

    if (face.dr == YZ_PLANE) {
        m.rotate(90, 0,1,0);
    }
    else if (face.dr == XZ_PLANE) {
        m.rotate(-90, 1,0,0);
    }

}




void Shape::makeTransformsMatrics(float angle, vector<Mat4>& mats) const
{
    if (mats.size() != fcn)
        mats.resize(fcn);
    MatStack m;
    startNeiTransform(m);


    checkNeiTranBFS(m, angle, [&mats](const Mat4& m, int fi) { mats[fi] = m; });

}