#include "ModelControlBase.h"
#include "SlvPainter.h"
#include "SlvCube.h"
#include "NoiseGenerator.h"


ModelControlBase::ModelControlBase(BaseGLWidget* gl, CubeDocBase *doc)
  : GLHandler(gl), m_doc(doc)
{

    m_nSingleChoise = -1;
    m_nHoverChoise = -1;
    m_nLastHoveChs = -1;

}

void ModelControlBase::switchIn() {
    m_bgl->m_minScaleReset = 10;

    reCalcSlvMinMax();
}

void ModelControlBase::reCalcSlvMinMax()
{
    SlvPainter &pnt = m_doc->getCurrentSolve()->painter;

    m_bgl->aqmin = pnt.qmin;
    m_bgl->aqmax = pnt.qmax;
}


void ModelControlBase::initialized()
{
    m_progFlat.init();
    m_progNoise.init();
    mglCheckErrorsC("progs");
}

void ModelControlBase::initTex()
{
    PicBucket::mutableInstance().gtexs.push_back(NoiseGenerator::make3Dnoise());
}



void ModelControlBase::myPaintGL()
{
    // draw the object
    drawTargets(false);

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
    

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POLYGON_OFFSET_FILL);

    mglCheckErrorsC("x1");
    // call the SlvPainter to do its job.
    scube->painter.paint(context, fTargets, singleChoise, upToStep, doc->m_conf.disp.nLines); 
    mglCheckErrorsC("x2");
}


void ModelControlBase::drawTargets(bool inChoise)
{
    SlvCube *slv = m_doc->getCurrentSolve();
    if (slv == NULL)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return;
    }

    if (slv->painter.isNull()) {
        slv->genPainter();
    }

    
    ShaderProgram* sel = NULL;
    if (inChoise) {
        glDisable(GL_BLEND);
        sel = &m_progFlat;
    }
    else {
        glEnable(GL_BLEND);
        sel = &m_progNoise;
    }

    ProgramUser u(sel);

    if (!inChoise)
        m_progNoise.lightPos.set(Vec3(0.0f, 0.0f, 1000.0f));


    paint(m_bgl, m_doc, slv, inChoise, m_nSingleChoise, m_doc->getUpToStep());

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
    if (m_doc->solvesExist())
    {
        m_nLastHoveChs = m_nHoverChoise;
        m_nHoverChoise = m_bgl->doChoise(x, y) - 1;
        //printf("%8X\n", m_nHoverChoise);
        if (m_nHoverChoise != m_nLastHoveChs)
            emitHoverPiece(m_nHoverChoise);
    }
    return false;
}