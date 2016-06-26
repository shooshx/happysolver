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

#include "BuildWorld.h"
#include "MyFile.h"



BuildDimension::BuildPage::BuildPage()
{
    for(int x = 0; x < BUILD_SIZE; ++x)
    {
        for(int y = 0; y < BUILD_SIZE; ++y)
        {
            fc[x][y] = 0;
        }
    }
}

void BuildWorld::initializeNew(bool boxed)
{
    size = Vec3i(BUILD_SIZE, BUILD_SIZE, BUILD_SIZE);
    fChangedFromGen = true;
    fChangedFromSave = false;
    m_bTested = false;

    clean(CLEAN_CLEAR);

    if (!boxed)
    { // tiled mode
        set(1, 4, 4, 4, FACE_STRT); // TBD: change to be in the center??
        nFaces = 1;
        fClosed = false; // it's clearly not closed
        doTransparent();
    }
    else
    { // boxed mode
        m_space.clear(BoundedBlock(0));

        setBox(BUILD_START_CUBE);
        //setBox(Vec3i(24, 24, 25));

        nFaces = 6;
        fClosed = true;
        reClacLimits();
    }

}


void BuildWorld::setBox(const Vec3i s) 
{
    m_space.axx(s).fill = 1;
    CoordBuild walls[6];
    getBuildCoords(s, walls);
    set(walls[0], FACE_STRT);
    for (int i = 1; i < 6; ++i)
        set(walls[i], FACE_NORM);
}

// get the the coordinates of the two cubes adjucent to this face
void BuildWorld::get3dCoords(CoordBuild s, Vec3i &g1, Vec3i &g2)
{
    switch (s.dim)
    {
    case YZ_PLANE: g1.x = s.page; g1.y = s.x; g1.z = s.y; g2.x = s.page-1; g2.y = s.x; g2.z = s.y; break;
    case XZ_PLANE: g1.x = s.x; g1.y = s.page; g1.z = s.y; g2.x = s.x; g2.y = s.page-1; g2.z = s.y; break;
    case XY_PLANE: g1.x = s.x; g1.y = s.y; g1.z = s.page; g2.x = s.x; g2.y = s.y; g2.z = s.page-1; break;
    }
}

// get the 6 faces around the cube
void BuildWorld::getBuildCoords(Vec3i g, CoordBuild b[6])
{
    b[0].dim = XY_PLANE; b[0].page = g.z;   b[0].x = g.x; b[0].y = g.y;
    b[1].dim = XY_PLANE; b[1].page = g.z+1; b[1].x = g.x; b[1].y = g.y;
    b[2].dim = XZ_PLANE; b[2].page = g.y;   b[2].x = g.x; b[2].y = g.z;
    b[3].dim = XZ_PLANE; b[3].page = g.y+1; b[3].x = g.x; b[3].y = g.z;
    b[4].dim = YZ_PLANE; b[4].page = g.x;   b[4].x = g.y; b[4].y = g.z;
    b[5].dim = YZ_PLANE; b[5].page = g.x+1; b[5].x = g.y; b[5].y = g.z;
}



// get the fc index of the face in the generated test shape
// see Shape::getShapeFcInd
int BuildWorld::getTestShapeFcInd(CoordBuild s) const
{
    Vec3i g1, gtmp;
    get3dCoords(s, g1, gtmp);
    g1.x = g1.x * 4 - m_gen_bounds.minx; 
    g1.y = g1.y * 4 - m_gen_bounds.miny; 
    g1.z = g1.z * 4 - m_gen_bounds.minpage;
    return m_testShape.locateFaceHardWay((EPlane)s.dim, g1); 
    // need to do it the hard way and not the optimized way because the
    // optimization data was not transformed
}



