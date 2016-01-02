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

#include "GlobDefs.h" // sizes
#include "PicsSelectWidget.h"

#include "CubeDoc.h"
#include "BuildWorld.h"
#include "Pieces.h"
#include "DataWidgets.h"
#include "SlvCube.h"

#include <QLabel>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QScrollArea>

// these constants define the positioning of the widgets in the PicsSelectWidget
#define BOT_X 95
#define BOT_Y 95
#define BOT_SPACE_X 5
#define BOT_SPACE_Y 60
#define BOT_TOTAL_X (BOT_X + BOT_SPACE_X)
#define BOT_TOTAL_Y (BOT_Y + BOT_SPACE_Y)
#define BOT_OFFS_X 12   // offset of all the bots from the side
#define BOT_OFFS_Y 27//67   // offset of all the bots from above
#define FRAME_OFFS_Y 37 // offset of all the frames from above
#define FRAME_OFFS_SPACE_LEFT 7  //X width of the frame
#define FRAME_OFFS_SPACE_RIGHT 70  // X width of the frame
#define EDIT_THICK 20

#define NUM_DEFS 6




PicsSelectWidget::PicsSelectWidget(QWidget *parent, CubeDoc *document)
	:QWidget(parent), m_doc(document), m_bFinishedCreate(false), 
	m_currentGlobalCount(-1), m_currentBuildTilesCount(-1),
	m_bIncomingSignalsBlocked(false), m_nGblocked(0), m_nFblocked(0)
{
	m_layout = new QVBoxLayout;
	setLayout(m_layout);
	m_layout->setMargin(0);


	m_selectAllBot = new QPushButton("Set all to 1");
	m_selectAllBot->setMinimumSize(70, MIN_BOT_HEIGHT);
	m_selectNoneBot = new QPushButton("Select None");
	m_selectNoneBot->setMinimumSize(70, MIN_BOT_HEIGHT);

	m_slvPalette = new QPalette();
	m_slvPalette->setColor(QPalette::Active, QPalette::WindowText, Qt::white);
	m_slvPalette->setColor(QPalette::Inactive, QPalette::WindowText, Qt::white);
	m_slvPalette->setColor(QPalette::Active, QPalette::Window, QColor(100, 130, 255));
	m_slvPalette->setColor(QPalette::Inactive, QPalette::Window, QColor(100, 130, 255));

	m_solveLabel = new QLabel(""); // not added to the widget, LEAKED
	m_solveLabel->setFrameShadow(QFrame::Sunken);
	m_solveLabel->setFrameShape(QFrame::Panel);
	m_solveLabel->setMinimumSize(150, 10);
	m_solveLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_solveLabel->setPalette(*m_slvPalette);
	updateSolveText(-1, -1);

	QLabel *cntText = new QLabel("Selected / Needed:");
	cntText->setMinimumSize(110, 10);
	cntText->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	m_countPalette = new QPalette();

	m_numLabel = new QLabel("");
	m_numLabel->setTextFormat(Qt::RichText);
	m_numLabel->setPalette(*m_countPalette);
	m_numLabel->setAutoFillBackground(true);
	m_numLabel->setFrameShadow(QFrame::Sunken);
	m_numLabel->setFrameShape(QFrame::Panel);
	m_numLabel->setMinimumSize(110, 10);
	//m_numLabel->setMaximumSize(80, 23);
	m_numLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	QFont bf = m_numLabel->font();
	bf.setBold(true);
	bf.setPointSize(bf.pointSize() + 4);
	m_numLabel->setFont(bf);

	QLabel *setText = new QLabel(tr("Current tab sets:"));
	setText->setMinimumSize(110, 10);
	setText->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	m_selectCurrent = new DataSpinBox(0);
	m_selectCurrent->setMaximumSize(45, 23);
	m_selectCurrent->setButtonSymbols(QAbstractSpinBox::PlusMinus);
	m_selectCurrent->setRange(-1, MAX_IDENTICAL_PIECES);
	m_selectCurrent->setSpecialValueText("X");

	QPushButton *back = new QPushButton(QIcon(":/images/buildview.png"), "Close");
	back->setMinimumSize(50, MIN_BOT_HEIGHT);

	QWidget *ctrl = new QWidget(this); // its needed that this would be a widget and not just a layout so that it won't shrink...
	QHBoxLayout *ctrlLayout = new QHBoxLayout;
	ctrlLayout->setSizeConstraint(QLayout::SetMinimumSize);
	ctrl->setLayout(ctrlLayout);
	m_layout->addWidget(ctrl);

	ctrlLayout->setSpacing(8);
	ctrlLayout->setMargin(9);

	ctrlLayout->addWidget(m_selectAllBot);
	ctrlLayout->addWidget(m_selectNoneBot);
//	ctrlLayout->addWidget(m_solveLabel);
	ctrlLayout->addWidget(cntText);
	ctrlLayout->addWidget(m_numLabel);
	ctrlLayout->addWidget(setText);
	ctrlLayout->addWidget(m_selectCurrent);
	ctrlLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred));

	ctrlLayout->addWidget(back);

	connect(m_selectAllBot, SIGNAL(clicked()), this, SLOT(pressedGlobalAll()));
	connect(m_selectNoneBot, SIGNAL(clicked()), this, SLOT(pressedGlobalNone()));
	connect(m_selectCurrent, SIGNAL(dvalueChanged(int, int)), this, SLOT(changedTabBox(int)));
	connect(back, SIGNAL(clicked()), this, SIGNAL(switchToDesign()));

}

