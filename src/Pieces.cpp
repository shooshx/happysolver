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


//#include "GlobDefs.h"

#include <stdlib.h>

#include "general.h"
#include "Pieces.h"
//#include "Texture.h"
#include "PicsSet.h"
#include "NoiseGenerator.h"
#include "tinyxml/tinyxml2.h"
#include "OpenGL/GLTexture.h"

#include <iostream>
#include <sstream>
#include <fstream>

#ifdef QT_CORE_LIB
#include <QPainter>
#include <QFile>
#include <QTextStream>
#endif
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

using namespace tinyxml2;

#define TEX_X 64
#define TEX_Y 64
#define TEX_BUF_SIZE (TEX_X * TEX_Y * 4) 

// the singleton
PicBucket *PicBucket::g_instance = nullptr;
PicBucket& PicBucket::createSingleton()
{ 
    if (g_instance == nullptr) // never gets erased...
        g_instance = new PicBucket; 
    return *g_instance;
}


static PicDef::PathPoint pathway[17] = {
    {{0, 0}, {true, {0,0}, {1,0}}, {1,0, 1,1} },
    {{1, 0}, {false,{2,0}, {2,1}}, {2,0, 2,1} },
    {{2, 0}, {false,{3,0}, {3,1}}, {3,0, 3,1} },
    {{3, 0}, {false,{4,0}, {4,1}}, {4,0, 4,1} },
    {{4, 0}, {true, {5,0}, {5,1}}, {5,1, 4,1} },
    {{4, 1}, {false,{5,2}, {4,2}}, {5,2, 4,2} },
    {{4, 2}, {false,{5,3}, {4,3}}, {5,3, 4,3} },
    {{4, 3}, {false,{5,4}, {4,4}}, {5,4, 4,4} },
    {{4, 4}, {true, {5,5}, {4,5}}, {4,5, 4,4} },
    {{3, 4}, {false,{3,5}, {3,4}}, {3,5, 3,4} },
    {{2, 4}, {false,{2,5}, {2,4}}, {2,5, 2,4} }, 
    {{1, 4}, {false,{1,5}, {1,4}}, {1,5, 1,4} },
    {{0, 4}, {true, {0,5}, {0,4}}, {0,4, 1,4} },
    {{0, 3}, {false,{0,3}, {1,3}}, {0,3, 1,3} },
    {{0, 2}, {false,{0,2}, {1,2}}, {0,2, 1,2} },
    {{0, 1}, {false,{0,1}, {1,1}}, {0,1, 1,1} },
    {{0, 0}, {false,{-1,-1},{-1,-1}},{-1,-1,-1,-1}} // last one is for wrap up with first. not really read
};



void PicDef::makeBoundingPath()
{
    pathlen = 0;
    for (int step = 0; step < 16; ++step)
    {
        PathPoint &ppd = pathway[step];
        PathPoint &ppdnx = pathway[step + 1];
        if (v.axx(ppd.pnt.x, ppd.pnt.y) == 1)
        {
            // frame
            if (ppd.frm.both) // this can actually be (step%4 == 0) but more coherent this way
            {
                path[pathlen++] = ppd.frm.l1;
                path[pathlen++] = ppd.frm.l2;
            }
            else
                path[pathlen++] = ppd.frm.l1;
            // prependiculars
            if (v.axx(ppdnx.pnt.x, ppdnx.pnt.y) == 0)
                path[pathlen++] = PathCoord(ppd.prp.ex, ppd.prp.ey);
        }
        else // v[][] == 0
        {
            // frame
            if (!ppd.frm.both)
                path[pathlen++] = ppd.frm.l2;
            // prependiculars
            if (v.axx(ppdnx.pnt.x, ppdnx.pnt.y) == 1)
                path[pathlen++] = PathCoord(ppd.prp.sx, ppd.prp.sy); // reversed
        }

    }
    path[pathlen++] = path[0]; // loop back to first one

}


