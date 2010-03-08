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
#include "Solutions.h"

#include "Cube.h"

// for the painter
#include "Shape.h" 
#include "GLWidget.h"

SlvCube::SlvCube(ShapePlace* plc, const PicsSet *picsc, const Shape *_shape) 
:slvsz(_shape->fcn), dt(NULL), picssz(picsc->size()), picdt(NULL), bConsidersSym(picsc->bConsiderSymetric), 
 painter(NULL), shape(_shape)
{
	Q_ASSERT(plc != NULL);

	int j;
	dt = new SlvPiece[slvsz];
	for(j = 0; j < slvsz; ++j)
	{
		dt[j].sc = plc[j].sc;
		dt[j].rt = plc[j].rt;
	}
	
	picdt = new SlvPic[picssz];
	for(j = 0; j < picssz; ++j)
	{
		picdt[j].gind = picsc->pics[j].gind;
		picdt[j].pind = picsc->pics[j].pind;
	}

}

SlvCube::SlvCube(const Shape* _shape) 
:slvsz(_shape->fcn), dt(NULL), picssz(-1), picdt(NULL), bConsidersSym(false), painter(NULL), shape(_shape)
{
}


SlvCube::~SlvCube()
{
	delete[] dt;
	delete[] picdt;
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

	for (int f = 0; f < slvsz; ++f)
	{
		PicType *pty = &pics.pics[dt[f].sc];
		dt[f].sdef = pty->thedef;
		dt[f].rtindx = pty->rtns[dt[f].rt].rtnindx;
	}

	painter.qmin = Coord3df(0.0, 0.0, 0.0);
	painter.qmax = painter.qmin;
	for (int i = 0; i < shape->fcn; ++i)
	{
		painter.qmax.pmax(Coord3df(shape->faces[i].ex + shape->faces[i].size()));
	}

	// lines
	Cube tmpcube(shape, &pics, NULL);
	tmpcube.genLinesIFS(this, painter.m_linesIFS);
}

void Solutions::clear(int solveSize)
{
	qDeleteAll(sv);
	sv.clear();
	slvsz = solveSize;
	resetChangedFromSave();
}



void Solutions::addBackCommon(SlvCube *tmp)
{
//	Q_ASSERT(tmp->slvsz == slvsz); // sanity check
	sv.push_back(tmp);
	setChangedFromSave();
}


#define HEADER_SLV "Solutions"
#define VAL_SLV_SIZE "vector size"
#define VAL_SLV_COUNT "count"

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
	for(i = 0; i < slvsz; ++i)
	{
		wrfl->writeNums(1, false, dt[i].sc);
	}
	wrfl->writeNums(0, true);
	wrfl->writeValue(VAL1_SLV_RT, false, 1);
	for(i = 0; i < slvsz; ++i)
	{
		wrfl->writeNums(1, false, dt[i].rt);
	}
	wrfl->writeNums(0, true);
	wrfl->writeValue(VAL1_SLV_SYM, false, 1);
	wrfl->writeNums(1, true, (int)bConsidersSym);
	wrfl->writeValue(VAL1_SLV_NPIC, false, 1);
	wrfl->writeNums(1, true, picssz);
	
	wrfl->writeValue(VAL1_SLV_GRP, false, 1);
	for(i = 0; i < picssz; ++i)
	{
		wrfl->writeNums(1, false, picdt[i].gind);
	}
	wrfl->writeNums(0, true);
	wrfl->writeValue(VAL1_SLV_PIC, false, 1);
	
	for(i = 0; i < picssz; ++i)
	{
		wrfl->writeNums(1, false, picdt[i].pind);
	}
	wrfl->writeNums(0, true);
	
	return true;
}

