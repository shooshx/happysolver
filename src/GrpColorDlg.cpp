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

#include <QIntValidator>

GrpColorDlg::GrpColorDlg(QWidget *parent, CubeDoc *doc) :QDialog(parent), m_doc(doc)
{
	ui.setupUi(this);

	QIntValidator *inv = new QIntValidator(0, 255, this);
	ui.ForeRedEdit->setValidator(inv);
	ui.ForeGreenEdit->setValidator(inv);
	ui.ForeBlueEdit->setValidator(inv);
	ui.BackRedEdit->setValidator(inv);
	ui.BackGreenEdit->setValidator(inv);
	ui.BackBlueEdit->setValidator(inv);

	connect(ui.ForeRedSlide, SIGNAL(valueChanged(int)), this, SLOT(foreRChanged(int)));
	connect(ui.ForeGreenSlide, SIGNAL(valueChanged(int)), this, SLOT(foreGChanged(int)));
	connect(ui.ForeBlueSlide, SIGNAL(valueChanged(int)), this, SLOT(foreBChanged(int)));
	connect(ui.BackRedSlide, SIGNAL(valueChanged(int)), this, SLOT(backRChanged(int)));
	connect(ui.BackGreenSlide, SIGNAL(valueChanged(int)), this, SLOT(backGChanged(int)));
	connect(ui.BackBlueSlide, SIGNAL(valueChanged(int)), this, SLOT(backBChanged(int)));

	connect(ui.ForeRedEdit, SIGNAL(textChanged(const QString&)), this, SLOT(foreRTChanged(const QString&)));
	connect(ui.ForeGreenEdit, SIGNAL(textChanged(const QString&)), this, SLOT(foreGTChanged(const QString&)));
	connect(ui.ForeBlueEdit, SIGNAL(textChanged(const QString&)), this, SLOT(foreBTChanged(const QString&)));
	connect(ui.BackRedEdit, SIGNAL(textChanged(const QString&)), this, SLOT(backRTChanged(const QString&)));
	connect(ui.BackGreenEdit, SIGNAL(textChanged(const QString&)), this, SLOT(backGTChanged(const QString&)));
	connect(ui.BackBlueEdit, SIGNAL(textChanged(const QString&)), this, SLOT(backBTChanged(const QString&)));

	connect(ui.resetButton, SIGNAL(clicked(bool)), this, SLOT(reset()));
	connect(ui.switchButton, SIGNAL(clicked(bool)), this, SLOT(switchC()));
	connect(ui.copyButton, SIGNAL(clicked(bool)), this, SLOT(copy()));
}

void GrpColorDlg::blockSignals(bool block)
{
	ui.ForeRedSlide->blockSignals(block);
	ui.ForeGreenSlide->blockSignals(block);
	ui.ForeBlueSlide->blockSignals(block);
	ui.BackRedSlide->blockSignals(block);
	ui.BackGreenSlide->blockSignals(block);
	ui.BackBlueSlide->blockSignals(block);
	ui.ForeRedEdit->blockSignals(block);
	ui.ForeGreenEdit->blockSignals(block);
	ui.ForeBlueEdit->blockSignals(block);
	ui.BackRedEdit->blockSignals(block);
	ui.BackGreenEdit->blockSignals(block);
	ui.BackBlueEdit->blockSignals(block);
}

