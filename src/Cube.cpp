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

#include "general.h"
#include "Pieces.h"
#include "Shape.h"
#include "Solutions.h"
#include "Cube.h"
#include "CubeDocBase.h" // needed for SHINT_*
#include "PicsSet.h"
#include "SlvCube.h"

//#include "MyObject.h"

#ifdef QT_CORE_LIB
#include "SolveThread.h"
#endif

#include <time.h>
#include <iostream>
#include <chrono>

using namespace std;

// check the time every X tms changes
#define TIME_CHECK_TMS_INTERVAL 200000

/** SillyRand is a simple linear random number generator. Its main feature
    is that it is simple, fast and inline
    It is used in Cube::NovaAnotherpic() for the purpose of selecting the next
    piece to try fitting in the design structure.
*/
class SillyRand 
{
public:
    /// seed the PRNG seed.
    static inline void silly_rand_init(unsigned int seed) { g_seed = seed; }
    /// get the next random number.
    /// Although taking up an int, the numbers are 16 bits.
    /// the higher 16 bits are taken because they provide better randomality.
    static inline unsigned int silly_rand() { return ((g_seed = g_seed * 214013 + 2531011) >> 16); }
private:
    /// the actual seed value.
    static unsigned int g_seed; 
};

unsigned int SillyRand::g_seed = 76576567;  // silly rand default seed



void Cube::clear(int cl)
{	
    use.clear();

    int i;
    for(i = 0; i<shape->fcn; ++i)
    {
        //plc[i].clear();
        plc[i].allclear();
    }

    for(i = 0; i < shape->size.x; ++i)
    {
        for(int j = 0; j < shape->size.y; ++j)
        {
            for(int k = 0; k < shape->size.z; ++k)
            {
                cub(i, j, k) = cl;
            }
        }
    }
}


Cube::Cube(const Shape* shapeset, const PicsSet* picset, const EngineConf* conf) 
    :pics(picset), shape(shapeset), 
     xsz(shape->size.x), ysz(shape->size.y), zsz(shape->size.z), 
     xTysz(xsz * ysz), use(pics)
{
    if (conf != nullptr)
        lconf = *conf; // copy it to local copy

    cub_ = new CubeCell[xsz * ysz * zsz];

    plc.resize(shape->fcn); 
    for (int i = 0; i < shape->fcn; ++i)
    {
        //plc[i].tryd.realloc(pics->compSize());
        plc[i].mtryd.realloc(pics->totalRtnCount);
    }

    clear();
    cout << picset->added.size() << " pieces, " << picset->comp.size() << " distinct-pieces, " << pics->totalRtnCount << " rtns, " << plc.size() << " places in shape" << endl;
}	


Cube::~Cube()
{
    delete[] cub_;
}


const int Cube::whichKJ[3][2][3] = { {{0,0,1},{0,1,0}}, {{1,0,0},{0,0,1}}, {{1,0,0},{0,1,0}} };

//const int frameX[] = {0, 1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1, 0, 0, 0, 0 };
//const int frameY[] = {0, 0, 0, 0, 0, 1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1 };


void Cube::putPic(const int n, const int r, const int fc)
{
    int j, k, b;
    const PicArr &pmat = pics->comp[plc[fc].sc].rtns[plc[fc].rt];
    int tval = shape->faces[fc].ex.x + (xsz*shape->faces[fc].ex.y) + (xTysz*shape->faces[fc].ex.z);

    plc[fc].sc = n;
    plc[fc].rt = r;
    
    switch (shape->faces[fc].dr)
    {
    case YZ_PLANE:
        for (b = 0; b < 16; ++b)
        {
            k = frameX[b];	j = frameY[b];
            if (pmat.axx(k,j) != 0)
            {
                cub_[tval + j*xsz + k*xTysz].val += 1;
            }
        }
        break;
    case XZ_PLANE:
        for (b = 0; b < 16; ++b)
        {
            k = frameX[b];	j = frameY[b];
            if (pmat.axx(k,j) != 0)
            {
                cub_[tval + k + j*xTysz].val += 1;	
            }
        }
        break;
    case XY_PLANE:
        for (b = 0; b < 16; ++b)
        {
            k = frameX[b];	j = frameY[b];
            if (pmat.axx(k,j) != 0)
            {
                cub_[tval + k + j*xsz].val += 1;
            }
        }
        break;
    }
}


