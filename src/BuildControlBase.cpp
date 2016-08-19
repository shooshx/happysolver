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

#include "BuildControlBase.h"
#include "Shape.h"
#include "Mesh.h"
#include "BuildWorld.h"
#include "CubeDocBase.h"
#include "Vec.h"

#include <set>

#define SHOW_START false //true


static void makeCylinder(Mesh& mesh, int slices, float radius, float length);


BuildControlBase::BuildControlBase(BaseGLWidget* gl, CubeDocBase* doc)
    :GLHandler(gl), m_doc(doc),
     m_bEditEnabled(true),
     m_fSetStrtMode(false),
     m_bInternalBoxRemove(false), m_bBoxRemove(false), 
     m_bLastBoxRemove(false),
     m_lastChoise(-1), 

     m_errCylindrAlpha(0.0), m_errCylindrAlphaDt(0.0),
     //m_bDoneUpdate(false),
     m_inFade(false),
     m_fadeFactor(0.0)
{
    reCalcBldMinMax();
    checkSides(); // build the initial test shape.

}

void BuildControlBase::initialized()
{
    m_prog.init();
    makeBuffers();
    makeCylinder(m_cylinder, 20, 0.1f, 1.2f);

}

void BuildControlBase::switchIn()
{
    m_bgl->m_minScaleReset = 6;
    m_bgl->aqmin = m_buildmin;
    m_bgl->aqmax = m_buildmax;
    m_bgl->m_cullFace = false;
}

void BuildControlBase::switchOut()
{
}


static void makeCylinder(Mesh& mesh, int slices, float radius, float length) 
{
    float coss[250] = {0};
    float sins[250] = {0};
    for(int i = 0; i < slices; ++i) {
        float angle = 2 * PI * i / slices;
        coss[i] = cos(angle) * radius;
        sins[i] = sin(angle) * radius;
    }

    mesh.m_type = Mesh::TRI_STRIP;
    mesh.m_hasIdx = true;
    mesh.addIdx(Mesh::TRI_FAN);
    mesh.addIdx(Mesh::TRI_FAN);
    Mesh::IdxBuf &d0 = mesh.m_addIdx[0];
    Mesh::IdxBuf &d1 = mesh.m_addIdx[1];
    mesh.m_vtx.push_back(Vec3(0.0f, 0.0f, 0.0f));
    mesh.m_vtx.push_back(Vec3(0.0f, 0.0f, length));
    d0.m_idx.push_back(0);
    d1.m_idx.push_back(1);

    for(int i = 0; i < slices; ++i) {
        mesh.m_vtx.push_back(Vec3(sins[i], coss[i], 0.0f));
        mesh.m_vtx.push_back(Vec3(sins[i], coss[i], length));
        int ia = i*2 + 2;
        mesh.m_idx.push_back(ia);
        mesh.m_idx.push_back(ia + 1);
        d0.m_idx.push_back(ia);
        d1.m_idx.push_back(ia+1);
    }
    mesh.m_idx.push_back(2);
    mesh.m_idx.push_back(3);
    d0.m_idx.push_back(2);
    d1.m_idx.push_back(3);

}


void BuildControlBase::myPaintGL(bool inChoise)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_bgl->model.push();
    double zv = m_bgl->zoomFactor() * m_preZoomFactor;
    m_bgl->model.scale(zv, zv, zv);
    m_bgl->modelMinMax(m_buildmin, m_buildmax);


    drawTargets(inChoise);

    m_bgl->model.pop();

