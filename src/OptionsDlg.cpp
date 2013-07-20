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

#include "OptionsDlg.h"
#include "Configuration.h"
#include <QMessageBox>
#include "GlobDefs.h"
#include "MyLib/MyInputDlg.h"
#include "colorFuncs.h"
#include "CubeDoc.h"

#include "Pieces.h"


OptionsDlg::OptionsDlg(QWidget *parent, Configuration *conf)
	:QDialog(parent), m_conf(conf), m_wasPers(-1)
	, m_bkColor(NULL, "bkColor", "bkColor", QColor(0,0,0))
{
	int i;
	ui.setupUi(this);

	//setMinimumSize(size());
	//setMaximumSize(size());

	connect(ui.SM_rand, SIGNAL(toggled(bool)), this, SLOT(updateSEEnables()));
	connect(ui.SM_seq, SIGNAL(toggled(bool)), this, SLOT(updateSEEnables()));
	connect(ui.SM_restart, SIGNAL(toggled(bool)), this, SLOT(updateSEEnables()));
	connect(ui.SM_luck, SIGNAL(toggled(bool)), this, SLOT(updateSEEnables()));
	connect(ui.SP_all, SIGNAL(toggled(bool)), this, SLOT(updateAllorFew()));
	connect(ui.SP_one, SIGNAL(toggled(bool)), this, SLOT(updateAllorFew()));
	connect(ui.SP_upto, SIGNAL(toggled(bool)), this, SLOT(updateAllorFew()));
	connect(ui.AP_face, SIGNAL(toggled(bool)), this, SLOT(updateAsym()));
	connect(ui.AP_any, SIGNAL(toggled(bool)), this, SLOT(updateAsym()));
	connect(ui.defaultsButton, SIGNAL(clicked()), this, SLOT(resetToDefaults()));
	connect(ui.SP_safter, SIGNAL(toggled(bool)), this, SLOT(updateAfter()));
	connect(ui.SP_cont, SIGNAL(toggled(bool)), this, SLOT(updateAfter()));
	
	m_SD_pass[0] = SD_Pass(ui.SD_pass1, ui.SD_p1s, ui.SD_p1r);
	m_SD_pass[1] = SD_Pass(ui.SD_pass2, ui.SD_p2s, ui.SD_p2r);
	m_SD_pass[2] = SD_Pass(ui.SD_pass3, ui.SD_p3s, ui.SD_p3r);
	m_SD_pass[3] = SD_Pass(ui.SD_pass4, ui.SD_p4s, ui.SD_p4r);
	for (i = 0; i < 4; ++i)
	{
		connect(m_SD_pass[i].p, SIGNAL(toggled(bool)), this, SLOT(updateSDPasses(bool)));
		connect(m_SD_pass[i].p, SIGNAL(toggled(bool)), m_SD_pass[i].bs, SLOT(setEnabled(bool)));
		connect(m_SD_pass[i].p, SIGNAL(toggled(bool)), m_SD_pass[i].br, SLOT(setEnabled(bool)));
	}

	ui.AP_faceWhere->addItem("Outside", (int)ASYM_OUTSIDE);
	ui.AP_faceWhere->addItem("Inside", (int)ASYM_INSIDE);

	ui.SD_highLevel->addItem("Flat (minimal)", (int)DPC_MINIMAL_SQUARE);
	ui.SD_highLevel->addItem("Soft Normal (default)", (int)DPC_SOFT_NORMAL);
	ui.SD_highLevel->addItem("Extra Soft", (int)DPC_SOFT_EXTRA);
	ui.SD_highLevel->addItem("Extra Soft Fast", (int)DPC_SOFT_EXTRA_FAST);
	ui.SD_highLevel->addItem("Custom...", (int)DPC_CUSTOM);
	connect(ui.SD_highLevel, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSDhighLevel(int)));

	ui.SD_lines->addItem("None", (int)LINES_NONE);
	ui.SD_lines->addItem("All", (int)LINES_ALL);
	ui.SD_lines->addItem("Whites", (int)LINES_WHITE);

	(new ColorSelIn(&m_bkColor, ui.bkColor, false, WidgetIn::DoModal))->reload();
	connect(&m_bkColor, SIGNAL(changed()), this, SLOT(changedSlvBkCol()));
	m_bkColor = toCol(m_conf->disp.slvBkColor);

// 	for(i = 0; i < 24; ++i)
// 		ui.SD_colorFlip->addItem(QString("%1").arg(colorPremut[i]), colorPremut[i]);
// 	connect(ui.SD_colorFlip, SIGNAL(currentIndexChanged(int)), this, SLOT(colorFlipChanged()));

	updateAll(m_conf);
}

void OptionsDlg::changedSlvBkCol() {
	m_conf->disp.slvBkColor = toVec(m_bkColor);
	emit updateSlv3D(HINT_SLV_PAINT);
}