void PicBucket::makeBitmapList()
{	
    M_ASSERT(grps.size() > 0);

    int gind, pind, i, x, y, tmp;

    unsigned char staticbuf[128 * 128 * 4];
    unsigned char *origcbuf = nullptr;
    Vec2i origsize(0, 0);
    int origfactor = 1;

    for(gind = 0; gind < grps.size(); ++gind)
    {
        //printf("bitmaps %d\n", gind);
        PicGroupDef &cgrp = grps[gind];
        const ImgBuf &gimg = *cgrp.tex;
        switch (cgrp.drawtype)
        {
        case DRAW_COLOR:
            origcbuf = staticbuf;
            origsize = Vec2i(64, 64);
            origfactor = 1;
            tmp = origsize.width * origsize.height * 4;
            for (i = 0; i < tmp; i += 4)
            {// little endian images
                origcbuf[i + 0] = int(cgrp.color.b * 255.0); 
                origcbuf[i + 1] = int(cgrp.color.g * 255.0);
                origcbuf[i + 2] = int(cgrp.color.r * 255.0);
                origcbuf[i + 3] = 255; //alpha
            }
            break;
        case DRAW_TEXTURE_NORM:
            origcbuf = staticbuf;
            origsize = Vec2i(gimg.width(), gimg.height());
            origfactor = (gimg.width() == 64)?1:2; // otherwise its 128.
            tmp = origsize.width * origsize.height * 4;
            for (i = 0;  i < tmp; i += 4)
            {
                origcbuf[i + 0] = int( float(gimg.bits()[i]) * cgrp.color.b);
                origcbuf[i + 1] = int( float(gimg.bits()[i + 1]) * cgrp.color.g);
                origcbuf[i + 2] = int( float(gimg.bits()[i + 2]) * cgrp.color.r);
                origcbuf[i + 3] = 255;
            }
            break;
        case DRAW_TEXTURE_BLEND:
        case DRAW_TEXTURE_MARBLE:
            origcbuf = (unsigned char*)gimg.bits(); // move to unsigned, rid of const.
            origsize = Vec2i(gimg.width(), gimg.height());
            origfactor = (gimg.width() == 64)?1:2; // otherwise its 128.
        }

        for(pind = 0; pind < cgrp.numPics(); ++pind)
        { // TBD: for every other then INDIVIDUAL do this for only one time for grp
            PicDef &cpic = cgrp.getPic(pind); //'p'

            
            ImgBuf *pimg; // needs to be in this scope so it won't get destroyed
            if (cgrp.isIndividual())
            {
                pimg = cpic.tex;//->mirrored(false, true); // transform it back 
                origcbuf = (unsigned char*)pimg->bits(); // move to unsigned, rid of const.
                origsize = Vec2i(pimg->width(), pimg->height());
                origfactor = (pimg->width() == 64)?1:2; // otherwise its 128.
            }

            ImgBuf img(TEX_X + 1, TEX_Y + 1);
            // image is 1 pixel longer and wider then the texture to save place for the line.
            // this was grueling.
            //img.fill(0);

            unsigned char *cbuf = img.bits();

            for (y = 0; y < TEX_Y; ++y)  // x,y over the target image
            {
                for (x = 0; x < TEX_X; ++x)
                { 
                    for (int i = 0; i < 4; ++i)
                    {
                        cbuf[(x+(TEX_X+1)*y)*4 + i] = origcbuf[(x * origfactor + origsize.width * y * origfactor)*4 + i];
                    }
                }
            }

            for (x = 0; x < 5; ++x)
            {
                for (int ry = 0; ry < 5; ++ry)
                {
                    y = ry; // inversion of DIB
                    if (cpic.v.axx(x,ry) == 0)
                    {
                        int px = int( double(x)/5.0 * double(TEX_X) );
                        int py = int( double(y)/5.0 * double(TEX_Y) );
                        int xw = int( double(TEX_X) / 5.0);
                        int yw = int( double(TEX_Y) / 5.0);

                        for (int lx = px * 4; lx < (px + xw + 1) * 4; lx += 4)
                        {
                            for (int ly = py * 4; ly < (py + yw + 1) * 4; ly += 4)
                            {
                                cbuf[ly * (TEX_X + 1) + lx + 3] = 0; // alpha;
                            }
                        }
                    }
                }
            }

            cpic.makeBoundingPath();

#ifdef QT_CORE_LIB
            // QImage wrapper around the buffer, does not copy of delete the buffer
            QImage qimg(img.bits(), img.width(), img.height(), QImage::Format_ARGB32);
            // this copies the image into the pixmap, avoid move c'tor by having qimg be named variable (r-value)
            cpic.pixmap = QPixmap::fromImage(qimg);
            QPolygon poly;
            for (i = 0; i < cpic.pathlen; ++i)
            { // path includes last point == first point
                poly.push_back(QPoint( (int)((cpic.path[i].x / 5.0)*64), (int)((cpic.path[i].y / 5.0)*64)));
            }

            QPainter painter(&cpic.pixmap);
            //painter.drawImage(0,0,img);
            //QPen pen(Qt::red, 2);                 // red solid line, 2 pixels wide
            //painter.setPen(pen);              // set the red, wide pen

            painter.drawPolyline(poly);
            painter.end();

            //cpic.pixmap.save(QString("c:/temp/cpic_%1_%2.png").arg(gind).arg(pind));
#endif

        }
    }

}



