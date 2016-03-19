#include "ModelControlBase.h"
#include "SlvPainter.h"
#include "SlvCube.h"
#include "NoiseGenerator.h"

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

ModelControlBase::ModelControlBase(BaseGLWidget* gl, CubeDocBase *doc)
    : GLHandler(gl), m_doc(doc), m_buildCtrl(gl, doc)
{

    m_nSingleChoise = -1;
    m_nHoverChoise = -1;
    m_nLastHoveChs = -1;

 //   m_buildCtrl.m_preZoomFactor = 4.0; // factor to fit the scale the the solutions
}

void ModelControlBase::switchIn() {
    m_bgl->m_minScaleReset = 10;

    reCalcSlvMinMax(); 
}

void ModelControlBase::reCalcSlvMinMax()
{
    static bool did = false;
    if (did)
        return; // HACK, don't want to recenter each time so that the build and model would be in sync
    did = true;  // don't recenter every time TBD hack
    auto slv = m_doc->getCurrentSolve();
    if (slv == nullptr)
        return;
    SlvPainter &pnt = slv->painter;

    m_bgl->aqmin = m_modelmin = pnt.qmin;
    m_bgl->aqmax = m_modelmax = pnt.qmax;
}


void ModelControlBase::initialized()
{
    m_progFlat.init();
    m_progNoise.init();
    mglCheckErrorsC("progs");

    m_buildCtrl.initialized();

}

#ifdef EMSCRIPTEN
class JsGlTexture : public GlTexture
{
public:
    void registerBind(const char* imgname) {
        EM_ASM_( registerTexBind(Pointer_stringify($0), $1), imgname, m_obj);
    }
};
#endif

