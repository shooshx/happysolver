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
    if (picset != nullptr) {
        for (int i = 0; i < shape->fcn; ++i)
            plc[i].mtryd.realloc(pics->totalRtnCount);
    }

    clear();
    //cout << picset->added.size() << " pieces, " << picset->comp.size() << " distinct-pieces, " << pics->totalRtnCount << " rtns, " << plc.size() << " places in shape" << endl;
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
    //const PicArr &pmat = pics->comp[plc[fc].sc].rtns[plc[fc].rt];
    const PicArr& pmat = pics->getComp(plc[fc].sc).rtns[plc[fc].rt];
    plc[fc].sc = n;
    plc[fc].rt = r;
    putPicArr(pmat, fc);
}

void Cube::putPicArr(const PicArr& pmat, const int fc)
{
    int j, k, b;
    int tval = shape->faces[fc].ex.x + (xsz*shape->faces[fc].ex.y) + (xTysz*shape->faces[fc].ex.z);

   
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
    if (plc[fc].sc == -1) // can be -2
        return;

    //const PicArr &pmat = pics->comp[plc[fc].sc].rtns[plc[fc].rt];
    const PicArr& pmat = pics->getComp(plc[fc].sc).rtns[plc[fc].rt];
    rmvPicArr(pmat, fc);
}

