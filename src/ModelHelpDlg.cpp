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

#include "GlobDefs.h"
#include "ModelHelpDlg.h"
#include "MyObject.h"
#include "CubeDoc.h"
#include "Pieces.h"
#include "ModelGLControl.h"
#include "Solutions.h"
#include "Shape.h"
#include "MainWindow.h"
#include "ActFocusComboBox.h"
#include "SlvCube.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMovie>
#include <QGroupBox>

PieceGLWidget::PieceGLWidget(QWidget *parent, CubeDoc *doc, QGLWidget *shareFrom) 
	:GLWidget(parent, shareFrom), m_doc(doc), m_lastSelectedPiece(-1), m_selectedPiece(-1),  
	 m_bSelectAll(false), m_bHasPushedMatrix(false)
{
	aqmin = Vec3(0.0, 0.0, 0.0);
	aqmax = Vec3(5.0, 5.0, 1.0);
}


void PieceGLWidget::setSelectedPiece(int p) 
{ 
	SlvCube *slv = m_doc->getCurrentSolve();
	if (slv == NULL)
		return;

	if (m_bSelectAll)
		return;

	if (m_bHasPushedMatrix)	{
		m_bHasPushedMatrix = false;
	}

	m_lastSelectedPiece = p;
	m_selectedPiece = p;
	const Shape::FaceDef &fdef = slv->shape->faces[m_selectedPiece];
	Vec3 nmin(fdef.ex);
	Vec3 nmax(nmin + Vec3(fdef.size()));

	setNewMinMax(nmin, nmax, false);

}

void PieceGLWidget::setSelectAll(int piece) 
{ 
	m_bSelectAll = piece >= 0; 
	if (m_bSelectAll)
	{
		m_lastSelectedPiece = piece; //m_selectedPiece;
		m_selectedPiece = -1; 

		SlvCube *slv = m_doc->getCurrentSolve();

		setNewMinMax(slv->painter.qmin, slv->painter.qmax, true);
	}
	else
	{
		setSelectedPiece(m_lastSelectedPiece);
		setNewMinMax(aqmin, aqmax, true); // called a second time with true, just for the true. TBD
	}
}


void PieceGLWidget::myPaintGL()
{
/*	SlvCube *slv = m_doc->getCurrentSolve();
	if ((slv == NULL) || ((m_selectedPiece == -1) && (!m_bSelectAll)))
	{ // clean the view from older things
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return;
	}

	ModelGLControl::paint(this, m_doc, slv, false, m_selectedPiece, -1);
	*/
}

/// we're moving independently from the main solution view.
void PieceGLWidget::mouseMoveEvent(QMouseEvent *event)
{
	makeCurrent();
	if (!m_bHasPushedMatrix) {
		m_bHasPushedMatrix = true;
	}
	GLWidget::mouseMoveEvent(event);
}
	
/// The main solution view is rotating, We need to keep up with it.
void PieceGLWidget::externRotate(GLWidget::EAxis axis, int x, int y)
{
	makeCurrent();
	if (m_bHasPushedMatrix) {
		m_bHasPushedMatrix = false;
	}
	rotate(axis, x, y);
	updateGL();
}

/// clear the view of any object in it. forget any state we may have had.
void PieceGLWidget::clear()
{
	if (m_bHasPushedMatrix) {
		m_bHasPushedMatrix = false;
	}
	m_bSelectAll = false;
	m_selectedPiece = -1;
	m_lastSelectedPiece = -1;
	updateGL();
}

///////////////////////////////////////////////////////////////////////////////