/// continue the creation of the widgets once the pieces XML was loaded.
/// this is where all the work of creating the family tabs and their content is performed.
void PicsSelectWidget::continueCreate()
{
	const PicBucket& bucket = PicBucket::instance();

	m_tabs = new QTabWidget(this);
	connect(m_tabs, SIGNAL(currentChanged(int)), this, SLOT(updateSetsSpinBox(int)));
	m_layout->addWidget(m_tabs);
	m_groups.resize(bucket.grps.size());

	for (int f = 0; f < bucket.families.size(); ++f)
	{
		const PicFamily &fam = bucket.families[f];

		SizedWidget *tab = new SizedWidget(QSize(6 * BOT_TOTAL_X + BOT_OFFS_X + FRAME_OFFS_SPACE_RIGHT - 5, NUM_DEFS * BOT_TOTAL_Y + FRAME_OFFS_Y), nullptr);
		//QWidget *tab = new QWidget();
		//tab->resize(QSize(6 * BOT_TOTAL_X + BOT_OFFS_X + FRAME_OFFS_SPACE_RIGHT, NUM_DEFS * BOT_TOTAL_Y + FRAME_OFFS_Y));

		QScrollArea *scroll = new QScrollArea();
		scroll->setWidget(tab);
		// the following is the voodoo needed to make the scroll area the same color as the tab
		// if this wasn't here, the scroll area would have gotten the default window color
		// this is significant in Windows XP with new apperance style where the tab color is 
		// different from the default window color
		scroll->viewport()->setAutoFillBackground(false);
		tab->setAutoFillBackground(false);
		// scroll area has a frame by default. get rid of it.
		scroll->setFrameShape(QFrame::NoFrame);

		m_tabs->addTab(scroll, fam.name.c_str());


		for (int g = fam.startIndex; g < fam.startIndex + fam.numGroups; ++g)
		{
			const PicGroupDef *grp = &(bucket.grps[g]);
			GroupCtrl &grpctrl = m_groups[g];
			int normg = (g - fam.startIndex); // normalized g with start of family

			QGroupBox *groupbox = new QGroupBox(grp->name.c_str(), tab);
			groupbox->move(BOT_OFFS_X-FRAME_OFFS_SPACE_LEFT, FRAME_OFFS_Y + normg*BOT_TOTAL_Y - 30);
			groupbox->resize(FRAME_OFFS_SPACE_RIGHT + grp->numPics() * BOT_TOTAL_X, BOT_Y + 55);

			grpctrl.num = new DataSpinBox(g, tab);
			grpctrl.num->move(grp->numPics() * BOT_TOTAL_X + BOT_OFFS_X + FRAME_OFFS_SPACE_RIGHT - 65, BOT_OFFS_Y + normg*BOT_TOTAL_Y + 40);
			grpctrl.num->resize(45, 23);
			grpctrl.num->setButtonSymbols(QAbstractSpinBox::PlusMinus);
			grpctrl.num->setRange(-1, MAX_IDENTICAL_PIECES); 
			grpctrl.num->setSpecialValueText("X");

			connect(grpctrl.num, SIGNAL(dvalueChanged(int, int)), this, SLOT(changedGrpBox(int, int)));


			for (int p = 0; p < bucket.grps[g].numPics(); ++p)
			{
				const PicDef *pic = &(grp->getPic(p));
				PicCtrl picctrl;
				int data = (g << 16) | p;

				DataPushButton *button = new DataPushButton(data, QIcon(pic->pixmap), QString(), tab);
				picctrl.bot = button;
				button->setCheckable(true);
				button->move(BOT_OFFS_X + p*BOT_TOTAL_X, BOT_OFFS_Y + normg * BOT_TOTAL_Y); // g normalized to start of family
				button->resize(BOT_X, BOT_Y);
				button->setIconSize(pic->pixmap.size()); // +1 because it's the texture + line, from Pieces.h
				connect(button, SIGNAL(pclicked(int, bool)), this, SLOT(pressedPicButton(int, bool)));

				DataSpinBox *spinbox = new DataSpinBox(data, tab);
				picctrl.num = spinbox;
				spinbox->setRange(0, MAX_IDENTICAL_PIECES);
				spinbox->move(BOT_OFFS_X + p*BOT_TOTAL_X + (BOT_X/5*2 + 2), BOT_OFFS_Y + normg*BOT_TOTAL_Y + BOT_Y + 5);
				spinbox->resize(BOT_X/5*3 - 4, EDIT_THICK + 1);
				spinbox->setButtonSymbols(QAbstractSpinBox::PlusMinus);
				
				connect(spinbox, SIGNAL(dvalueChanged(int, int)), this, SLOT(changedNumBox(int, int)));

				QLabel *solnum = new QLabel("0", tab);
				picctrl.snum = solnum;
				solnum->move(BOT_OFFS_X + p*BOT_TOTAL_X + 2, BOT_OFFS_Y + normg*BOT_TOTAL_Y + BOT_Y + 6);
				solnum->resize(BOT_X/5*2 - 4, EDIT_THICK); 
				solnum->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
				solnum->setFrameShadow(QFrame::Sunken);
				solnum->setFrameShape(QFrame::Panel);
				
				solnum->setPalette(*m_slvPalette);
				solnum->setVisible(false);


				grpctrl.pics.push_back(picctrl);
			}

		}

	}

	setBuildTilesCount(m_doc->getBuild().tilesCount());
	// prevent the build Help sidebar to get notifications. it might not be there yet.
	// eventually the sidebar will take care of itself.
	blockSignals(true); 
	changeToResetValues();
	blockSignals(false);

	m_bFinishedCreate = true;
}