void Cube::rmvPic(const int fc)
{
    if (plc[fc].sc == -1) 
        return;

    int j, k, b;
    const PicArr &pmat = pics->comp[plc[fc].sc].rtns[plc[fc].rt];
    int tval = shape->faces[fc].ex.x + (xsz*shape->faces[fc].ex.y) + (xTysz*shape->faces[fc].ex.z);
    
    switch (shape->faces[fc].dr)
    {
    case YZ_PLANE:
        for (b = 0; b < 16; ++b)
        {
            k = frameX[b];	j = frameY[b];
            if (pmat.axx(k,j) != 0)
            {
                cub_[tval + j*xsz + k*xTysz].val -= 1;
            }
        }
        break;
    case XZ_PLANE:
        for (b = 0; b < 16; ++b)
        {
            k = frameX[b];	j = frameY[b];
            if (pmat.axx(k,j) != 0)
            {
                cub_[tval + k + j*xTysz].val -= 1;	
            }
        }
        break;
    case XY_PLANE:
        for (b = 0; b < 16; ++b)
        {
            k = frameX[b];	j = frameY[b];
            if (pmat.axx(k,j) != 0)
            {
                cub_[tval + k + j*xsz].val -= 1;
            }
        }
        break;
    }
}

TPicBits Cube::getCurrentCirc(int fc)
{
    M_ASSERT(plc[fc].sc == -1);
    PicArr pmat;
    int tval = shape->faces[fc].ex.x + (xsz*shape->faces[fc].ex.y) + (xTysz*shape->faces[fc].ex.z);
    int j, k, b;

    switch (shape->faces[fc].dr)
    {
    case YZ_PLANE:
        for (b = 0; b < 16; ++b)
        {
            k = frameX[b];	j = frameY[b];
            pmat.set(k,j) = cub_[tval + j*xsz + k*xTysz].val;
        }
        break;
    case XZ_PLANE:
        for (b = 0; b < 16; ++b)
        {
            k = frameX[b];	j = frameY[b];
            pmat.set(k,j) = cub_[tval + k + j*xTysz].val;
        }
        break;
    case XY_PLANE:
        for (b = 0; b < 16; ++b)
        {
            k = frameX[b];	j = frameY[b];
            pmat.set(k,j) = cub_[tval + k + j*xsz].val;
        }
        break;
    }
    return pmat.getBits();

}


bool Cube::isGoodSym(const PicType &pt, int rti, int fc)
{
    if (!pt.isSym )
        return true;

    Shape::EFacing dest = (lconf.nAsym == ASYM_OUTSIDE)?(Shape::FACING_OUT):(Shape::FACING_IN); 	

    return pt.rtns[rti].turned != (shape->faces[fc].facing == dest);

}


inline void TriedPieces::clear() {
    memset(&m_dt[0], 0, m_dt.size() * sizeof(m_dt[0]));
    cnt = 0;
}

typedef unsigned int uint;

bool Cube::makePossibilities2(int fc, ShapePlace &plcfc)
{
    // the current circumference state of the piece we're dealing with
    uint circ = getCurrentCirc(fc);
    uint fmask = shape->faces[fc].fmask;

    plcfc.possible.clear();
    //const vector<PicsSet::RtnInfo>& allRtn = pics->allRtn;
    int compCount = pics->comp.size();

    for(int ci = 0; ci < compCount; ++ci) 
    {
        if (use.allUsed( ci ))
            continue;
        const PicType &pt = pics->comp[ci];
        const int rtnnum = pt.rtnnum;
        for(int rti = 0; rti < rtnnum; ++rti) 
        {
            uint rtn = pt.bits[rti];
            // check if there are teeth that appear in both
            if ((circ & rtn) != 0)
                continue;
            // if the piece was to be in this place this is what it would cover
            uint cover = circ | rtn;
            // check that it matchs fmask
            if ((cover & fmask) != fmask)
                continue;
            if (pics->considerSymetric && !isGoodSym(pt, rti, fc)) 
                continue;
            plcfc.possible.push_back(TypeRef(ci, rti));
        }
    }
    return !plcfc.possible.empty();
}




