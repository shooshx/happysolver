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

#include "PicPainter.h"
#include "Pieces.h"

#include "GLWidget.h"
#include "Configuration.h" // DisplayConf
#include <vector>
using namespace std;
 
MyAllocator PicPainter::g_smoothAllocator;

#define POLY_A 0x0001
#define POLY_B 0x0002
#define POLY_C 0x0004
#define POLY_D 0x0008
#define POLY_UA 0x0010
#define POLY_UB 0x0020
#define POLY_UC 0x0040
#define POLY_UD 0x0080

const PicPainter::BuildFrame PicPainter::build[17] = 
{ 
	{{0, 0}, {{0, 1}, {1, 1}}, {{-1, -1}, {-1, -1}}, true,  { POLY_C, POLY_A },  { POLY_D, POLY_B } }, //0 
	{{1, 0}, {{1, 0}, {1, 1}}, {{1, 1}, {2, 1}},     false, { POLY_A, POLY_UB }, { POLY_D, POLY_C } }, //1
	{{2, 0}, {{2, 0}, {2, 1}}, {{2, 1}, {3, 1}},     false, { POLY_A, POLY_UB }, { POLY_D, POLY_C } }, //2
	{{3, 0}, {{3, 0}, {3, 1}}, {{3, 1}, {4, 1}},   	 false, { POLY_A, POLY_UB }, { POLY_D, POLY_C } }, //3
	{{4, 0}, {{4, 0}, {4, 1}}, {{-1, -1}, {-1, -1}}, true,  { POLY_A, POLY_D },  { POLY_B, POLY_C } }, //4
	{{4, 1}, {{5, 1}, {4, 1}}, {{4, 1}, {4, 2}},     false, { POLY_D, POLY_UC }, { POLY_B, POLY_A } }, //5
	{{4, 2}, {{5, 2}, {4, 2}}, {{4, 2}, {4,  3}},    false, { POLY_D, POLY_UC }, { POLY_B, POLY_A } }, //6
	{{4, 3}, {{5, 3}, {4, 3}}, {{4, 3}, {4, 4}},     false, { POLY_D, POLY_UC }, { POLY_B, POLY_A } }, //7
	{{4, 4}, {{5, 4}, {4, 4}}, {{-1, -1}, {-1, -1}}, true,  { POLY_D, POLY_B },  { POLY_C, POLY_A } }, //8
	{{3, 4}, {{4, 5}, {4, 4}}, {{4, 4}, {3, 4}},     false, { POLY_B, POLY_UA }, { POLY_C, POLY_D } }, //9
	{{2, 4}, {{3, 5}, {3, 4}}, {{3, 4}, {2, 4}},     false, { POLY_B, POLY_UA }, { POLY_C, POLY_D } }, //10
	{{1, 4}, {{2, 5}, {2, 4}}, {{2, 4}, {1, 4}},     false, { POLY_B, POLY_UA }, { POLY_C, POLY_D } }, //11
	{{0, 4}, {{1, 5}, {1, 4}}, {{-1, -1}, {-1, -1}}, true,  { POLY_B, POLY_C },  { POLY_A, POLY_D } }, //12
	{{0, 3}, {{0, 4}, {1, 4}}, {{1, 4}, {1, 3}},     false, { POLY_C, POLY_UD }, { POLY_A, POLY_B } }, //13
	{{0, 2}, {{0, 3}, {1, 3}}, {{1, 3}, {1, 2}},     false, { POLY_C, POLY_UD }, { POLY_A, POLY_B } }, //14
	{{0, 1}, {{0, 2}, {1, 2}}, {{1, 2}, {1, 1}},     false, { POLY_C, POLY_UD }, { POLY_A, POLY_B } }, //15
	{{0, 0}, {{0, 0}, {0, 0}}, {{0, 0}, {0, 0}},     false, { 0, 0}, {0, 0} } //16
}; // last one is for wrap up with first. not really read