ImgBuf* PicBucket::newTexture(ImgBuf* t, bool in3d)
{
    texs.push_back(t);

    if (in3d) {
        //ImgBuf* b = im.rgbSwapped();
        GlTexture *gt = new GlTexture();
#ifdef EMSCRIPTEN
        int iformat = GL_RGBA;
#else
        int iformat = GL_BGRA;
#endif
        gt->init(GL_TEXTURE_2D, Vec2i(t->width(), t->width()), 1, GL_RGBA, iformat, GL_UNSIGNED_BYTE, t->bits(), GL_LINEAR, GL_LINEAR);
        gtexs.push_back(gt);
    }
    else {
        gtexs.push_back(nullptr);
    }
    return t;

}


ImgBuf* PicGroupDef::blendImage(ImgBuf* baseTex)
{
    ImgBuf *img = new ImgBuf(baseTex->width(), baseTex->height());
    int size = baseTex->width() * baseTex->height() * 4;
    uchar *cbuf = img->bits();

    for (int c = 0;  c < size; c += 4)
    {
        float fb = float(baseTex->bits()[c]);
        float fg = float(baseTex->bits()[c + 1]);
        float fr = float(baseTex->bits()[c + 2]);	

        cbuf[c + 0] = int( (255.0-fb) * color.b + fb * exColor.b);
        cbuf[c + 1] = int( (255.0-fg) * color.g + fg * exColor.g);
        cbuf[c + 2] = int( (255.0-fr) * color.r + fr * exColor.r);
        cbuf[c + 3] = 255;
    }
    return img;	
}


static EDrawType getDrawType(const string& s) 
{
    if (s == "COLOR") return DRAW_COLOR;
    if (s == "TEXTURE_NORM") return DRAW_TEXTURE_NORM;
    if (s == "TEXTURE_BLEND") return DRAW_TEXTURE_BLEND;
    if (s == "TEXTURE_MARBLE") return DRAW_TEXTURE_MARBLE;
    if (s == "TEXTURE_INDIVIDUAL_HALF") return DRAW_TEXTURE_INDIVIDUAL_HALF;
    if (s == "TEXTURE_INDIVIDUAL_WHOLE") return DRAW_TEXTURE_INDIVIDUAL_WHOLE;
    M_ASSERT(false);
    return (EDrawType)0;
}


PicDef& PicGroupDef::getPic(int myi) {
    return PicBucket::mutableInstance().pdefs[picsi[myi]];
}
const PicDef& PicGroupDef::getPic(int myi) const {
    return PicBucket::mutableInstance().pdefs[picsi[myi]];
}

const PicGroupDef* PicDef::mygrp() const {
    return &PicBucket::instance().grps[mygrpi];
}