// called to put a new piece in a place (where sc == -1)
// or to remove an existing pic and put anohter
bool Cube::maskAssemble(int fc)
{
    ShapePlace &plcfc = plc[fc];
    int selPoss = -1;
    if (plcfc.sc == -1) // there's nothing there
    {
        if (!makePossibilities2(fc, plcfc))
        {
            plcfc.mclear();
            return false;
        }
        // there are valid possibilities, we just created them
        if (lconf.fRand)
            selPoss = SillyRand::silly_rand() % plcfc.possible.size();
        else
            selPoss = 0; // TBD: random
    }
    else // something there
    {
        rmvPic(fc);
        use.subOne(plcfc.sc);
        int possCount = plcfc.possible.size();
        if (plcfc.mtryd.cnt == possCount) // no one else to try
        {
            plcfc.mclear();
            return false;
        }

        if (lconf.fRand)
            selPoss = SillyRand::silly_rand() % possCount;
        else
            selPoss = 1; // if we're here it means there is more than one possibility
        while (plcfc.mtryd.get(selPoss)) { // can be either one of any of the possible rtns in the set
            selPoss = (selPoss + 1) % possCount;
        }
    }

    TypeRef sel = plcfc.possible[selPoss];
    plcfc.mtryd.set(selPoss, true);
    plcfc.sc = sel.typeInd;
    plcfc.rt = sel.rtnInd;
    use.addOne(plcfc.sc);

    putPic(plcfc.sc, plcfc.rt, fc);
    return true;
}
  

#ifdef QT_CORE_LIB // TBD thread

void Cube::puttgr(Solutions *slvs, SolveThread *thread)
{
    bool sessionDone = false, selfExit = false;
    int sessionSlvNum = 0; // session is a series of consequtive solves that are related
    int goSlvNum = 0; // go is the super session, is the entire thread
    int luckOffset = qRound((lconf.nLuck * shape->fcn) / 100.0);
    // make sure its larger the 1 (if its there, we want it to be effective) and smaller the the maximum (just to be safe)
    luckOffset = mMin(shape->fcn, luckOffset);
    if (lconf.nLuck > 0)
        luckOffset = qMax(1, luckOffset);
    cout << "luck offset=" << luckOffset << endl;


    thread->m_stats.tms = 1;	// tms - times of replacing a piece
    thread->m_stats.lucky = false;

    chrono::steady_clock::time_point lastRestart = chrono::steady_clock::now();
    //QTime lastRestart = QTime::currentTime();

    clear();
    SillyRand::silly_rand_init(time(nullptr));

    int p = 0;		// p - the place we fill now (index to plc)
    while (!((p == 0) && (plc[0].mtryd.tryedAll())) && (!thread->fExitnow) && (!selfExit))
    {
/////////// actual work ///////////////////////////////////////////////////////
        if ((p != shape->fcn) && (maskAssemble(p)))
            p++;
        else 
            p--;	// if reached last piece backtrack
/////////// handle administration /////////////////////////////////////////////
    
        thread->m_stats.tms++;

        if (p > thread->m_stats.maxp)
            thread->m_stats.maxp = p;

        if ((lconf.fLuck) && (p >= shape->fcn - luckOffset))
            thread->m_stats.lucky = true; // luck parameter. TBD-maybe not to right now.. last session?
            
        if (p >= shape->fcn) //solution found
        {
            SlvCube *curslv = generateConcreteSlv();
            slvs->addBackCommon(curslv);
            ++goSlvNum;

            if (thread != nullptr)
            {
                if (goSlvNum == 1)
                    emit thread->solvePopUp(slvs->size() - 1); // go to the last entered

                emit thread->slvProgUpdated(SHINT_SOLUTIONS, slvs->size());
                thread->msleep(50); // to avoid starvation on the gui from outside input
            }

            switch (lconf.nPersist)
            {
            case PERSIST_ALL: break; // just continue
            case PERSIST_ONLY_FIRST: sessionDone = true; break;
            case PERSIST_UPTO:
                sessionSlvNum++;
                if (sessionSlvNum >= lconf.nUpto) sessionDone = true;
                break;
            } 

            if (sessionDone)
            {
                sessionDone = false;
                clear();
                p = 0;
                sessionSlvNum = 0;
            }

            if ((lconf.fAfter) && (goSlvNum >= lconf.nAfter))
            {
                selfExit = true;
            }

        }
        if (lconf.fRand && lconf.fRestart && ((thread->m_stats.tms % TIME_CHECK_TMS_INTERVAL) == 0)) // restart option
        {
            chrono::steady_clock::time_point curtime = chrono::steady_clock::now();
            chrono::milliseconds time_span = chrono::duration_cast<chrono::milliseconds>(curtime - lastRestart);
            //QTime curtime = QTime::currentTime();
            int interval = time_span.count();

            if (interval >= lconf.nRestart) 
            {
                if ( ((lconf.fLuck) && (!thread->m_stats.lucky)) || (!lconf.fLuck) )
                {
                    clear();
                    p = 0;
                    sessionSlvNum = 0;
                }
                thread->m_stats.lucky = false;
                lastRestart = curtime;
            }
        }
    }

    if ((p == 0) && (plc[0].mtryd.tryedAll()) && (goSlvNum == 0))
        emit thread->fullEnumNoSlv();

}

