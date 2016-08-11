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

#include "Space3D.h"



// this is similar to scan line algorithm, only in 3d 
int BoundedBlockSpace3D::passFill()
{
    int vol = 0;
    for(int x = 0; x < szx; ++x) 
    {
        for(int y = 0; y < szy; ++y)
        {
            int start = -1;
            int fillValue = 1;
            for (int z = 0; z < szz; ++z)
            {
                auto& c = axx(x, y, z);
                if (c.back)
                    start = z;
                if (c.front) {
                    if (start != -1) {
                        // fill segment only if it ended
                        if (fillValue == 1) {
                            for(int r = start; r <= z; ++r) {
                                c.fill = fillValue;
                                vol += fillValue;
                            }
                        }
                        fillValue = 1 - fillValue; // swich scan line from fill to no fill
                    }
                    start = -1;
                }
            }
        }
    }
    return vol;
}

void BoundedBlockSpace3D::erectWalls(int dim, Vec3i front, Vec3i back)
{
	if ((front.x < 0) || (front.y < 0) || (front.z < 0) || (front.x >= szx) || (front.y >= szy) || (front.z >= szz) ||
		(back.x < 0) || (back.y < 0) || (back.z < 0) || (back.x >= szx) || (back.y >= szy) || (back.z >= szz))
		return;

	BoundedBlock &frontB = axx(front); 
	BoundedBlock &backB = axx(back);

	switch (dim) 
	{
	case XY_PLANE: frontB.back = true; backB.front = true; break;
	case YZ_PLANE: frontB.left = true; backB.right = true; break;
	case XZ_PLANE: frontB.up = true; backB.down = true; break;
	}
}



/*

int FloodFill(Space3D<BoundedBlock>& space, int x, int y, int z)
{
	if ((x < 0) || (y < 0) || (z < 0) || (x >= space.szx) || (y >= space.szy) || (z >= space.szz)) return 0;

	BoundedBlock& block = space.axx(x, y, z);
	if (block.fill != 1) return 0;
	
	block.fill = 0;
	int ret = 0;
	if (!block.back)  ret += FloodFill(space, x, y, z - 1);
	if (!block.front) ret += FloodFill(space, x, y, z + 1);
	if (!block.left)  ret += FloodFill(space, x - 1, y, z);
	if (!block.right) ret += FloodFill(space, x + 1, y, z);
	if (!block.up)    ret += FloodFill(space, x, y - 1, z);
	if (!block.down)  ret += FloodFill(space, x, y + 1, z);

	return ret + 1;
}
*/