static const int half_orient_ind_pic[8][2] = { {0, 1}, {6, 4}, {7, 5}, {2, 3}, {3, 6}, {5, 0}, {4, 2}, {1, 7}};
static const int half_orient_data[8][4] = { {0, 1, 2, 3}, {3, 2, 1, 0}, {1, 2, 3, 0}, {0, 3, 2, 1}, {2, 1, 0, 3}, {1, 0, 3, 2}, {3, 0, 1, 2}, {2, 3, 0, 1}};


// return 1 or 0
bool PicPainter::uncub(int x, int y) const
{
	return (m_pdef->v.axx(x, y));
}

void PicPainter::PlaceInto(int pntn, Coord3df *shpp, Coord3df *pnti1, Coord3df *pnti2, EPlaceType type) const
{
	int i;

	for (i = 0; i < pntn; ++i)
	{
		Coord3df &p = pnti1[i];
		p[0] = double(shpp[i][2]);
		p[1] = double(shpp[i][1]);
		p[2] = double(shpp[i][0]);

		Coord3df &np = pnti2[pntn - i - 1];
		np = p;

		switch (type)
		{
		case PLACE_NORM: np[0] += 1; break;
		case PLACE_X: np[1] += 1; break;
		case PLACE_Y: np[2] += 1; break;
		}

	}	
}