bool PicBucket::loadXML(const char* data)
{
    //QDomDocument doc;
    XMLDocument doc;

    if (doc.Parse(data) != XML_NO_ERROR) {
        cout << "error parsing XML file:" << endl;
        return false;
    }

    XMLElement* main = doc.FirstChildElement();

    map<string, int> grpnames; // maps the name of a group to its index. used for copying pieces
    //QDomElement docElem = doc.documentElement();
    XMLElement* texe = main->FirstChildElement("textures");

    ////////////// texture files ////////////////////////
    XMLElement* t = texe->FirstChildElement("texture");
    while (t != nullptr)
    {
        bool in3d = t->Attribute("in3d", "true") != nullptr;
        const char *txnamep = t->Attribute("filename");
        if (txnamep != nullptr)
        {
#ifdef QT_CORE_LIB
            string txname(txnamep);
            QImage img(txname.c_str(), "PNG");
            if (img.isNull())
            {
                cout << "Failed to load texture file: " << endl;
                return false;
            }
            ImgBuf *imgbuf = new ImgBuf(img.width(), img.height(), img.bits());
            
            newTexture(imgbuf, in3d);
#endif
        }


        t = t->NextSiblingElement();
    }

//	unsigned int xxx3 = GetTickCount();

    ////////////////////// pics ////////////////////////////////
    XMLElement* xfams = main->FirstChildElement("outlines");

    int grpCount = 0;
    // count the number of cubes (groups) we have
    XMLElement* xfam = xfams->FirstChildElement("family");
    while (xfam != nullptr)
    {
        int ind = xfam->IntAttribute("index"); // the index it should show at the main build helper menu
        if (ind >= families.size()) {
            families.resize(ind + 1);
        }
        PicFamily &cfam = families[ind];
        cfam.startIndex = grpCount;
        cfam.name = xfam->Attribute("name");
        cfam.onResetSetCount = xfam->IntAttribute("onResetSetCount");
        cfam.iconFilename = xfam->Attribute("icon");

        cfam.numGroups = 0; // xfam->ElementsByTagName("group").size();
        XMLElement* xgrps = xfam->FirstChildElement("group");
        while (xgrps != nullptr) {
            ++cfam.numGroups;
            xgrps = xgrps->NextSiblingElement("group");
        }
        grpCount += cfam.numGroups;
        xfam = xfam->NextSiblingElement(); // next family
    }

    pdefs.reserve((grpCount+2)*6); // +2 extra for added edited cubes
    pdefs.resize(grpCount*6); 

    sumPics = 0;
    int grpi = 0; // (i) group number
    int pdefi = 0;

    xfam = xfams->FirstChildElement("family");
    while (xfam != nullptr)
    {
        XMLElement* cube = xfam->FirstChildElement("group"); // a group of pieces is a cube
        while (cube != nullptr)
        {
            PicGroupDef cgrp;
            cgrp.name = cube->Attribute("name");
            grpnames[cgrp.name] = grpi;
    
            XMLElement* fill = cube->FirstChildElement("fill");

            cgrp.drawtype = getDrawType(fill->Attribute("type"));

            cgrp.color = Vec3(float(fill->IntAttribute("r")) / 255.0f,
                              float(fill->IntAttribute("g")) / 255.0f,
                              float(fill->IntAttribute("b")) / 255.0f);
            cgrp.exColor = Vec3(float(fill->IntAttribute("exR")) / 255.0f,
                                float(fill->IntAttribute("exG")) / 255.0f,
                                float(fill->IntAttribute("exB")) / 255.0f);
            cgrp.blackness = (EBlackness)fill->IntAttribute("k");

            ImgBuf* baseTex = nullptr;   
            int txind = 0;
            if (fill->Attribute("texind") != nullptr)
            {
                txind = fill->IntAttribute("texind");
                if (txind >= 0 && txind < texs.size()) 
                    baseTex = texs[txind];
                else 
                    cout << "missing texture image" << txind << endl;

            }

            cgrp.tex = baseTex;

    
            if (baseTex != nullptr) 
            {
                if (cgrp.drawtype == DRAW_TEXTURE_BLEND || cgrp.drawtype == DRAW_TEXTURE_MARBLE) {
                    cgrp.tex = newTexture(cgrp.blendImage(baseTex), false);
                }
            }


            if (cgrp.drawtype == DRAW_TEXTURE_BLEND || cgrp.drawtype == DRAW_TEXTURE_MARBLE) {
                cgrp.gtex = gtexs[0]; //the noise tex
            }
            else if (cgrp.isIndividual()) 
            {
                GlTexture* baseGTex = nullptr;
                // see ModelControlBase::initTex()
                if (txind >= 0 && txind + 1 < gtexs.size() && gtexs[txind + 1] != nullptr) {
                    baseGTex = gtexs[txind + 1]; // gtex starts with the noise tex at 0
                }
                else
                    cout << "missing gl-texture " << txind << endl;

                cgrp.gtex = baseGTex;
            }

            //QDomNodeList xpics = cube->ElementsByTagName("piece");
            XMLElement* xpic = cube->FirstChildElement("piece");
            if (xpic != nullptr)
            {
                //cgrp.pics.resize();
                int pici = 0;
                while (xpic != nullptr)
                {
                    PicDef& curdef = pdefs[pdefi];
                    curdef.mygrpi = grpi;
                    curdef.indexInGroup = pici;

                    //QDomElement ep = xpics.at(pici).toElement();
                    //QString text = xpic->GetText();
                    //text.remove(' ');
                    string text = xpic->GetText();
                    text.erase(std::remove(text.begin(), text.end(), ' '), text.end());
                    int txti = 0;
                    for (int y = 0; y < 5; ++y)
                    {
                        for (int x = 0; x < 5; ++x)
                        {
                            // get the first number;
                            if (text.at(txti) == '0')
                                curdef.v.set(x, y) = 0;
                            else 
                                curdef.v.set(x, y) = 1;
                            ++txti;

                        }
                    }
                    if (cgrp.isIndividual() && cgrp.gtex != nullptr)
                    {
                        curdef.xOffs = xpic->IntAttribute("x1");
                        curdef.yOffs = xpic->IntAttribute("y1");
                        if (cgrp.tex != nullptr)
                            curdef.tex = newTexture(cgrp.tex->copy(curdef.xOffs, curdef.yOffs, 128, 128)/*.mirrored(false, true)*/, false);
                    }
                    curdef.v.makeRtns(curdef.defRtns);
                    cgrp.picsi.push_back(pdefi); // the index in the bucket
                    ++pdefi;
                    ++pici;
                    xpic = xpic->NextSiblingElement("piece");
                }
            }
            else // no "piece" elements
            {
                XMLElement* xpics = cube->FirstChildElement("pieces");
                string copyfrom = xpics->Attribute("copy");
                if (grpnames.find(copyfrom) == grpnames.end())
                {
                    cout << "can't copy piece from: " << copyfrom << endl;
                    return false;
                }
                
                int ifrom = grpnames[copyfrom];
                int copyCount = grps[ifrom].numPics();
                //defs[i].pics.resize(defs[ifrom].numPics());
                for (int pici = 0; pici < copyCount; ++pici)
                {
                    PicDef &curdef = pdefs[pdefi];
                    curdef.mygrpi = grpi;
                    curdef.indexInGroup = pici;

                    const PicDef &fromDef = grps[ifrom].getPic(pici);
                    curdef.v = fromDef.v; // copy only the data of the piece


                    if (cgrp.isIndividual())
                    {
                        curdef.xOffs = fromDef.xOffs;
                        curdef.yOffs = fromDef.yOffs;
                        curdef.tex = newTexture(cgrp.tex->copy(curdef.xOffs, curdef.yOffs, 128, 128)/*.mirrored(false, true)*/, false);
                    }

                    curdef.v.makeRtns(curdef.defRtns);
                    cgrp.picsi.push_back(pdefi);
                    ++pdefi;
                }

            }

            grps.push_back(cgrp);
            sumPics += cgrp.numPics();
            ++grpi;
            cube = cube->NextSiblingElement();
        } // cubes
        xfam = xfam->NextSiblingElement(); // next family
    } // families

    // set the reset selection
    setToFamResetSel();

    makeAllComp();

    return true;

}

