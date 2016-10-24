#include "ModelControlBase.h"
#include "SlvPainter.h"
#include "SlvCube.h"
#include "NoiseGenerator.h"


#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#define ARROW_BUTTON_ANIM_TIME 0.6
#define RESET_ROTATION_TIME 0.5

ModelControlBase::ModelControlBase(BaseGLWidget* gl, CubeDocBase *doc)
    : GLHandler(gl), m_doc(doc), m_buildCtrl(gl, doc)
    , m_spherePopAnim(ARROW_BUTTON_ANIM_TIME)
    , m_resetPositionAnim(RESET_ROTATION_TIME, gl)
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
#ifndef EMSCRIPTEN
    static bool did = false;
    if (did)
        return; // HACK, don't want to recenter each time so that the build and model would be in sync (in js, I take of not to call this func when not needed)
    did = true;  // don't recenter every time TBD hack
#endif

    const Shape* measureShp = nullptr;
    auto slv = m_doc->getCurrentSolve();
    if (slv != nullptr) 
        measureShp = slv->shape;
    else 
        measureShp = m_doc->getCurrentShape(); // happens when moving to the editor and there is just a shape that was just generated

    if (measureShp == nullptr) {
        return;
    }
  //  SlvPainter &pnt = slv->painter;

    m_bgl->aqmin = m_modelmin = measureShp->qmin; // always 0,0,0 from genPainter
    m_bgl->aqmax = m_modelmax = measureShp->qmax;

    //cout << "~MDL " << m_bgl->aqmax << " : " << m_bgl->aqmin << endl;

    m_buildCtrl.reCalcBldMinMax();
    m_bldDiff = (Vec3(23, 23, 23) - m_buildCtrl.m_buildmin) * 4;
   // m_bldDiff = BUILD_START_CUBE * 4 - m_doc->getCurrentShape()->buildBounds; - results in the same thing
    // this accounts for the difference in coordinates between the build and the generated shape

   // cout << "DFF " << m_bldDiff << " :: " << m_modelmin << endl;
    //auto b = m_doc->getCurrentShape()->buildBounds;
    //cout << "BND " << b.minx -96 << "," << b.miny -96<< "," << b.minpage -96<< endl;
}