void PicPainter::placeSidePolygon(MyObject& obj, int b, bool is1, int x, int y) const
{
	static Coord3df pnti1[4], pnti2[4], pntiA[4], pntiB[4], pntiC[4], pntiD[4];

	Coord3df shpp[4];
	TexAnchor ancs1[4], ancs2[4], baseancf[4], baseancs[4], ancsR1[4], ancsR2[4], ancsR3[4], ancsR4[4];
	int i;

	double bx, by, rbx, rby; // makers of texture coordinates.. fronts and side-patches

	rbx = bx = double(x)/5.0;
	rby = by = double(y)/5.0;

	if (m_pdef->mygrp->drawtype == DRAW_TEXTURE_INDIVIDUAL_HALF)
	{
		rbx = 0.0; rby = 0.0; // side patches of this has their own coordinates
	}

	baseancf[0] = TexAnchor(bx, by);
	baseancf[1] = TexAnchor(bx + 0.2, by);
	baseancf[2] = TexAnchor(bx + 0.2, by + 0.2);
	baseancf[3] = TexAnchor(bx, by + 0.2);
	
	baseancs[0] = TexAnchor(rbx, rby);
	baseancs[1] = TexAnchor(rbx + 0.2, rby);
	baseancs[2] = TexAnchor(rbx + 0.2, rby + 0.2);
	baseancs[3] = TexAnchor(rbx, rby + 0.2);

	if (m_pdef->mygrp->drawtype == DRAW_TEXTURE_INDIVIDUAL_HALF)
	{ // rescale it since the texture is a big one
		double xfact = m_pdef->mygrp->sideTex->img.width(), yfact = m_pdef->mygrp->sideTex->img.height();
		for(i = 0; i < 4; ++i) // pntn == 4
		{
			baseancs[i].x += ((double)(m_pdef->mygrp->sideTexX) / xfact);
			baseancs[i].y += ((double)(m_pdef->mygrp->sideTexY) / yfact);
		}
	}

	ancs1[0] = baseancf[0]; ancs2[0] = baseancf[3]; 
	ancs1[1] = baseancf[1]; ancs2[1] = baseancf[2]; 
	ancs1[2] = baseancf[2]; ancs2[2] = baseancf[1];
	ancs1[3] = baseancf[3]; ancs2[3] = baseancf[0];

	shpp[0] = Coord3df(x, y, 0); 	 shpp[1] = Coord3df(x+1, y, 0);
	shpp[2] = Coord3df(x+1, y+1, 0); shpp[3] = Coord3df(x, y+1, 0);
	PlaceInto(4, shpp, pnti1, pnti2, PLACE_NORM);

	if (is1)
	{ // polys that make the flat surface.
		obj.addPoly(pnti1, ancs1, NULL); //if its an out of place, it's textured
		obj.addPoly(pnti2, ancs2, m_pdef->tex);
	}

	/////////////////////// side patches //////////////////
	if (b < 0)
		return; // enough of this shananigan, we're not on the sides.

	if (m_pdef->mygrp->drawtype == DRAW_TEXTURE_INDIVIDUAL_HALF)
	{
		for (i = 0; i < 4; ++i)
		{
			ancsR1[half_orient_data[3][i]] = baseancs[i];
			ancsR2[half_orient_data[3][i]] = baseancs[i];

			ancsR3[half_orient_data[0][i]] = baseancs[i];
			ancsR4[half_orient_data[1][i]] = baseancs[i];
		}
	}
	else
	{  // randomly turned...
		for(i = 0; i < 4; ++i)
		{
			ancsR3[i] = ancsR1[i] = baseancs[3-i];
			ancsR4[i] = ancsR2[i] = baseancs[i];
		}
	}

	shpp[0] = Coord3df(x, y, 0);   shpp[1] = Coord3df(x, y, 1);
	shpp[2] = Coord3df(x+1, y, 1); shpp[3] = Coord3df(x+1, y, 0);
	PlaceInto(4, shpp, pntiA, pntiB, PLACE_X);

	shpp[0] = Coord3df(x, y, 0);   shpp[1] = Coord3df(x, y+1, 0);
	shpp[2] = Coord3df(x, y+1, 1); shpp[3] = Coord3df(x, y, 1);
	PlaceInto(4, shpp, pntiC, pntiD, PLACE_Y);

	int polyf = 0;
	if (is1)
	{
		if (build[b].corner)
			polyf |= build[b].pold.in1 | build[b].pold.in0;
		else
			polyf |= build[b].pold.in1;
		
		int prevb = (b == 0)?15:(b - 1), nextb = b + 1;

		if (uncub(build[prevb].pnt.x, build[prevb].pnt.y) != is1)
			polyf |= build[b].sds.prev0;
		if (uncub(build[nextb].pnt.x, build[nextb].pnt.y) != is1)
			polyf |= build[b].sds.next0;
	}
	else if (!build[b].corner)
	{
		polyf |= build[b].pold.in0;
	}


	if (polyf & POLY_A) obj.addPoly(pntiA, ancsR1, m_pdef->mygrp->sideTex); //if its an out of place, it's textured
	if (polyf & POLY_B) obj.addPoly(pntiB, ancsR2, m_pdef->mygrp->sideTex);
	if (polyf & POLY_C) obj.addPoly(pntiC, ancsR3, m_pdef->mygrp->sideTex); //if its an out of place, it's textured
	if (polyf & POLY_D) obj.addPoly(pntiD, ancsR4, m_pdef->mygrp->sideTex);
	qSwap(pntiA[1], pntiA[3]); qSwap(ancsR1[1], ancsR1[3]);
	qSwap(pntiB[1], pntiB[3]); qSwap(ancsR2[1], ancsR2[3]);
	qSwap(pntiC[1], pntiC[3]); qSwap(ancsR3[1], ancsR3[3]);
	qSwap(pntiD[1], pntiD[3]); qSwap(ancsR4[1], ancsR4[3]);
	if (polyf & POLY_UA) obj.addPoly(pntiA, ancsR1, m_pdef->mygrp->sideTex); //if its an out of place, it's textured
	if (polyf & POLY_UB) obj.addPoly(pntiB, ancsR2, m_pdef->mygrp->sideTex);
	if (polyf & POLY_UC) obj.addPoly(pntiC, ancsR3, m_pdef->mygrp->sideTex); //if its an out of place, it's textured
	if (polyf & POLY_UD) obj.addPoly(pntiD, ancsR4, m_pdef->mygrp->sideTex);


}


