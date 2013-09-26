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

#include "ModelGLWidget.h"
#include "CubeDoc.h"
#include "MyObject.h"
#include "general.h"
#include "Pieces.h"
#include "Shape.h"
#include "Solutions.h"
#include "Cube.h"
#include "Texture.h"
#include "SlvCube.h"
#include "NoiseGenerator.h"
#include "OpenGL/glGlob.h"


ModelGLWidget::ModelGLWidget(GLWidget* gl, CubeDoc *document)
  : GLHandler(gl), m_gl(gl), m_doc(document)
{
    //m_fUseLight = m_doc->m_conf.disp.bLight;
    //m_bSkewReset = true; //false;

    m_nSingleChoise = -1;
    m_nHoverChoise = -1;
    m_nLastHoveChs = -1;

}

void ModelGLWidget::switchIn() {
    m_bgl->m_minScaleReset = 10;

    reCalcSlvMinMax();
}

void ModelGLWidget::reCalcSlvMinMax()
{
    SlvPainter &pnt = m_doc->getCurrentSolve()->painter;

    m_bgl->aqmin = pnt.qmin;
    m_bgl->aqmax = pnt.qmax;
}


void ModelGLWidget::initialized()
{
    m_progFlat.init();
    m_progNoise.init();
    mglCheckErrorsC("progs");
}

void ModelGLWidget::initTex()
{
    PicBucket::mutableInstance().gtexs.push_back(NoiseGenerator::make3Dnoise());
}



void ModelGLWidget::myPaintGL()
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


void ModelGLWidget::paint(BaseGLWidget* context, CubeDoc *doc, SlvCube *scube, bool fTargets, int singleChoise, int upToStep)
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


void ModelGLWidget::drawTargets(bool inChoise)
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



void ModelGLWidget::scrPress(bool rightButton, int x, int y) 
{
    if (rightButton) {
        m_nSingleChoise = m_bgl->doChoise(x, y) - 1;
        if (m_nSingleChoise != -1)
            emit chosenSinglePiece(m_nSingleChoise);
    }
}

void ModelGLWidget::scrRelease(bool rightButton) 
{
    if (rightButton) {
        m_nSingleChoise = -1;
        emit chosenSinglePiece(-1);
    }
}

void ModelGLWidget::scrMove(bool rightButton, bool ctrlPressed, int x, int y)
{
    if (m_doc->solvesExist())
    {
        m_nLastHoveChs = m_nHoverChoise;
        m_nHoverChoise = m_bgl->doChoise(x, y) - 1;
        //printf("%8X\n", m_nHoverChoise);
        if (m_nHoverChoise != m_nLastHoveChs)
            emit changedHoverPiece(m_nHoverChoise);
    }

}

// ---------------

void ModelGLWidget::updateView(int lHint)
{
    switch (GET_SLV_HINT(lHint))
    {
    case HINT_SLV_READY:
        // to be on the safe side (not to reference something that is not there)
        m_nHoverChoise = -1;
        m_nSingleChoise = -1;
        m_nLastHoveChs = -1;

        m_gl->setUpdatesEnabled(false); // make sure paintGL will not get called while we generate the ifs.
        // a call to painGL would cause a race condition on who generates the ifs first
        // and a memory leak.

        m_gl->makeCurrent();
        m_doc->getCurrentSolve()->genPainter();
        m_gl->setUpdatesEnabled(true);

        reCalcSlvMinMax();
        m_gl->doReset(); // TBD: check if we really need to do a reset, check if its the first solution.

    case HINT_SLV_NXPR:
    case HINT_SLV_NONE:
    case HINT_SLV_PAINT:
        m_gl->updateGL();
        break;
    }
}