// check in
void OptionsDlg::updateAll(Configuration *conf)
{
	ui.TabWidget->setCurrentIndex(m_conf->nOptionsSelTab);

	ui.SM_rand->setChecked(conf->engine.fRand);
	ui.SM_seq->setChecked(!conf->engine.fRand);
	ui.SM_restart->setChecked(conf->engine.fRestart);
	ui.SM_rEvery->setValue(conf->engine.nRestart / 1000.0);
	ui.SM_luck->setChecked(conf->engine.fLuck);
	ui.SM_lOffset->setValue(conf->engine.nLuck);

	ui.SP_all->setChecked(conf->engine.nPersist == PERSIST_ALL);
	ui.SP_one->setChecked(conf->engine.nPersist == PERSIST_ONLY_FIRST);
	ui.SP_upto->setChecked(conf->engine.nPersist == PERSIST_UPTO);
	ui.SP_uptoHow->setText(QString("%1").arg(conf->engine.nUpto));
	ui.SP_uptoHow->setValidator(new QIntValidator(1, 2000000000, this));
	ui.SP_safter->setChecked(conf->engine.fAfter);
	ui.SP_cont->setChecked(!conf->engine.fAfter);
	ui.SP_afterNum->setValidator(new QIntValidator(1, 2000000000, this));
	ui.SP_afterNum->setText(QString("%1").arg(conf->engine.nAfter));

	// Assembly
	ui.AP_any->setChecked(conf->engine.nAsym == ASYM_REGULAR);
	ui.AP_face->setChecked(conf->engine.nAsym != ASYM_REGULAR);
	ui.AP_faceWhere->setCurrentIndex( (conf->engine.nAsym == ASYM_INSIDE)?1:0 ); // if not enabled show "Outside"

	// Solution Display

	updateSDlowLevel(conf->disp);
	ui.SD_highLevel->setCurrentIndex(-1); // reset it so that the next call would cause a signal to be sent
	ui.SD_highLevel->setCurrentIndex(ui.SD_highLevel->findData(conf->disp.preConf));

//	ui.SD_colorFlip->setCurrentIndex(ui.SD_colorFlip->findData(conf->disp.nSwapTexEndians));
}

// check out
void OptionsDlg::accept()
{
	if (!ui.SP_uptoHow->hasAcceptableInput())
	{
		QMessageBox::warning(this, APP_NAME, "Input Error!\nNumber of solutions in a series should be\ngreater or equal then 1", QMessageBox::Ok, 0);
		ui.TabWidget->setCurrentIndex(0);
		return;
	}

	if (!ui.SP_afterNum->hasAcceptableInput())
	{
		QMessageBox::warning(this, APP_NAME, "Input Error!\nNumber of solutions should be\ngreater or equal then 1", QMessageBox::Ok, 0);
		ui.TabWidget->setCurrentIndex(0);
		return;
	}

	m_conf->engine.fRand = ui.SM_rand->isChecked();
	m_conf->engine.fLuck = ui.SM_luck->isChecked();
	m_conf->engine.nLuck = ui.SM_lOffset->value();
	m_conf->engine.fRestart = ui.SM_restart->isChecked();
	m_conf->engine.nRestart = (int)(ui.SM_rEvery->value() * 1000.0);
	m_conf->engine.fAfter = ui.SP_safter->isChecked();
	m_conf->engine.nAfter = ui.SP_afterNum->text().toInt();
	m_conf->engine.nPersist = ui.SP_all->isChecked()?PERSIST_ALL:(ui.SP_one->isChecked()?PERSIST_ONLY_FIRST:PERSIST_UPTO);
	m_conf->engine.nUpto = ui.SP_uptoHow->text().toInt();
	m_conf->engine.nAsym = ui.AP_any->isChecked()?ASYM_REGULAR:((ui.AP_faceWhere->currentIndex() == 0)?ASYM_OUTSIDE:ASYM_INSIDE);

	DisplayConf &dspc = m_conf->disp;
	dspc.preConf = (EDisplayPreConf)ui.SD_highLevel->itemData(ui.SD_highLevel->currentIndex()).toInt();
	if (dspc.preConf == DPC_CUSTOM)
	{
		dspc.bLight = ui.SD_light->isChecked();
		dspc.bVtxNormals = ui.SD_vtxNormals->isChecked();
		dspc.nLines = (ELinesDraw)ui.SD_lines->itemData(ui.SD_lines->currentIndex()).toInt();
		dspc.numberOfPasses = 0;
		for(int i = 0; i < 4; ++i)
		{
			if (m_SD_pass[i].p->isChecked())
				dspc.numberOfPasses = i + 1;
			dspc.passRound[i] = m_SD_pass[i].br->isChecked();
		}
	}
	else
	{
		m_conf->disp.fromPreConf(m_conf->disp.preConf);
	}
//	m_conf->disp.nSwapTexEndians = ui.SD_colorFlip->itemData(ui.SD_colorFlip->currentIndex()).toInt();

	m_conf->nOptionsSelTab = ui.TabWidget->currentIndex();

	m_conf->toRegistry();
	QDialog::accept();
}