#if 0
    QString str1("%1tiles: %2");
    QString str2;


    if (m_doc->getBuild().getChangedFromSave())
        str1 = str1.arg("*"); //add the * if the shape was changed from the last save
    else
        str1 = str1.arg("");

    str1 = str1.arg(m_doc->getBuild().nFaces);

    GL_BEGIN_TEXT();

    glColor3f(0.3f, 0.3f, 1.0f);
    glRasterPos2f(-0.95f, -0.92f);

    mglPrint(str1);

    if (!m_bBoxedMode)
    {
        if (m_doc->getBuild().fClosed)
        {
            str2 = "           Closed Shape%1";
            glColor3f(0.0f, 0.0f, 1.0f);
        }
        else
        {
            str2 = "           Opened Shape%1";
            glColor3f(1.0f, 0.2f, 0.2f);
        }
        if (m_doc->getBuild().getChangedFromGen())
            str2 = str2.arg("+"); //add the + if the shape was changed from the last generate
        else
            str2 = str2.arg("");

        glRasterPos2f(-0.95f, -0.92f);
        mglPrint(str2);
    }

    GL_END_TEXT();
#endif

    //m_bDoneUpdate = true;

}

class QuadAdder
{
public:
    QuadAdder(Mesh& mesh) : m_mesh(mesh) {
        m_mesh.m_type = Mesh::TRIANGLES;
        m_mesh.m_hasColors = true;
        m_mesh.m_hasNames = true;
        m_mesh.m_hasIdx = false;
        m_mesh.m_hasTag = true;
        m_mesh.clear();
    }

    void add(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, const Vec4& color, uint name, uint tag)
    {
        m_mesh.m_vtx.push_back(a);   
        m_mesh.m_vtx.push_back(b);   
        m_mesh.m_vtx.push_back(c);   

        m_mesh.m_vtx.push_back(a);
        m_mesh.m_vtx.push_back(c);
        m_mesh.m_vtx.push_back(d);

        Vec4b nv = Vec4b::fromName(name);
        float ftag = (float)tag;
        for(int i = 0; i < 6; ++i)
        {
            m_mesh.m_name.push_back(nv); 
            m_mesh.m_tag.push_back(ftag); 
            m_mesh.m_color4.push_back(color);
        }
    }

private:
    Mesh& m_mesh;

};

class LineAdder 
{
public:
    LineAdder(Mesh& mesh) : m_mesh(mesh), m_rep(&mesh.m_vtx) {
        m_mesh.m_type = Mesh::LINES;
        m_mesh.m_hasColors = false;
        m_mesh.m_hasNames = true;
        m_mesh.m_hasIdx = true;
        m_mesh.m_uniformColor = true;
        m_mesh.clear();
    }

    struct SortedPair {
        SortedPair(int _a, int _b) :a(_a), b(_b) {
            if (b < a)
                swap(a, b);
        }
        bool operator<(const SortedPair& o) const {
            if (a == o.a)
                return b < o.b;
            return a < o.a;
        }
        int a, b;
    };

    void addPair(int a, int b) {
        SortedPair s(a, b);
        auto it = m_added.find(s);
        if (it != m_added.end())
            return;
        m_mesh.m_idx.push_back(a);
        m_mesh.m_idx.push_back(b);
        m_added.insert(s);
    }

    void add(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, const Vec4& color)
    {
        int ia, ib, ic, id;
        m_rep.add(a, &ia); m_rep.add(b, &ib); m_rep.add(c, &ic); m_rep.add(d, &id);
        addPair(ia, ib);   addPair(ib, ic);   addPair(ic, id);   addPair(id, ia);
        m_mesh.m_uColor = color;
    }

    void addWithDir(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, const Vec4& color)
    {
        add(a,b,c,d,color);
        Vec3 e = a+(b-a)*0.20;
        Vec3 g = a+(d-a)*0.20;
        Vec3 h = b+(c-b)*0.20;
        int ie, ig, ih;
        m_rep.add(e, &ie); m_rep.add(g, &ig); m_rep.add(h, &ih);
        addPair(ie, ig);
        addPair(ig, ih);
    }

private:
    Mesh& m_mesh;
    VecRep m_rep;
    set<SortedPair> m_added;
};


