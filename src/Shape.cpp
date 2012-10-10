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

#include <QMessageBox>
#include <QLinkedList>
#include <stdio.h>



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
	sides = new SideDef[sdn];

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
	corners = new CornerDef[cnn];
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
	if (faces != NULL) delete[] faces;
	faces = NULL;
	if (sides != NULL) delete[] sides;
	sides = NULL;
	if (corners != NULL) delete[] corners;
	corners = NULL;
	if (errorSides != NULL) delete[] errorSides;
	errorSides = NULL;

	fcn = 0; sdn = 0; cnn = 0;
	sdnError = 0;
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


int Shape::locateFace(EPlane ldr, Vec3i lex) const
{
	if ((lex.x < 0) || (lex.y < 0) || (lex.z < 0) ||
		(lex.x >= size.x) || (lex.y >= size.y) || (lex.z >= size.z)) return -1;

	return m_opt_facesLoc[ldr].axx(lex, 4);
}

/// locate face when m_opt_facesLoc is invalidated (after generate completes) 
int Shape::locateFaceHardWay(EPlane ldr, Vec3i lex) const
{
	for(int i=0; i<fcn; ++i)
	{
		if ((faces[i].dr == ldr) &&
			(faces[i].ex.x == lex.x) &&
			(faces[i].ex.y == lex.y) &&
			(faces[i].ex.z == lex.z)) return i;
	}
	return -1;
}


int Shape::checkSide(EAxis ldr, int x, int y, int z, QLinkedList<SideDef> &slst, QLinkedList<SideDef> &slstError)
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
		slst.push_front(SideDef(ldr, Vec3i(x, y, z), nei));
		++sdn;
	}
	else if (nec > 2) 
	{
		slstError.push_front(SideDef(ldr, Vec3i(x, y, z), nei));
		++sdnError;
		return 0;	// found a side with more than 2 faces
	}
	return 1;
}

int Shape::checkCorner(int x, int y, int z, QLinkedList<CornerDef> &clst)
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
		if ((fcl = locateFace(lpln[i], lex[i])) != -1) nei[nec++] = fcl;
	}
	// limiting conditions to when it can be 3, 4, 5, 6	
	if ((nec >= 3) && (nec <= 6))
	{
		for(i=nec; i<12; ++i)
			nei[i] = -1;
		clst.push_front(CornerDef(Vec3i(x, y, z), nei));

		++cnn;
	}
	else
	{
		if (nec > 6) 
			return 0;	// found a corner with more than 6 faces
	}
	return 1;
}


/////////////////////////////// GENERATOR /////////////////////////////////////