#endif

//////////////////////////////////////////////////////////////////////////


// take the data in the plc and deceminate it to real Pieces from the PicTypes
SlvCube* Cube::generateConcreteSlv() 
{
    // don't use vector<bool> since it is evil
    vector<vector<int> > used(pics->comp.size()); // how much of each PicType we used
    for(int i = 0; i < used.size(); ++i) {
        used[i].resize(pics->comp[i].addedInds.size());
        std::fill(used[i].begin(), used[i].end(), 0);
    }

    vector<ShapePlace> abs_plc(plc.size());
    for(int i = 0; i < plc.size(); ++i) 
    {
        int ti = plc[i].sc;
        const PicType& pt = pics->comp[ti];

        auto &ut = used[ti];
        int select = SillyRand::silly_rand() % ut.size();
        while (ut[select] != 0)
            select = (select + 1) % ut.size(); // must end since we placed a part
        ut[select] = 1;

        const PicType::AddedRef& added = pt.addedInds[select];
        abs_plc[i].sc = added.addedInd;
        // the order they were added to the solution is NO LONGER the same order as they are selected from the set.
        int dr = rotationAdd(pt.rtns[plc[i].rt].rtnindx, added.defRot);
        abs_plc[i].rt = dr;
    }

    return new SlvCube(plc, abs_plc, pics, shape);
}



void Cube::putorig(int p, int abs_sc, int abs_rt)
{
    plc[p].sc = abs_sc;
    plc[p].rt = abs_rt;
    
    const int cdr = shape->faces[p].dr; 
    const int cx = shape->faces[p].ex.x, cy = shape->faces[p].ex.y, cz = shape->faces[p].ex.z;
    int j, k, b;	
    //const PicArr &pmat = pics->comp[sc].rtns[rt];
    const PicArr &pmat = pics->getDef(abs_sc)->defRtns[abs_rt];
    
    for (b = 0; b < 16; ++b)
    {
        j = frameX[b];
        k = frameY[b];
        
        if (pmat.axx(k,j))
        {
            cub(cx + whichKJ[cdr][0][0] * k,
                cy + whichKJ[cdr][1][1] * j,
                cz + whichKJ[cdr][0][2] * k + whichKJ[cdr][1][2] * j) = p; //csc; // number of the pic in the pic structure
        }
    }
}

int Cube::uncub(int p, int k, int j)
{
    const int cx = shape->faces[p].ex.x, cy = shape->faces[p].ex.y, cz = shape->faces[p].ex.z;
    const int cdr = shape->faces[p].dr;

    return 	cub(cx + whichKJ[cdr][0][0] * k,
                cy + whichKJ[cdr][1][1] * j,
                cz + whichKJ[cdr][0][2] * k + whichKJ[cdr][1][2] * j);
}

/////////////////////////////// INGENIOUS VOODOO ////////////////////////////