void ModelControlBase::initialized()
{
    m_progFlat.init();
    mglCheckErrorsC("progs");

    m_progNoise.init();
    {
        ProgramUser pu(&m_progNoise);
        m_progNoise.lightPos.set(Vec3(0.0f, 0.0f, 1000.0f));    
    }

    m_buildCtrl.initialized();

    if (m_addArrows) {
        m_arrowMesh.reset(new Mesh);
        m_arrowMesh->load("arrow_mesh");
        m_arrowMesh->m_uniformColor = true;

        m_sphereMesh.reset(new Mesh);
        m_sphereMesh->load("sphere_mesh");
        m_sphereMesh->m_uniformColor = true;
    }
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

// called in the initialization
// preps the textures in the pieces bucket for the xml loading
void ModelControlBase::initTex()
{
    auto& bucket = PicBucket::mutableInstance();
#ifdef QT_CORE_LIB
    bucket.gtexs.push_back(shared_ptr<GlTexture>(NoiseGenerator::make3Dnoise()));
#else
    auto tex = make_shared<JsGlTexture>();
    tex->init(GL_TEXTURE_2D, Vec2i(1024, 512), 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, nullptr, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    tex->registerBind("noisetex");
    bucket.gtexs.push_back(tex);

    bucket.gtexs.push_back(nullptr); // black
    bucket.gtexs.push_back(nullptr); // marble

	// little genius textures
 /*   tex = make_shared<JsGlTexture>(); disabled since everthing moved to svg
    tex->init(GL_TEXTURE_2D, Vec2i(1024, 1024), 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, nullptr, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    tex->registerBind("genious");
    bucket.gtexs.push_back(tex);
    
    tex = make_shared<JsGlTexture>();
    tex->init(GL_TEXTURE_2D, Vec2i(1024, 1024), 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, nullptr, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    tex->registerBind("newgenious");
    bucket.gtexs.push_back(tex);
   */
#endif
}

void ModelControlBase::drawAxisArrows()
{
    int sz = min(m_bgl->m_cxClient, m_bgl->m_cyClient) * 0.14;
    glViewport(0, 0, sz, sz);

    // area for clicking
    float sphereMargin = sz * 0.3;
    m_arrowPressArea = Recti(sphereMargin, m_bgl->m_cyClient - sz + sphereMargin, sz - 2 * sphereMargin, sz - 2 * sphereMargin);
    EM_ASM_(testRect($0, $1, $2, $3), m_arrowPressArea.x1, m_arrowPressArea.y1, m_arrowPressArea.x2, m_arrowPressArea.y2);
   // cout << "Arrows SZ=" << sz << " mr=" << sphereMargin << "  " << m_arrowPressArea.x1 << "," << m_arrowPressArea.y1 << "," << m_arrowPressArea.x2 << "," << m_arrowPressArea.y2 << endl;

    ProgramUser pu(&m_progNoise);
    m_progNoise.drawtype.set(DRAW_COLOR);

    m_bgl->proj.push();
    m_bgl->proj.cur() = m_bgl->m_fixedAspectProj;

    // sphere
    m_bgl->model.push();
    m_bgl->model.identity();  
    float animv = m_spherePopAnim.m_value;
    m_bgl->model.scale(0.6 + (animv * 0.25)); // when hovering make it bigger
    m_progNoise.setModelMat(m_bgl->model.cur());
    m_progNoise.trans.set(m_bgl->transformMat());
    m_sphereMesh->m_uColor = Vec3(1.0, 1.0 - animv*0.4 - 0.3*m_arrowSpherePressed, 0.0); // when hovering fade from yellow to orange
    m_sphereMesh->paint();
    m_bgl->model.pop();

    // arrows
    m_bgl->model.push();
    m_bgl->model.scale(0.105, 0.105, 0.105);

    m_progNoise.setModelMat(m_bgl->model.cur());
    m_progNoise.trans.set(m_bgl->transformMat());
    m_arrowMesh->m_uColor = Vec3(1.0, 0.2, 0.2);
    m_arrowMesh->paint();

    m_bgl->model.rotate(90, -1, 0, 0);
    m_progNoise.setModelMat(m_bgl->model.cur());
    m_progNoise.trans.set(m_bgl->transformMat());
    m_arrowMesh->m_uColor = Vec3(0.2, 0.7, 0.2);
    m_arrowMesh->paint();

    m_bgl->model.rotate(90, 0, 1, 0);
    m_progNoise.setModelMat(m_bgl->model.cur());
    m_progNoise.trans.set(m_bgl->transformMat());
    m_arrowMesh->m_uColor = Vec3(0.2, 0.2, 1.0);
    m_arrowMesh->paint();
    m_bgl->model.pop();
    
    m_bgl->proj.pop();

    glViewport(0, 0, m_bgl->m_cxClient, m_bgl->m_cyClient);

}

void ModelControlBase::myPaintGL(bool inChoise)
{
    if (!inChoise) // in choise, it's already doing its own clear with black, not this back color
    {
        SlvCube *scube = m_doc->getCurrentSolve();
        Vec3 bkc = m_doc->m_conf.disp.slvBkColor;
        if (m_nSingleChoise >= 0 && scube != nullptr && scube->getPieceGrpDef(m_nSingleChoise)->blackness > BLACK_NOT)
            glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        else
            glClearColor(bkc.r, bkc.g, bkc.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (m_addArrows) {
            drawAxisArrows();
        }
    }

    // draw the object
//    double zv = zoomFactor();
    double zv = m_bgl->zoomFactor();

    m_bgl->model.push();
    m_bgl->model.scale(zv, zv, zv);
    m_bgl->model.translate(m_bldDiff.x, m_bldDiff.y, m_bldDiff.z);
    m_bgl->modelMinMax(m_modelmin, m_modelmax);
    //cout << "PAINT " << m_modelmin << " : " << m_modelmax << endl;
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

    m_arrowSpherePressed = m_addArrows && m_arrowPressArea.isInside(x, y);
}

void ModelControlBase::scrRelease(bool rightButton, int x, int y) 
{
    if (rightButton) {
        m_nSingleChoise = -1;
        emitChosenPiece(-1);
    }

    if (m_arrowSpherePressed && m_arrowPressArea.isInside(x, y)) {
        m_resetPositionAnim.reset(m_bgl->model.cur(), m_bgl->getInitRotation());
        m_bgl->addProgressable(&m_resetPositionAnim);
    }
    m_arrowSpherePressed = false;
}


bool SlerpProgress::reset(const Mat4 startt, const Mat4& endt)
{
    m_qstart = Quaternion::fromMat(startt);
    m_qend = Quaternion::fromMat(endt);
    return FloatProgress::reset();
}
bool SlerpProgress::progress(float deltaSec)
{
    bool stillGoing = FloatProgress::progress(deltaSec);
    // need to make the final adjustments even if the anim was just done
    Quaternion qm = Quaternion::slerp(m_qstart, m_qend, m_value);
    Mat4 mm = qm.toMatrix();
    m_bgl->model.set(mm);

    if (!stillGoing) {
        m_bgl->invalidateChoice(); // do it only not because selection is not needed during the animation and it will just waste cpu
        return false;
    }
    return true;
}


bool ModelControlBase::shapeHoverForBuild(int choise, bool rightButton, bool ctrlPressed, int x, int y)
{
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

bool ModelControlBase::scrMove(bool rightButton, bool ctrlPressed, int x, int y)
{
    if (m_arrowPressArea.isInside(x, y)) {
        //   cout << "POP-START " << x << " " << y << endl;
        m_bgl->addProgressable(&m_spherePopAnim);
        return true;
    }
    else {
        if (m_spherePopAnim.reset()) {
            m_arrowSpherePressed = false;
            return true; // need to make sure we make an update frame, never mind we loose a single move event
        }
    }


    // if buttons are pressed, don't sample because it lowers the FPS of rotating significantly
    if (rightButton)
        return false;

    int choise = m_bgl->doChoise(x, y) - 1;

#ifndef EMSCRIPTEN // used to be used in QT to show the current selected piece
    if (m_doc->solvesExist())
    {
        m_nLastHoveChs = m_nHoverChoise;
        m_nHoverChoise = choise;
        if (m_nHoverChoise != m_nLastHoveChs)
            emitHoverPiece(m_nHoverChoise);
    }
#endif

    return shapeHoverForBuild(choise, rightButton, ctrlPressed, x, y);
}

bool ModelControlBase::scrDblClick(bool hasCtrl, int x, int y) 
{ 
    int choise = m_bgl->doChoise(x, y) - 1;
    //cout << "CHS " << choise << endl;
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

    restartSolve(true, false);
    return true;
}



bool FloatProgress::progress(float deltaSec) {
    m_value += deltaSec / m_ttime;
   // cout << "POP " << m_value << endl;
    if (m_value >= 1.0) {
        m_value = 1.0;
        return false;
    }
    return true;
}


void ModelControlBase::restartSolve(bool withCurrentAsStarter, bool keepPrev)
{
    if (m_doc->isSlvEngineRunning()) {
        m_doc->solveStop();
    }

    SlvCube* starter = nullptr;
    if (withCurrentAsStarter) {
        m_doc->transferShape(); // does generate
        starter = m_doc->getCurrentSolve();
    }
    m_doc->solveGo(starter, keepPrev);

}