void ModelControlBase::initTex()
{
    auto& bucket = PicBucket::mutableInstance();
#ifdef QT_CORE_LIB
    bucket.gtexs.push_back(NoiseGenerator::make3Dnoise());
#else
    auto tex = new JsGlTexture;
    tex->init(GL_TEXTURE_2D, Vec2i(1024, 512), 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, nullptr, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    tex->registerBind("noisetex");
    bucket.gtexs.push_back(tex);

    bucket.gtexs.push_back(nullptr); // black
    bucket.gtexs.push_back(nullptr); // marble

	// little genius textures
    tex = new JsGlTexture;
    tex->init(GL_TEXTURE_2D, Vec2i(1024, 1024), 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, nullptr, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    tex->registerBind("genious");
    bucket.gtexs.push_back(tex);
    
    tex = new JsGlTexture;
    tex->init(GL_TEXTURE_2D, Vec2i(1024, 1024), 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, nullptr, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    tex->registerBind("newgenious");
    bucket.gtexs.push_back(tex);
   
#endif
}



void ModelControlBase::myPaintGL(bool inChoise)
{
    // draw the object
//    double zv = zoomFactor();
    double zv = m_bgl->zoomFactor();

    m_bgl->model.push();
    m_bgl->model.scale(zv, zv, zv);
    m_bgl->modelMinMax(m_modelmin, m_modelmax);
    drawTargets(inChoise);
    m_bgl->model.pop();

    m_bgl->model.push();
    m_bgl->model.scale(zv * 4, zv * 4, zv * 4); // in build, every unit is a cube. in model every unit is a tooth
    m_bgl->modelMinMax(m_buildCtrl.m_buildmin, m_buildCtrl.m_buildmax);
    m_buildCtrl.drawTargets(inChoise);

    m_bgl->model.pop();


    // draw the text in the bottom left corner which indicates which solution this is, out fo how many.
    // this can't be done with lighting since it will change the text color.
#if 0
    glDisable(GL_LIGHTING);

    mglCheckErrorsC("x21");
    GL_BEGIN_TEXT();
    
    glColor3f(0.3f, 0.3f, 1.0f);
    glRasterPos2f(-0.95f, -0.92f);
    mglCheckErrorsC("x22");

    mglPrint(QString("%1/%2").arg(m_doc->getCurrentSolveIndex() + 1).arg(m_doc->getSolvesNumber()));

    mglCheckErrorsC("x23");
    GL_END_TEXT();

    // re-enable lighting 
    glEnable(GL_LIGHTING);
#endif
}


void ModelControlBase::paint(BaseGLWidget* context, CubeDocBase *doc, SlvCube *scube, bool fTargets, int singleChoise, int upToStep)
{	
    if (singleChoise > scube->numPieces())
    { // caused by untimely updates of the single piece view. show the first one
        singleChoise = 0; // not -1 because that will show the entire model
    }

    if (!fTargets)
    {
        //glEnable(GL_COLOR_MATERIAL);
        Vec3 bkc = doc->m_conf.disp.slvBkColor;
        if ((singleChoise >= 0) && (scube->getPieceGrpDef(singleChoise)->blackness > BLACK_NOT))
            glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        else
            glClearColor(bkc.r, bkc.g, bkc.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    

 //   glEnable(GL_BLEND);
 //   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 //   glEnable(GL_POLYGON_OFFSET_FILL);

    mglCheckErrorsC("x1");
    // call the SlvPainter to do its job.
    scube->painter.paint(context, fTargets, singleChoise, upToStep, doc->m_conf.disp.nLines); 
    mglCheckErrorsC("x2");
}


void ModelControlBase::drawTargets(bool inChoise)
{

    SlvCube *slv = m_doc->getCurrentSolve();
    if (slv == nullptr)
    {
        // just do nothing, don't change the frame
      //  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
      //  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return;
    }

    if (slv->painter.isNull()) {
        slv->genPainter();
    }

    
    ShaderProgram* sel = nullptr;
    if (inChoise) {
  //      glDisable(GL_BLEND);
        sel = &m_progFlat;
    }
    else {
  //      glEnable(GL_BLEND);
        sel = &m_progNoise;
    }

    ProgramUser u(sel);

    if (!inChoise) {
        m_progNoise.lightPos.set(Vec3(0.0f, 0.0f, 1000.0f));
        m_progNoise.fadeFactor.set(m_buildCtrl.m_fadeFactor);
    }

    int upTo = m_doc->getUpToStep();

    if (slv != m_lastSlv || m_lastUpTo != upTo) {
        m_bgl->invalidateChoice();
        m_lastSlv = slv;
        m_lastUpTo = upTo;
    }
    

    paint(m_bgl, m_doc, slv, inChoise, m_nSingleChoise, upTo);

}



void ModelControlBase::scrPress(bool rightButton, int x, int y) 
{
    if (rightButton) {
        m_nSingleChoise = m_bgl->doChoise(x, y) - 1;
        if (m_nSingleChoise != -1)
            emitChosenPiece(m_nSingleChoise);
    }
}

void ModelControlBase::scrRelease(bool rightButton) 
{
    if (rightButton) {
        m_nSingleChoise = -1;
        emitChosenPiece(-1);
    }
}

bool ModelControlBase::scrMove(bool rightButton, bool ctrlPressed, int x, int y)
{
    int choise = m_bgl->doChoise(x, y) - 1;

#ifndef EMSCRIPTEN
    if (m_doc->solvesExist())
    {
        m_nLastHoveChs = m_nHoverChoise;
        m_nHoverChoise = choise;
        //printf("%8X\n", m_nHoverChoise);
        if (m_nHoverChoise != m_nLastHoveChs)
            emitHoverPiece(m_nHoverChoise);
    }
#endif

    const Shape* shp = m_doc->getCurrentShape();
    if (shp == nullptr)
        return false;
    bool ret = false;
    if (choise >= 0 && choise < shp->fcn)
    {
        CoordBuild cb = shp->fcToBuildCoord(choise);
        ret = m_buildCtrl.choiseMouseMove(MAKE_NAME(cb.dim, cb.page, cb.x, cb.y), ctrlPressed);
    }
    else 
    {
        ret = m_buildCtrl.scrMove(rightButton, ctrlPressed, x, y); //outside a piece, maybe its on the build?
    }
    return ret;
}

bool ModelControlBase::scrDblClick(bool hasCtrl, int x, int y) 
{ 
    int choise = m_bgl->doChoise(x, y) - 1;
    cout << "CHS " << choise << endl;
    const Shape* shp = m_doc->getCurrentShape();
    if (choise < shp->fcn) {
        CoordBuild cb = shp->fcToBuildCoord(choise);
        if (!m_buildCtrl.choiseDblClick(MAKE_NAME(cb.dim, cb.page, cb.x, cb.y)))
            return false;
    }
    else {
        if (!m_buildCtrl.scrDblClick(hasCtrl, x, y))
            return false;
    }

    restartSolve();
    return true;
}

void ModelControlBase::restartSolve()
{
    if (m_doc->isSlvEngineRunning()) {
        m_doc->solveStop();
    }

    m_doc->transferShape(); // does generate
    m_doc->solveGo();

}