void BuildControlBase::makeBuffers() 
{
    QuadAdder realTiles(m_realTiles);
    QuadAdder transTiles(m_transTiles);
    LineAdder realLines(m_realLines);
    LineAdder transLines(m_transLines);

#if 0 // some old test
    if (m_bBoxRemove) {
        const Shape& shp = m_doc->getBuild().getTestShape();
        Vec3 offs = Vec3(24,24,24) - Vec3(shp.faces[0].ex)/4.0;
        for(int i = 0; i < shp.testQuads.size(); i += 5)
        {
            Vec3 c = shp.testQuads[i+4];
            if (c.r == 1.0)
                realLines.addWithDir(shp.testQuads[i]/4+offs, shp.testQuads[i+1]/4+offs, shp.testQuads[i+2]/4+offs, shp.testQuads[i+3]/4+offs, Vec4(c.r, c.g, c.b, 1.0));
            else
                transLines.addWithDir(shp.testQuads[i]/4+offs, shp.testQuads[i+1]/4+offs, shp.testQuads[i+2]/4+offs, shp.testQuads[i+3]/4+offs, Vec4(c.r, c.g, c.b, 1.0));
        }

        return;
    }
#endif

    const BuildWorld &build = m_doc->getBuild();
    for(int dim = 0; dim < 3; ++dim)
    {
        const SqrLimits &lim = build.m_limits[dim];
        for(int page = lim.minpage; page < lim.maxpage; ++page)
        {
            for(int x = lim.minx; x < lim.maxx ; ++x)
            {
                for(int y = lim.miny; y < lim.maxy; ++y)
                {
                    int theget = build.get(dim, page, x, y);
                    if (GET_VAL(theget) == 0)
                        continue;
                    uint name = MAKE_NAME(dim, page, x, y);

                    Vec3 a,b,c,d;
                    switch (dim)
                    {
                    case YZ_PLANE:
                        a = Vec3(page, x, y);
                        b = Vec3(page, x + 1, y);
                        c = Vec3(page, x + 1, y + 1);
                        d = Vec3(page, x, y + 1);
                        break;
                    case XZ_PLANE:
                        a = Vec3(x, page, y);
                        b = Vec3(x + 1, page, y );
                        c = Vec3(x + 1, page, y + 1);
                        d = Vec3(x, page, y + 1);
                        break;
                    case XY_PLANE:
                        a = Vec3(x, y, page);
                        b = Vec3(x + 1, y, page);
                        c = Vec3(x + 1, y + 1, page);
                        d = Vec3(x, y + 1, page);
                        break;
                    }	

                    int valshow = GET_VAL_SHOW(theget);
                    Vec4 color;

                    if (GET_TYPE(theget) == TYPE_VIR) // blue
                    {
                        //cout << "VIR " << theget << endl;
                        color = Vec4(0.5f, 0.5f, 1.0f, 0.5f);
                        transTiles.add(a, b, c, d, color, name, 1);
                        transLines.add(a, b, c, d, Vec4(0.2f, 0.2f, 1.0f, 0.5f));
                        //cout << "TRANS " << a << " " << b << " " << c << " " << d << endl;
                    }
                    else
                    {
                        uint tag = 0;
                        if (valshow == FACE_NORM_SELR || valshow == FACE_STRT_SELR) {
                            color = Vec4(0.0f, 0.25f, 0.25f, 0.0f); // red (real color is 1-this, in shader
                            tag = 2;
                        }
                        else if (valshow == FACE_STRT && (m_fSetStrtMode || SHOW_START)) 
                            color = Vec4(1.0f, 1.0f, 0.0f, 1.0f); // yellow start tile
                        else
                            color = Vec4(1.0f, 1.0f, 1.0f, 1.0f); // normal white 
                        realTiles.add(a, b, c, d, color, name, tag);
                        realLines.add(a, b, c, d, Vec4(0.2f, 0.2f, 0.2f, 1.0f));
                    }
                }
            }
        }
    }

    m_realTiles.makeSelfBos(false);
    m_transTiles.makeSelfBos(false);
    m_realLines.makeSelfBos(false);
    m_transLines.makeSelfBos(false);


}
            


