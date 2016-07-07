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

#ifndef __CUBE_H_INCLUDED__
#define __CUBE_H_INCLUDED__

#include "Pieces.h"
#include "Shape.h"
#include "Solutions.h"
#include "CubeAcc.h"
#include "SlvCube.h"

#include "Configuration.h"
#include <chrono>

/** \file
	Declares the Cube class which holds the solution engine.
*/

class MyObject;
class LinesCollection;
class SolveContext;

/** Cube holds the solution engine which integrates Shape and PicsSet to produce instances of SlvCube.
	Cube is constructed on demand when the user presses the "Solve It!" button. it is
	constructed with a Shape just generated from the current BuildWorld and with a
	PicsSet produced from the current pieces selection. puttgr() is the entry point
	of the solution engine. it is called from within a SolveThread which is a worker thread
	dedicated for finding solutions. The solution engine output is fed through the Solutions instance
	given to puttgr() and signaled out through SolveThread's signals.
	An instance of this class is created in SolveThread::run() for the main solution engine. Another
	temporary instnce is created in SlvCube::genPainter() for creating line objects.
*/
class Cube
{
public:
	Cube(const Shape* shapeset, const PicsSet* picset, const EngineConf* conf); 
	~Cube();

    void initPuttgr(SolveContext *thread, SlvCube* starter);
    void puttgr(Solutions *slvs, SolveContext *thread, SlvCube* starter, int doSteps); // main function
	void prnSolves(Solutions *solve);

	// getSolveIFS has to run on a different cube then the solutions running cube since it's on another thread
	void genLinesIFS(SlvCube *slvc, LinesCollection &ifs);

	inline int &cub(int x, int y, int z) { return cub_[x + (xsz * y) + (xTysz * z)].val; }
	inline int cub(int x, int y, int z) const { return cub_[x + (xsz * y) + (xTysz * z)].val; }


private:
	/// CubeCell is the basic voxel value in the Cube 3D space.
	struct CubeCell
	{
		CubeCell(): val(0) {}
		int val;    // original cube value (summed upon putpic)
		            // while genIFS - the number of the face present in this cell
	};

	// TBD: use Space3D
	inline CubeCell &cell(int x, int y, int z) { return cub_[x + (xsz * y) + (xTysz * z)]; }
	int uncub(int p, int x, int y);

	void placeInto(int pntn, int f, Vec3 *shpp, Vec3 *pnti1, Vec3 *pnti2);

	bool isLineBlack(int p, int l);
	void placeSidePolygon(int b, MyObject& obj, int f, int curf, bool is1, int x, int y, bool backface);

	//void symetricGetNextRtn(int sc, int fc, int &nextrt);
	void clear(int cl = 0);

	inline void putPic(const int n, const int r, const int p);	// put piece n in place p
    inline void putPicArr(const PicArr& pmat, const int fc);

	inline void putorig(int p, int abs_sc, int abs_rt); 
	void rmvPic(const int p);
    void rmvPicArr(const PicArr& pmat, const int fc);

	//bool superCheck(const int fc);
	//bool anotherpic(ShapePlace &plcfc, int fc);
	//bool assemble(int fc);

	TPicBits getCurrentCirc(int fc);
	//bool makePossibilities(int fc, ShapePlace &plcfc);
	bool makePossibilities2(int fc, ShapePlace &plcfc);
	bool maskAssemble(int fc);

	SlvCube* generateConcreteSlv(SlvCube* starter);

	bool isGoodSym(const PicType &pt, int rti, int fc);

private:

	EngineConf lconf; ///< a local copy of the options

	/// this PicsSet was created on demand in CubeDoc::solveGo() according
	/// to the current piece selection.
	const PicsSet *pics; 
	const Shape *shape; ///< a pointer to the instance in CubeDoc

	const int xsz, ysz, zsz;
	const int xTysz; ///< the value of xsz times ysz. keep the sized close by for optimizations. 

	CubeCell *cub_;	///< main 3d ploting area
	vector<ShapePlace> plc;	///< places data, size sn, in each a size pn try bool array

	UsedPieces use;	///< pn sized array. which pieces are in use in this cube. 


/////////// code tables

	static const int whichKJ[3][2][3]; // optimization of add/remove

};


/** RunStats holds basic statistics and state for the solution engine.
	Currently the only statistic is the number of piece chages the engine 
	has performed and whether the engine is in 'lucky' state.
*/
struct RunStats
{
public:
	void reset() 
	{
		tms = 0;
		lucky = false;
		maxp = 0;
        maxpSlv.reset();
	}

	volatile mint64 tms = 0; ///< number of piece changes
	volatile bool lucky = false; ///< luck state of the engine.
	volatile int maxp = 0;  ///< the maximal place reached on the shape
    unique_ptr<SlvCube> maxpSlv; ///< a solution with the maximum place reached
};

// this is a reentrant context in which we can just run a few iterations of the solve engine and exit
class SolveContext
{
public:
    void setRuntime(Solutions *slvs, Shape *shp, PicsSet *pics, EngineConf* conf, SlvCube* starterSlv)
    {
        m_pics = pics;
        m_conf = conf;
        m_slvs = slvs;
        m_shp = shp;
        M_ASSERT(m_shp != nullptr);
        m_starterSlv = starterSlv;
    }

    bool isDone() const {
        return fExitnow || selfExit;
    }

    void init();
    void doRun(int doSteps);
    virtual void notifyLastSolution(bool firstInGo)
    {}
    virtual void notifyFullEnum() 
    {}
    virtual void notifyNotEnoughPieces()
    {}

    virtual void doStart()
    {}
    virtual void doWait()
    {}

public:
    // config
    bool m_keepPrevSlvs = false;

    // runtime environment
    PicsSet *m_pics = nullptr;
    Solutions *m_slvs = nullptr;
    Shape *m_shp = nullptr;
    EngineConf *m_conf = nullptr; // pointer to the m_conf in the document. copied on run()
    SlvCube* m_starterSlv = nullptr;

    // state management
    volatile int fExitnow = 0; // should be 1 or 0
    volatile bool fRunning = false;
    RunStats m_stats;

    // running state
    std::unique_ptr<Cube> m_rlcube;

    bool selfExit = false;
    int sessionSlvNum = 0; // session is a series of consequtive solves that are related
    int goSlvNum = 0; // go is the super session, is the entire thread
    int luckOffset = 0;
    int p = 0; // the piece index in the shape where we're currently putting a piece
    chrono::steady_clock::time_point lastRestart;
};


#endif // __CUBE_H_INCLUDED__