void PicPainter::init(const DisplayConf& dpc, GLWidget *mainContext)
{
	if (m_displayLst != 0)
		glDeleteLists(m_displayLst, 1);

	m_displayConf = dpc;

	MyObject obj(&g_smoothAllocator);
	
	generateStraightShape(dpc, obj);

	for(int i = 0; i < dpc.numberOfPasses; ++i)
	{
		obj.subdivide(dpc.passRound[i]);
	}
	obj.clacNormals(dpc.bVtxNormals);

	mainContext->makeCurrent();

	m_displayLst = glGenLists(1);
	glNewList(m_displayLst, GL_COMPILE);
	realPaint(obj, false, mainContext);
	glEndList();

	g_smoothAllocator.clear();
}

/// do the actual painting of a single piece in the actual OpenGL view.
/// call the display list of the piece.
void PicPainter::paint(bool fTargets, GLWidget *context) const
{
	glCallList(m_displayLst);
}

void PicPainter::realPaint(MyObject& obj, bool fTargets, GLWidget *context)
{
	glPolygonOffset(1.0, 1.0); // go backward, draw polygons TBD- move to slvpainter

	PicGroupDef *def = m_pdef->mygrp;
	bool hasTex = def->isTexExist();
	if (!fTargets)
	{
		int texId = 0;

		if ((m_pdef->tex != NULL) && (m_pdef->tex->ind < context->m_textures.size()) && (m_pdef->tex->ind != -1))
			texId = context->m_textures[m_pdef->tex->ind];
		else if ((def->tex != NULL) && (def->tex->ind < context->m_textures.size()) && (def->tex->ind != -1))
			texId = context->m_textures[def->tex->ind];

		switch (m_pdef->mygrp->drawtype) 
		{
			case DRAW_COLOR:
				glDisable(GL_TEXTURE_2D);
				glColor3f(def->r, def->g, def->b);
				break;
			case DRAW_TEXTURE_BLEND:
				glColor3f(1.0f, 1.0f, 1.0f);
			case DRAW_TEXTURE_NORM:
				glEnable(GL_TEXTURE_2D);
			//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); is default
				glBindTexture(GL_TEXTURE_2D, texId);
				break;
			case DRAW_TEXTURE_INDIVIDUAL_HALF:
			case DRAW_TEXTURE_INDIVIDUAL_WHOLE:
				// the enable texture comes later
				glDisable(GL_TEXTURE_2D);
			//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); is default
				//glBindTexture(GL_TEXTURE_2D, texId); sides have a different texture.
				break; // handle for every polygon
		}
	}

	int ch = 0;
	Texture *lastTex = NULL; // start disabled
	for(int pli = 0; pli < obj.nPolys; ++pli) //polygons
	{
		MyPolygon &curpl = *obj.poly[pli];

		if ((!fTargets) && (isIndividual(def->drawtype)) && (curpl.tex != lastTex))
		{
			if (curpl.tex == NULL)
			{
				glDisable(GL_TEXTURE_2D);
			}
			else
			{
				glEnable(GL_TEXTURE_2D); // sides have a different texture
				if ((curpl.tex->ind < context->m_textures.size()) && (curpl.tex->ind != -1))
					glBindTexture(GL_TEXTURE_2D, context->m_textures[curpl.tex->ind]);
			}
			lastTex = curpl.tex;
			++ch;
		}


		glBegin(GL_QUADS);
		for(int pni = 0; pni < 4; ++pni) //points
		{
			MyPoint *curpn = curpl.vtx[pni];
			
			if (obj.verterxNormals)
				glNormal3fv(curpn->n.ptr());
			else
				glNormal3fv(curpl.center.ptr());

			if (!fTargets)
			{
				// in blend and color, stay with the same color all the way.
				if ((def->drawtype != DRAW_TEXTURE_BLEND) && (def->drawtype != DRAW_COLOR)) 
				{
					if (isIndividual(def->drawtype) && (curpl.tex != NULL))
					{
						glColor3f(1.0f, 1.0f, 1.0f); // background for whole textures needs to be white
					}
					else
						glColor3f(def->r, def->g, def->b);
				}

				if (hasTex)
				{
					glTexCoord2f(curpl.texAncs[pni].x, curpl.texAncs[pni].y);
				}
			}
			else
				glColor3f(1.0f, 1.0f, 1.0f);
		
	
			glVertex3fv(curpn->p.ptr());
			
		}
		glEnd();
	} // for pli

	//qDebug("%d", ch);
	glDisable(GL_TEXTURE_2D);

}