void BuildWorld::clean(ECleanMethod meth) //, WorldLimits& withlim, bool na)
{
    int dim, page, x, y, theget;

    if (meth == CLEAN_CLEAR) 
    {
        nFaces = 0;
        fClosed = true; // a non shape is a closed shape...
        m_space.clear();
        m_limits.Init();
    }

    for(dim = 0; dim < 3; ++dim)
    {
        SqrLimits lim = m_limits[dim];
        for(page = lim.minpage; page < lim.maxpage; ++page)
        {
            for(x = lim.minx; x < lim.maxx ; ++x)
            {
                for(y = lim.miny; y < lim.maxy; ++y)
                {
                    theget = get(dim, page, x, y); 

                    if (theget != 0)
                    {
                        if (meth == CLEAN_CLEAR)
                        {
                            set(dim, page, x, y, 0);
                        }
                        else if (meth == CLEAN_TRANS)
                        {
                            if (GET_TYPE(theget) == TYPE_VIR) // only touch virtual ones
                                set(dim, page, x, y, GET_SHOW(theget)); // maintain the show flag, (during DoTransparent)
                        }
                        else if (meth == CLEAN_TRANS_SHOW)
                        {
                            if (GET_TYPE(theget) == TYPE_VIR) {
                                set(dim, page, x, y, 0); // rid of virtual ones
                                //cout << "--cleaned" << endl;
                            }
                            else
                                set(dim, page, x, y, GET_VAL(theget));
                        }
                    }
                }
            }
        }
    }
}