// update existing or add new (editor)
void PicBucket::updateGrp(int grpi, PicArr arrs[6])
{
    M_ASSERT(grpi <= grps.size());
    bool add = false;
    M_ASSERT(grpi <= grps.size());
    //cout << "GRP " << grpi << " " << grps.size() << endl;
    if (grpi == grps.size()) {
        grps.push_back(PicGroupDef());
        add = true;
    }
    PicGroupDef& cgrp = grps[grpi];
    cgrp.drawtype = DRAW_COLOR;
    cgrp.color = Vec3(1,1,1);

    PicDisp::g_smoothAllocator.init(640, 640, 0);
    PicDisp::g_smoothAllocator.clearMaxAlloc();


    for(int i = 0; i < 6; ++i) {
        int pdefi;
        if (add) {
            pdefi = pdefs.size();
            pdefs.push_back(PicDef());
            cout << "Added Group " << pdefi << endl;
            cgrp.picsi.push_back(pdefi);
            M_ASSERT(cgrp.picsi.size() <= 6);
        }
        else {
            M_ASSERT(cgrp.picsi.size() == 6);
            pdefi = cgrp.picsi[i];
        }

        PicDef& curdef = pdefs[pdefi];
        curdef.mygrpi = grpi;
        curdef.indexInGroup = i;
        curdef.v = arrs[i];
        curdef.v.makeRtns(curdef.defRtns);
    }

    makeAllComp();

    distinctMeshes(false);
    PicDisp::g_smoothAllocator.checkMaxAlloc();

}


void PicBucket::makeAllComp()
{
    PicsSet ps;
    for (int i = 0; i < pdefs.size(); ++i) {
        ps.add(i, true); // TBD - consider sym
    }
    allComp = ps.comp;
    for(int i = 0; i < allComp.size(); ++i)
    {
        for(auto& ad : allComp[i].addedInds)
        {
            auto& r = pdefs[ad.addedInd];
            r.indInAllComp = i;
            r.defRot = ad.defRot;
        }
    }
    cout << "Compressed " << ps.comp.size() << " shapes" << endl;
}


void PicBucket::setToFamResetSel()
{
    for(int f = 0; f < families.size(); ++f)
    {
        const PicFamily &fam = families[f];
        int setTo = fam.onResetSetCount;
        for (int g = fam.startIndex; g < fam.startIndex + fam.numGroups; ++g)
        {
            PicGroupDef &grp = grps[g];
            for (int p = 0; p < grp.numPics(); ++p)
            {
                grp.getPic(p).setSelected(setTo);
            }
        }
    }
}

// prepares PicDisps for all pdefs with the correct roatation configured and corrolate pdefs to the disps
// - createDisps - should we create the disps or are they already created
void PicBucket::distinctMeshes(bool createDisps)
{
    PicsSet ps;
    for(int i = 0; i < pdefs.size(); ++i) {
        ps.add(i, false);
    }
    cout << ps.comp.size() << " distinct meshes out of " << pdefs.size() << " pieces meshes=" << m_meshes.size() << endl;

    if (createDisps)
        m_meshes.clear();

    for(int i = 0; i < ps.comp.size(); ++i) 
    {
        auto comp = ps.comp[i];
        auto sig = comp.rtns[0].getBits();
        PicDisp* pd;
        if (createDisps) {
            pd = new PicDisp(PicArr(sig));
            m_meshes[sig].reset(pd);
        }
        else {
            //cout << "sig " << hex << sig << dec << endl;
            auto it = m_meshes.find(sig);
            if (it == m_meshes.end()) { // did not find signatutre in preloaded meshes
                pd = new PicDisp(PicArr(sig));
                cout << "Generating mesh for " << hex << sig << dec << endl;
                pd->initNoSubdiv();
                pd->m_mesh.makeSelfBos(true);

                m_meshes[sig].reset(pd);
            }
            else
                pd = it->second.get();
        }

        for(int j = 0; j < comp.addedInds.size(); ++j) 
        {
            auto added = comp.addedInds[j];
            pdefs[added.addedInd].dispRot = added.defRot; // TBD - this is not duplicated in allComp
           // cout << "PDEF " << &pdefs[added.addedInd] << "  DISP=" << pd << endl;
            pdefs[added.addedInd].disp = pd;
        }
    }

   // cout << "BLAT" << endl;

}