void Shape::readAxis(const BuildWorld *build, int iss, int jss, int pgss, EPlane planedr, QLinkedList<FaceDef> &flst, int *reqfirst, SqrLimits &bound)
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

					if (GET_VAL(tmp) == FACE_STRT) *reqfirst = fcn;
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
int Shape::faceNeiFirstOpt(int whos, TransType trans[])
{
	int win = -1, wrank = 0, cand, rank; // winner, candidate
	int i, j;
	for(i = 0; i < 4; ++i)
	{
		cand = faces[whos].nei[i];
		if (trans[cand].to == -1)
		{
			rank = 0;
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


EGenResult Shape::reArrangeFacesDFS(FaceDef faces[], FaceDef revis[], TransType trans[])
{
	int revi = 0;
	int addi = 0;
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

	if (addi + 1 <  fcn) // addi points to the last index added, this is 1 less the the size
	{
		return GEN_RESULT_NOT_CONNECT;
	}
	return GEN_RESULT_OK;
}



EGenResult Shape::generate(const BuildWorld *build)
{
//	unsigned int s, t1, t2, t25, t3, t4, t5, t6;
//	s = GetTickCount();

	int i, j, x, y, z, reqf = -1;
	
	for (i = 0; i < 3; ++i)
	{
		if (!m_opt_facesLoc[i].isCreated())
			m_opt_facesLoc[i].create(BUILD_SIZE, BUILD_SIZE, BUILD_SIZE);
		m_opt_facesLoc[i].clear(-1); // clear face optimizing array
	}

	deallocate();
//	t1 = GetTickCount();

	Vec3i fcSize(build->size);

	rotfirst = false; // ####### change

	fcn = 0;
	sdn = 0;
	cnn = 0;
	sdnError = 0;

	SqrLimits bounds; //page plays z
	bounds.Inverse(BUILD_SIZE * 4);
		
	///////// BUILD FACES ///
	QLinkedList<FaceDef> flst;

	readAxis(build, fcSize.y, fcSize.z, fcSize.x, YZ_PLANE, flst, &reqf, bounds);
	readAxis(build, fcSize.x, fcSize.z, fcSize.y, XZ_PLANE, flst, &reqf, bounds);
	readAxis(build, fcSize.x, fcSize.y, fcSize.z, XY_PLANE, flst, &reqf, bounds);
	build->m_gen_bounds = bounds; // save the bounds of the last generate

//	t2 = GetTickCount();
	// fix it all according to the discovered bounds
	size.x = bounds.maxx - bounds.minx + 4 + 1; // need the +4 if we have a piece in the side with no cover
	size.y = bounds.maxy - bounds.miny + 4 + 1;
	size.z = bounds.maxpage - bounds.minpage + 4 + 1; // page plays z

	faces = new FaceDef[fcn];
	QLinkedList<FaceDef>::const_iterator faceit = flst.constBegin(); 
	for(i = 0; i < fcn; ++i)
	{	// convert the linked list to an array, NORMAIZE the coodrinates to the size of the array, and dispose of the list
		faces[i].dr = faceit->dr;
		faces[i].ex = Vec3i(faceit->ex.x - bounds.minx, faceit->ex.y - bounds.miny, faceit->ex.z - bounds.minpage);
		m_opt_facesLoc[faces[i].dr].axx(faces[i].ex, 4) = i; // build face optimizing array

		++faceit;
	}
	flst.clear();

//	t25 = GetTickCount();
	///////////// BUILD SIDES AND CORNERS ///////////////
	QLinkedList<SideDef> slst, slstError;
	QLinkedList<CornerDef> clst;

	bool illegalCorners = false; // sanity check for corners
	bool illegalSides = false;
	for(x = 0; x < size.x; x += 4)
	{
		for(y = 0; y < size.y; y += 4)
		{
			for(z = 0; z < size.z; z+= 4)
			{	
				// look for sides in the all axis
				// need a seperate if for each one so none will get skipped
				if (!checkSide(X_AXIS, x, y, z, slst, slstError))
					illegalSides = true;
				if (!checkSide(Y_AXIS, x, y, z, slst, slstError))
					illegalSides = true;
				if (!checkSide(Z_AXIS, x, y, z, slst, slstError)) 
					illegalSides = true;

				if (!(checkCorner(x, y, z, clst))) 
					illegalCorners = true;
			}
		}
	}

//	t3 = GetTickCount();
	// this shouldn't happen since it should have been discovered as illegal sides
	// illegal conrners can't happen without illegal corners
	if (illegalCorners && (!illegalSides))
		return GEN_RESULT_UNKNOWN;

	// if there are illegal sides they are in the list, vectorify them and exit

/////////////////////////////////////////////////////////////////////
// Vectorify
	if (illegalSides)
	{
		errorSides = new SideDef[sdnError];
		// convert the linked list to an array, and dispose of the list
		QLinkedList<SideDef>::const_iterator sideIt = slstError.constBegin();
		for(i = 0; i < sdnError; ++i) 
		{	
			errorSides[i] = *sideIt;
			errorSides[i].ex.x += bounds.minx; // un-normalize it to build space
			errorSides[i].ex.y += bounds.miny;
			errorSides[i].ex.z += bounds.minpage;
			++sideIt;
		}
		sdn = 0; cnn = 0;
		return GEN_RESULT_ILLEGAL_SIDE;
	}

	sides = new SideDef[sdn];
	// convert the linked list to an array, and dispose of the list
	QLinkedList<SideDef>::const_iterator sideIt = slst.constBegin();
	for(i = 0; i < sdn; ++i)
	{	
		sides[i] = *sideIt;
		++sideIt;
	}

	corners = new CornerDef[cnn];
	QLinkedList<CornerDef>::const_iterator cornerIt = clst.constBegin();
	for(i = 0; i < cnn; ++i)
	{	// convert the linked list to an array, and dispose of the list
		corners[i] = *cornerIt;
		++cornerIt;
	}

//	t4 = GetTickCount();
	
///////////////////////////////////////////////////////////
// transform faces order for a building sequence
	// first some sanity check to see that its possible (maybe could be done before)
	if (reqf == -1) 
		return GEN_RESULT_NO_START; // no start found
	reqf = fcn - reqf - 1; // order is upsidedown

	// to help the face travesal transform routine, set the face-sides information temporatly
	// needed so faceNei() could work properly in O(1)
	make_sides_facenei();

	// prepare ground for transform
	FaceDef *revis = new FaceDef[fcn];
	TransType *trans = new TransType[fcn];

	revis[0] = faces[reqf];
	trans[reqf].to = 0;
	trans[0].who = reqf; 

	// TBD-
	//     not randomised

	// call the transform
	EGenResult aret = reArrangeFacesDFS(faces, revis, trans);
	if (aret != GEN_RESULT_OK)
		return aret;

	// cleanup and transform for the sides and corners

	for(i = 0; i < sdn; ++i)
	{
		sides[i].nei[0] = trans[sides[i].nei[0]].to;
		sides[i].nei[1] = trans[sides[i].nei[1]].to;
	}

	for(i = 0; i < cnn; ++i)
	{
		for(j = 0; j < 6; ++j)
		{
			if (corners[i].nei[j] != -1)
				corners[i].nei[j] = trans[corners[i].nei[j]].to;
		}
	}

	delete[] faces;
	faces = revis; 

	delete[] trans;

//	t5 = GetTickCount();
///////////////////////////////////////////////////////////////////
// some additional tasks

	makeReverseNei();
	makeVolumeAndFacing();
	makePieceCheckBits();

//	t6 = GetTickCount();
//	QMessageBox::information(g_main, APP_NAME, QString("1=%1\n2=%2\n25=%3\n3=%4\n4=%5\n5=%6").arg(t1-s).arg(t2-t1).arg(t25-t2).arg(t3-t25).arg(t4-t3).arg(t5-t4).arg(t6-t5), QMessageBox::Ok);

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
		for (int j = 0; j < 4; ++j) {
			if (neiLow[(j-1+4)%4] && neiLow[j])
				fmask |= faceNeiMasker;
			faceNeiMasker <<= 4;
		}

		f.fmask = fmask;

		printf("%3d  mask=%04x\n", i, fmask);
	}
	printf("\n");
}


// for each face, save if it's facing outside or inside, for example
//    +--In--+
// Out|    In|
//    +-Out--+
bool Shape::makeVolumeAndFacing()
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

//	QMessageBox::information(NULL, APP_NAME, QString("old: %1").arg(volume), QMessageBox::Ok);
	
	for (i = 0; i < fcn; ++i)
	{
		faces[i].facing = (space.axx(faces[i].ex.x/4 + 1, faces[i].ex.y/4 + 1, faces[i].ex.z/4 + 1).fill)?FACING_IN:FACING_OUT;
		if (faces[i].dr == XY_PLANE) faces[i].facing = (EFacing)(1 - faces[i].facing); // switch it's direction.
		// I have no idea why that is like that.
	}
	return true;
}


void Shape::make_sides_facenei() {
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
bool Shape::makeReverseNei()
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

	return true;

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
	movedTo.fill(-1, fcn);
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

