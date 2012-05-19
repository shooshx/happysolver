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
#include "CubeDoc.h" // needed for SHINT_*
#include "PicsSet.h"
#include "SlvCube.h"

#include "MyObject.h"
#include "SolveThread.h"

#include <time.h>
#include <iostream>
#include <assert.h>

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
	if (conf != NULL)
		lconf = *conf; // copy it to local copy

	cub_ = new CubeCell[xsz * ysz * zsz];

	plc.resize(shape->fcn); 
	for (int i = 0; i < shape->fcn; ++i)
	{
		//plc[i].tryd.realloc(pics->compSize());
		plc[i].mtryd.realloc(pics->totalRtnCount);
	}

	clear();
	cout << picset->addedSize() << " pieces, " << picset->compSize() << " distinct-pieces, " << pics->totalRtnCount << " rtns, " << plc.size() << " places in shape" << endl;
}	


Cube::~Cube()
{
	delete[] cub_;
}


const int Cube::whichKJ[3][2][3] = { {{0,0,1},{0,1,0}}, {{1,0,0},{0,0,1}}, {{1,0,0},{0,1,0}} };

const int frameX[] = {0, 1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1, 0, 0, 0, 0 };
const int frameY[] = {0, 0, 0, 0, 0, 1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1 };


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
	assert(plc[fc].sc == -1);
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

void Cube::symetricGetNextRtn(int sc, int fc, int &nextrt)
{
/*	TBD
	if (!isIndividual(pics->pics[sc].thegrp()->drawtype) )
		return;

	const PicType& ppp = pics->pics[sc];
	Shape::EFacing dest = (lconf.nAsym == ASYM_OUTSIDE)?(Shape::FACING_OUT):(Shape::FACING_IN); 	

	while ( (nextrt < ppp.rtnnum) && (ppp.rtns[nextrt].turned != (shape->faces[fc].facing == dest)) )
	   ++nextrt;
*/
}







