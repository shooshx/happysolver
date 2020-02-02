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
#include "Shape.h"
#include <iostream>


void Solutions::clear(int solveSize)
{
	for(auto it = sv.begin(); it != sv.end(); ++it)
		delete *it;
	sv.clear();
	slvsz = solveSize;
	resetChangedFromSave();
}

bool filterOnlySinglePiecePerFam(SlvCube* slv) 
{
	set<int> seenGinds;
	for(const auto& slvpiece: slv->dt) {
		int gind = -1, pind = -1;
		PicBucket::instance().getGP(slvpiece.abs_sc, &gind, &pind);
		
		if (seenGinds.find(gind) != seenGinds.end()) {
			return false;
		}
		seenGinds.insert(gind);
	}
	return true;
}

bool Solutions::filterDup(SlvCube* slv) 
{
	for(const auto& slvpiece: slv->dt) {
		slv->sc_set.insert(slvpiece.abs_sc);
	}
	for(const auto* eslv: sv) {
		if (slv->sc_set == eslv->sc_set) {
			//cout << "found dup" << endl;
			return false;
		}
	}
	return true;
}

void Solutions::addBackCommon(SlvCube *tmp, bool keepOnlyOne)
{
//	M_ASSERT(tmp->slvsz == slvsz); // sanity check
//    cout << "SLV adding at " << sv.size() << " ET:" << tmp->debug_prn() << endl;
	if (true) {
		if (!filterOnlySinglePiecePerFam(tmp)) {
			delete tmp;
			return;
		}
		if (!filterDup(tmp)) {
			delete tmp;
			return;
		}
	}

    if (keepOnlyOne)
        sv.clear();
	sv.push_back(tmp);
	setChangedFromSave();
}

#define HEADER_SLV "Solutions"
#define VAL_SLV_SIZE "vector size"
#define VAL_SLV_COUNT "count"
#define VAL_SLV_NUMD "s#%d"


bool Solutions::saveTo(MyFile *wrfl, SlvCube *slv) // slv points to a specific solution to save or nullptr
{
	if (wrfl->getState() != STATE_OPEN_WRITE)
		return false;

	wrfl->writeHeader(HEADER_SLV);
	
	wrfl->writeValue(VAL_SLV_SIZE, false);
	wrfl->writeNums(1, true, slvsz);

	wrfl->writeValue(VAL_SLV_COUNT, false);

	if (slv != nullptr)
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
                cout << "reading solution " << indx << " failed";
				return false;
			}
			
			addBackCommon(slv);
		}
	}
	resetChangedFromSave();
    cout << "read " << count << " solutions" << endl;
	return true;
}


void Solutions::setChangedFromSave() 
{ 
	if (m_bChangedFromSave)
		return;
	m_bChangedFromSave = true; 
	if (changedFromSave)
		changedFromSave->notify(true);
}
void Solutions::resetChangedFromSave() 
{		
	if (!m_bChangedFromSave)
		return;
	m_bChangedFromSave = false; 
	if (changedFromSave)
		changedFromSave->notify(false);
}


void Solutions::transform(const TTransformVec &moveTo)
{
    for (auto& s : sv) {
		s->transform(moveTo);
	}
}

void Solutions::toNewShape(const Shape* newShp)
{
    if (sv.size() == 0)
        return;
    const Shape* oldShp = sv[0]->shape;
    vector<int> oldToNew(oldShp->fcn); // map faces index
    fill(oldToNew.begin(), oldToNew.end(), -1);
    Vec3i boundsDelta = newShp->buildBounds.getMin() - oldShp->buildBounds.getMin();
    //cout << "Bound-Delta=" << boundsDelta.x << "," << boundsDelta.y << "," << boundsDelta.z << endl;

    for(int oi = 0; oi < oldShp->fcn; ++oi)
    {
        // search for it in the new shape
        const auto& oface = oldShp->faces[oi];
        for(int ni = 0; ni < newShp->fcn; ++ni) 
        {
            const auto& nface = newShp->faces[ni];
            if (oface.ex - boundsDelta == nface.ex && oface.dr == nface.dr && oface.facing == nface.facing) {
                oldToNew[oi] = ni;
                break;
            }
        }
    }

    for(auto& s: sv) 
    {
        vector<SlvCube::SlvPiece> newdt(newShp->fcn);
        for (int i = 0; i < oldShp->fcn; ++i) {
            if (oldToNew[i] != -1)
                newdt[oldToNew[i]] = s->dt[i];
        }
        s->dt = newdt;
        s->shape = newShp;
        s->painter.makeMats();
    }
}
