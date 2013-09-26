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

#define __msxml_h__ // msxml also defines XMLDocument
#define __urlmon_h__
#define _OBJBASE_H_
#define __oaidl_h__
#define _OLEAUTO_H_
#define __objidl_h__
#define __oleidl_h__
#define _OLE2_H_

//#include "GlobDefs.h"


#include "general.h"
#include "Pieces.h"
#include "Texture.h"
#include "PicsSet.h"
#include "NoiseGenerator.h"
#include "tinyxml/tinyxml2.h"
#include "OpenGL/GLTexture.h"

#include <fstream>

#ifdef _WINDOWS
#include <QPainter>
#endif


using namespace tinyxml2;

#define TEX_X 64
#define TEX_Y 64
#define TEX_BUF_SIZE (TEX_X * TEX_Y * 4) 

// the singleton
PicBucket *PicBucket::g_instance = NULL;
void PicBucket::createSingleton() 
{ 
    if (g_instance == NULL) // never gets erased...
        g_instance = new PicBucket; 
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
    unsigned char *origcbuf = NULL;
    QSize origsize(0, 0);
    int origfactor = 1;

    for(gind = 0; gind < grps.size(); ++gind)
    {
        //printf("bitmaps %d\n", gind);
        PicGroupDef &cgrp = grps[gind];
        const QImage &gimg = cgrp.tex->img;
        switch (cgrp.drawtype)
        {
        case DRAW_COLOR:
            origcbuf = staticbuf;
            origsize = QSize(64, 64);
            origfactor = 1;
            tmp = origsize.width() * origsize.height() * 4;
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
            origsize = gimg.size();
            origfactor = (gimg.width() == 64)?1:2; // otherwise its 128.
            tmp = origsize.width() * origsize.height() * 4;
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
            origsize = gimg.size();
            origfactor = (gimg.width() == 64)?1:2; // otherwise its 128.
        }

        for(pind = 0; pind < cgrp.numPics(); ++pind)
        { // TBD: for every other then INDIVIDUAL do this for only one time for grp
            PicDef &cpic = cgrp.getPic(pind); //'p'

            QImage pimg; // needs to be in this scope so it won't get destroyed
            if (cgrp.isIndividual())
            {
                pimg = cpic.tex->img.mirrored(false, true); // transform it back 
                origcbuf = (unsigned char*)pimg.bits(); // move to unsigned, rid of const.
                origsize = pimg.size();
                origfactor = (pimg.width() == 64)?1:2; // otherwise its 128.
            }

            QImage img(TEX_X + 1, TEX_Y + 1, QImage::Format_ARGB32);
            // image is 1 pixel longer and wider then the texture to save place for the line.
            // this was grueling.
            img.fill(0x00000000);

            unsigned char *cbuf = img.bits();

            for (y = 0; y < TEX_Y; ++y)  // x,y over the target image
            {
                for (x = 0; x < TEX_X; ++x)
                { 
                    for (int i = 0; i < 4; ++i)
                    {
                        cbuf[(x+(TEX_X+1)*y)*4 + i] = origcbuf[(x*origfactor + origsize.width()*y*origfactor)*4 + i];
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

#ifdef _WINDOWS
            cpic.pixmap = QPixmap::fromImage(img);
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
#endif

        }
    }

}



Texture* PicBucket::newTexture(const QImage& im, bool in3d)
{
    Texture *t = new Texture(im);
    texs.push_back(t);

    if (in3d) {
        QImage b = im.rgbSwapped();
        GlTexture *gt = new GlTexture();
        gt->init(GL_TEXTURE_2D, Vec2i(b.width(), b.width()), 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, b.bits(), GL_LINEAR, GL_LINEAR);
        gtexs.push_back(gt);
    }
    else {
        gtexs.push_back(NULL);
    }
    return t;

}

QImage PicGroupDef::blendImage(Texture* baseTex)
{
    QImage img(baseTex->img.width(), baseTex->img.height(), QImage::Format_ARGB32);
    int size = baseTex->img.width() * baseTex->img.height() * 4;
    unsigned char *cbuf = img.bits();

    const QImage& baseTexImg = baseTex->img;
    for (int c = 0;  c < size; c += 4)
    {
        float fb = float(baseTexImg.bits()[c]);
        float fg = float(baseTexImg.bits()[c + 1]);
        float fr = float(baseTexImg.bits()[c + 2]);	

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



bool PicBucket::loadXML(const string& data)
{
    //QDomDocument doc;
    XMLDocument doc;

    if (doc.Parse(data.c_str()) != XML_NO_ERROR) {
        printf("error parsing XML file:\n");
        return false;
    }

    XMLElement* main = doc.FirstChildElement();

    map<string, int> grpnames; // maps the name of a group to its index. used for copying pieces
    //QDomElement docElem = doc.documentElement();
    XMLElement* texe = main->FirstChildElement("textures");

    ////////////// texture files ////////////////////////
    XMLElement* t = texe->FirstChildElement("texture");
    while (t != NULL)
    {
        bool in3d = t->Attribute("in3d", "true") != NULL;
        const char *txnamep = t->Attribute("filename");
        if (txnamep != NULL)
        {
            string txname(txnamep);
            QImage img(txname.c_str(), "PNG");
            
            if (img.isNull())
            {
                printf("Failed to load texture file: ");
                return false;
            }
            
            newTexture(img, in3d);
        }


        t = t->NextSiblingElement();
    }

//	unsigned int xxx3 = GetTickCount();

    ////////////////////// pics ////////////////////////////////
    XMLElement* xfams = main->FirstChildElement("outlines");

    int grpCount = 0;
    // count the number of cubes (groups) we have
    XMLElement* xfam = xfams->FirstChildElement("family");
    while (xfam != NULL)
    {
        int ind = xfam->IntAttribute("index"); // the index it should show at the main build helper menu
        if (ind >= families.size())
            families.resize(ind + 1);
        PicFamily &cfam = families[ind];
        cfam.startIndex = grpCount;
        cfam.name = xfam->Attribute("name");
        cfam.onResetSetCount = xfam->IntAttribute("onResetSetCount");
        cfam.iconFilename = xfam->Attribute("icon");

        cfam.numGroups = 0; // xfam->ElementsByTagName("group").size();
        XMLElement* xgrps = xfam->FirstChildElement("group");
        while (xgrps != NULL) {
            ++cfam.numGroups;
            xgrps = xgrps->NextSiblingElement("group");
        }
        grpCount += cfam.numGroups;
        xfam = xfam->NextSiblingElement(); // next family
    }

    pdefs.resize(grpCount*6);
    // the painter includes a pointer to the def so it needs to be initialized in the real vector
    for(int i = 0; i < grpCount*6; ++i)
        pdefs[i].reset();


    sumPics = 0;
    int grpi = 0; // (i) group number
    int pdefi = 0;

    xfam = xfams->FirstChildElement("family");
    while (xfam != NULL)
    {
        XMLElement* cube = xfam->FirstChildElement("group"); // a group of pieces is a cube
        while (cube != NULL)
        {
            PicGroupDef cgrp;
            cgrp.name = cube->Attribute("name");
            grpnames[cgrp.name] = grpi;
    
            XMLElement* fill = cube->FirstChildElement("fill");

            cgrp.drawtype = getDrawType(fill->Attribute("type"));

            Texture* baseTex = NULL;
            GlTexture* baseGTex = NULL;
            if (fill->Attribute("texind") != NULL)
            {
                int txind = fill->IntAttribute("texind");
                M_ASSERT((txind >= 0) && (txind <= texs.size()));
                baseTex = texs[txind];
                baseGTex = gtexs[txind+1]; // gtex starts with the noise tex at 0
            }

            cgrp.tex = baseTex;

            cgrp.color = Vec3( float(fill->IntAttribute("r")) / 255.0f, 
                               float(fill->IntAttribute("g")) / 255.0f, 
                               float(fill->IntAttribute("b")) / 255.0f);
            cgrp.exColor = Vec3( float(fill->IntAttribute("exR")) / 255.0f, 
                                 float(fill->IntAttribute("exG")) / 255.0f,
                                 float(fill->IntAttribute("exB")) / 255.0f);
            cgrp.blackness = (EBlackness)fill->IntAttribute("k");
    

            if (cgrp.drawtype == DRAW_TEXTURE_BLEND || cgrp.drawtype == DRAW_TEXTURE_MARBLE)
            {
                cgrp.tex = newTexture(cgrp.blendImage(baseTex), false);
                cgrp.gtex = gtexs[0]; //the noise tex
            }
            else if (cgrp.isIndividual()) {
                cgrp.gtex = baseGTex;
            }

            //QDomNodeList xpics = cube->ElementsByTagName("piece");
            XMLElement* xpic = cube->FirstChildElement("piece");
            if (xpic != NULL)
            {
                //cgrp.pics.resize();
                int pici = 0;
                while (xpic != NULL)
                {
                    PicDef& curdef = pdefs[pdefi];
                    curdef.mygrpi = grpi;
                    curdef.indexInGroup = pici;

                    //QDomElement ep = xpics.at(pici).toElement();
                    QString text = xpic->GetText();
                    text.remove(' ');
                    for (int y = 0; y < 5; ++y)
                    {
                        for (int x = 0; x < 5; ++x)
                        {
                            // get the first number;
                            if (text.at(0) == '0') 
                                curdef.v.set(x, y) = 0;
                            else 
                                curdef.v.set(x,y) = 1;
                            text.remove(0, 1);

                        }
                    }
                    if (cgrp.isIndividual())
                    {
                        curdef.xOffs = xpic->IntAttribute("x1");
                        curdef.yOffs = xpic->IntAttribute("y1");
                        curdef.tex = newTexture(cgrp.tex->img.copy(curdef.xOffs, curdef.yOffs, 128, 128).mirrored(false, true), false);
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
                    printf("can't copy piece from: %s\n", copyfrom.c_str());
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
                        curdef.tex = newTexture(cgrp.tex->img.copy(curdef.xOffs, curdef.yOffs, 128, 128).mirrored(false, true), false);
                    }

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



    return true;

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


void PicBucket::distinctMeshes()
{
    PicsSet ps;
    for(int i = 0; i < pdefs.size(); ++i) {
        ps.add(i, false);
    }
    printf("%d distinct meshes out of %d pieces\n", ps.comp.size(), pdefs.size());

    m_meshes.clear();
    for(int i = 0; i < ps.comp.size(); ++i) {
        auto comp = ps.comp[i];
        shared_ptr<PicDisp> pd(new PicDisp);
        pd->m_arr = comp.rtns[0];
        for(int j = 0; j < comp.addedInds.size(); ++j) {
            auto added = comp.addedInds[j];
            pdefs[added.addedInd].dispRot = added.defRot;
            pdefs[added.addedInd].disp = pd;
        }
        m_meshes.push_back(pd);
    }

}


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



void PicBucket::buildMeshes(const DisplayConf& dpc, ProgressCallback* prog)
{
    // fill meshes
    distinctMeshes();

    if (prog)
        prog->init(m_meshes.size());

    // grows if needed, never shrinks.
    PoolStats& ps = poolStats[dpc.numberOfPasses];
    PicDisp::getAllocator().init(ps.points, ps.poly, ps.he);
    PicDisp::getAllocator().clearMaxAlloc();

    int cnt = 0;
    bool cancel = false;

    for (int pi = 0; pi < m_meshes.size(); ++pi)
    {
        if (prog && !prog->setValue(pi))
            break;
        m_meshes[pi]->init(dpc);
    }

    if (prog)
        prog->setValue(m_meshes.size());
    PicDisp::getAllocator().checkMaxAlloc();
}

#define BUF_LEN 256

bool PicBucket::loadUnified(const string& filename) 
{
    distinctMeshes();

    ifstream iff(filename);
    if (!iff.good())
        return false;
    char linebuf[BUF_LEN] = {0};
    shared_ptr<Mesh::CommonData> cd(new Mesh::CommonData);
    double a,b,c;
    char *pa;
    Mesh *mesh = NULL;
    while (!iff.eof())
    {
        iff.getline(linebuf, BUF_LEN);
        uint len = strnlen(linebuf, BUF_LEN);
        if (len < 3)
            continue;
        if (linebuf[0] == 'v') { // vertex
            pa = linebuf + 2;
            a = strtod(pa, &pa);
            b = strtod(pa, &pa);
            c = strtod(pa, &pa);
            cd->vtx.push_back(Vec3(a, b, c));
            a = strtod(pa, &pa);
            b = strtod(pa, &pa);
            c = strtod(pa, &pa);
            cd->normals.push_back(Vec3(a, b, c));
        }
        else if (linebuf[0] == '*') {// mesh header
            ushort sig = strtoul(linebuf + 1, NULL, 16);
            for (auto it = m_meshes.begin(), endit = m_meshes.end();  it != endit; ++it)
                if (sig == (*it)->bits()) {
                    // finishes prev mesh, calc normals
                    mesh = &(*it)->m_mesh;
                    mesh->m_type = Mesh::QUADS;
                    mesh->m_hasIdx = mesh->m_hasNormals = true;
                    mesh->m_common = cd;
                    break;
                }
        }
        else {
            pa = linebuf;
            for(int fi = 0; fi < 4; ++fi) {
                int i = strtoul(pa, &pa, 10);
                mesh->m_idx.push_back(i);
            }
        }
    }
    return true;
}



// load a single big file with all the meshes separated by ****XXXX where XXXX is the bits of the pic
bool PicBucket::loadMeshes(const string& filename) 
{
    distinctMeshes();

    ifstream iff(filename);
    if (!iff.good())
        return false;
    char linebuf[BUF_LEN] = {0};
    Mesh *mesh = NULL;
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
            ushort sig = strtoul(linebuf + 4, NULL, 16);
            // find the next mesh
            for (auto it = m_meshes.begin(), endit = m_meshes.end();  it != endit; ++it)
                if (sig == (*it)->bits()) {
                    // finishes prev mesh, calc normals
                    if (mesh != NULL) {
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


int PicBucket::selectedCount() const
{
    int count = 0;
    for (int pi = 0; pi < pdefs.size(); ++pi)
    {
        count += pdefs[pi].getSelected();
    }
    return count;
}


int rotationAdd(int base, int defRot) 
{
    if ((defRot < 4) == (base < 4))
        return (base - defRot + 4) % 4;
    else
        return ((base + defRot) % 4) + 4;

}

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