#ifdef QT_CORE_LIB
/** PoolStats is the datum of the poolStats list which holds the optimal sizes of MyAllcator pools.
    Sizes for the MyPoint, MyPolygon and HalfEdge pools, measured as a function of the number of
    subdivision passe using MyObject::subdivide().
    the poolStats list is used in PicBucket::buildMeshes().
*/
struct PoolStats
{
    int points, poly, he;
};

static PoolStats poolStats[] = {
    { 100, 100, 0 },         // 0 passes: 72, 70, 0
    { 300, 370, 300 },       // 1 pass: 282, 350, 280
    { 1200, 1500, 1200 },    // 2 passes: 1122, 1470, 1120
    { 4500, 6000, 4500 },    // 3 passes: 4482, 5950, 4480
    { 17950, 23900, 17950 }}; // 4 passes: 17922, 23870, 17920


class MeshUnifier
{
public:
    void add(const Mesh& m, TPicBits bits);
    void save(const char* path);

    stringstream m_idxlines;
    vector<pair<Vec3,Vec3>> m_uvtx; // pos,norm

};

void MeshUnifier::add(const Mesh& m, TPicBits bits)
{
    int vCount = m_uvtx.size();
    vector<int> indexInUni(m.m_vtx.size()); // for every vertex in the mesh, where it is in the uni vertex arr
    for(int vi = 0; vi < m.m_vtx.size(); ++vi) {
        const auto& v = m.m_vtx[vi];
        const auto& n = m.m_normals[vi];
        int ui;
        for(ui = 0; ui < m_uvtx.size(); ++ui) {
            const auto& uvn = m_uvtx[ui];
            if (v == uvn.first && n == uvn.second)
                goto nextmv;
        }
        m_uvtx.push_back(make_pair(v, n));
    nextmv:
        indexInUni[vi] = ui;
        continue;
    }

    m_idxlines << hex << "*" << bits << dec << "\n";
    for(int i = 0; i < m.m_idx.size(); i+=4) {
        m_idxlines << indexInUni[m.m_idx[i]] << " " << indexInUni[m.m_idx[i + 1]] << " " << indexInUni[m.m_idx[i + 2]] << " " << indexInUni[m.m_idx[i + 3]] << "\n";
    }
    cout << "added " << m_uvtx.size() - vCount << "  to " << m_uvtx.size() << endl;
}

void MeshUnifier::save(const char* path)
{
    ofstream f(path);
    if (!f.good()) {
        cout << "ERROR: can't open " << path << endl;
        return;
    }
    for(int ui = 0; ui < m_uvtx.size(); ++ui) {
        const auto& vn = m_uvtx[ui];
        f << "v " << vn.first.x << " " << vn.first.y << " " << vn.first.z << " " << vn.second.x << " " << vn.second.y << " " << vn.second.z << "\n";
    }
    f << m_idxlines.str();
}

void PicBucket::buildMeshes(const DisplayConf& dpc, ProgressCallback* prog)
{
    // fill meshes
    distinctMeshes(true);

    if (prog)
        prog->init(m_meshes.size());

    // grows if needed, never shrinks.
    PoolStats& ps = poolStats[dpc.numberOfPasses];
    PicDisp::g_smoothAllocator.init(ps.points, ps.poly, ps.he);
    PicDisp::g_smoothAllocator.clearMaxAlloc();

    //MeshUnifier muni;

    int cnt = 0;
    bool cancel = false;

    for (auto& disp: m_meshes)
    {
        if (prog && !prog->setValue(cnt++)) // progress
            break;
        //disp.second->init(); // full subdivision
        disp.second->initNoSubdiv();
        disp.second->m_mesh.makeSelfBos(true);
        //muni.add(disp->m_mesh, disp->bits());
    }

    if (prog)
        prog->setValue(m_meshes.size());
    PicDisp::g_smoothAllocator.checkMaxAlloc();

    //muni.save("c:/temp/no_sub_unified.txt");
    //exit(1);
}


struct AddedArr {
    PicArr p;
    TPicBits rtbits[8]; // not condensed, just the bits
};