// this is really where most of the magic happens.
void Cube::placeInto(int pntn, int f, Vec3 *shpp, Vec3 *pnti1, Vec3 *pnti2)
{
    int i;
    int dr = shape->faces[f].dr;

    for (i = 0; i < pntn; ++i)
    {
        Vec3 &p = pnti1[i];
        p[0] = double(shape->faces[f].ex.x + (shpp[i][0] * (dr == XY_PLANE)) + (shpp[i][0] * (dr == XZ_PLANE)) + (shpp[i][2] * (dr == YZ_PLANE)));
        p[1] = double(shape->faces[f].ex.y + (shpp[i][1] * (dr == XY_PLANE)) + (shpp[i][2] * (dr == XZ_PLANE)) + (shpp[i][1] * (dr == YZ_PLANE)));
        p[2] = double(shape->faces[f].ex.z + (shpp[i][2] * (dr == XY_PLANE)) + (shpp[i][1] * (dr == XZ_PLANE)) + (shpp[i][0] * (dr == YZ_PLANE)));

        Vec3 &np = pnti2[pntn - i - 1];
        np = p;
        np[0] += (1*(dr == YZ_PLANE)); 
        np[1] += (1*(dr == XZ_PLANE));
        np[2] += (1*(dr == XY_PLANE));

    }	
}


class MeshLinesAdder 
{
public:
    MeshLinesAdder(Mesh* m) : m_mesh(m), m_rep(&m->m_vtx) {
        m_mesh->m_type = Mesh::LINES;
        m_mesh->m_hasIdx = true;
    }

    void addLine(const Vec3& a, const Vec3& b, bool isBlack, ELineType type) 
    {
        if (!isBlack || (type != LINE_ALWAYS && type != LINE_ONLY_WHOLE))
            return;

        int ia = 0, ib = 0;
        m_rep.add(a, &ia);
        m_rep.add(b, &ib);
        m_mesh->m_idx.push_back(ia);
        m_mesh->m_idx.push_back(ib);
    }

private:
    Mesh *m_mesh;
    VecRep m_rep;
};


bool Cube::isLineBlack(int p, int l)
{
    bool isBlack = false;
    EBlackness pb = (p == -1)?BLACK_NOT:(pics->getDef(p)->mygrp()->blackness);
    EBlackness lb = (l == -1)?BLACK_NOT:(pics->getDef(l)->mygrp()->blackness);

    if ((p != -1) && (l != -1))
        isBlack = (pb > BLACK_NOT) && (lb > BLACK_NOT);
    else if (p != -1)
        isBlack = (pb > BLACK_BOTH); 
    else if (l != -1)
        isBlack = (lb > BLACK_BOTH);

    return isBlack;
}