inline void TriedPieces::clear() {
	//std::fill(m_dt.begin(), m_dt.end(), false);
	// 		for(int i = 0; i < m_dt.size(); ++i) {
	// 			m_dt[i] = 0;
	// 		}
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
		//selPoss = 1; // TBD: random
		if (lconf.fRand)
			selPoss = SillyRand::silly_rand() % possCount;
		else
			selPoss = 1; // if we're here it means there is more than one possibility
		while (plcfc.mtryd.get(selPoss)) {
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
  



void Cube::puttgr(Solutions *slvs, SolveThread *thread)
{
	bool sessionDone = false, selfExit = false;
	int sessionSlvNum = 0; // session is a series of consequtive solves that are related
	int goSlvNum = 0; // go is the super session, is the entire thread
	int luckOffset = qRound((lconf.nLuck * shape->fcn) / 100.0);
	// make sure its larger the 1 (if its there, we want it to be effective) and smaller the the maximum (just to be safe)
	luckOffset = qMin(shape->fcn, luckOffset);
	if (lconf.nLuck > 0)
		luckOffset = qMax(1, luckOffset);
	cout << "luck offset=" << luckOffset << endl;


	thread->m_stats.tms = 1;	// tms - times of replacing a piece
	thread->m_stats.lucky = false;

	QTime lastRestart = QTime::currentTime();

	clear();
	SillyRand::silly_rand_init(time(NULL));

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

			if (thread != NULL)
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
			QTime curtime = QTime::currentTime();
			int interval = lastRestart.msecsTo(curtime);

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

//////////////////////////////////////////////////////////////////////////


// take the data in the plc and deceminate it to real Pieces from the PicTypes
SlvCube* Cube::generateConcreteSlv() 
{
	vector<int> counts(pics->compSize()); // how much of each PicType we used
	std::fill(counts.begin(), counts.end(), 0);

	vector<ShapePlace> abs_plc(plc.size());
	for(int i = 0; i < plc.size(); ++i) 
	{
		int ti = plc[i].sc;
		const PicType& pt = pics->comp[ti];
		const PicType::AddedRef& added = pt.addedInds[counts[ti]];
		counts[ti]++;
		abs_plc[i].sc = added.addedInd;


		int br = pt.rtns[plc[i].rt].rtnindx; // base rotation
		int ds = added.defRot; // Piece definition start 
		int dr = -1; // def rotation
		if ((ds < 4) == (br < 4))
			dr = (br - ds + 4) % 4;
		else
			dr = ((br + ds) % 4) + 4;

		abs_plc[i].rt = dr;
	}

	return new SlvCube(plc, abs_plc, pics, shape);
}



void Cube::putorig(const int n, const int r, const int p)
{
	plc[p].sc = n;
	plc[p].rt = r;
	
	const int cdr = shape->faces[p].dr; //, csc = plc[p].sc; //, crt = plc[p].rt;
	const int cx = shape->faces[p].ex.x, cy = shape->faces[p].ex.y, cz = shape->faces[p].ex.z;
	int j, k, b;	
	const PicArr &pmat = pics->comp[plc[p].sc].rtns[plc[p].rt];
	
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
void Cube::placeInto(int pntn, int f, Coord3df *shpp, Coord3df *pnti1, Coord3df *pnti2)
{
	int i;
	int dr = shape->faces[f].dr;

	for (i = 0; i < pntn; ++i)
	{
		Coord3df &p = pnti1[i];
		p[0] = double(shape->faces[f].ex.x + (shpp[i][0] * (dr == XY_PLANE)) + (shpp[i][0] * (dr == XZ_PLANE)) + (shpp[i][2] * (dr == YZ_PLANE)));
		p[1] = double(shape->faces[f].ex.y + (shpp[i][1] * (dr == XY_PLANE)) + (shpp[i][2] * (dr == XZ_PLANE)) + (shpp[i][1] * (dr == YZ_PLANE)));
		p[2] = double(shape->faces[f].ex.z + (shpp[i][2] * (dr == XY_PLANE)) + (shpp[i][1] * (dr == XZ_PLANE)) + (shpp[i][0] * (dr == YZ_PLANE)));

		Coord3df &np = pnti2[pntn - i - 1];
		np = p;
		np[0] += (1*(dr == YZ_PLANE)); 
		np[1] += (1*(dr == XZ_PLANE));
		np[2] += (1*(dr == XY_PLANE));

	}	
}



float Cube::getLineColor(int p, int l)
{
	bool isBlack = false;
/*	EBlackness pb = (p == -1)?BLACK_NOT:(pics->pics[p].thegrp()->blackness);
	EBlackness lb = (l == -1)?BLACK_NOT:(pics->pics[l].thegrp()->blackness);

	if ((p != -1) && (l != -1))
		isBlack = (pb > BLACK_NOT) && (lb > BLACK_NOT);
	else if (p != -1)
		isBlack = (pb > BLACK_BOTH); 
	else if (l != -1)
		isBlack = (lb > BLACK_BOTH);
*/ // TBD
	if (isBlack)
		return 0.8f;
	return 0.0f;
}


// move to slvpainter.cpp
// expected that the ifs is clear
void Cube::genLinesIFS(SlvCube *slvc, LinesCollection &ifs)
{ 
	int f, p, b, lp;
	Coord3df pnti1[4], pnti2[4];
	Coord3df shpp[4];
	int curf, curp, lcurp;


	clear(-1);
	for (f = 0; f < shape->fcn; ++f) 
	{
		putorig(slvc->dt[f].sc, slvc->dt[f].rt, f);
	}
	
	for (f = 0; f < shape->fcn; ++f)
	{
		curf = plc[f].sc; // curf - the pic that is placed in this side

		MyObject &obj = ifs[f];
		obj.setNakedLineColor(getLineColor(curf, -1));

		p = uncub(f, PicPainter::build[15].pnt.x, PicPainter::build[15].pnt.y);
		curp = (p != -1)?plc[p].sc:-1; // restart the lcurp
		//ASSERT(curp != -1);
		for (b = 0; b < 16; ++b)
		{
			lp = p;
			lcurp = curp; // curp - the pic that this specific small-block belongs too.
			p = uncub(f, PicPainter::build[b].pnt.x, PicPainter::build[b].pnt.y); // the face number in this slot

			curp = (p != -1)?plc[p].sc:-1; 

			if (lcurp != curp)
			{ // seperation lines - prependiculars
				float linecol = getLineColor(curp, lcurp);

				shpp[0][0] = PicPainter::build[b].ln[0].x; shpp[0][1] = PicPainter::build[b].ln[0].y; shpp[0][2] = 0.0;
				shpp[1][0] = PicPainter::build[b].ln[1].x; shpp[1][1] = PicPainter::build[b].ln[1].y; shpp[1][2] = 0.0;

				placeInto(2, f, shpp, pnti1, pnti2);

				MyLine::ELineType t = ((curp == curf) || (lcurp == curf))?(MyLine::LINE_ALWAYS):(MyLine::LINE_ONLY_WHOLE);
				obj.addLine(&pnti1[0], &pnti1[1], linecol, linecol, linecol, t);
				obj.addLine(&pnti2[0], &pnti2[1], linecol, linecol, linecol, t);
				
				if (t != MyLine::LINE_ONLY_WHOLE) // don't do it if the pair is not real (it will be in the air...)
				{
					MyLine::ELineType v = ((curp != -1) && (lcurp != -1))?(MyLine::LINE_ONLY_LONE):(MyLine::LINE_ALWAYS);
					obj.addLine(&pnti1[0], &pnti2[1], linecol, linecol, linecol, v);
				}
			}
			if ((curp != curf) && (PicPainter::build[b].bot[0].x != -1)) // bottom seperating lines
			{                     // ^^ basically that its not a corner
				float linecol = getLineColor(curp, curf);

				shpp[0][0] = PicPainter::build[b].bot[0].x; shpp[0][1] = PicPainter::build[b].bot[0].y; shpp[0][2] = 0.0;
				shpp[1][0] = PicPainter::build[b].bot[1].x; shpp[1][1] = PicPainter::build[b].bot[1].y; shpp[1][2] = 0.0;

				placeInto(2, f, shpp, pnti1, pnti2);
				obj.addLine(&pnti1[0], &pnti1[1], linecol, linecol, linecol, MyLine::LINE_ALWAYS);
				obj.addLine(&pnti2[0], &pnti2[1], linecol, linecol, linecol, MyLine::LINE_ALWAYS);
			}

		}

	}

	ifs.vectorify();


}

///////////////////////////// END INGENIOUS VOODOO ///////////////////////////



#if 0

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
	Q_ASSERT(tried.size() == used.size());

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