bool hadLoneCorner(const PicArr& a) {
    return  ((a.axx(0, 0) == 1 && a.axx(0, 1) == 0 && a.axx(1, 0) == 0) ||
             (a.axx(4, 0) == 1 && a.axx(4, 1) == 0 && a.axx(3, 0) == 0) ||
             (a.axx(0, 4) == 1 && a.axx(1, 4) == 0 && a.axx(0, 3) == 0) ||
             (a.axx(4, 4) == 1 && a.axx(3, 4) == 0 && a.axx(4, 3) == 0));
}

void PicBucket::buildAllMeshes() 
{
    vector<AddedArr> arrs;
    // generate all possible pieces
    for(int i = 0; i <= 65535; ++i) 
    {
        AddedArr na;
        na.p.fromBits(i);
        M_ASSERT(na.p.getBits() == i); // sanity
        // check disconnected corner
        if (hadLoneCorner(na.p)) {
             continue;
        }
        na.p.makeRtnsBits(na.rtbits);
        for(const auto& exaa: arrs) 
        {
            for(int innewi = 0; innewi < 8; ++innewi) {
                for (int inexi = 0; inexi < 8; ++inexi) {
                    if (exaa.rtbits[inexi] == na.rtbits[innewi])
                        goto alreadyExist;
                }
            }
        }
        arrs.push_back(na);
    alreadyExist:
        if ((i % 2000) == 0)
            cout << i << "  got=" << arrs.size() << "\n";

        continue;
    }

    MeshUnifier muni;

    cout << "---" << arrs.size() << endl;
    for(const auto& a: arrs) {
        cout << "------" << a.rtbits[0] << endl;
        PicDisp disp;
        disp.m_arr.fromBits(a.rtbits[0]);
        disp.initNoSubdiv();
        disp.m_mesh.makeSelfBos(true);
        muni.add(disp.m_mesh, disp.bits());
    }

    muni.save("c:/temp/all__no_sub_unified.txt");
}

// in Qt, s is the filename
// in emscripten uses the function below
bool PicBucket::loadUnified(const char* s) 
{
    QFile iff(s);
    if (!iff.open(QIODevice::ReadOnly | QIODevice::Text)) {
        cout << "ERROR: failed opening " << s << endl;
        return false;
    }
    QTextStream in(&iff);

    m_meshes.clear();

    shared_ptr<Mesh::CommonData> cd(new Mesh::CommonData);
    double a,b,c;
    char *pa = nullptr;
    Mesh *mesh = nullptr;
    int vtxCount = 0, meshCount = 0;

    while (!in.atEnd())
    {
        QString line = in.readLine();
        QByteArray ba = line.toLatin1();
        const char* linebuf = ba.data();
        uint len = ba.size();

        if (len < 3)
            continue;
        if (linebuf[0] == 'v') { // vertex
            pa = (char*)linebuf + 2;
            a = strtod(pa, &pa);
            b = strtod(pa, &pa);
            c = strtod(pa, &pa);
            cd->vtx.push_back(Vec3(a, b, c));
            a = strtod(pa, &pa);
            b = strtod(pa, &pa);
            c = strtod(pa, &pa);
            cd->normals.push_back(Vec3(a, b, c));
            ++vtxCount;
        }
        else if (linebuf[0] == '*') 
        {// mesh header
            TPicBits sig = strtoul(linebuf + 1, nullptr, 16);
            auto* disp = new PicDisp(PicArr(sig));
            m_meshes[sig].reset(disp);

            mesh = &disp->m_mesh;
            mesh->m_type = Mesh::TRIANGLES; //Mesh::QUADS;
            mesh->m_hasIdx = mesh->m_hasNormals = true;
            mesh->m_common = cd;

            ++meshCount;
        }
        else {
            pa = (char*)linebuf;
            int vi[4];
            for(int fi = 0; fi < 4; ++fi) {
                vi[fi] = strtoul(pa, &pa, 10);
                M_ASSERT(vi[fi] < 65536);
            }
            mesh->m_idx.push_back(vi[0]);
            mesh->m_idx.push_back(vi[2]);
            mesh->m_idx.push_back(vi[1]);
            mesh->m_idx.push_back(vi[0]);
            mesh->m_idx.push_back(vi[3]);
            mesh->m_idx.push_back(vi[2]);

        }
    }

    cd->makeSelfBos();
    for(auto& d: m_meshes) {
        d.second->m_mesh.makeIdxBo(true);
    }

    cout << "Unified Mesh read " << vtxCount << " vtx " << meshCount << " meshes" << endl;

    distinctMeshes(false);

    return true;
}

#endif

#ifdef EMSCRIPTEN