void PicsSelectWidget::changedGrpBox(int count, int g)
{
	if (count == -1)
		return;

	int p;
	const PicBucket& bucket = PicBucket::instance();
	const PicGroupDef &grp = bucket.grps[g];

	for (p = 0; p < grp.numPics(); ++p)
	{
		grp.getPic(p).setSelected(count);
	}

	setUpdatesEnabled(false);
	for (p = 0; p < grp.numPics(); ++p)
	{
		updatePic(g, p, count); // save all the UpdateCounts
	}
	updateCount();
	setUpdatesEnabled(true);

}


// here because of headers
void PicsSelectWidget::changeTab(int data) 
{ 
	if (data == -1)
		return;
	m_tabs->setCurrentIndex(data); 
}

void PicsSelectWidget::setBuildTilesCount(int n)
{
	m_currentBuildTilesCount = n;
	updateNumLabel();
}

void PicsSelectWidget::updateNumLabel()
{
	QColor back;
	switch (m_doc->getPicsWarning())
	{
	case CubeDoc::WARN_NOT_ENOUGH_PIECES: back.setRgb(255, 111, 114); break;
	case CubeDoc::WARN_VERY_LITTLE_PIECES: back.setRgb(255, 194, 63); break;
	case CubeDoc::WARN_NONE: back.setRgb(105, 255, 112); break;
	}

	m_countPalette->setColor(QPalette::Active, QPalette::Window, back);
	m_countPalette->setColor(QPalette::Inactive, QPalette::Window, back);
	m_numLabel->setPalette(*m_countPalette);

	m_numLabel->setText(QString("%1 / %2").arg(m_currentGlobalCount).arg(m_currentBuildTilesCount));

}