void OptionsDlg::updateSDlowLevel(const DisplayConf& dspc)
{
	for (int i = 0; i < 4; ++i)
	{
		bool en = dspc.numberOfPasses > i;
		m_SD_pass[i].p->setChecked(en);
		m_SD_pass[i].bs->setChecked(!dspc.passRound[i]);
		m_SD_pass[i].bs->setEnabled(en);
		m_SD_pass[i].br->setChecked(dspc.passRound[i]);
		m_SD_pass[i].br->setEnabled(en);
	}
	ui.SD_light->setChecked(dspc.bLight);
	ui.SD_vtxNormals->setChecked(dspc.bVtxNormals);
	ui.SD_lines->setCurrentIndex(ui.SD_highLevel->findData(dspc.nLines));
}


void OptionsDlg::updateSEEnables()
{
	ui.SM_restart->setEnabled(ui.SM_rand->isChecked());
	ui.SM_rEvery->setEnabled(ui.SM_rand->isChecked() && ui.SM_restart->isChecked());
	ui.label_restart1->setEnabled(ui.SM_rand->isChecked() && ui.SM_restart->isChecked());
	ui.label_restart2->setEnabled(ui.SM_rand->isChecked() && ui.SM_restart->isChecked());

	ui.SM_luck->setEnabled(ui.SM_rand->isChecked());
	ui.SM_lOffset->setEnabled(ui.SM_rand->isChecked() && ui.SM_luck->isChecked());
	ui.label_luck1->setEnabled(ui.SM_rand->isChecked() && ui.SM_luck->isChecked());
	ui.label_luck2->setEnabled(ui.SM_rand->isChecked() && ui.SM_luck->isChecked());

	// one and upto make sense only if we're random. if we're sequenced they'll find the same
	// solution(s) over and over again
	ui.SP_one->setEnabled(ui.SM_rand->isChecked());
	ui.SP_upto->setEnabled(ui.SM_rand->isChecked());
	ui.SP_uptoHow->setEnabled(ui.SM_rand->isChecked() && ui.SP_upto->isChecked());
	ui.label_upto->setEnabled(ui.SM_rand->isChecked());

	if (ui.SM_seq->isChecked())
	{ // this is all done to preserve who was checked before we change it brutally
		// once this is done, the only way to get back is choosing random, which unties this.
		if (m_wasPers == -1) m_wasPers = ui.SP_all->isChecked()?0:(ui.SP_one->isChecked()?1:(ui.SP_upto->isChecked()?2:-1));
		ui.SP_all->setChecked(true);
	}
	else if (m_wasPers != -1)
	{
		ui.SP_one->setChecked(m_wasPers == 1);
		ui.SP_upto->setChecked(m_wasPers == 2);
		m_wasPers = -1;
	}

}

void OptionsDlg::updateAllorFew()
{
	ui.SP_uptoHow->setEnabled(ui.SP_upto->isChecked());
}

void OptionsDlg::updateAfter()
{
	ui.SP_afterNum->setEnabled(ui.SP_safter->isChecked());
}

void OptionsDlg::updateAsym()
{
	ui.AP_faceWhere->setEnabled(ui.AP_face->isChecked());
}

void OptionsDlg::resetToDefaults()
{ // do not touch the m_opts
	Configuration tmpconf;
	tmpconf.fromDefaults();
	updateAll(&tmpconf);
}


void OptionsDlg::updateSDPasses(bool justChecked)
{
	int p;
	if (justChecked)
	{
		p = 3;
		while ((p >= 0) && (!m_SD_pass[p].p->isChecked()))
			--p;
		p++;
	}
	else
	{
		p = 0;
		while ((p < 4) && (m_SD_pass[p].p->isChecked()))
			++p;
	}


	for(int i = 0; i < 4; ++i)
	{
		m_SD_pass[i].p->setChecked(i < p);
	}
}

void OptionsDlg::updateSDhighLevel(int index)
{
	if (index == -1)
		return;
	DisplayConf tmpd;
	EDisplayPreConf dpc = (EDisplayPreConf)ui.SD_highLevel->itemData(index).toInt();
	tmpd.fromPreConf(dpc);
	if (dpc != DPC_CUSTOM)
		updateSDlowLevel(tmpd);
	ui.SD_custom->setEnabled(dpc == DPC_CUSTOM);
}


void OptionsDlg::colorFlipChanged()
{
//	int n = ui.SD_colorFlip->itemData(ui.SD_colorFlip->currentIndex()).toInt();
//	PicBucket::mutableInstance().updateSwapEndian(n);
}