// move to slvpainter.cpp
// expected that the ifs is clear
void Cube::genLinesIFS(SlvCube *slvc, LinesCollection &ifs)
{ 
    int f, p, b, lp;
    Vec3 pnti1[4], pnti2[4];
    Vec3 shpp[4];
    int curf, curp, lcurp;


    clear(-1);
    for (f = 0; f < shape->fcn; ++f) 
    {
        auto df = slvc->dt[f];
        putorig(f, df.abs_sc, df.abs_rt); // rt is not used
    }
    
    ifs.resize(shape->fcn);

    for (f = 0; f < shape->fcn; ++f)
    {
        curf = plc[f].sc; // curf - the pic that is placed in this side

        MeshLinesAdder obj(&ifs[f]);
        //MyObject &obj = ifs[f];
        //obj.setNakedLineColor(getLineColor(curf, -1));

        p = uncub(f, PicDisp::build[15].pnt.x, PicDisp::build[15].pnt.y);
        curp = (p != -1)?plc[p].sc:-1; // restart the lcurp
        //ASSERT(curp != -1);
        for (b = 0; b < 16; ++b)
        {
            lp = p;
            lcurp = curp; // curp - the pic that this specific small-block belongs too.
            p = uncub(f, PicDisp::build[b].pnt.x, PicDisp::build[b].pnt.y); // the face number in this slot

            curp = (p != -1)?plc[p].sc:-1; 

            if (lcurp != curp)
            { // seperation lines - prependiculars
                bool linecol = isLineBlack(curp, lcurp);

                shpp[0][0] = PicDisp::build[b].ln[0].x; shpp[0][1] = PicDisp::build[b].ln[0].y; shpp[0][2] = 0.0;
                shpp[1][0] = PicDisp::build[b].ln[1].x; shpp[1][1] = PicDisp::build[b].ln[1].y; shpp[1][2] = 0.0;

                placeInto(2, f, shpp, pnti1, pnti2);

                pnti1[0].z -= 1; // I have no idea why these are needed but for some reason they are
                pnti1[1].z -= 1;
                pnti2[0].z -= 1;
                pnti2[1].z -= 1;

                ELineType t = ((curp == curf) || (lcurp == curf))?(LINE_ALWAYS):(LINE_ONLY_WHOLE);
                obj.addLine(pnti1[0], pnti1[1], linecol, t);
                obj.addLine(pnti2[0], pnti2[1], linecol, t);
                
                if (t != LINE_ONLY_WHOLE) // don't do it if the pair is not real (it will be in the air...)
                {
                    ELineType v = ((curp != -1) && (lcurp != -1))?(LINE_ONLY_LONE):(LINE_ALWAYS);
                    obj.addLine(pnti1[0], pnti2[1], linecol, v);
                }
            }
            if ((curp != curf) && (PicDisp::build[b].bot[0].x != -1)) // bottom seperating lines
            {                     // ^^ basically that its not a corner
                bool linecol = isLineBlack(curp, curf);

                shpp[0][0] = PicDisp::build[b].bot[0].x; shpp[0][1] = PicDisp::build[b].bot[0].y; shpp[0][2] = 0.0;
                shpp[1][0] = PicDisp::build[b].bot[1].x; shpp[1][1] = PicDisp::build[b].bot[1].y; shpp[1][2] = 0.0;

                placeInto(2, f, shpp, pnti1, pnti2);

                pnti1[0].z -= 1;
                pnti1[1].z -= 1;
                pnti2[0].z -= 1;
                pnti2[1].z -= 1;

                obj.addLine(pnti1[0], pnti1[1], linecol, LINE_ALWAYS);
                obj.addLine(pnti2[0], pnti2[1], linecol, LINE_ALWAYS);
            }

        }

    }

    //ifs.vectorify();


}

///////////////////////////// END INGENIOUS VOODOO ///////////////////////////



#if 0
void Cube::symetricGetNextRtn(int sc, int fc, int &nextrt)
{
    TBD
    if (!pics->pics[sc].thegrp()->isIndividual() )
        return;

    const PicType& ppp = pics->pics[sc];
    Shape::EFacing dest = (lconf.nAsym == ASYM_OUTSIDE)?(Shape::FACING_OUT):(Shape::FACING_IN); 	

    while ( (nextrt < ppp.rtnnum) && (ppp.rtns[nextrt].turned != (shape->faces[fc].facing == dest)) )
       ++nextrt;

}


bool Cube::makePossibilities(int fc, ShapePlace &plcfc)
{
    // the current circumference state of the piece we're dealing with
    TPicBits circ = getCurrentCirc(fc);
    TPicBits fmask = shape->faces[fc].fmask;

    plcfc.possible.clear();
    const vector<PicsSet::RtnInfo>& allRtn = pics->allRtn;
    int allRtnCount = allRtn.size();

    for(int ri = 0; ri < allRtnCount; ++ri) 
    {
        const PicsSet::RtnInfo& rtnRi = allRtn[ri];
        if (use.allUsed( rtnRi.ref.typeInd )) {
            continue;
        }
        TPicBits rtn = allRtn[ri].bits;
        // check if there are teeth that appear in both
        if ((circ & rtn) != 0)
            continue;
        // if the piece was to be in this place this is what it would cover
        TPicBits cover = circ | rtn;
        // check that it matchs fmask
        if ((cover & fmask) != fmask)
            continue;
        plcfc.possible.push_back(allRtn[ri].ref);
    }
    return !plcfc.possible.empty();
}