void BuildControlBase::drawErrorCyliders()
{
    const Shape &tstshp = m_doc->getBuild().getTestShape();

    for (int i = 0; i < tstshp.errorSides.size(); ++i)
    {
        const Shape::SideDef &sd = tstshp.errorSides[i];
        float x = sd.ex.x / 4.0,y = sd.ex.y / 4.0, z = sd.ex.z / 4.0;

        MatStack &model = m_bgl->model;
        model.push();

        switch (sd.dr)
        {
        case X_AXIS: model.translate(x - 0.1, y, z); model.rotate(90, 0, 1, 0);  break;
        case Y_AXIS: model.translate(x, y + 1.1, z); model.rotate(90, 1, 0, 0);  break;
        case Z_AXIS: model.translate(x, y, z - 0.1); break;
        }

        //glColor4f(1.0, 0.0, 0.0, m_errCylindrAlpha);
        m_prog.colorAatt.set(Vec4(1.0, 0.0, 0.0, m_errCylindrAlpha));
        m_prog.trans.set(m_bgl->transformMat());

        m_cylinder.paint();

        model.pop();
        m_prog.trans.set(m_bgl->transformMat());
    }
}


void BuildControlBase::drawTargets(bool inChoise)
{
    ProgramUser use(&m_prog);
    m_prog.trans.set(m_bgl->transformMat());

    m_prog.fadeFactor.set(m_fadeFactor);

    //glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_POLYGON_OFFSET_FILL);

    glPolygonOffset(1.0, 1.0);
    m_realTiles.paint(inChoise);

    if (!inChoise) {
        drawErrorCyliders();

        glPolygonOffset(0, 0);
        m_realLines.paint();

        glPolygonOffset(1.0, 1.0);
        m_transTiles.paint();
        glPolygonOffset(0, 0);
        m_transLines.paint();

    }
}


void BuildControlBase::checkSides()
{
    if (m_doc->getBuild().testShape() == GEN_RESULT_ILLEGAL_SIDE)
    { // has errors
        m_errCylindrAlpha = 1.0f;
        m_errCylindrAlphaDt = -0.1f;
    }
}


bool BuildControlBase::boxedDblClick(int choise, int x, int y)
{
    if ((choise == -1) || (!m_bEditEnabled))
        return false;

    BuildWorld& build = m_doc->getBuild();

    bool remove = hXor(m_bBoxRemove, m_bInternalBoxRemove);

    if (remove && (build.nFaces <= 6)) // last cube, don't remove, ignore command silently
        return false;

    CoordBuild bb[6];
    Vec3i g;
    if (!getChoiseTiles(choise, remove, bb, g))
        return false; // out of bounds, do nothing

    bool hasStrt = false;
    CoordBuild strtOrig; // the original dimention of the strt tile, if it's there
    for (int j = 0; j < 6; ++j) {// check if we're going to step on a start tile
        if (GET_VAL(build.get(bb[j])) == FACE_STRT)
        { 
            hasStrt = true; 
            strtOrig = bb[j];
            break; 
        }
    }

    // new way of replacing the start tile, search for a neighbor of the old start that is not in the new box.
    // This way, the start tile is always in the previoud solution
    // TBD - do this better with the current solution so that yellow is always on the previous solution
    CoordBuild startOptions[12];
    if (hasStrt)
    {
        build.getAllNei(strtOrig, startOptions);
        for(int i = 0; i < 12; ++i) {
            if (GET_TYPE(build.get(startOptions[i])) != TYPE_REAL)
                startOptions[i].dim = -1; // make it not viable
        }
    }
        
    int facePut = -1;
    for (int j = 0; j < 6; ++j) 
    {
        if (GET_TYPE(build.get(bb[j])) == TYPE_REAL)  //do the removes
        {
            build.set(bb[j], 0);
            --build.nFaces;
        }
        else
        {
            build.set(bb[j], FACE_NORM);
            ++build.nFaces;
        }
    }
    
    if (hasStrt) // take care of strt tile if it hasn't been taken care of yet
    {
        int i = 0;
        for (i = 0; i < 12; ++i) 
        {
            if (startOptions[i].dim != -1 && GET_TYPE(build.get(startOptions[i])) == TYPE_REAL) {
                build.set(startOptions[i], FACE_STRT);
                break; // TBD choose the oldest one
            }
        }
        if (i == 12) { // not found. can happen if we removed the last cube of a separate part and it had the start tile
            build.search(FACE_NORM, FACE_STRT, false, true);
        }
        
#if 0 // old way of searching for a start tile - always puts it on the new box. that's bad for incremental building        
        if (facePut != -1) // there are faces, 
        {
            int j;
            // search for a face with the orginal dimention
            for (j = 0; j < 6; ++j) {
                if (build.get(bb[j]) == FACE_NORM && bb[j].dim == strtOrig.dim) {
                    build.set(bb[j], FACE_STRT);
                    break;
                }
            }
            if (j == 6) { // did not find same dim
                for (j = 0; j < 6; ++j) { // choose a face we put NORM in and change it to STRT
                    if (build.get(bb[j]) == FACE_NORM) { 
                        build.set(bb[j], FACE_STRT); 
                        break; 
                    }
                }
            }
        }
        else // there are no faces to transfer the STRT to... settle for anything
        {
            build.search(FACE_NORM, FACE_STRT, false, true);
        }
#endif
    }

    build.m_space.axx(g).fill = remove?0:1;

    build.justChanged();
    build.reClacLimits();

    checkSides(); // testShape does the generate, created cyliders

    emitTilesCount(build.nFaces);

    // make buffers and then simluate move to doChoise on the new buffers
    makeBuffers();
    m_bgl->invalidateChoice();
    doMouseMove(x, y, m_bInternalBoxRemove);
    return true;
}