void GrpColorDlg::update()
{
	if (!m_doc->solvesExist())
	{
		m_grp = NULL;
		return;
	}
	QString s;

	blockSignals(true); // avoid update to cycle back to the view
	SlvCube *slv = m_doc->getCurrentSolve();
	m_gind = slv->picdt[0].gind;
	m_grp = &PicBucket::mutableInstance().defs[m_gind];
	ui.ForeRedSlide->setValue((int)(m_grp->r * 255.0));
	s = QString("%1").arg(m_grp->r * 255.0);
	ui.ForeRedL->setText(s);
	ui.ForeRedEdit->setText(s);
	ui.ForeGreenSlide->setValue((int)(m_grp->g * 255.0));
	s = QString("%1").arg(m_grp->g * 255.0);
	ui.ForeGreenL->setText(s);
	ui.ForeGreenEdit->setText(s);
	ui.ForeBlueSlide->setValue((int)(m_grp->b * 255.0));
	s = QString("%1").arg(m_grp->b * 255.0);
	ui.ForeBlueL->setText(s);
	ui.ForeBlueEdit->setText(s);
	ui.BackRedSlide->setValue((int)(m_grp->exR * 255.0));
	s = QString("%1").arg(m_grp->exR * 255.0);
	ui.BackRedL->setText(s);
	ui.BackRedEdit->setText(s);
	ui.BackGreenSlide->setValue((int)(m_grp->exG * 255.0));
	s = QString("%1").arg(m_grp->exG * 255.0);
	ui.BackGreenL->setText(s);
	ui.BackGreenEdit->setText(s);
	ui.BackBlueSlide->setValue((int)(m_grp->exB * 255.0));
	s = QString("%1").arg(m_grp->exB * 255.0);
	ui.BackBlueL->setText(s);
	ui.BackBlueEdit->setText(s);
	blockSignals(false);
}

void GrpColorDlg::reset()
{
	ui.ForeRedSlide->setValue(ui.ForeRedL->text().toInt());
	ui.ForeGreenSlide->setValue(ui.ForeGreenL->text().toInt());
	ui.ForeBlueSlide->setValue(ui.ForeBlueL->text().toInt());
	ui.BackRedSlide->setValue(ui.BackRedL->text().toInt());
	ui.BackGreenSlide->setValue(ui.BackGreenL->text().toInt());
	ui.BackBlueSlide->setValue(ui.BackBlueL->text().toInt());
}

void GrpColorDlg::switchC()
{
	setUpdatesEnabled(false);
	int br = ui.ForeRedSlide->value();
	int bg = ui.ForeGreenSlide->value();
	int bb = ui.ForeBlueSlide->value();
	ui.ForeRedSlide->setValue(ui.BackRedSlide->value());
	ui.ForeGreenSlide->setValue(ui.BackGreenSlide->value());
	ui.ForeBlueSlide->setValue(ui.BackBlueSlide->value());
	ui.BackRedSlide->setValue(br);
	ui.BackGreenSlide->setValue(bg);
	ui.BackBlueSlide->setValue(bb);
	setUpdatesEnabled(true);
}

void GrpColorDlg::copy()
{
	QApplication::clipboard()->setText(QString("(%1, %2, %3), (%4, %5, %6)").arg(ui.ForeRedSlide->value()).arg( 
		ui.ForeGreenSlide->value()).arg(ui.ForeBlueSlide->value()).arg(ui.BackRedSlide->value()).arg(
		ui.BackGreenSlide->value()).arg(ui.BackBlueSlide->value()));
}

void GrpColorDlg::foreRChanged(int v) 
{ 
	ui.ForeRedEdit->setText(QString("%1").arg(v)); 
	if (m_grp != NULL)
	{
		m_grp->r = v/255.0;
		emit changed(m_gind);
	}
}
void GrpColorDlg::foreGChanged(int v) 
{ 
	ui.ForeGreenEdit->setText(QString("%1").arg(v)); 
	if (m_grp != NULL)
	{
		m_grp->g = v/255.0;
		emit changed(m_gind);
	}
}
void GrpColorDlg::foreBChanged(int v) 
{ 
	ui.ForeBlueEdit->setText(QString("%1").arg(v)); 
	if (m_grp != NULL)
	{
		m_grp->b = v/255.0;
		emit changed(m_gind);
	}
}
void GrpColorDlg::backRChanged(int v) 
{ 
	ui.BackRedEdit->setText(QString("%1").arg(v)); 
	if (m_grp != NULL)
	{
		m_grp->exR = v/255.0;
		emit changed(m_gind);
	}
}
void GrpColorDlg::backGChanged(int v) 
{ 
	ui.BackGreenEdit->setText(QString("%1").arg(v)); 
	if (m_grp != NULL)
	{
		m_grp->exG = v/255.0;
		emit changed(m_gind);
	}
}
void GrpColorDlg::backBChanged(int v) 
{ 
	ui.BackBlueEdit->setText(QString("%1").arg(v)); 
	if (m_grp != NULL)
	{
		m_grp->exB = v/255.0;
		emit changed(m_gind);
	}
}
