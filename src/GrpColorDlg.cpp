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

#include "GrpColorDlg.h"
#include "CubeDoc.h"
#include "Pieces.h"
#include "SlvCube.h"
#include "MyLib/MyInputDlg.h"
#include "colorFuncs.h"


GrpColorDlg::GrpColorDlg(QWidget *parent, CubeDoc *doc) 
	:QDialog(parent), m_doc(doc)
	, m_colA(nullptr, "colA", "colA", QColor())
	, m_colB(nullptr, "colB", "colB", QColor())
{
	ui.setupUi(this);

	(new ColorSelIn(&m_colA, ui.botA, false, WidgetIn::DoModal))->reload();
	(new ColorSelIn(&m_colB, ui.botB, false, WidgetIn::DoModal))->reload();
	connect(&m_colA, SIGNAL(changed()), this, SLOT(colA_changed()));
	connect(&m_colB, SIGNAL(changed()), this, SLOT(colB_changed()));

	update();
}



void GrpColorDlg::update()
{
	if (!m_doc->solvesExist()) {
		m_grp = nullptr;
		return;
	}

	SlvCube *slv = m_doc->getCurrentSolve();
	m_gind = slv->picdt[0].gind;
	m_grp = &PicBucket::mutableInstance().grps[m_gind];

	m_prevA = toCol(m_grp->color);
	m_prevB = toCol(m_grp->exColor);
	m_colA = m_prevA;
	m_colB = m_prevB;

	ui.prevA->setText(colToText(m_prevA));
	ui.prevB->setText(colToText(m_prevB));
	ui.newA->setText(colToText(m_prevA));
	ui.newB->setText(colToText(m_prevB));

}

void GrpColorDlg::colA_changed() {
	if (m_grp == nullptr)
		return;
	m_grp->color = toVec(m_colA);
	ui.newA->setText(colToText(m_colA));
	emit changed(HINT_SLV_PAINT);
}
void GrpColorDlg::colB_changed() {
	if (m_grp == nullptr)
		return;
	m_grp->exColor = toVec(m_colB);
	ui.newB->setText(colToText(m_colB));
	emit changed(HINT_SLV_PAINT);
}

void GrpColorDlg::on_botReset_clicked()
{
	m_colA = m_prevA;
	m_colB = m_prevB;
}

void GrpColorDlg::on_botSwap_clicked()
{
	QColor tmp = m_colA;
	m_colA = m_colB.val();
	m_colB = tmp;
}

void GrpColorDlg::copy()
{

}