void BuildControlBase::reloadWorld()
{
    m_doc->getBuild().reClacLimits();
    makeBuffers();
    m_bgl->invalidateChoice();
}


bool BuildControlBase::fadeTimeout(float delta)
{
    bool ret = false;
    if (m_inFade) {
        m_fadeFactor += delta;
        if (m_fadeFactor >= 1.0)
            m_inFade = false;
        ret = true;
    }

   /* BuildWorld& build = m_doc->getBuild();
    if (build.getTestResult() == GEN_RESULT_ILLEGAL_SIDE)
    {
        m_errCylindrAlpha += m_errCylindrAlphaDt;
        if ((m_errCylindrAlpha >= 1.0) || (m_errCylindrAlpha <= 0.1))
            m_errCylindrAlphaDt = -m_errCylindrAlphaDt;
        ret = true;
    }*/

    return ret;
}

bool BuildControlBase::getChoiseTiles(int choise, bool remove, CoordBuild bb[6], Vec3i& g)
{
    CoordBuild c(GET_DIM(choise), GET_PAGE(choise), GET_X(choise), GET_Y(choise));
    BuildWorld& build = m_doc->getBuild();

    Vec3i g1, g2;
    BuildWorld::get3dCoords(c, g1, g2);
    //cout << "FILLE " << (int)build.m_space.axx(g1).fill << endl;
    if (hXor(build.m_space.axx(g1).fill == 1, remove))
        g = g2;
    else
        g = g1;

    if ((g.x < 1) || (g.x >= build.m_space.szx - 2) ||   // stay away from zeros and 49s
        (g.y < 1) || (g.y >= build.m_space.szy - 2) || 
        (g.z < 1) || (g.z >= build.m_space.szz - 2))
        return false;

    //int xxx = build.m_space.axx(g).fill;
//    M_ASSERT(build.m_space.axx(g).fill == (remove ? 1 : 0)); // XXXa really nasty bug.

    BuildWorld::getBuildCoords(g, bb);
    return true;
}



bool BuildControlBase::isInRemove() {
    return hXor(m_bBoxRemove, m_bInternalBoxRemove);
}

// returns true if an updateGL is needed, false if not
bool BuildControlBase::doMouseMove(int x, int y, bool ctrlPressed)
{
    if (m_fSetStrtMode)
        return false;
    
    int choise;
    if (x != -1) // support non-mouse updates
        choise = m_bgl->doChoise(x, y);
    else
        choise = m_lastChoise;

    return choiseMouseMove(choise, ctrlPressed);
}

