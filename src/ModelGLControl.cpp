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

#include "ModelGLControl.h"
#include "CubeDoc.h"
#include "MyObject.h"
#include "general.h"
#include "Pieces.h"
#include "Shape.h"
#include "Solutions.h"
#include "Cube.h"
#include "SlvCube.h"

#include "OpenGL/glGlob.h"


ModelGLControl::ModelGLControl(GLWidget* gl, CubeDoc *doc)
  : ModelControlBase(gl, doc), m_gl(gl)
{
}

void ModelGLControl::updateView(int lHint)
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
   //     m_gl->doReset(); // SINGLE-VIEW // TBD: check if we really need to do a reset, check if its the first solution.

    case HINT_SLV_NXPR:
    case HINT_SLV_NONE:
    case HINT_SLV_PAINT:
        m_gl->updateGL();
        break;
    }
}