// for the case of a piece that was put there from the starter solution but is not in our current set
void Cube::rmvPicArr(const PicArr& pmat, const int fc)
{
    int j, k, b;
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

// TBD - make this more efficient by accessing bits directly
TPicBits Cube::getCurrentCirc(int fc)
{
    M_ASSERT(plc[fc].sc < 0); // -1 or -2
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



typedef unsigned int uint;


bool Cube::makePossibilities2(int fc, ShapePlace &plcfc)
{
    // the current circumference state of the piece we're dealing with
    uint circ = getCurrentCirc(fc);
    uint fmask = shape->faces[fc].fmask;

    plcfc.possible.clear();
    //const vector<PicsSet::RtnInfo>& allRtn = pics->allRtn;
    int compCount = pics->compRef.size();

    for(int ci = 0; ci < compCount; ++ci) 
    {
        if (use.allUsed( ci ))
            continue;
        //const PicType &pt = pics->comp[ci];
        const PicType &pt = pics->getComp(ci);
        const int rtnnum = pt.rtnnum;
        for(int rti = 0; rti < rtnnum; ++rti) 
        {
            uint rtn = pt.bits[rti];
            // check if there are teeth that appear in both
            if ((circ & rtn) != 0) {
                continue;
            }
            // if the piece was to be in this place this is what it would cover
            uint cover = circ | rtn;
            // check that it matchs fmask
            if ((cover & fmask) != fmask) { // does it cover everything?
                continue;
            }
            if (pics->considerSymetric && !isGoodSym(pt, rti, fc)) {
                continue;
            }
            plcfc.possible.push_back(TypeRef(ci, rti));
        }
    }
    return !plcfc.possible.empty();
}




// called to put a new piece in a place (where sc == -1)
// or to remove an existing pic and put anohter
bool Cube::maskAssemble(int fc)
{
    M_ASSERT(fc >= 0);
    ShapePlace &plcfc = plc[fc];
    int selPoss = -1;
    if (plcfc.sc < 0) // there's nothing there
    {
        if (plcfc.sc == -2)
        {  // it's a starter piece, need to remove it but there are not possibilities calculated yet
            M_ASSERT(plcfc.start_sc >= 0);
            auto pdef = PicBucket::instance().pdefs[plcfc.start_sc];
            const PicArr& pmat = pdef.defRtns[plcfc.start_rt];
            rmvPicArr(pmat, fc);
            plcfc.start_sc = -1;
            plcfc.start_rt = -1;
        }
        if (!makePossibilities2(fc, plcfc))
        {
            plcfc.mclear();
            return false;
        }
        // there are valid possibilities, we just created them
        if (lconf.fRand)
            selPoss = SillyRand::silly_rand() % plcfc.possible.size();
        else
            selPoss = 0; 
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

    auto sel = plcfc.possible[selPoss]; // ### reference??
    plcfc.mtryd.set(selPoss, true);

    plcfc.sc = sel.typeInd; // index in compRef
    plcfc.rt = sel.rtnInd;
    use.addOne(plcfc.sc);

    putPic(plcfc.sc, plcfc.rt, fc);
    return true;
}
  

void SolveContext::init()
{
    m_stats.reset();
    m_rlcube.reset(new Cube(m_shp, m_pics, m_conf));
    fRunning = true;
    m_rlcube->initPuttgr(this, m_starterSlv);
}

void Cube::initPuttgr(SolveContext *thread, SlvCube* starter)
{
    thread->selfExit = false;
    thread->sessionSlvNum = 0; // session is a series of consequtive solves that are related
    thread->goSlvNum = 0; // go is the super session, is the entire thread
    thread->luckOffset = mRound((lconf.nLuck * shape->fcn) / 100.0);
    // make sure its larger the 1 (if its there, we want it to be effective) and smaller the the maximum (just to be safe)
    thread->luckOffset = mMin(shape->fcn, thread->luckOffset);
    if (lconf.nLuck > 0)
        thread->luckOffset = mMax(1, thread->luckOffset);
    //cout << "luck offset=" << luckOffset << endl;

    thread->m_stats.tms = 1;	// tms - times of replacing a piece
    thread->m_stats.lucky = false;

    thread->lastRestart = chrono::steady_clock::now();

    clear();
    SillyRand::silly_rand_init(time(nullptr));

    int p = 0;		// p - the place we fill now (index to plc)
    if (starter != nullptr)
    { // set tiles from pervious solution
        while (true)
        {
            int abssc = starter->dt[p].abs_sc;
            if (abssc == -1) // put there by the default ctor
                break;
            int absrt = starter->dt[p].abs_rt;            

            ShapePlace &plcfc = plc[p];

            auto pdef = PicBucket::instance().pdefs[abssc];
            // search for this starter piece comp in my compRef
            int cri = 0;
            while (cri < pics->compRef.size() && pics->compRef[cri].allCompInd != pdef.indInAllComp)
                ++cri; 
            bool found = false;
            // search if this pdef is in the compRef we found (means this piece is part of this go)
            if (cri < pics->compRef.size()) {
                int crDefi = 0;
                const auto& cr = pics->compRef[cri];
                while (crDefi < cr.pdefsInds.size() && cr.pdefsInds[crDefi].pdefInd != abssc)
                    ++crDefi;
                found = crDefi < cr.pdefsInds.size();
            }

            bool didPlace = false; // can fail if we have less of a piece we used to have before
                                   // if this happens, we treat it like a foreign piece (-2). if the user
                                   // wants to get rid of it, she can remove the cube from the shape and readd it
            if (found)
            { // we're using something this is in our compRef, need to recreate sc and rt and possibilities

                int compsc = cri; //starter->dt[p].comp_sc;
                M_ASSERT(compsc >= 0);

                const auto& c = pics->getComp(cri);
                const auto& d = PicBucket::instance().pdefs[abssc];
                int comp_absrt = rotationSub(absrt, d.defRot); // starter def rotatation to comp rotation
                int rti = 0; // find what is the rti for the abs_rt of the comp that we take from the starter
                while (rti < c.rtnnum && c.rtns[rti].rtnindx != comp_absrt)
                    ++rti;

                //int comprt = starter->dt[p].comp_rt;

                bool okposs = makePossibilities2(p, plcfc);
                if (okposs)
                {
                    // check that the comp from starter is in the possibilities
                    int selPoss = 0;
                    for (; selPoss < plcfc.possible.size(); ++selPoss) {
                        const auto& ps = plcfc.possible[selPoss];
                        if (ps.typeInd == compsc && ps.rtnInd == rti)
                            break;
                    }
                
                    if (selPoss < plcfc.possible.size()) // although this piece is in the set, it was already used so it was not in the possibilites which take into account the count of the pieces
                    {
                        TypeRef sel = plcfc.possible[selPoss];
                        plcfc.mtryd.set(selPoss, true);
                        plcfc.sc = sel.typeInd;
                        plcfc.rt = sel.rtnInd;
                        use.addOne(plcfc.sc);

                        putPic(plcfc.sc, plcfc.rt, p);

                        plcfc.start_sc = abssc;
                        plcfc.start_rt = absrt;

                        plcfc.start_compsc = compsc;
                        plcfc.start_rti = rti;

                        didPlace = true;
                    }
                }
            }
            
            if (!didPlace)
            {
                plcfc.sc = -2; // means it's a piece from the starter
                plcfc.rt = -2;
                plcfc.start_sc = abssc;
                plcfc.start_rt = absrt;

                plcfc.start_compsc = -1;
                plcfc.start_rti = -1;

                const PicArr& pmat = pdef.defRtns[absrt];
                putPicArr(pmat, p);
            }
            // mark one instance of this pieces compRef as used, if it's in compRef

            ++p;
            if (p == shape->fcn)
                break; // shouldn't really happen
        };
    }
    //cout << "p= " << p << endl;
    thread->p = p;

}

void SolveContext::doRun(int doSteps)
{
    m_rlcube->puttgr(m_slvs, this, m_starterSlv, doSteps);
    if (isDone())
        fRunning = false; // needed here because the Stop button change depends on it
}


void Cube::puttgr(Solutions *slvs, SolveContext *thread, SlvCube* starter, int doSteps)
{
    int p = thread->p;
    int didSteps = 0;

    bool wasDone = (p == shape->fcn); // was done from the initialization, need to skip placing piece

    // p == -1 means we tried all possibilities from piece 0 and backtracked - this means there was a full enumeration
    while ((p != -1) && !thread->fExitnow && !thread->selfExit && didSteps != doSteps)
    {
/////////// actual work ///////////////////////////////////////////////////////
        if (!wasDone)
        {
            if ((p != shape->fcn) && maskAssemble(p))
                p++;
            else 
                p--;	// if reached last piece backtrack
        }
/////////// handle administration /////////////////////////////////////////////

    
        ++didSteps;
        ++thread->m_stats.tms;


        if (pics->picCount < shape->fcn - p) {// we suddently don't have enough pieces to fill all the places
            cout << "NOT-ENOUGH steps=" << didSteps << " count=" << pics->picCount << endl;
            thread->notifyNotEnoughPieces();
            thread->selfExit = true;
            break;
        }

        if (p > thread->m_stats.maxp) {
            thread->m_stats.maxp = p;
            thread->m_stats.maxpSlv.reset(generateConcreteSlv(starter));
        }

        if ((lconf.fLuck) && (p >= shape->fcn - thread->luckOffset))
            thread->m_stats.lucky = true; // luck parameter. TBD-maybe not to right now.. last session?
            
        if (p == shape->fcn) //solution found
        {
            SlvCube *curslv = generateConcreteSlv(starter);
            slvs->addBackCommon(curslv, lconf.nPersist == PERSIST_ONLY_ONE);
            ++thread->goSlvNum;

            thread->notifyLastSolution(thread->goSlvNum == 1);
            bool sessionDone = false;
            switch (lconf.nPersist)
            {
            case PERSIST_ALL: 
                break; // just continue
            case PERSIST_ONLY_FIRST: 
            case PERSIST_ONLY_ONE:
                sessionDone = true; 
                break;
            case PERSIST_UPTO:
                thread->sessionSlvNum++;
                if (thread->sessionSlvNum >= lconf.nUpto)
                    sessionDone = true;
                break;
            } 

            if (sessionDone)
            {
                sessionDone = false;
                clear();
                p = 0;
                thread->sessionSlvNum = 0;
            }
            cout << "done " << lconf.fAfter << " " << thread->goSlvNum << "/" << lconf.nAfter << endl;
            if (lconf.fAfter && thread->goSlvNum >= lconf.nAfter)
            {
                thread->selfExit = true;
            }

        }
        if (lconf.fRand && lconf.fRestart && ((thread->m_stats.tms % TIME_CHECK_TMS_INTERVAL) == 0)) // restart option
        {
            chrono::steady_clock::time_point curtime = chrono::steady_clock::now();
            chrono::milliseconds time_span = chrono::duration_cast<chrono::milliseconds>(curtime - thread->lastRestart);
            //QTime curtime = QTime::currentTime();
            int interval = time_span.count();

            if (interval >= lconf.nRestart) 
            {
                if ( ((lconf.fLuck) && (!thread->m_stats.lucky)) || (!lconf.fLuck) )
                {
                    clear();
                    p = 0;
                    thread->sessionSlvNum = 0;
                }
                thread->m_stats.lucky = false;
                thread->lastRestart = curtime;
            }
        }
    }

    if (p == -1 && thread->goSlvNum == 0) {
        if (thread->m_stats.maxpSlv.get() != nullptr) { // in case there's 0 pieces, there won't be a maxSlv
            slvs->addBackCommon(thread->m_stats.maxpSlv.release());
        }
        thread->notifyFullEnum();
        thread->selfExit = true;
    }

    thread->p = p;
}



//////////////////////////////////////////////////////////////////////////


// take the data in the plc and deceminate it to real Pieces from the PicTypes
SlvCube* Cube::generateConcreteSlv(SlvCube* starter)
{
    // don't use vector<bool> since it is evil
    vector<vector<int> > used(pics->compRef.size()); // how much of each PicType we used
    for(int ui = 0; ui < used.size(); ++ui) {
        used[ui].resize(pics->compRef[ui].pdefsInds.size());
        std::fill(used[ui].begin(), used[ui].end(), 0);
    }
    
    vector<ShapePlace> abs_plc(plc.size());
    int i = 0;
    if (starter != nullptr) 
    { // set tiles from previous solution
        for (; i < plc.size(); ++i)
        {
            // if we decided on something other than the piece in the starter slv
            if (plc[i].sc == -2)
            {
                int asc = plc[i].start_sc;
                if (asc == -1)
                    break;

                abs_plc[i].sc = asc;
                abs_plc[i].rt = plc[i].start_rt;
            }
            else if (plc[i].sc == -1)
            {
                continue;
            }
            else
            {
                if (plc[i].start_compsc == -1)
                    break;
                if (plc[i].sc != plc[i].start_compsc || plc[i].rt != plc[i].start_rti)
                    break;
                int asc = starter->dt[i].abs_sc;
                if (asc == -1)
                    break;

                // mark used
                int ti = plc[i].sc;
                const auto& pt = pics->compRef[ti];
                auto &ut = used[ti];
                // find the index of the actual piece placed in the starter solution in the list of pieces added in this compRef
                int select = 0;
                while (select < pt.pdefsInds.size() && pt.pdefsInds[select].pdefInd != asc)
                    ++select;
                M_ASSERT(select < pt.pdefsInds.size());
                ut[select] = 1;

                abs_plc[i].sc = plc[i].start_sc;
                abs_plc[i].rt = plc[i].start_rt;

            }
        }
    }
    //cout << "ci=" << i << endl;

    for(; i < plc.size(); ++i) 
    {
        int ti = plc[i].sc;
        if (ti == -1) // it's unoccupied
            continue;
        const PicType& pt = pics->getComp(ti);

        auto &ut = used[ti];
        int select = 0;
        if (lconf.fRand) 
            select = SillyRand::silly_rand() % ut.size();
        while (ut[select] != 0)
            select = (select + 1) % ut.size(); // must end since we placed a part
        ut[select] = 1; // a piece that was added multiple times, will be there multiple times

        const auto& added = pics->compRef[ti].pdefsInds[select]; //pt.addedInds[select];
        abs_plc[i].sc = added.pdefInd;
        // the order they were added to the solution is NO LONGER the same order as they are selected from the set.
        //cout << "A " << pt.rtns[plc[i].rt].rtnindx << endl;
        int a = plc[i].rt;
        int b = pt.rtns[plc[i].rt].rtnindx;
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

    const PicArr &pmat = PicBucket::instance().pdefs[abs_sc].defRtns[abs_rt];
    
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