bool PicPainter::exportToObj(QTextStream& meshout, QTextStream& mtlout, uint& numVerts,
							 uint &numTexVerts, uint &numNormals, uint &numObjs,
							 float fMatrix[16]) const
{
	// 1. Generate Mesh
	MyObject obj(&g_smoothAllocator);
	generateStraightShape(m_displayConf, obj);
	g_smoothAllocator.clear();

	// not very efficient...
	if (m_pdef->mygrp->drawtype == DRAW_COLOR)
	{
		PicGroupDef *def = m_pdef->mygrp;
		mtlout << "newmtl material" << ++numObjs << "\n";
		mtlout << "  Ns 32\n  d 1\n  Tr 1\n  Tf 1 1 1\n  illum 2\n  Ka 0.0000 0.0000 0.0000\n  Ks 0.3500 0.3500 0.3500\n";
		mtlout << "  Kd " << def->r << " " << def->g << " " <<  def->b << "\n";
	}

	meshout << "g Object" << numObjs << "\n";

	if (m_pdef->mygrp->drawtype == DRAW_COLOR)
	{
		meshout << "usemtl material" << numObjs << "\n";
	} 
	// others not supported...

	// Now we've got mesh
	// 2. Generate Mesh
	return realExportToObj(meshout, obj, numVerts, numTexVerts, numNormals, fMatrix);
}


void transformVector(const float fMatrix[16], const Coord3df vec, Coord3df& outVec)
{
	outVec[0] = fMatrix[0] * vec[0] + fMatrix[4] * vec[1] + fMatrix[8 ] * vec[2] + fMatrix[12];
	outVec[1] = fMatrix[1] * vec[0] + fMatrix[5] * vec[1] + fMatrix[9 ] * vec[2] + fMatrix[13];
	outVec[2] = fMatrix[2] * vec[0] + fMatrix[6] * vec[1] + fMatrix[10] * vec[2] + fMatrix[14];
}

float filterZero(float x) // don't allow numbers like 1.1e-16 to the output
{
	if (abs(x) < EPSILON)
		return 0.0f;
	return x;
}