// TBD: would have been better if all the signals were to originate from PicBucket...
void PicsSelectWidget::updateCount()
{
	int newN = PicBucket::instance().selectedCount();
	if (newN == m_currentGlobalCount)
		return; 
	m_currentGlobalCount = newN;
	// this is OK since there is no atomic operation which can both add pieces and remove piece
	// except Loading, which is handled in updateView()

	updateFamSelected(); // update the count of all the families
	updateSetsSpinBox(m_tabs->currentIndex());
	emit changedPieceCount(newN);
	updateNumLabel(); // update the label only after the document re-calculated the new warning
}

void PicsSelectWidget::updatePic(int g, int p, int num)
{
	PicCtrl& ctrl = m_groups[g].pics[p];
	if (ctrl.settledNum == num)
		return;

	ctrl.settledNum = num;
	ctrl.bot->setChecked((num > 0)?true:false);
	ctrl.num->setValue(num);

	m_groups[g].num->setCompoundValue(calcGrpSelected(g));
}


void PicsSelectWidget::updateSolNum(int g, int p, int num)
{
	PicCtrl &pc = m_groups[g].pics[p];
	pc.snum->setVisible((num > 0));
	pc.snum->setAutoFillBackground((num > 0)); 
	// make the background show. this is done only here and not always
	// since when its off I want the background to be transparent and show the tab beneath
	pc.snum->setText(QString("%1").arg(num));
}


void PicsSelectWidget::updateSolveText(int first, int second) 
{
	QString str;
	if ((first == -1) && (second == -1))
	{
		str = "No solution displayed";
		m_solveLabel->setAutoFillBackground(false); // make the background transparent
		m_solveLabel->setEnabled(false);
	}
	else
	{
		str = QString("Solution: %1/%2").arg(first).arg(second);
		m_solveLabel->setAutoFillBackground(true); // make the background show
		m_solveLabel->setEnabled(true);
	}
	m_solveLabel->setText(str);
}

void PicsSelectWidget::updateSetsSpinBox(int index)
{
	famSignalBlock(true);
	m_selectCurrent->setCompoundValue(PicBucket::instance().families[index].nSetsSelected);
	famSignalBlock(false);
}

void PicsSelectWidget::updateFamSelected() // update the fam count according the the selected state
{
	const PicBucket& bucket = PicBucket::instance();

	for (int f = 0; f < bucket.families.size(); ++f)
	{
		const PicFamily &fam = bucket.families[f];
		int equalSel = bucket.getPic(fam.startIndex, 0).getSelected();
		bool equal = true;
		fam.nSelected = 0;

		for (int g = fam.startIndex; g < fam.startIndex + fam.numGroups; ++g)
		{
			const PicGroupDef &grp = bucket.grps[g];
			for (int p = 0; p < grp.numPics(); ++p)
			{
				int gNSel = grp.getPic(p).getSelected();
				if (gNSel != equalSel)
					equal = false;
				fam.nSelected += gNSel;
			}
		}
		if (equal) 
			fam.nSetsSelected = equalSel;
		else
			fam.nSetsSelected = -1; // not all equal
	}
}

int PicsSelectWidget::calcGrpSelected(int g) // return the count of the pics of this grp or -1 if they don't agree
{
	const PicBucket& bucket = PicBucket::instance();

	const PicGroupDef &grp = bucket.grps[g];
	int equalSel = grp.getPic(0).getSelected();
	bool equal = true;

	for (int p = 1; (p < grp.numPics()) && equal; ++p)
	{
		if (grp.getPic(p).getSelected() != equalSel)
			equal = false;	
	}
	if (equal)
		return equalSel;
	else
		return -1;
}

//////////////////////////// command handlers /////////////////////////

void PicsSelectWidget::pressedPicButton(int data, bool checked)
{
	int g = (data >> 16) & 0xFFFF, p = data & 0xFFFF;

	const PicDef *pd = &(PicBucket::instance().getPic(g, p)); 

	if (pd->getSelected() > 0)
	{
		pd->lastnSelected = pd->getSelected();
		pd->setSelected(0);
	}
	else
	{
		pd->setSelected(pd->lastnSelected);
	}
	famSignalBlock(true);
	updatePic(g, p, pd->getSelected());
	updateCount();
	famSignalBlock(false);
}

void PicsSelectWidget::changedNumBox(int num, int data)
{
	int g = (data >> 16) & 0xFFFF, p = data & 0xFFFF;
	
	const PicDef *pd = &(PicBucket::instance().getPic(g, p)); 

	pd->setSelected(num); // index and actual number are the same
	updatePic(g, p, num);
	updateCount();
}

