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


// fill with zeros!
int BoundedBlockSpace3D::FloodFill(int x, int y, int z)
{
	if ((x < 0) || (y < 0) || (z < 0) || (x >= szx) || (y >= szy) || (z >= szz)) 
		return 0;

	if (axx(x, y, z).fill == 0) 
		return 0;

	axx(x, y, z).fill = 0;
	int ret = 1;

	// run the scan line
	int minz = z, maxz = z;
	while ((minz > 0) && (!axx(x, y, minz).back))
	{
		axx(x, y, --minz).fill = 0;
		++ret;
	}
	while ((maxz < szz-1) && (!axx(x, y, maxz).front))
	{
		axx(x, y, ++maxz).fill = 0;
		++ret;
	}

	for(z = minz; z <= maxz; ++z)
	{
		BoundedBlock& block =  axx(x, y, z);
		if (!block.left)  ret += FloodFill(x - 1, y, z);
		if (!block.right) ret += FloodFill(x + 1, y, z);
		if (!block.up)    ret += FloodFill(x, y - 1, z);
		if (!block.down)  ret += FloodFill(x, y + 1, z);
	}

	return ret;
}

void BoundedBlockSpace3D::ErectWalls(int dim, Coord3d front, Coord3d back)
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
