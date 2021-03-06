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

#include "BaseGLWidget.h"
#include "Configuration.h" // DisplayConf
#include "OpenGL/Shaders.h"
#include "OpenGL/glGlob.h"
#include "ObjExport.h"
#include "CubeMesher.h"

#include <vector>
#include <sstream>
#include <fstream>
using namespace std;
 

MyAllocator PicDisp::g_smoothAllocator;


#define POLY_A 0x0001
#define POLY_B 0x0002
#define POLY_C 0x0004
#define POLY_D 0x0008
#define POLY_UA 0x0010
#define POLY_UB 0x0020
#define POLY_UC 0x0040
#define POLY_UD 0x0080



const PicDisp::BuildFrame PicDisp::build[17] = 
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




void PicDisp::PlaceInto(int pntn, Vec3 *shpp, Vec3 *pnti1, Vec3 *pnti2, EPlaceType type) const
{
    int i;

    for (i = 0; i < pntn; ++i)
    {
        Vec3 &p = pnti1[i];
        p[0] = double(shpp[i][2]);
        p[1] = double(shpp[i][1]);
        p[2] = double(shpp[i][0]);

        Vec3 &np = pnti2[pntn - i - 1];
        np = p;

        switch (type)
        {
        case PLACE_NORM: np[0] += 1; break;
        case PLACE_X: np[1] += 1; break;
        case PLACE_Y: np[2] += 1; break;
        }

    }	
}



/// the actual internal lookup table.
/// Cube::genLinesIFS() also uses this table for creating the lines objects.
//static const BuildFrame build[17];



void PicDisp::placeSidePolygon(MyObject& obj, int b, bool is1, int x, int y) const
{
    static Vec3 pnti1[4], pnti2[4], pntiA[4], pntiB[4], pntiC[4], pntiD[4];

    Vec3 shpp[4];
    TexAnchor ancs1[4], ancs2[4], baseancf[4], baseancs[4], ancsR1[4], ancsR2[4], ancsR3[4], ancsR4[4];
    int i;

    double bx, by, rbx, rby; // makers of texture coordinates.. fronts and side-patches

    rbx = bx = double(x)/5.0;
    rby = by = double(y)/5.0;

/*	const PicGroupDef *mygrp = m_pdef->mygrp();
    if (mygrp->drawtype == DRAW_TEXTURE_IMAGE)
    {
        rbx = 0.0; rby = 0.0; // side patches of this has their own coordinates
    }
*/

    baseancf[0] = TexAnchor(bx, by);
    baseancf[1] = TexAnchor(bx + 0.2, by);
    baseancf[2] = TexAnchor(bx + 0.2, by + 0.2);
    baseancf[3] = TexAnchor(bx, by + 0.2);
    
    baseancs[0] = TexAnchor(rbx, rby);
    baseancs[1] = TexAnchor(rbx + 0.2, rby);
    baseancs[2] = TexAnchor(rbx + 0.2, rby + 0.2);
    baseancs[3] = TexAnchor(rbx, rby + 0.2);

/*	if (mygrp->drawtype == DRAW_TEXTURE_IMAGE)
    { // rescale it since the texture is a big one
        double xfact = mygrp->sideTex->img.width(), yfact = mygrp->sideTex->img.height();
        for(i = 0; i < 4; ++i) // pntn == 4
        {
            baseancs[i].x += ((double)(mygrp->sideTexX) / xfact);
            baseancs[i].y += ((double)(mygrp->sideTexY) / yfact);
        }
    }
*/
    Texture *tex = nullptr; //m_pdef->tex;

    ancs1[0] = baseancf[0]; ancs2[0] = baseancf[3]; 
    ancs1[1] = baseancf[1]; ancs2[1] = baseancf[2]; 
    ancs1[2] = baseancf[2]; ancs2[2] = baseancf[1];
    ancs1[3] = baseancf[3]; ancs2[3] = baseancf[0];

    shpp[0] = Vec3(x, y, 0); 	 shpp[1] = Vec3(x+1, y, 0);
    shpp[2] = Vec3(x+1, y+1, 0); shpp[3] = Vec3(x, y+1, 0);
    PlaceInto(4, shpp, pnti1, pnti2, PLACE_NORM);

    if (is1)
    { // polys that make the flat surface.
        obj.addPoly(pnti1, ancs1, nullptr); //if its an out of place, it's textured
        obj.addPoly(pnti2, ancs2, tex);
    }

    /////////////////////// side patches //////////////////
    if (b < 0)
        return; // enough of this shananigan, we're not on the sides.

    if (false) //mygrp->drawtype == DRAW_TEXTURE_IMAGE)
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

    shpp[0] = Vec3(x, y, 0);   shpp[1] = Vec3(x, y, 1);
    shpp[2] = Vec3(x+1, y, 1); shpp[3] = Vec3(x+1, y, 0);
    PlaceInto(4, shpp, pntiA, pntiB, PLACE_X);

    shpp[0] = Vec3(x, y, 0);   shpp[1] = Vec3(x, y+1, 0);
    shpp[2] = Vec3(x, y+1, 1); shpp[3] = Vec3(x, y, 1);
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

    Texture* sideTex = nullptr; // mygrp->sideTex

    if (polyf & POLY_A) obj.addPoly(pntiA, ancsR1, sideTex); //if its an out of place, it's textured
    if (polyf & POLY_B) obj.addPoly(pntiB, ancsR2, sideTex);
    if (polyf & POLY_C) obj.addPoly(pntiC, ancsR3, sideTex); //if its an out of place, it's textured
    if (polyf & POLY_D) obj.addPoly(pntiD, ancsR4, sideTex);
    std::swap(pntiA[1], pntiA[3]); std::swap(ancsR1[1], ancsR1[3]);
    std::swap(pntiB[1], pntiB[3]); std::swap(ancsR2[1], ancsR2[3]);
    std::swap(pntiC[1], pntiC[3]); std::swap(ancsR3[1], ancsR3[3]);
    std::swap(pntiD[1], pntiD[3]); std::swap(ancsR4[1], ancsR4[3]);
    if (polyf & POLY_UA) obj.addPoly(pntiA, ancsR1, sideTex); //if its an out of place, it's textured
    if (polyf & POLY_UB) obj.addPoly(pntiB, ancsR2, sideTex);
    if (polyf & POLY_UC) obj.addPoly(pntiC, ancsR3, sideTex); //if its an out of place, it's textured
    if (polyf & POLY_UD) obj.addPoly(pntiD, ancsR4, sideTex);


}