/// this method is part of the signal storm control mechanism that is
/// in place to avoid the cascading signaling from cross referenced widgets.
/// blocks everything. virtually all signals are blocked.
/// this method also employs a stack so it can be called multiple
/// times from nestsing recusion levels.
void PicsSelectWidget::globalSignalBlock(bool block)
{ // signal storm control
	if (block) // this mechanism is here to prevent an untimely unblocking due to recursion
	{ 
		m_nGblocked++; 
		m_nFblocked++;
		if (m_nGblocked > 1)
			return;
	}
	else
	{ 
		m_nGblocked--; 
		m_nFblocked--;
		if (m_nGblocked > 0)
			return;
	}

	int g, p;
	for (g = 0; g < m_groups.size() ; ++g)
	{
		GroupCtrl &gc = m_groups[g];
		gc.num->blockSignals(block);
		for (p = 0; p < gc.pics.size(); ++p)
		{
			gc.pics[p].num->blockSignals(block);
			gc.pics[p].bot->blockSignals(block);
		}
	}
	m_selectCurrent->blockSignals(block);
	m_bIncomingSignalsBlocked = block;
}

/// this method is part of the signal storm control mechanism that is
/// in place to avoid the cascading signaling from cross referenced widgets.
void PicsSelectWidget::famSignalBlock(bool block)
{ // signal storm control
	if (block)
	{ 
		m_nFblocked++;
		if (m_nFblocked > 1)
			return;
	}
	else
	{ 
		m_nFblocked--;
		if (m_nFblocked > 0)
			return;
	}

	m_selectCurrent->blockSignals(block);
	m_bIncomingSignalsBlocked = block;
}


void PicsSelectWidget::pressedGlobalAll()
{
	int g, p;
	for (g = 0; g < PicBucket::instance().grps.size() ; ++g)
	{
		const PicGroupDef &gp = PicBucket::instance().grps[g];
		for (p = 0; p < gp.numPics(); ++p)
		{
			gp.getPic(p).setSelected(1);
		}
	}

	globalSignalBlock(true);
	setUpdatesEnabled(false);
	for (g = 0; g < PicBucket::instance().grps.size() ; ++g) // UpdateCount optimization
	{
		const PicGroupDef &gp = PicBucket::instance().grps[g];
		for (p = 0; p < gp.numPics(); ++p)
		{
			updatePic(g, p, 1);
		}
	}
	updateCount();
	setUpdatesEnabled(true);
	globalSignalBlock(false);

}

void PicsSelectWidget::pressedGlobalNone()
{
	int g, p;
	for (g = 0; g < PicBucket::instance().grps.size() ; ++g)
	{
		PicGroupDef &gp = PicBucket::mutableInstance().grps[g];
		for (p = 0; p < gp.numPics(); ++p)
		{
			PicDef& def = gp.getPic(p);
			def.setSelected(0);
			def.lastnSelected = 1;
		}
	}

	globalSignalBlock(true);
	setUpdatesEnabled(false);
	for (g = 0; g < PicBucket::instance().grps.size() ; ++g) // UpdateCount optimization
	{
		const PicGroupDef &gp = PicBucket::instance().grps[g];
		for (p = 0; p < gp.numPics(); ++p)
		{
			updatePic(g, p, 0);
		}
	}

	updateCount();
	setUpdatesEnabled(true);
	globalSignalBlock(false);
}