bool BuildWorld::search(int face, int changeTo, bool onlyShow, bool doret)
{ //changeTo == -1 means don't change
    int dim, page, x, y;

    for(dim = 0; dim < 3; ++dim)
    {
        SqrLimits lim = m_limits[dim];
        for(page = lim.minpage; page < lim.maxpage; ++page)
        {
            for(x = lim.minx; x < lim.maxx ; ++x)
            {
                for(y = lim.miny; y < lim.maxy; ++y)
                {
                    int theget = get(dim, page, x, y);
                    if (!onlyShow)
                    {
                        if (GET_VAL(theget) == face)
                        {
                            if (changeTo != -1)
                            {
                                set(dim, page, x, y, changeTo);
                            }
                            if (doret) return true;
                        }
                    }
                    else
                    {
                        if (GET_SHOW(theget) == GET_SHOW(face))
                        {
                            if (changeTo != -1)
                            {
                                set(dim, page, x, y, GET_SHOW(changeTo) | GET_VAL(theget));
                            }
                            if (doret) return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}



bool BuildWorld::set(int dim, int page, int x, int y, int set)
{
    if ((x >= 0) && (x < BUILD_SIZE) && (y >= 0) && (y < BUILD_SIZE) && (page >= 0) && (page < BUILD_SIZE) && (dim >= 0) && (dim < 3))
    {
        dm[dim].pages[page].fc[x][y] = set;
        return true;
    }
    return false;
}

bool BuildWorld::set(const CoordBuild &c, int set)
{
    if ((c.x >= 0) && (c.x < BUILD_SIZE) && (c.y >= 0) && (c.y < BUILD_SIZE) && (c.page >= 0) && (c.page < BUILD_SIZE) && (c.dim >= 0) && (c.dim < 3))
    {
        dm[c.dim].pages[c.page].fc[c.x][c.y] = set;
        return true;
    }
    return false;
}




void BuildWorld::reClacLimits()
{
    int dim, page, x, y;

    WorldLimits wlimits = m_limits;
    m_limits.Inverse(); // for next iteration, make it as small as possible

    for(dim = 0; dim < 3; ++dim)
    {
        const SqrLimits &dimlmt  = wlimits[dim];
        for(page = dimlmt.minpage; page < dimlmt.maxpage; ++page)
        {
            for(x = dimlmt.minx; x < dimlmt.maxx; ++x)
            {
                for(y = dimlmt.miny; y < dimlmt.maxy; ++y)
                {
                    int curget = get(dim, page, x, y);
                    if (curget != 0)
                    {
                        m_limits[dim].MaxMinInc(page, x, y); // expand it
                        m_limits[dim].MaxMinInc(page+1, x+1, y+1); // expand it
                        // the last line is needed since all our iterations are '<' and not '<='
                    }
                }
            }
        }
    }
}


static const BuildWorld::TransSqr transp[3][4][3] =  
{{	{ { YZ_PLANE, { {0, 0}, {1, -1}, {2, 0}  } }, { XZ_PLANE, { {1, 0},  {0, -1}, {2, 0} } }, { XZ_PLANE, { {1, 0},  {0, 0},  {2, 0}  } } }, 
    { { YZ_PLANE, { {0, 0}, {1, +1}, {2, 0}  } }, { XZ_PLANE, { {1, +1}, {0, -1}, {2, 0} } }, { XZ_PLANE, { {1, +1}, {0, 0},  {2, 0}  } } },
    { { YZ_PLANE, { {0, 0}, {1, 0},  {2, -1} } }, { XY_PLANE, { {2, 0},  {0, 0},  {1, 0} } }, { XY_PLANE, { {2, 0},  {0, -1}, {1, 0}  } } },
    { { YZ_PLANE, { {0, 0}, {1, 0},  {2, +1} } }, { XY_PLANE, { {2, +1}, {0, 0},  {1, 0} } }, { XY_PLANE, { {2, +1}, {0, -1}, {1, 0}  } } }},

{	{ { XZ_PLANE, { {0, 0}, {1, -1}, {2, 0}  } }, { YZ_PLANE, { {1, 0},  {0, -1}, {2, 0} } }, { YZ_PLANE, { {1, 0},  {0, 0},  {2, 0}  } } },
    { { XZ_PLANE, { {0, 0}, {1, +1}, {2, 0}  } }, { YZ_PLANE, { {1, +1}, {0, -1}, {2, 0} } }, { YZ_PLANE, { {1, +1}, {0, 0},  {2, 0}  } } },
    { { XZ_PLANE, { {0, 0}, {1, 0},  {2, -1} } }, { XY_PLANE, { {2, 0},  {1, 0},  {0, 0} } }, { XY_PLANE, { {2, 0},  {1, 0},  {0, -1} } } },
    { { XZ_PLANE, { {0, 0}, {1, 0},  {2, +1} } }, { XY_PLANE, { {2, +1}, {1, 0},  {0, 0} } }, { XY_PLANE, { {2, +1}, {1, 0},  {0, -1} } } }},

{	{ { XY_PLANE, { {0, 0}, {1, -1}, {2, 0}  } }, { YZ_PLANE, { {1, 0},  {2, 0},  {0, 0} } }, { YZ_PLANE, { {1, 0},  {2, 0},  {0, -1} } } },
    { { XY_PLANE, { {0, 0}, {1, +1}, {2, 0}  } }, { YZ_PLANE, { {1, +1}, {2, 0},  {0, 0} } }, { YZ_PLANE, { {1, +1}, {2, 0},  {0, -1} } } },
    { { XY_PLANE, { {0, 0}, {1, 0},  {2, -1} } }, { XZ_PLANE, { {2, 0},  {1, 0},  {0, 0} } }, { XZ_PLANE, { {2, 0},  {1, 0},  {0, -1} } } },
    { { XY_PLANE, { {0, 0}, {1, 0},  {2, +1} } }, { XZ_PLANE, { {2, +1}, {1, 0},  {0, 0} } }, { XZ_PLANE, { {2, +1}, {1, 0},  {0, -1} } } }}};


void BuildWorld::getAllNei(const CoordBuild& in, CoordBuild out[12])
{
    int outi = 0;
    for (int side = 0; side < 4; ++side)
    { // all neibours
        for (int sqr = 0; sqr < 3; ++sqr)
        { // 3 sides of the neibour
            CoordBuild& oc = out[outi++];

            oc.dim = transp[in.dim][side][sqr].plotDim;

            int fchs = transp[in.dim][side][sqr].prm[0][0];
            oc.page = (fchs == 0) * in.page + (fchs == 1) * in.x + (fchs == 2) * in.y + transp[in.dim][side][sqr].prm[0][1];

            fchs = transp[in.dim][side][sqr].prm[1][0];
            oc.x = (fchs == 0) * in.page + (fchs == 1) * in.x + (fchs == 2) * in.y + transp[in.dim][side][sqr].prm[1][1];

            fchs = transp[in.dim][side][sqr].prm[2][0];
            oc.y = (fchs == 0) * in.page + (fchs == 1) * in.x + (fchs == 2) * in.y + transp[in.dim][side][sqr].prm[2][1];

        }
    }
}

/// this is a fragment left from the days of tile editing. it serves
/// the purpose of finding which tiles are legal to be new tiles.
/// nowa days all it does effectivly is calculate m_limits properly.
/// this code should be refactored into a different method.
void BuildWorld::doTransparent()
{
    int dim, page, x, y, side, sqr;
    int param[3][5]; //[sqrnum][paramnum dim, first, second, third, get(...)]

    clean(CLEAN_TRANS);

    for(dim = 0; dim < 3; ++dim)
    {
        const SqrLimits &dimlmt  = m_limits[dim];
        for(page = dimlmt.minpage; page < dimlmt.maxpage; ++page)
        {
            for(x = dimlmt.minx; x < dimlmt.maxx; ++x)
            {
                for(y = dimlmt.miny; y < dimlmt.maxy; ++y)
                {
                    if (GET_TYPE(get(dim, page, x, y)) == TYPE_REAL)
                    {
                        for(side = 0; side < 4; ++side)
                        { // all neibours
                            for(sqr = 0; sqr < 3; ++sqr)
                            { // 3 sides of the neibour
                                param[sqr][0] = transp[dim][side][sqr].plotDim;

                                int fchs = transp[dim][side][sqr].prm[0][0];
                                param[sqr][1] = (fchs == 0) * page + (fchs == 1) * x + (fchs == 2) * y + transp[dim][side][sqr].prm[0][1];

                                fchs = transp[dim][side][sqr].prm[1][0];
                                param[sqr][2] = (fchs == 0) * page + (fchs == 1) * x + (fchs == 2) * y + transp[dim][side][sqr].prm[1][1];

                                fchs = transp[dim][side][sqr].prm[2][0];
                                param[sqr][3] = (fchs == 0) * page + (fchs == 1) * x + (fchs == 2) * y + transp[dim][side][sqr].prm[2][1];

                                param[sqr][4] = get(param[sqr][0], param[sqr][1], param[sqr][2], param[sqr][3]);
                                
                            }

                            if (((GET_VAL(param[0][4]) == 0) || (GET_TYPE(param[0][4]) == TYPE_VIR)) && 
                                ((GET_VAL(param[1][4]) == 0) || (GET_TYPE(param[1][4]) == TYPE_VIR)) && 
                                ((GET_VAL(param[2][4]) == 0) || (GET_TYPE(param[2][4]) == TYPE_VIR)))
                            {  // if one of them is not a real one, make all of them blue
                                for(sqr = 0; sqr < 3; ++sqr)
                                {
                                    if ((GET_VAL(param[sqr][4]) != FACE_DONT_TRANS))
                                    {
                                        //cout << "add FACE_TRANS" << endl;
                                        set(param[sqr][0], param[sqr][1], param[sqr][2], param[sqr][3], FACE_TRANS | GET_SHOW(param[sqr][4]));
                                    }
                                }
                            }
                            else
                            {
                                for(sqr = 0; sqr < 3; ++sqr)
                                {
                                    if (GET_TYPE(param[sqr][4]) != TYPE_REAL) //override any virtual
                                    {
                                        //cout << "DONT_TRANS" << endl;
                                        set(param[sqr][0], param[sqr][1], param[sqr][2], param[sqr][3], FACE_DONT_TRANS);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    reClacLimits(); // recalculate the limits only after we're done changing it all
    fClosed = !search(FACE_TRANS);
}


void SqrLimits::MaxMinInc(int page, int x, int y)
{
    if (page + 1 > maxpage)	maxpage = page + 1;
    if (page - 1 < minpage) minpage = page - 1;

    if (x + 1 > maxx) maxx = x + 1;
    if (x - 1 < minx) minx = x - 1;

    if (y + 1 > maxy) maxy = y + 1;
    if (y - 1 < miny) miny = y - 1;
}


void SqrLimits::MaxMin(int page, int x, int y)
{
    if (page > maxpage)	maxpage = page;
    if (page < minpage) minpage = page;
    
    if (x > maxx) maxx = x;
    if (x < minx) minx = x;
    
    if (y > maxy) maxy = y;
    if (y < miny) miny = y;
}

void SqrLimits::Inverse(int size)
{
    maxpage = 0;
    minpage = size;
    maxx = 0;
    minx = size;
    maxy = 0;
    miny = size;
}

void SqrLimits::Init(int size)
{
    maxpage = size;
    minpage = 0;
    maxx = size;
    minx = 0;
    maxy = size;
    miny = 0;
}


#define HEADER_GEN "Generate definition"
#define VAL_GEN_SIZE "Size"
#define VAL_GEN_YZPLANE "YZ pages data"
#define VAL_GEN_XZPLANE "XZ pages data"
#define VAL_GEN_XYPLANE "XY pages data"


bool BuildWorld::loadFrom(MyFile *rdfl)
{
    if (rdfl->getState() != STATE_OPEN_READ)
        return false;

    clean(CLEAN_CLEAR); // clean it with the old limits.

    if (!rdfl->seekHeader(HEADER_GEN)) 
        return false;

    if (!rdfl->seekValue(VAL_GEN_SIZE))	
        return false;
    rdfl->readNums(3, &size.x, &size.y, &size.z);

    M_ASSERT((size.x == size.y) && (size.y == size.z));
    M_ASSERT(size.x <= BUILD_SIZE);

    m_limits.Init(size.x); 

    if (rdfl->seekValue("LimitsX"))
        rdfl->readNums(6, &m_limits[0].minpage, &m_limits[0].maxpage, &m_limits[0].minx, &m_limits[0].maxx, &m_limits[0].miny, &m_limits[0].maxy);
    if (rdfl->seekValue("LimitsY"))
        rdfl->readNums(6, &m_limits[1].minpage, &m_limits[1].maxpage, &m_limits[1].minx, &m_limits[1].maxx, &m_limits[1].miny, &m_limits[1].maxy);
    if (rdfl->seekValue("LimitsZ"))
        rdfl->readNums(6, &m_limits[2].minpage, &m_limits[2].maxpage, &m_limits[2].minx, &m_limits[2].maxx, &m_limits[2].miny, &m_limits[2].maxy);

    rdfl->seekValue(VAL_GEN_SIZE, 0, true); // reset the pointer to the start. if there's not limits, next seeks will fail

    int dim, page, x, y, rd, strts = 0;

    for(dim = 0; dim < 3; ++dim)
    {
        switch (dim)
        {
        case YZ_PLANE:
            if (!rdfl->seekValue(VAL_GEN_YZPLANE)) return false;
            break;
        case XZ_PLANE:
            if (!rdfl->seekValue(VAL_GEN_XZPLANE)) return false;
            break;
        case XY_PLANE:
            if (!rdfl->seekValue(VAL_GEN_XYPLANE)) return false;
            break;
        }
        for(page = m_limits[dim].minpage; page < m_limits[dim].maxpage; ++page)
        {
            for(x = m_limits[dim].minx; x < m_limits[dim].maxx ; ++x)
            {
                for(y = m_limits[dim].miny; y < m_limits[dim].maxy; ++y)
                {
                    if (rdfl->readNums(1, &rd) < 1) return false;
                    if (rd != 0)
                    {
                        switch (rd)
                        {
                        case 1:
                            set(dim, page, x, y, FACE_NORM);
                            break;
                        case 2:
                            set(dim, page, x, y, FACE_STRT);
                            strts++;
                            break;
                        }
                        nFaces++;
                    }
                    
                }
            }
        }
    }
    if (strts != 1)
        return false;
    //doTransparent(); not needed in boxed
    bootstrapSpace();
    justSave();
    justInvalidatedTest();

    return true;
}


void BuildWorld::bootstrapSpace()
{
    // possible optimization (not needed)
        // find the min max 3d points.. using the limits
        // initialize the bounded box to 1

    m_space.clear(BoundedBlock(1));

    int dim, page, x, y;

    for(dim = 0; dim < 3; ++dim)
    {
        const SqrLimits &dimlmt  = m_limits[dim];
        for(page = dimlmt.minpage; page < dimlmt.maxpage; ++page)
        {
            for(x = dimlmt.minx; x < dimlmt.maxx; ++x)
            {
                for(y = dimlmt.miny; y < dimlmt.maxy; ++y)
                {
                    if (GET_TYPE(get(dim, page, x, y)) == TYPE_REAL)
                    {
                        Vec3i g1, g2;
                        get3dCoords(CoordBuild(dim, page, x, y), g1, g2);
                        m_space.ErectWalls(dim, g1, g2);
                        // this somehow, automagically works.
                    }
                }
            }
        }
    }

    // you really need the following line for this gig to work. don't remove it.
    int volume = (m_space.szx * m_space.szy * m_space.szz) - m_space.FloodFill(0, 0, 0);


}




bool BuildWorld::saveTo(MyFile *wrfl)
{
    if (wrfl->getState() != STATE_OPEN_WRITE)
        return false;
    
    clean(CLEAN_TRANS_SHOW);
    reClacLimits();

    wrfl->writeHeader(HEADER_GEN);

    wrfl->writeValue(VAL_GEN_SIZE, false);
    wrfl->writeNums(3, true, size.x, size.y, size.z);

    wrfl->writeValue("LimitsX", false);
    wrfl->writeNums(6, true, m_limits[0].minpage, m_limits[0].maxpage, m_limits[0].minx, m_limits[0].maxx, m_limits[0].miny, m_limits[0].maxy);
    wrfl->writeValue("LimitsY", false);
    wrfl->writeNums(6, true, m_limits[1].minpage, m_limits[1].maxpage, m_limits[1].minx, m_limits[1].maxx, m_limits[1].miny, m_limits[1].maxy);
    wrfl->writeValue("LimitsZ", false);
    wrfl->writeNums(6, true, m_limits[2].minpage, m_limits[2].maxpage, m_limits[2].minx, m_limits[2].maxx, m_limits[2].miny, m_limits[2].maxy);


    int dim, page, x, y, wr;

    for(dim = 0; dim < 3; ++dim)
    {
        switch (dim)
        {
        case YZ_PLANE:
            wrfl->writeValue(VAL_GEN_YZPLANE, true);
            break;
        case XZ_PLANE:
            wrfl->writeValue(VAL_GEN_XZPLANE, true);
            break;
        case XY_PLANE:
            wrfl->writeValue(VAL_GEN_XYPLANE, true);
            break;
        }
        for(page = m_limits[dim].minpage; page < m_limits[dim].maxpage; ++page)
        {
            for(x = m_limits[dim].minx; x < m_limits[dim].maxx ; ++x)
            {
                for(y = m_limits[dim].miny; y < m_limits[dim].maxy; ++y)
                {
                    switch (get(dim, page, x, y))
                    {
                    case FACE_NORM:
                        wr = 1;
                        break;
                    case FACE_STRT:
                        wr = 2;
                        break;
                    case 0:
                        wr = 0;
                        break;
                    default:
                        wr = 0;
                    }
                    wrfl->writeNums(1, false, wr);
                }
                wrfl->writeNums(0, true);
            }
            wrfl->writeNums(0, true);
        }
    }

    //doTransparent(); not needed in boxed // reclaim the blue ones

    return true;
}


EGenResult BuildWorld::testShape()
{
    if (!m_bTested)
    {
        m_testResult = m_testShape.generate(this);
        m_bTested = true;

//		unGenerate(&m_testShape);//not used
    }
    return m_testResult;
}


/// Do the opposite action to generate(). take the shape and make a build from it.
/// currently not in use.
/// \see Shape::generate()
void BuildWorld::unGenerate(const Shape *shp, SqrLimits *bound)
{
    clean(BuildWorld::CLEAN_CLEAR);

    int s = FACE_STRT; // first one gets STRT
    // place it in the middle
    int offsetX = (size.x - (shp->size.x / 4))/2;
    int offsetY = (size.y - (shp->size.y / 4))/2;
    int offsetZ = (size.z - (shp->size.z / 4))/2;

    for(int i = 0; i < shp->fcn; ++i)
    {
        Shape::FaceDef &f = shp->faces[i];
        Vec3i ex = f.ex;
        ex.x = ex.x / 4 + offsetX; 
        ex.y = ex.y / 4 + offsetY;
        ex.z = ex.z / 4 + offsetZ;

        switch (f.dr)
        {
        case YZ_PLANE: set(f.dr, ex.x, ex.y, ex.z, s); break;
        case XZ_PLANE: set(f.dr, ex.y, ex.x, ex.z, s); break;
        case XY_PLANE: set(f.dr, ex.z, ex.x, ex.y, s); break;
        }
        s = FACE_NORM;

        Vec3i bex = ex * 4;
        bound->MaxMin(bex.z, bex.x, bex.y); // page plays z;
    }

    reClacLimits();
    bootstrapSpace();

    nFaces = shp->fcn;
    justGen();

    //recalc size?
}

