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
#include "SlvCube.h"


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