/// update the view according to the current state in the PicBucket.
void PicsSelectWidget::updateView(int hint) // SLOT
{
	if (GET_PIC_HINT(hint) == HINT_PIC_NULL)
		return; // nothing to do here. move along
	if (!m_bFinishedCreate)
		return;

	const SlvCube* curslv = m_doc->getCurrentSolve();
	const PicBucket& bucket = PicBucket::instance();
	int pichint = GET_PIC_HINT(hint);
	bool doReadCheckFromSlv = ( (curslv != nullptr) && ((pichint & HINT_PIC_READSLVCHECK) != 0) );
	bool doUpdateCheck = (pichint & HINT_PIC_UPDATECHECK) != 0;

	int g, p;
	// reset everything
	for (int pi = 0; pi < bucket.pdefs.size() ; ++pi)
	{
		const PicDef &def = bucket.pdefs[pi];
		def.nUsed = 0;
		if (doReadCheckFromSlv)
		{
			def.setSelected(0);
		}
	}
	if (curslv != nullptr)
	{ // make everybody in the solution light up
		for (int plci = 0; plci < curslv->dt.size(); ++plci)
		{
			int abs_sc = curslv->dt[plci].abs_sc;
			++(bucket.getPic(curslv->picdt[abs_sc].gind, curslv->picdt[abs_sc].pind).nUsed);
		}

		if (doReadCheckFromSlv)
		{
			// make everybody that is selected be pressed
			for (int pi = 0; pi < curslv->picdt.size(); ++pi)
			{
				bucket.getPic(curslv->picdt[pi].gind, curslv->picdt[pi].pind).addSelected(1);
			}
		}
	}

	// commit the information to the gui
	setUpdatesEnabled(false);

	// this needs to be outside since we may have zeroed something. and UpdateCount optimization.
	for (g = 0; g < bucket.grps.size() ; ++g)
	{	
		for (p = 0; p < bucket.grps[g].numPics(); ++p)
		{
			const PicDef &def = bucket.getPic(g, p);
			updateSolNum(g, p, def.nUsed);
			if (doReadCheckFromSlv || doUpdateCheck)
			{
				updatePic(g, p, def.getSelected());
			}
		}
	}	

	if (curslv == nullptr)
		updateSolveText(-1, -1);
	else
		updateSolveText(m_doc->getCurrentSolveIndex() + 1, m_doc->getSolvesNumber());

	m_currentGlobalCount = -1; // reset the optimization since the new number can be equal to the old
	updateCount();
	setUpdatesEnabled(true);

}


void PicsSelectWidget::changedTabBox(int count) // SLOT
{
	changeFamBox(count, m_tabs->currentIndex());
}

void PicsSelectWidget::changeFamBox(int count, int f) // SLOT
{
	if (m_bIncomingSignalsBlocked)
		return;
	if (count == -1)
		return;

	int g, p;
	const PicBucket& bucket = PicBucket::instance();
	const PicFamily &fam = bucket.families[f];
	for (g = fam.startIndex; g < fam.startIndex + fam.numGroups; ++g)
	{
		const PicGroupDef &grp = bucket.grps[g];
		for (p = 0; p < grp.numPics(); ++p)
		{
			grp.getPic(p).setSelected(count);
		}
	}

	globalSignalBlock(true);
	setUpdatesEnabled(false);

	// we we got an external fam update, set the current fam to the one change
	if ((f != m_tabs->currentIndex()) && isVisible())
		m_tabs->setCurrentIndex(f);

	for (g = fam.startIndex; g < fam.startIndex + fam.numGroups; ++g)
	{
		const PicGroupDef &grp = bucket.grps[g];
		for (p = 0; p < grp.numPics(); ++p)
		{
			updatePic(g, p, count); // save all the UpdateCounts
		}
	}

	updateCount();
	setUpdatesEnabled(true);
	globalSignalBlock(false);
}

void PicsSelectWidget::changeToResetValues() // SLOT
{
	int g, p, f;
	PicBucket& bucket = PicBucket::mutableInstance();
	bucket.setToFamResetSel();

	setUpdatesEnabled(false);

	// this is not superflous. if we're not connected to anyone else, we need it
	// although with the BuildDlg feedback this may seem unnecessary.
	for(f = 0; f < bucket.families.size(); ++f)
	{
		const PicFamily &fam = bucket.families[f];
		int setTo = fam.onResetSetCount;
		for (g = fam.startIndex; g < fam.startIndex + fam.numGroups; ++g)
		{
			const PicGroupDef &grp = bucket.grps[g];
			for (p = 0; p < grp.numPics(); ++p)
			{
				updatePic(g, p, setTo); // save all the UpdateCounts
			}
		}
	}

	updateCount();
	setUpdatesEnabled(true);
}

void PicsSelectWidget::enableEdit(bool v)
{
	m_tabs->setEnabled(v);
	m_selectAllBot->setEnabled(v);
	m_selectNoneBot->setEnabled(v);
	m_solveLabel->setEnabled(v);
	m_numLabel->setEnabled(v);
	m_selectCurrent->setEnabled(v);
}

void PicsSelectWidget::slvProgStatsUpdate(int hint, int data)
{
	Q_UNUSED(data)
	switch (hint)
	{
	case SHINT_START: enableEdit(false); break;
	case SHINT_STOP: enableEdit(true); break;
	}
}