bool PicPainter::realExportToObj(QTextStream& meshout, MyObject& obj, uint& numVerts,
								 uint &numTexVerts, uint &numNormals,
								 float fMatrix[16]) const
{
	// TODO:
	PicGroupDef *def = m_pdef->mygrp;
	bool hasTex = def->isTexExist();

	//switch (m_pdef->mygrp->drawtype) 
	//{
	//case DRAW_COLOR:
	//	glDisable(GL_TEXTURE_2D);
	//	glColor3f(def->r, def->g, def->b);
	//	break;
	//case DRAW_TEXTURE_BLEND:
	//	glColor3f(1.0f, 1.0f, 1.0f);
	//case DRAW_TEXTURE_NORM:
	//	glEnable(GL_TEXTURE_2D);
	//	//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); is default
	//	glBindTexture(GL_TEXTURE_2D, texId);
	//	break;
	//case DRAW_TEXTURE_INDIVIDUAL_HALF:
	//	// the enable texture comes later
	//	glDisable(GL_TEXTURE_2D);
	//	//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); is default
	//	//glBindTexture(GL_TEXTURE_2D, texId); sides have a different texture.
	//	break; // handle for every polygon
	//}


	//int ch = 0;
//	Texture *lastTex = NULL; // start disabled

	vector<Coord3df> vertices(obj.nPolys * 4);
	int realNumVers = 0;

	vector<Coord2df> texVertices(obj.nPolys * 4);
	int realNumTexVers = 0;

	vector<int> triangles(obj.nPolys * 8);
	int triCounter = 0;

	for(int pli = 0; pli < obj.nPolys; ++pli) //polygons
	{
		MyPolygon &curpl = *obj.poly[pli];

		for(int pni = 0; pni < 4; ++pni) //points
		{
			MyPoint *curpn = curpl.vtx[pni];

			Coord3df vert;
			transformVector(fMatrix, curpn->p, vert);

			int j = 0;
			for (j = 0; j < realNumVers; ++j)
			{
				if (vertices[j].isNear(vert))
					break;
			}
			if (j >= realNumVers)
			{
				vertices[realNumVers++] = vert;
			}
			triangles[triCounter++] = j;

			if (hasTex)
			{
				Coord2df texVert(curpl.texAncs[pni].x, curpl.texAncs[pni].y);
				int j = 0;
				for (j = 0; j < realNumTexVers; ++j)
				{
					if (texVertices[j].isNear(texVert))
					{
						break;
					}
				}
				if (j >= realNumTexVers)
				{
					texVertices[realNumTexVers++] = texVert;
				}

				triangles[triCounter++] = j;
			}
			else
			{

				triangles[triCounter++] = 0;
			}
		}
	}

	// We found unique vertices. Now let's put them to file
	for (int i = 0; i < realNumVers; ++i)
	{
		meshout << "v " << filterZero(vertices[i].x) << " " << filterZero(vertices[i].y) << " " << filterZero(vertices[i].z) << "\n";
	}

	if (hasTex)
	{
		for (int i = 0; i < realNumTexVers; ++i)
		{
			meshout << "vt " << texVertices[i].x << " " << texVertices[i].y << "\n";
		}
	}

	triCounter = 0;

	for(int pli = 0; pli < obj.nPolys; ++pli) //polygons
	{
		// Now add 2 triangles
		if (hasTex)
		{
			meshout << "f " << triangles[triCounter + 0] + numVerts << "/" <<
			 				triangles[triCounter + 1] + numTexVerts << " " <<
							triangles[triCounter + 2] + numVerts << "/" <<
							triangles[triCounter + 3] + numTexVerts << " " <<
							triangles[triCounter + 4] + numVerts << "/" <<
							triangles[triCounter + 5] + numTexVerts << " " <<
							triangles[triCounter + 6] + numVerts << "/" <<
							triangles[triCounter + 7] + numTexVerts << "\n";
		}
		else
		{
			meshout << "f " << triangles[triCounter + 0] + numVerts << " " <<
				  			 triangles[triCounter + 2] + numVerts << " " <<
							 triangles[triCounter + 4] + numVerts << " " <<
							 triangles[triCounter + 6] + numVerts << "\n";
		}


		/*if (hasTex)
		{
			sprintf(buf, "f %d/%d %d/%d %d/%d\n", triangles[triCounter + 0] + numVerts,
												  triangles[triCounter + 1] + numTexVerts,
												  triangles[triCounter + 4] + numVerts,
												  triangles[triCounter + 5] + numTexVerts,
												  triangles[triCounter + 6] + numVerts,
												  triangles[triCounter + 7] + numTexVerts);
		}
		else
		{
			sprintf(buf, "f %d %d %d\n", triangles[triCounter + 0] + numVerts,
										 triangles[triCounter + 4] + numVerts,
										 triangles[triCounter + 6] + numVerts);
		}
		meshFile.writeStr(buf);*/

		triCounter += 8;
	}
	numVerts += realNumVers;
	numTexVerts += realNumTexVers;

	return true;
}

void PicPainter::generateStraightShape(const DisplayConf& dpc, MyObject& obj) const
{
	// needed anyway. used by the basic object
	g_smoothAllocator.clear();

	int i;
	for (i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			placeSidePolygon(obj, -1, true, 1 + i, 1 + j);
		}
	}

	for (int b = 0; b < 16; ++b)
	{
		bool is1 = uncub(build[b].pnt.x, build[b].pnt.y); // the face number in this slot
		placeSidePolygon(obj, b, is1, build[b].pnt.x, build[b].pnt.y);
	}

	obj.vectorify();
	obj.clacNormals(dpc.bVtxNormals);
}