void BuildControlBase::clearChoise() // for when we're moving the mouse rotating and don't want to see a selection
{
    choiseMouseMove(-1, false);
}

bool BuildControlBase::choiseMouseMove(int choise, bool ctrlPressed)
{
   // cout << "c" << endl;
    m_bInternalBoxRemove = ctrlPressed;
    bool remove = isInRemove();
    if ((choise == m_lastChoise) && (remove == m_bLastBoxRemove)) {
       // cout << "err " << choise << " " << m_lastChoise << " " << remove << " " << m_bLastBoxRemove << endl;
        return false; // check if remove state just changed so we need to redraw
    }
  //  cout << "MOVE " << remove << " " << m_bLastBoxRemove << " " << choise << endl;

    m_lastChoise = choise;
    m_bLastBoxRemove = remove;
    EActStatus act = remove ? REMOVE : ADD;

    BuildWorld& build = m_doc->getBuild();
    
    if (choise >= 0) // -1 comes from above
    { // something chosen

        CoordBuild bb[6];
        Vec3i g;
        if ((getChoiseTiles(choise, remove, bb, g) && (g != m_lastCubeChoise)))
        { // selection was changed
            m_doc->clearRemoveFlags();
          //  cout << "get " << choise << " " << hex << choise << dec << "  " << g << "  fill=" << (int)build.m_space.axx(g).fill << endl;

            //printf("%8X (%d,%d,%d) != (%d,%d,%d)\n", choise, g.x, g.y, g.z, m_lastCubeChoise.x, m_lastCubeChoise.y, m_lastCubeChoise.z);
            if (choise < 0x10000)
                throw HCException("bad choise"); //DebugBreak();

            if (m_bEditEnabled)
            {
                //cout << "CLEAN " << remove << endl;
                build.clean(BuildWorld::CLEAN_TRANS_SHOW);

                if (!remove)
                {
                    for (int j = 0; j < 6; ++j) // record the actions needed,
                    {
                        auto t = GET_TYPE(build.get(bb[j]));
                        if (t != TYPE_REAL)  //do the removes
                        {
                            build.set(bb[j], FACE_TRANS_SEL);
                           // cout << "TTT " << t << "  " << bb[j].x << "," << bb[j].y << "," << bb[j].page << "," << bb[j].dim << endl;
                        }
                        //else
                        //    cout << "XXX " << t << endl;
                    }
                    m_fadeFactor = 0.0f;
                    m_inFade = true;
                }
                else if (build.nFaces > 6) // if its the last one, don't show the red
                {
                    for (int j = 0; j < 6; ++j) // record the actions needed,
                    {
                        int theget = build.get(bb[j]);
                        if (GET_TYPE(theget) == TYPE_REAL)  //do the removes
                        {
                            build.set(bb[j], theget | SHOW_REOMOVE);
                        }

                        auto shp = m_doc->getCurrentShape();
                        if (shp != nullptr)
                        {
                            int fc = shp->getShapeFcInd(bb[j]);
                            if (fc != -1) {
                                m_doc->m_flagPiece[fc] = 1;
                            }
                        }
                    }
                    m_fadeFactor = 0.0f;
                    m_inFade = true;
                }
                else
                    act = CANT_REMOVE;

                makeBuffers();
            }
            else
                act = EDIT_DISABLE;

            m_lastCubeChoise = g;
            emitTileHover(choise, act);
        }
        // don't emit changedTileHover
    }
    else
    {
        //printf("  -1 cleared %d\n", makeBufs);
        // clean the trans place or remove from the last time
        build.clean(BuildWorld::CLEAN_TRANS_SHOW);
        m_doc->clearRemoveFlags();
        m_lastCubeChoise = Vec3i(-1,-1,-1);
        makeBuffers();
        emitTileHover(choise, act);
    }
    //printf("end move\n");
    return true;
}