bool Cube::assemble(int fc)
{
    bool forcein = true;

    ShapePlace &plcfc = plc[fc];

    if (plcfc.sc == -1)   // not occupied 
    {
        if (!anotherpic(plcfc, fc))
            return false; // nothing find to occupy it
        forcein = false;
    }
    else
        rmvPic(fc); // it came from the outside, meaning it was commited to the cube

    const int curScRtnnum = pics->comp[plcfc.sc].rtnnum; // optimization

    while (forcein || (!superCheck(fc))) 
    {	// there is someone who needs to be removed in this place
        ++plcfc.rt;
        if (pics->bConsiderSymetric)
        {
            symetricGetNextRtn(plcfc.sc, fc, plcfc.rt);
        }

        // rotate it // as long as there are avail rotations
        if ( !((plcfc.rt < curScRtnnum) && ((fc != 0) || (shape->rotfirst))) )
        { // change it to something else
            use.subOne(plcfc.sc);
            if (!anotherpic(plcfc, fc)) // any other pics wanna try this out?
                return false; // no? regress back
            // yes? carry on rotations
        }

        forcein = false;
    }
    putPic(plcfc.sc, plcfc.rt, fc); // commit to the cube
    return true;
}

/// check if the is an index in which both lists have a value of false.
static bool unusedPiecesExist(const TriedPieces& tried, const UsedPieces& used)
{
    M_ASSERT(tried.size() == used.size());

    for (int i = 0; i < tried.size(); ++i)
    {
        if (!tried.get(i) && !used.allUsed(i))
            return true;
    }
    return false;
}


bool Cube::anotherpic(ShapePlace &plcfc, int fc)
{
    if (unusedPiecesExist(plcfc.tryd, use)) // is there a point looking for one (didn't try all of them)
    {   // choose piece
        int sc = 0, nextrt = 0; // sc - not rand, sequential - start from 0
        if (lconf.fRand)
        {
            sc = SillyRand::silly_rand() % pics->compSize(); //pn - number of pics
        }
        while ( plcfc.tryd.get(sc) || use.allUsed(sc)) 
        {
            ++sc;
            if (sc == pics->compSize()) 
                sc = 0;
        }
        // choose first rotation
        if (pics->bConsiderSymetric)
        {
            symetricGetNextRtn(sc, fc, nextrt);
        }
        // put it
        plcfc.sc = sc;
        plcfc.rt = nextrt;

        use.addOne(sc);
        plcfc.tryd.set(sc, true);
        //plcfc.tryd.multset(pics->pics[sc].rep, pics->pics[sc].repnum);

        return true;
    }
    else
    {
        plcfc.sc = -1;
        plcfc.rt = 0;    // remove current one	
        plcfc.tryd.clear();

        return false;
    }
}



#define TYPE_SIDE 0
#define TYPE_CORNER 1

// check if the thing in plc[fc].sc/rt really fits there
bool Cube::superCheck(const int fc)
{
    register int j, k, b, ind = -1, type, cur, fcdr = shape->faces[fc].dr;
    const PicArr &pmat = pics->comp[plc[fc].sc].rtns[plc[fc].rt];
    int tval = shape->faces[fc].ex.x + (xsz*shape->faces[fc].ex.y) + (xTysz*shape->faces[fc].ex.z);

    for (b = 0; b < 16; ++b)
    {
        k = frameX[b];	j = frameY[b];

        int picval = pmat.axx(k,j);
        switch (fcdr)
        {
        case YZ_PLANE: picval += cub_[tval + j*xsz + k*xTysz].val; break;
        case XZ_PLANE: picval += cub_[tval + k + j*xTysz].val; break;
        case XY_PLANE: picval += cub_[tval + k + j*xsz].val; break;
        }

        if (picval > 1) // its stepped on more then once.
            return false; // just abort now

        if ((b & 0x3) == 0) { ++ind; type = TYPE_CORNER; } // b % 4 == 0
        else type = TYPE_SIDE;

        if (picval == 0) // its empty
        {
            if (type == TYPE_SIDE)
            {
                cur = shape->faces[fc].sides[ind];
                if (cur != -1)
                {
                    Shape::SideDef &side = shape->sides[cur];
                    if ((plc[side.nei[0]].sc != -1) && (plc[side.nei[1]].sc != -1)) 
                        return false;
                }
            }
            else // if it's corner
            {
                cur = shape->faces[fc].corners[ind];
                if (cur != -1)
                {
                    Shape::CornerDef &corner = shape->corners[cur];
                    k = 0; // reuse of j and k
                    for (j = 0; j < corner.numnei; ++j)
                    {
                        if (plc[corner.nei[j]].sc != -1) ++k;
                    }
                    if (k == corner.numnei) 
                        return false;
                }
            }
        }
    }
    return true;
}




#endif 