ModelHelpDlg::ModelHelpDlg(QWidget *parent, MainWindow* main, CubeDoc *doc, QGLWidget *shareFrom) 
  :SizedWidget(QSize(MIN_HLP_PANE_WIDTH,0), parent), m_main(main), m_doc(doc)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_splitter = new QSplitter(Qt::Vertical, this);
	m_splitter->setChildrenCollapsible(false);
	QVBoxLayout *toplayout = new QVBoxLayout;
	toplayout->setMargin(0);
	toplayout->setSpacing(0);
	setLayout(toplayout);
	toplayout->addWidget(m_splitter);

	QWidget *glwin = new QWidget(NULL);
	QHBoxLayout *gllayout = new QHBoxLayout;
	gllayout->setMargin(3);
	gllayout->setSpacing(0);
	glwin->setLayout(gllayout);
	m_pieceView = new PieceGLWidget(glwin, m_doc, shareFrom);
	//m_pieceView->setUsingLight(m_doc->m_conf.disp.bLight);

	gllayout->addSpacing(4);
	gllayout->addWidget(m_pieceView);
	gllayout->addSpacing(4);
	m_splitter->addWidget(glwin);

	QWidget *topwin = new QWidget(NULL); // additional widget needed for free resizing
	m_splitter->addWidget(topwin);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setMargin(10);
	layout->setSpacing(10);
	topwin->setLayout(layout);
	

	QGroupBox *group = new QGroupBox("Current Piece");
	QHBoxLayout *glayout = new QHBoxLayout; // need a layout to make it strech
	group->setLayout(glayout);

	m_picImage = new QLabel;
//	m_picImage->setMaximumSize(200, 100);
	m_picImage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	glayout->addWidget(m_picImage);

	m_animLabel = new QLabel("");
	m_animLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_cubeAnim = new QMovie(":/images/cube_ts.gif");
	m_cubeAnim->setCacheMode(QMovie::CacheAll); //needed for looping
	m_animLabel->setMovie(m_cubeAnim);

	QVBoxLayout *vlay = new QVBoxLayout;
	vlay->setMargin(0);
	vlay->setSpacing(6);

	m_goBot = new ActionPushButton(m_main->m_goAct);
	m_goBot->showText(true);
	m_goBot->setMinimumSize(50, 37);
	vlay->addWidget(m_goBot);
	m_goBot->hide();

	QHBoxLayout *sblayout = new QHBoxLayout;
	sblayout->addStretch();
	ActionPushButton *stepbot = new ActionPushButton(m_main->m_showAsmDlgAct);
	stepbot->setMinimumSize(60, MIN_BOT_HEIGHT);
	stepbot->setAltText("Steps");
	stepbot->showText(true);
	sblayout->addWidget(stepbot);
	vlay->addLayout(sblayout);

	QHBoxLayout *zlayout = new QHBoxLayout;
	zlayout->setSpacing(0);
	zlayout->setMargin(3);
	QGroupBox *zoomc = new QGroupBox("Zoom");
	m_zoomSlider = new QSlider(Qt::Horizontal);
	m_zoomSlider->setRange(ZOOM_MIN, ZOOM_MAX);
	m_zoomSlider->setValue(100);
	connect(m_zoomSlider, SIGNAL(valueChanged(int)), this, SIGNAL(zoomChanged(int)));
	zoomc->setLayout(zlayout);
	zlayout->addWidget(m_zoomSlider);
	vlay->addWidget(zoomc);


	QHBoxLayout *lblayout = new QHBoxLayout;
	ActionPushButton *prevbot = new ActionPushButton(m_main->m_slvPrevAct);
	prevbot->setMinimumSize(50, MIN_BOT_HEIGHT);
	ActionPushButton *nextbot = new ActionPushButton(m_main->m_slvNextAct);
	nextbot->setMinimumSize(50, MIN_BOT_HEIGHT);
	ActFocusComboBox *combo = new ActFocusComboBox(m_main->m_slvNumAct);
	combo->setEditable(true);
	combo->setInsertPolicy(QComboBox::NoInsert);
	combo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
	combo->setMinimumContentsLength(4);

	lblayout->addStretch(1);
	lblayout->addWidget(prevbot);
	lblayout->addWidget(combo);
	lblayout->addWidget(nextbot);
	lblayout->addStretch(1);
	vlay->addLayout(lblayout);

