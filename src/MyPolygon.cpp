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

#include "math.h"
#include "MyPolygon.h"
#include "Pieces.h"

#include "Texture.h"



void MyPolygon::calcNorm() //use the regular point and not the temp one
{
	// assume all points are on the same plane;
	bool normAgain = true;
	int pntA = 0, pntB = 1, pntC = 2;
	double nx, ny, nz;
	double xa, ya, za, xb, yb, zb, xc, yc, zc;

	while (normAgain)
	{	

		xa = vtx[pntA]->p[0], ya = vtx[pntA]->p[1], za = vtx[pntA]->p[2];
		xb = vtx[pntB]->p[0], yb = vtx[pntB]->p[1], zb = vtx[pntB]->p[2];
		xc = vtx[pntC]->p[0], yc = vtx[pntC]->p[1], zc = vtx[pntC]->p[2];
	
		
		nx = (ya-yb)*(za-zc)-(ya-yc)*(za-zb);
		ny = -((xa-xb)*(za-zc)-(xa-xc)*(za-zb));
		nz = (xa-xb)*(ya-yc)-(xa-xc)*(ya-yb);
		
		normAgain = ((fabs(nx) < 0.00000001) && (fabs(ny) < 0.00000001) && (fabs(nz) <0.00000001));
		if (normAgain)
		{
			if (pntC < 3)
			{
				pntC++;
			}
			else
			{
				if (pntB < 2)
				{
					pntB++;
					pntC = pntB + 1;
				}
				else
				{
					if (pntA < 1)
					{
						pntA++;
						pntB = pntA + 1;
						pntC = pntA + 2;
					}
					else
					{
						Q_ASSERT(false);
					}
				}
			}
		}
		Q_ASSERT((pntB < 4) && (pntC < 4));
		
	}
	double lng = sqrt(nx*nx + ny*ny + nz*nz);


	center[0] = nx / lng;
	center[1] = ny / lng;
	center[2] = nz / lng;

}