bool PicDisp::uncub(int x, int y) const {
     return m_arr.axx(x, y);
}


void PicDisp::generateStraightShape(const DisplayConf& dpc, MyObject& obj) const
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


void PicDisp::init(const DisplayConf& dpc)
{
    MyObject obj(&g_smoothAllocator);
    
    generateStraightShape(dpc, obj);

    for(int i = 0; i < dpc.numberOfPasses; ++i)	{
        obj.subdivide(dpc.passRound[i]);
    }
    obj.clacNormals(dpc.bVtxNormals);
    obj.toMesh(m_mesh, true);

   // stringstream ss;
   // ss << "c:/temp/orig/piece_" << hex << m_arr.getBits() << "_" << rand() << ".obj";
   // m_mesh.save(ss.str(), true);


    g_smoothAllocator.clear();
}



void PicDisp::initNoSubdiv()
{
    PicCubeMesher cm(&m_mesh, m_arr);
    cm.create(&g_smoothAllocator);
    g_smoothAllocator.clear();
}





/// do the actual painting of a single piece in the actual OpenGL view.

void PicPainter::paint(bool fTargets, const Vec3& name, BaseGLWidget *context, bool invertTex, int flag, int dispRot, bool faceOut) const
{
    glPolygonOffset(1.0, 1.0); // go backward, draw polygons TBD- move to slvpainter

    const PicGroupDef *def = m_pdef->mygrp();
    //bool hasTex = def->isTexExist();

    BaseProgram* bprog = static_cast<BaseProgram*>(ShaderProgram::current());

    auto tm = context->transformMat();

    bprog->trans.set(context->transformMat());

    mglCheckErrorsC("x8");
    if (!fTargets)
    {
        NoiseSlvProgram* prog = ShaderProgram::currentt<NoiseSlvProgram>();
        if (prog != nullptr) 
        {
            prog->drawtype.set(def->drawtype);
            prog->colorAu.set(def->color);
            // textured pieces can be either one color or half and half
            prog->colorB.set((!def->twoColor && def->drawtype == DRAW_TEXTURE_IMAGE) ? def->color : def->exColor);

         //   cout << "COL " << def->color << " -- " << def->exColor << endl;
            prog->flag.set(flag); // used for red marking cube to be removed


            if (def->gtex != nullptr) {
                //cout << "HAS-TEX h=" << def->gtex->handle() << " " << def->drawtype << endl;
                mglActiveTexture(0);
                glBindTexture(GL_TEXTURE_2D, def->gtex->handle());
                prog->noisef.set(0);
            }

            prog->setModelMat(context->model.cur());

            if (def->isIndividual()) 
            {
                prog->texOffset.set(Vec3(m_pdef->texX, m_pdef->texY, invertTex?1.0f:0.0f));
                prog->texScale.set(Vec2(m_pdef->texScaleX / 5.0f, m_pdef->texScaleY / 5.0f)); // div my 5 since the position coordinates are [0,4], not [0,1]
                switch(dispRot) { // rotate texture according to the piece rotation
                case 1:  prog->texTrans.set(Mat2(0,-1,1,0));  break;
                case 3:  prog->texTrans.set(Mat2(0,1,-1,0));  break;
                case 4:  prog->texTrans.set(Mat2(0,1,1,0));  break;
                case 6:  prog->texTrans.set(Mat2(0,-1,-1,0));  break;
                case 2:  prog->texTrans.set(Mat2(-1,0,0,-1));  break;
                case 5:  prog->texTrans.set(Mat2(-1,0,0,1)); break;
                case 7:  prog->texTrans.set(Mat2(1,0,0,-1)); break;
                default: prog->texTrans.set(Mat2(1,0,0,1));  break;
                }
            }

            // 1:-1 controls if we light up the face (x axis) in addition to the side of the piece (yz)
            prog->isBlackSign.set( (def->blackness > 0) ? (faceOut ? 1:-1) : 0 );
           

        }
        mglCheckErrorsC("x9a");
    }
    else {
        bprog->colorAu.set(name);
        mglCheckErrorsC("x9b");
    }
    M_ASSERT(m_pdef->disp != nullptr);
    //cout << "pdef=" << m_pdef << " disp=" << m_pdef->disp << endl;
    m_pdef->disp->m_mesh.paint();
}


bool PicPainter::exportToObj(ObjExport& oe, const Mat4& fMatrix) const
{
    DisplayConf displayConf;
    MyObject obj(&PicDisp::g_smoothAllocator);
    m_pdef->disp->generateStraightShape(displayConf, obj);
    PicDisp::g_smoothAllocator.clear();

    Mesh mesh;
    obj.toMesh(mesh, true);
    oe.addMesh(m_pdef->mygrp(), mesh, fMatrix);
    return true;
}