bool BuildControlBase::scrMove(bool rigthBot, bool ctrlPressed, int x, int y)
{
    
    bool needupdate = doMouseMove(x, y, ctrlPressed);

  //  m_bDoneUpdate = false;
  //  if (needupdate && (!m_bDoneUpdate))
  //      m_gl->updateGL();
    return needupdate;
}

bool BuildControlBase::scrDblClick(bool ctrlPressed, int x, int y)
{
    m_bInternalBoxRemove = ctrlPressed;
    int choise = m_bgl->doChoise(x, y);
    //cout << "DblClick: " << hex << choise << endl;
    return choiseDblClick(choise);
}

bool BuildControlBase::choiseDblClick(int choise)
{
    //cout << "CHOISE " << GET_DIM(choise) << "," << GET_X(choise) << "," << GET_Y(choise) << "," << GET_PAGE(choise) << endl;
    if (!m_fSetStrtMode)
        return boxedDblClick(choise, -1, -1);
    else {
        return tiledDblClick(choise);
    }
}


void BuildControlBase::reCalcBldMinMax()
{
    BuildWorld &build = m_doc->getBuild();
    build.clean(BuildWorld::CLEAN_TRANS_SHOW);
    build.reClacLimits();

    const SqrLimits &lYZ = build.m_limits[YZ_PLANE];
    m_buildmin = Vec3(lYZ.minpage, lYZ.minx, lYZ.miny);
    m_buildmax = Vec3(lYZ.maxpage, lYZ.maxx, lYZ.maxy);

    const SqrLimits &lXZ = build.m_limits[XZ_PLANE];
    m_buildmin.pmin(Vec3(lXZ.minx, lXZ.minpage, lXZ.miny));
    m_buildmax.pmax(Vec3(lXZ.maxx, lXZ.maxpage, lXZ.maxy));

    const SqrLimits &lXY = build.m_limits[XY_PLANE];
    m_buildmin.pmin(Vec3(lXY.minx, lXY.miny, lXY.minpage));
    m_buildmax.pmax(Vec3(lXY.maxx, lXY.maxy, lXY.maxpage));

    //cout << "~BLD " << m_buildmax << " : " << m_buildmin << " = " << m_buildmax - m_buildmin << endl;

    //m_buildmax += Vec3(-1, -1, -1); - instead this-
    // I have really no idea why this is needed but it works. previous -1,-1,-1 had trouble with flat 2x3
    if (!build.m_limits[YZ_PLANE].isInverse(BUILD_SIZE))
        m_buildmax.x -= 1;
    if (!build.m_limits[XZ_PLANE].isInverse(BUILD_SIZE))
        m_buildmax.y -= 1;
    if (!build.m_limits[XY_PLANE].isInverse(BUILD_SIZE))
        m_buildmax.z -= 1;


}


bool BuildControlBase::tiledDblClick(int choise)
{
    if (choise == -1)
        return false;

    BuildWorld& build = m_doc->getBuild();
    CoordBuild c(GET_DIM(choise), GET_PAGE(choise), GET_X(choise), GET_Y(choise));
    int theget = build.get(c);

    if (GET_VAL(theget) == FACE_TRANS)
    {
        build.set(c, FACE_NORM);
        build.nFaces++;
        build.justChanged();
        build.doTransparent();
    }
    else
    {	
        if (GET_TYPE(theget) == TYPE_REAL)
        {
            if (!m_fSetStrtMode)
            {
                if (build.nFaces > 1)
                {
                    build.set(c, 0);
                    build.nFaces--;
                    build.justChanged();
                    build.doTransparent();
                }
            }
            else
            {
                if (theget != FACE_STRT)
                {
                    build.search(FACE_STRT, FACE_NORM); // clean the current strt
                    build.set(c, FACE_STRT);
                    build.justChanged();
                    checkSides(); // does the generate. (for recalculating the stepping)
                }
            }
        }
    }
    makeBuffers();
    m_bgl->invalidateChoice();
    return true;
}