bool SlvCube::readFrom(MyFile *rdfl, int _slvsz)
{
	slvsz = _slvsz;
	
	dt = new SlvPiece[slvsz];	
	int i;
	
	if (!rdfl->seekValue(VAL1_SLV_SC, 1)) return false;
	for(i = 0; i < slvsz; ++i)
	{
		if (rdfl->readNums(1, &dt[i].sc) < 1) return false;
	}
	if (!rdfl->seekValue(VAL1_SLV_RT, 1)) return false;
	for(i = 0; i < slvsz; ++i)
	{
		if (rdfl->readNums(1, &dt[i].rt) < 1) return false;
	}
	
	int nSym = 0; // default = 0
	if (rdfl->seekValue(VAL1_SLV_SYM, 1)) 
	{
		if (rdfl->readNums(1, &nSym) < 1) return false;
	}
	bConsidersSym = (nSym != 0);

	if (!rdfl->seekValue(VAL1_SLV_NPIC, 1)) return false;
	if (rdfl->readNums(1, &picssz) < 1) return false;
	
	if (!rdfl->seekValue(VAL1_SLV_GRP, 1)) return false;
	
	picdt = new SlvPic[picssz];
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

					  

bool Solutions::saveTo(MyFile *wrfl, SlvCube *slv) // slv points to a specific solution to save or NULL
{
	if (wrfl->getState() != STATE_OPEN_WRITE)
		return false;

	wrfl->writeHeader(HEADER_SLV);
	
	wrfl->writeValue(VAL_SLV_SIZE, false);
	wrfl->writeNums(1, true, slvsz);

	wrfl->writeValue(VAL_SLV_COUNT, false);

	if (slv != NULL)
	{
		wrfl->writeNums(1, true, 1);
		wrfl->writeNums(0, true);
		slv->saveTo(wrfl, 0);
	}
	else
	{
		wrfl->writeNums(1, true, sv.size());
		for (int ind = 0; ind < sv.size(); ++ind)
		{
			sv[ind]->saveTo(wrfl, ind);
			wrfl->writeNums(0, true);
		}
	}
	resetChangedFromSave();
	return true;
}

bool Solutions::readFrom(MyFile *rdfl, const Shape* withShape)
{
	if (rdfl->getState() != STATE_OPEN_READ)
		return false;

	clear();

	if (!rdfl->seekHeader(HEADER_SLV)) return false;

	if (!rdfl->seekValue(VAL_SLV_SIZE)) return false;
	if (rdfl->readNums(1, &slvsz) < 1) return false;

	int count;
	if (!rdfl->seekValue(VAL_SLV_COUNT)) return false;
	if (rdfl->readNums(1, &count) < 1) return false;

	if (count > 0)
	{
		for (int indx = 0; indx < count; ++indx)
		{
			char str[20];
			sprintf(str, VAL_SLV_NUMD, indx);
			if (!rdfl->seekValue(str)) return false;
			
			SlvCube* slv = new SlvCube(withShape);
			if (!slv->readFrom(rdfl, slvsz))
			{
				delete slv;
				return false;
			}
			
			addBackCommon(slv);
		}
	}
	resetChangedFromSave();
	return true;
}


void Solutions::setChangedFromSave() 
{ 
	if (m_bChangedFromSave)
		return;
	m_bChangedFromSave = true; 
	emit changedFromSave(true);
}
void Solutions::resetChangedFromSave() 
{		
	if (!m_bChangedFromSave)
		return;
	m_bChangedFromSave = false; 
	emit changedFromSave(false);
}


void Solutions::transform(const TTransformVec &moveTo)
{
	for(TSlvList::iterator it = sv.begin(); it != sv.end(); ++it)
	{
		(*it)->transform(moveTo);
	}
}

void SlvCube::transform(const TTransformVec &moveTo)
{
	SlvPiece *newdt = new SlvPiece[slvsz];	
	for(int i = 0; i < slvsz; ++i)
	{
		newdt[moveTo[i]] = dt[i];
	}
	delete[] dt;
	dt = newdt;
}


void SlvPainter::paintPiece(int f, GLWidget* context, bool fTargets) const
{
	const PicDef *pdef = scube->dt[f].sdef;
	Shape::FaceDef *face = &scube->shape->faces[f];
	int rtnindx = scube->dt[f].rtindx;

	glPushMatrix();

	glTranslated(face->ex.x, face->ex.y, face->ex.z);

	switch (face->dr)
	{
	case XY_PLANE: 
		glTranslated(0, 0, 1);
		glRotated(90, 0, 1, 0); 
		break;
	case XZ_PLANE: 
		glRotated(90, 0, 1, 0);
		glRotated(90, 0, 0, 1); 
		break;
	case YZ_PLANE: 
		break;
	}

	glTranslatef(0.5, 2.5, 2.5);
	glRotated(rtnindx * -90, 1, 0, 0);

	if (rtnindx >= 4)
	{
		glRotated(180, 0, 0, 1);
		glRotated(90, 1, 0, 0);
	}

	glTranslatef(-0.5, -2.5, -2.5);

	glLoadName(f);
	pdef->painter.paint(fTargets, context);


	glPopMatrix();
}

void SlvPainter::paintLines(const MyObject &obj, bool singleChoise, GLWidget *context, ELinesDraw cfgLines) const
{
	if (context->isUsingLight())
		glDisable(GL_LIGHTING);

	glPolygonOffset(0.0, 0.0); // go forward, draw the lines

	glBegin(GL_LINES);

	for (int lni = 0; lni < obj.nLines; ++lni)
	{
		MyLine &curln = obj.lines[lni];

		if ( ((curln.type == MyLine::LINE_ONLY_LONE) && (!singleChoise)) || 
			 ((curln.type == MyLine::LINE_ONLY_WHOLE) && (singleChoise)) )
			continue;

		if ((cfgLines == LINES_WHITE) && (curln.color[0] < 0.1))
			continue;


		if (singleChoise) // means we're alone now
			glColor3f(obj.nakedLinesColor, obj.nakedLinesColor, obj.nakedLinesColor);
		else
			glColor3fv(curln.color.ptr());

		glVertex3fv(curln.p1->p.ptr());
		glVertex3fv(curln.p2->p.ptr());

	} // for lni

	glEnd();


	if (context->isUsingLight())
		glEnable(GL_LIGHTING);

}

void SlvPainter::paint(GLWidget* context, bool fTargets, int singleChoise, int upToStep, ELinesDraw cfgLines) const
{
	if (singleChoise < 0)
	{
		for (int f = 0; f < scube->slvsz; ++f)
		{
			if ((upToStep >= 0) && (f >= upToStep)) // step by step support
				break;

			paintPiece(f, context, fTargets);

			if ((!fTargets) && (cfgLines != LINES_NONE))
			{
				bool linesSingle = false;
				if (upToStep >= 0)  // step by step support
				{
					// this is somewhat of a PATCH that doesn't work completely well to make the edges have somewhat proper lines
					// since we don't want to regenerate to IFS for every stage (and even that doesn't work so well, see flat10x10)
					const int *knei = scube->shape->faces[f].nei;
					linesSingle = (knei[0] > upToStep) || (knei[1] > upToStep) || (knei[2] > upToStep) || (knei[3] > upToStep);
				}

				paintLines(m_linesIFS[f], linesSingle, context, cfgLines);
			}
		}
	}
	else
	{
		paintPiece(singleChoise, context, fTargets);
		if ((!fTargets) && (cfgLines == LINES_ALL)) // in single choise, do lines only if ALL (and not if BLACK)
			paintLines(m_linesIFS[singleChoise], true, context, cfgLines);
	}
}
