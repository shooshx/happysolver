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


ModelGLWidget::ModelGLWidget(QWidget *parent, CubeDoc *document)
  :GLWidget(parent), m_doc(document)
{
	m_bBackFaceCulling = true; //false;
	m_fUseLight = m_doc->m_conf.disp.bLight;
	m_bSkewReset = true; //false;

	m_nSingleChoise = -1;
	m_nHoverChoise = -1;
	m_nLastHoveChs = -1;
	m_viewState = GLWidget::Perspective;

	// it's still semantically const, c++ doesn't allow to initialize it otherwise
	const_cast<int &>(m_minScaleReset) = 10;

	setMouseTracking(true);
	
}


void ModelGLWidget::paint(GLWidget* that, CubeDoc *doc, SlvCube *scube, bool fTargets, int singleChoise, int upToStep)
{	
	if (singleChoise > scube->numPieces())
	{ // caused by untimely updates of the single piece view. show the first one
		singleChoise = 0; // not -1 because that will show the entire model
	}

	if (!fTargets)
	{
		glEnable(GL_COLOR_MATERIAL);

		if ((singleChoise >= 0) && (scube->getPieceGrpDef(singleChoise)->blackness > BLACK_NOT))
			glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
		else
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	else 
	{ // only drawing targets, no need to bother with colors. maybe disable lighting as well?
		glDisable(GL_COLOR_MATERIAL);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_LINE_SMOOTH);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // just make sure

	// call the SlvPainter to do its job.
	scube->painter.paint(that, fTargets, singleChoise, upToStep, doc->m_conf.disp.nLines); 
}


void ModelGLWidget::myPaintGL()
{
	// draw the object
	drawTargets(false);

	// draw the text in the bottom left corner which indicates which solution this is, out fo how many.
	// this can't be done with lighting since it will change the text color.
	if (isUsingLight())
		glDisable(GL_LIGHTING);

	GL_BEGIN_TEXT();
	
	glColor3f(0.3f, 0.3f, 1.0f);
	glRasterPos2f(-0.95f, -0.92f);

	mglPrint(QString("%1/%2").arg(m_doc->getCurrentSolveIndex() + 1).arg(m_doc->getSolvesNumber()));

	GL_END_TEXT();

	// re-enable lighting 
	if (isUsingLight())
		glEnable(GL_LIGHTING);
}


void ModelGLWidget::reCalcSlvMinMax()
{
	SlvPainter &pnt = m_doc->getCurrentSolve()->painter;

	aqmin = pnt.qmin;
	aqmax = pnt.qmax;
}


void ModelGLWidget::updateView(int lHint)
{
	switch (GET_SLV_HINT(lHint))
	{
	case HINT_SLV_READY:
		// to be on the safe side (not to reference something that is not there)
		m_nHoverChoise = -1;
		m_nSingleChoise = -1;
		m_nLastHoveChs = -1;

		setUpdatesEnabled(false); // make sure paintGL will not get called while we generate the ifs.
		// a call to painGL would cause a race condition on who generates the ifs first
		// and a memory leak.

		makeCurrent();
		m_doc->getCurrentSolve()->genPainter();
		setUpdatesEnabled(true);

		reCalcSlvMinMax();
		DoReset(); // TBD: check if we really need to do a reset, check if its the first solution.

	case HINT_SLV_NXPR:
	case HINT_SLV_NONE:
	case HINT_SLV_PAINT:
		updateGL();
		break;
	}
}


// MOUSE HANDLERS ///////////////////////////////////////////

void ModelGLWidget::drawTargets(bool inChoise)
{
	SlvCube *slv = m_doc->getCurrentSolve();
	if (slv == NULL)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return;
	}

	if (slv->painter.isNull()) 
	{
		slv->genPainter();
	}

	paint(this, m_doc, slv, inChoise, m_nSingleChoise, m_doc->getUpToStep());

}

void ModelGLWidget::mousePressEvent(QMouseEvent *event) 
{
	if (event->button() == Qt::RightButton)
	{
		m_nSingleChoise = DoChoise(event->x(), event->y());
		if (m_nSingleChoise != -1)
			emit chosenSinglePiece(m_nSingleChoise);
	}
	GLWidget::mousePressEvent(event);
	updateGL(); // the GLWidget mouse event doesn't have a reason to update
}

void ModelGLWidget::mouseReleaseEvent(QMouseEvent *event) 
{
	if (event->button() == Qt::RightButton)
	{
		m_nSingleChoise = -1;
		emit chosenSinglePiece(-1);
		makeCurrent();
		mouseMoveEvent(event); // simulate a mouse move event to send an update about the hove
	}

	GLWidget::mouseReleaseEvent(event);
	updateGL(); // the GLWidget mouse event doesn't have a reason to update

}

void ModelGLWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (m_doc->solvesExist())
	{
		m_nLastHoveChs = m_nHoverChoise;
		m_nHoverChoise = DoChoise(event->x(), event->y());
		if (m_nHoverChoise != m_nLastHoveChs)
			emit changedHoverPiece(m_nHoverChoise);
	}

	makeCurrent(); 
	GLWidget::mouseMoveEvent(event);
}



