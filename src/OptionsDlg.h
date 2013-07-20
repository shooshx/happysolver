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

#ifndef __OPTIONSDLG_H__INCLUDED__
#define __OPTIONSDLG_H__INCLUDED__

#include <QDialog>
#include "ui_OptionsDlg.h"
#include "MyLib/ParamBase.h"

/** \file
	Declares the OptionsDlg class used for configuration editing.
*/

class Configuration;
class DisplayConf;

/** OptionsDlg is the configuration dialog of the application.
	It controls all the parameters of the Configuration class.
	OptionsDlg holds a pointer to the Configuration insance from CubeDoc
	and it changes it when and if the dialog is accepted (OK is pressed)
	This class is created by MainWindow when the user selects the options
	menu command.
	\see Configuration CubeDoc MainWindow
*/
class OptionsDlg : public QDialog
{
	Q_OBJECT
public:
	OptionsDlg(QWidget* parent, Configuration *conf);
	virtual ~OptionsDlg() {}

private slots:
	void updateSEEnables();
	void updateAllorFew();
	void updateAfter();
	void updateAsym();
	virtual void accept();
	void resetToDefaults();
	void updateSDPasses(bool justChecked);
	void updateSDhighLevel(int index);

	void colorFlipChanged();
	void changedSlvBkCol();

signals:
	void updateSlv3D(int hint);
	
private:
	void updateAll(Configuration *conf);
	void updateSDlowLevel(const DisplayConf& dspc);


	Ui::OptionsDlg ui;

	Configuration *m_conf; ///< written to only on accept
	int m_wasPers; // helper for enable used to save the last state, -1 means its not set (between randomized and sequential)

	/// SD_Pass holds the widgets of a single subdivision pass in the display tab of OptionsDlg.
	struct SD_Pass
	{
		SD_Pass() :p(NULL), bs(NULL), br(NULL) {}
		SD_Pass(QCheckBox *_p, QRadioButton *_bs, QRadioButton *_br) :p(_p), bs(_bs), br(_br) {}
		QCheckBox *p;
		QRadioButton *bs, *br;
	} m_SD_pass[4];

	TypeProp<QColor> m_bkColor;
};


#endif
