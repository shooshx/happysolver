#include "SlvCube.h"
#include "Shape.h"
#include "PicsSet.h"
#include "Pieces.h"
#include "Cube.h"


SlvCube::SlvCube(const vector<ShapePlace>& plc, const vector<ShapePlace>& abs_plc, const PicsSet *picset, const Shape *_shape) 
	: painter(NULL), shape(_shape)
{
	for(int j = 0; j < plc.size(); ++j)
	{
		// sc is the index in the added array of the set
		// rt is the absolute [0,7] rotation index
		dt.push_back(SlvPiece(abs_plc[j].sc, abs_plc[j].rt));
	}
	
	for(int j = 0; j < picset->added.size(); ++j)
	{
		int gind = -1, pind = -1;
		PicBucket::instance().getGP(picset->added[j].defInd, &gind, &pind);
		picdt.push_back(SlvPic(gind, pind));
	}

}

SlvCube::SlvCube(const Shape* _shape) 
	: painter(NULL), shape(_shape)
{
}


const PicGroupDef* SlvCube::getPieceGrpDef(int n) const 
{ 
	return dt[n].sdef->mygrp(); 
}


/*void SlvCube::genIFS(Shape *shp)
{
	if (ifs != NULL)
		return;
	// building this PicsSet and Cube here is the best choise for complete non-statefullness of Solutions
	PicsSet pics(picdt, picssz, bConsidersSym);  // this ctor takes from the bucket only the data, not the selection
	Cube tmpcube(shp, &pics);
	// why do we need another cube? maybe coz the other cube is busy with the thread.
	// maybe make a more permanent cube

	ShapeIFS *tmp = new ShapeIFS;
	tmpcube.genSolveIFS(this, *tmp); // create the ifs from the current cube
	ifs = tmp;
}*/

void SlvCube::genPainter()
{
	if (!painter.isNull())
		return;

	painter.setSlvCube(this); // make it not null only now.
	PicsSet pics(this);  // this ctor takes from the bucket only the data, not the selection
	// no need to consider symmetry, even if it was considered when this solution was built since we only take things
	// from added, which is not affected by symmetry

	for (int f = 0; f < dt.size(); ++f)
	{
		const AddedPic &ap = pics.added[dt[f].abs_sc]; 
		dt[f].sdef = &PicBucket::instance().pdefs[ap.defInd];
		//dt[f].rtindx = dt[f].rt; //pty->rtns[dt[f].rt].rtnindx;
	}

	painter.qmin = Vec3(0.0, 0.0, 0.0);
	painter.qmax = painter.qmin;
	for (int i = 0; i < shape->fcn; ++i)
	{
		painter.qmax.pmax(Vec3(shape->faces[i].ex + shape->faces[i].size()));
	}

	// lines
	Cube tmpcube(shape, &pics, NULL);
	tmpcube.genLinesIFS(this, painter.m_linesIFS);
}

#define VAL_SLV_NUMD "s#%d"
#define VAL1_SLV_SC "sc"
#define VAL1_SLV_RT "rt"
#define VAL1_SLV_GRP "grp"
#define VAL1_SLV_PIC "pic"
#define VAL1_SLV_NPIC "numpic"

#define VAL1_SLV_SYM "symm" // optional, defaults to 0



bool SlvCube::saveTo(MyFile *wrfl, int index)
{
	if (wrfl->getState() != STATE_OPEN_WRITE)
		return false;
	
	char str[20];
	sprintf(str, VAL_SLV_NUMD, index);
	wrfl->writeValue(str, true);
	
	int i;
	wrfl->writeValue(VAL1_SLV_SC, false, 1);
	for(i = 0; i < dt.size(); ++i)
	{
		wrfl->writeNums(1, false, dt[i].abs_sc);
	}
	wrfl->writeNums(0, true);
	wrfl->writeValue(VAL1_SLV_RT, false, 1);
	for(i = 0; i < dt.size(); ++i)
	{
		wrfl->writeNums(1, false, dt[i].abs_rt);
	}
	wrfl->writeNums(0, true);
	wrfl->writeValue(VAL1_SLV_SYM, false, 1);
	wrfl->writeNums(1, true, (int)false);
	wrfl->writeValue(VAL1_SLV_NPIC, false, 1);
	wrfl->writeNums(1, true, picdt.size());
	
	wrfl->writeValue(VAL1_SLV_GRP, false, 1);
	for(i = 0; i < picdt.size(); ++i)
	{
		wrfl->writeNums(1, false, picdt[i].gind);
	}
	wrfl->writeNums(0, true);
	wrfl->writeValue(VAL1_SLV_PIC, false, 1);
	
	for(i = 0; i < picdt.size(); ++i)
	{
		wrfl->writeNums(1, false, picdt[i].pind);
	}
	wrfl->writeNums(0, true);
	
	return true;
}

bool SlvCube::readFrom(MyFile *rdfl, int slvsz)
{
	dt.resize(slvsz);	
	int i;
	
	if (!rdfl->seekValue(VAL1_SLV_SC, 1)) return false;
	for(i = 0; i < slvsz; ++i)
	{
		if (rdfl->readNums(1, &dt[i].abs_sc) < 1) return false;
	}
	if (!rdfl->seekValue(VAL1_SLV_RT, 1)) return false;
	for(i = 0; i < slvsz; ++i)
	{
		if (rdfl->readNums(1, &dt[i].abs_rt) < 1) return false;
	}
	
	int nSym = 0; // default = 0
	if (rdfl->seekValue(VAL1_SLV_SYM, 1)) 
	{
		if (rdfl->readNums(1, &nSym) < 1) return false;
	}
	//bConsidersSym = (nSym != 0);

	if (!rdfl->seekValue(VAL1_SLV_NPIC, 1)) return false;
	int picssz = -1;
	if (rdfl->readNums(1, &picssz) < 1) return false;
	
	if (!rdfl->seekValue(VAL1_SLV_GRP, 1)) return false;
	
	picdt.resize(picssz);
	for(i = 0; i < picssz; ++i)
	{
		if (rdfl->readNums(1, &picdt[i].gind) < 1) return false;
	}
	if (!rdfl->seekValue(VAL1_SLV_PIC, 1)) return false;
	for(i = 0; i < picssz; ++i)
	{
		if (rdfl->readNums(1, &picdt[i].pind) < 1) return false;
	}
	
	return true;

}

					  

void SlvCube::transform(const TTransformVec &moveTo)
{
	vector<SlvPiece> newdt(dt.size());
	for(int i = 0; i < dt.size(); ++i)
	{
		newdt[moveTo[i]] = dt[i];
	}
	dt = std::move(newdt);
}