bool PicBucket::loadUnifiedJs()
{
    shared_ptr<Mesh::CommonData> cd(new Mesh::CommonData);

    cd->m_vtxBo.bind();
    EM_ASM(GLctx.bufferData(GLctx.ARRAY_BUFFER, unimesh.vtx, GLctx.STATIC_DRAW));
    cd->m_vtxBo.m_size = EM_ASM_INT_V(return unimesh.vtx.length);

    cd->m_normBo.bind();
    EM_ASM(GLctx.bufferData(GLctx.ARRAY_BUFFER, unimesh.norm, GLctx.STATIC_DRAW));
    cd->m_normBo.m_size = EM_ASM_INT_V(return unimesh.norm.length);

    int len = EM_ASM_INT_V(return unimesh_keys.length);

    m_meshes.clear();
    for (int i = 0; i < len; ++i) 
    {
        double sig = EM_ASM_DOUBLE(return parseInt(unimesh_keys[$0]), i);
        if (isnan(sig))
            continue; // 'vtx' or 'norm'

        auto d = new PicDisp(PicArr(sig));
        m_meshes[sig].reset(d);

        d->m_mesh.m_idxBo.bind();
        EM_ASM_(GLctx.bufferData(GLctx.ELEMENT_ARRAY_BUFFER, unimesh[$0], GLctx.STATIC_DRAW), sig);
        d->m_mesh.m_idxBo.m_size = EM_ASM_INT(return unimesh[$0].length, d->bits());
        d->m_mesh.m_type = Mesh::TRIANGLES;
        d->m_mesh.m_hasIdx = d->m_mesh.m_hasNormals = true;
        d->m_mesh.m_common = cd;
    }
    distinctMeshes(false);

    return true;
}

#endif


#if 0
// load a single big file with all the meshes separated by ****XXXX where XXXX is the bits of the pic
bool PicBucket::loadMeshes(const string& filename) 
{
    distinctMeshes();

    ifstream iff(filename);
    if (!iff.good())
        return false;
    char linebuf[BUF_LEN] = {0};
    Mesh *mesh = nullptr;
    double a,b,c;
    char *pa, *pb;
    vector<Vec2> texc;
    bool startFaces = false;
    while (!iff.eof())
    {
        iff.getline(linebuf, BUF_LEN);
        uint len = strnlen(linebuf, BUF_LEN);
        if (len < 3)
            continue;
        if (linebuf[0] == '*') {// mesh header
            // calc normals
            ushort sig = strtoul(linebuf + 4, nullptr, 16);
            // find the next mesh
            for (auto it = m_meshes.begin(), endit = m_meshes.end();  it != endit; ++it)
                if (sig == (*it)->bits()) {
                    // finishes prev mesh, calc normals
                    if (mesh != nullptr) {
                        mesh->calcTrianglesNormals();
                    }
                    mesh = &(*it)->m_mesh;
                    mesh->m_type = Mesh::TRIANGLES;
                    mesh->m_hasIdx = mesh->m_hasNormals = true;
                    texc.clear();
                    startFaces = true;
                    break;
                }
        }
        else if (linebuf[0] == 'v' && linebuf[1] == 't') { // tex coord
            // save them in a buffer, for reference from faces
            pa = linebuf + 3;
            a = strtod(pa, &pb);
            b = strtod(pb, &pa);
            texc.push_back(Vec2(a, b));
        }
        else if (linebuf[0] == 'v') { // vertex
            pa = linebuf + 2;
            a = strtod(pa, &pb);
            b = strtod(pb, &pa);
            c = strtod(pa, &pb);
            mesh->m_vtx.push_back(Vec3(a, b, c));
        }
        else if (linebuf[0] == 'f') { // face
            if (startFaces) {
                //mesh->m_texCoord.resize(mesh->m_vtx.size()); // no more vertices, the size of texCoord is the same as vtx
                startFaces = false;
            }
            pa = linebuf + 2;
            for(int fi = 0; fi < 3; ++fi) {
                uint v = strtoul(pa, &pb, 10) - 1;  // one based indexes
                uint t = strtoul(pb + 1, &pa, 10) - 1; // skip slash
                mesh->m_idx.push_back(v);
                //mesh->m_texCoord[v] = texc[t];

            }
        }
    }
    mesh->calcTrianglesNormals(); // last mesh
    return true;
}
#endif

int PicBucket::selectedCount() const
{
    int count = 0;
    for (int pi = 0; pi < pdefs.size(); ++pi)
    {
        count += pdefs[pi].getSelected();
    }
    return count;
}

// comp rot to def rot
int rotationAdd(int base, int defRot) 
{
    if ((defRot < 4) == (base < 4))
        return (base - defRot + 4) % 4;
    else
        return ((base + defRot) % 4) + 4;

}

// def rot to comp rot
int rotationSub(int x, int defRot)
{
    if (defRot < 4) {
        if (x < 4)
            return (x + defRot + 4) % 4;
        else
            return ((x - defRot) % 4) + 4;
    }
    else {
        if (x < 4)
            return ((x + defRot) % 4) + 4;
        else
            return (x - defRot + 4) % 4;
    }

}