/*	QHBoxLayout *ublayout = new QHBoxLayout;
	ActionPushButton *newbot = new ActionPushButton(m_main->m_resetSolAct);
	newbot->setMinimumSize(60, 30);
	newbot->showText(true);
	ublayout->addWidget(newbot);
	ublayout->addStretch();
	ActionPushButton *openbot = new ActionPushButton(m_main->m_openAct);
	openbot->setMinimumSize(50, 30);
	ublayout->addWidget(openbot);
	ActionPushButton *savebot = new ActionPushButton(m_main->m_saveAct);
	savebot->setMinimumSize(50, 30);
	ublayout->addWidget(savebot);

	vlay->addLayout(ublayout);*/

/*	QPushButton *back = new QPushButton(QIcon(":/images/buildview.png"), "Back to Design");
	// this one doesn't need tooltip...
	back->setMinimumSize(90, 30);
	connect(back, SIGNAL(clicked()), this, SIGNAL(switchToDesign()));
	vlay->addWidget(back);
*/
	layout->setSizeConstraint(QLayout::SetMinimumSize);
//	layout->addWidget(m_pieceView);
	layout->addWidget(group);
	layout->addWidget(m_animLabel);
	layout->addStretch();
	layout->addLayout(vlay);
	



}

void ModelHelpDlg::resizeEvent(QResizeEvent *event)
{
	if (event->oldSize().isEmpty())
	{ // first time here. this could probably be done better.
		QList<int> szs;
		szs.push_back(190);
		int x = m_splitter->height() - 190;
		szs.push_back(x);
		m_splitter->setSizes(szs);
	}
}


QSize ModelHelpDlg::minimumSizeHint() const 
{ 
	return QSize(0,0); 
}

void ModelHelpDlg::updateZoom(int v)
{
	m_zoomSlider->setValue(v);
}

void ModelHelpDlg::updatePixmapLabel(int piece)
{
	if (piece < 0)
		return;
	SlvCube *slv = m_doc->getCurrentSolve();
	const SlvCube::SlvPic &sps = slv->picdt[slv->dt[piece].abs_sc];
	m_picImage->setPixmap(PicBucket::instance().getPic(sps.gind, sps.pind).pixmap);

}

void ModelHelpDlg::changeViewPiece(int piece)
{
	if (piece < 0) // possibly optimize - filter out during selct all
		return; // means there is not thing under the mouse. ignore it and stay with the last one

	updatePixmapLabel(piece);
	m_pieceView->makeCurrent();
	m_pieceView->setSelectedPiece(piece);
	m_pieceView->updateGL();
}

void ModelHelpDlg::changedExternViewPiece(int piece)
{
	updatePixmapLabel(piece);
	m_pieceView->makeCurrent();
	m_pieceView->setSelectAll(piece);
	m_pieceView->updateGL();
}

void ModelHelpDlg::clear()
{
	m_picImage->clear();
	m_pieceView->clear();
}

void ModelHelpDlg::updateView(int lHint)
{
	switch (GET_SLV_HINT(lHint))
	{
	case HINT_SLV_READY:
	case HINT_SLV_NXPR:
	case HINT_SLV_NONE:
		clear(); // this is problematic since it can arrive some time after the load and after a load related invalidate
		break;
	}
}

void ModelHelpDlg::statsUpdate(int hint, int data)
{
	switch (hint)
	{ // TBD - might be able to thin those down
	case SHINT_WARNING:
	case SHINT_START:
	case SHINT_STOP:
	case SHINT_STATUS:
		if (!m_doc->isSlvEngineRunning())
		{
			m_animLabel->hide();
			m_goBot->hide();
			if (m_cubeAnim->state() == QMovie::Running)
				m_cubeAnim->stop();
		}
		else
		{
			m_animLabel->show();
			m_goBot->show();
			if (m_cubeAnim->state() != QMovie::Running)
				m_cubeAnim->start();

		}
	break;
	}

}

