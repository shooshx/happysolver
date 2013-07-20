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

#ifndef __GRPCOLORDLG_H__INCLUDED__
#define __GRPCOLORDLG_H__INCLUDED__

#include <QDialog>
#include "ui_GrpColor.h"
#include "MyLib/ParamBase.h"

class CubeDoc;
class PicGroupDef;

/** \file
	Declares the GrpColorDlg class.
*/

/** GrpColorDlg controls the color variables of the texture of a solution.
	it is used for the purpose of tweaking the colors of textures of
	a certain piece group.
*/
class GrpColorDlg :public QDialog
{
	Q_OBJECT
public:
	GrpColorDlg(QWidget *parent, CubeDoc *doc);
	virtual ~GrpColorDlg() {}

private:
	void blockSignals(bool block);

	Ui::GrpColor ui;
	CubeDoc *m_doc;
	PicGroupDef *m_grp;
	int m_gind;

	TypeProp<QColor> m_colA;
	TypeProp<QColor> m_colB;
	QColor m_prevA, m_prevB;

signals:
	void changed(int gind);
	void visibilityChanged(bool showen);

public slots:
	void update();

private slots:

	void on_botReset_clicked();
	void on_botSwap_clicked();
	void copy();

	void colA_changed();
	void colB_changed();

protected:
	virtual void hideEvent(QHideEvent* event) { emit visibilityChanged(false); QDialog::hideEvent(event); }
	virtual void showEvent(QShowEvent* event) { emit visibilityChanged(true); QDialog::showEvent(event); }

};

#endif
