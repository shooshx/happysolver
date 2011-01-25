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
#include "BuildHelpDlg.h"

#include <QPushButton>
#include <QButtonGroup>
#include <QLabel>
#include "DataWidgets.h"
#include "Pieces.h"
#include "Configuration.h"
#include "CubeDoc.h"
#include "MainWindow.h"



BuildHelpDlg::BuildHelpDlg(QWidget* parent, MainWindow* main, CubeDoc *doc) 
	:SizedWidget(QSize(MIN_HLP_PANE_WIDTH, 0), parent), m_doc(doc), m_main(main), m_curPressedId(-1)
{
	//setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	//QWidget *topwin = this; // additional widget not needed for free resizing. the tab is the additional window
	QWidget *topwin = new QWidget(this); // apperantly it is needed. tough.
	QVBoxLayout *toplayout = new QVBoxLayout;
	toplayout->setMargin(0);
	toplayout->setSpacing(0);
	toplayout->addWidget(topwin);
	this->setLayout(toplayout);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setMargin(6);
	layout->setSpacing(0);
	topwin->setLayout(layout);

	QGroupBox *editGrp = new QGroupBox("Editor");
	QVBoxLayout *botslayout = new QVBoxLayout;
	botslayout->setMargin(5);
	editGrp->setLayout(botslayout);

	m_addBot = new QPushButton(QIcon(":/images/addtile.png"), "Double click Adds a cube");
	m_addBot->setMinimumSize(50, MIN_BOT_HEIGHT);
	m_addBot->setCheckable(true);

	m_removeBot = new QPushButton(QIcon(":/images/removetile.png"), "Double click Removes a cube");
	m_removeBot->setMinimumSize(50, MIN_BOT_HEIGHT);
	m_removeBot->setCheckable(true);

//	botslayout->addWidget(new QLabel("<b>Double click a tile to:"));
	botslayout->addWidget(m_addBot);
	botslayout->addWidget(m_removeBot);

	m_buildBots = new QButtonGroup(this);
	m_buildBots->setExclusive(true);
	m_buildBots->addButton(m_addBot);
	m_buildBots->setId(m_addBot, 1);
	m_buildBots->addButton(m_removeBot);
	m_buildBots->setId(m_removeBot, 2);
	connect(m_buildBots, SIGNAL(buttonClicked(int)), this, SLOT(actionPressed(int)));

	m_actHelpLabel = new QLabel("");
	m_actHelpLabel->setTextFormat(Qt::RichText);
//	m_actHelpLabel->setMinimumSize(200, 21);
	m_actHelpLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	m_tilesText = QString("%1").arg(m_doc->getBuild().tilesCount());

	QGroupBox *solverGrp = new QGroupBox("Solver");
	QVBoxLayout *slvrLayout = new QVBoxLayout; // contains the first line and a bottom pane
	solverGrp->setLayout(slvrLayout);
	m_buildTilesLabel = new QLabel("Needed pieces: " + m_tilesText);
	m_buildTilesLabel->setTextFormat(Qt::RichText);
	//m_buildTilesLabel->setMinimumSize(50, 25);
	m_buildTilesLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_boldf = m_buildTilesLabel->font();
	m_boldf.setBold(true);
	m_boldf.setPointSize(m_boldf.pointSize() + 4);
	m_buildTilesLabel->setFont(m_boldf);
	slvrLayout->addWidget(m_buildTilesLabel);

	QHBoxLayout *iconLayout = new QHBoxLayout; // contains the icon and a warning
	slvrLayout->addLayout(iconLayout); 
	m_animLabel = new QLabel("");
	m_cubeAnim = new QMovie(":/images/cube_ts.mng");
	m_cubeAnim->setCacheMode(QMovie::CacheAll); //needed for looping
	iconLayout->addWidget(m_animLabel);

	m_warnLabel = new QLabel;
	m_warnLabel->setTextFormat(Qt::RichText);
//	m_warnLabel->resize(100, 50);
	m_warnLabel->setMinimumSize(1, 1);
	m_warnLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_warnLabel->setWordWrap(true);
	m_warnLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	iconLayout->addWidget(m_warnLabel);

	m_goBot = new ActionPushButton(m_main->m_goAct);
	m_goBot->showText(true);
	m_goBot->setMinimumSize(50, 35);
	slvrLayout->addWidget(m_goBot);

	m_picSel = new QGroupBox("Pieces Selector", this);

	QVBoxLayout *blayout = new QVBoxLayout;
	blayout->setMargin(0);
	blayout->setSpacing(6);

	QGroupBox *zoomc = new QGroupBox("Zoom");
	QHBoxLayout *zlayout = new QHBoxLayout;
	zlayout->setSpacing(0);
	zlayout->setMargin(3);
	m_zoomSlider = new QSlider(Qt::Horizontal);
	m_zoomSlider->setRange(ZOOM_MIN, ZOOM_MAX);
	m_zoomSlider->setValue(100);
	connect(m_zoomSlider, SIGNAL(valueChanged(int)), this, SIGNAL(zoomChanged(int)));
	zoomc->setLayout(zlayout);
	zlayout->addWidget(m_zoomSlider);
	blayout->addWidget(zoomc);

	m_selPieces = new QLabel("Total: 0");
	m_selPieces->setTextFormat(Qt::RichText);
	m_selPieces->setMinimumSize(27, 25);
	m_selPieces->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_selPieces->setFont(m_boldf);

	layout->setSizeConstraint(QLayout::SetMinimumSize); // keeps it from squashing
	layout->addWidget(editGrp);
	layout->addWidget(m_picSel);
	layout->addStretch();
	layout->addWidget(solverGrp);
	layout->addLayout(blayout);

	actionPressed(1); //add/remove action

}


QSize BuildHelpDlg::minimumSizeHint() const 
{ 
	return QSize(0,0); 
}

/// called when the pieces XML is done downloading and it is ok
/// to create the piece familiy selection widgets.
void BuildHelpDlg::completePicsWidgets() // SLOT
{
	const PicBucket& bucket = PicBucket::instance();
	m_picSel->hide(); // this is quite odd. (if we don't do that the widgets will not show) TBD

//	m_picSel->setMinimumWidth(200);
	QGridLayout *glayout = new QGridLayout;
	glayout->setSpacing(8);
	glayout->setSizeConstraint(QLayout::SetMinimumSize);
	m_picSel->setLayout(glayout);

	glayout->setColumnStretch(0, 1);
	glayout->addWidget(new QLabel("Type"), 0, 1, 1, 2);
	glayout->addWidget(new QLabel("Sets"), 0, 3);
	glayout->addWidget(new QLabel("Pieces"), 0, 4);
	glayout->setColumnStretch(5, 1);

	int f;
	for (f = 0; f < bucket.families.size(); ++f)
	{
		const PicFamily &fam = bucket.families[f];
		FamCtrl ctrl;

		ctrl.icon = new QLabel();
		ctrl.icon->setPixmap(QPixmap(QString(":/%1").arg(fam.iconFilename)));
		ctrl.icon->setMinimumSize(21, 21);
		glayout->addWidget(ctrl.icon, f + 1, 1);

		ctrl.name = new QLabel(fam.name);
		//ctrl.name->setMinimumSize(100, 22);
		glayout->addWidget(ctrl.name, f + 1, 2);
		
		ctrl.num = new DataSpinBox(f, m_picSel);
		ctrl.num->setMinimumSize(40, 22);
		ctrl.num->setButtonSymbols(QAbstractSpinBox::PlusMinus);
		ctrl.num->setRange(-1, MAX_IDENTICAL_PIECES); 
		ctrl.num->setSpecialValueText("X");
		ctrl.num->setToolTip(QString("Number of %1 sets").arg(fam.name));
		glayout->addWidget(ctrl.num, f + 1, 3);
		connect(ctrl.num, SIGNAL(dvalueChanged(int, int)), this, SIGNAL(changedFamBox(int, int)));

	/*	ctrl.custom = new DataPushButton(f, "...", m_picSel); we don't want this button
		ctrl.custom->setMaximumSize(25, 22);
		ctrl.custom->setToolTip("More specific selection...");
		glayout->addWidget(ctrl.custom, f + 1, 4);
		connect(ctrl.custom, SIGNAL(pclicked(int, bool)), this, SIGNAL(switchToPicsTab(int)));*/

		ctrl.count = new QLabel("0");
		ctrl.count->setMaximumSize(27, 22);
		ctrl.count->setFrameShadow(QFrame::Sunken);
		ctrl.count->setFrameShape(QFrame::Panel);

		glayout->addWidget(ctrl.count, f + 1, 4);

		m_fams.push_back(ctrl);

	}

    QFrame *line = new QFrame;
    line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Raised);
	glayout->addWidget(line, f + 1, 1, 1, 4);

	ActionPushButton *picsBot = new ActionPushButton(m_main->m_picsViewAct);
	picsBot->showText(true);
	picsBot->setAltText("Advanced");
	picsBot->setToolTip("Select individual pieces");
	picsBot->setMinimumSize(80, MIN_BOT_HEIGHT);
	glayout->addWidget(picsBot, f + 2, 1, 1, 2, Qt::AlignLeft);

	glayout->addWidget(m_selPieces, f + 2, 1, 1, 4, Qt::AlignRight);

	updateFamCount();

	m_picText = QString("%1").arg(PicBucket::instance().selectedCount());
	updateWarnMsgs();

	m_picSel->show();

	update();
}

/// the zoom was externllu changed probably by the mouse wheel.
/// update the slider with the new status.
void BuildHelpDlg::updateZoom(int v)
{
	m_zoomSlider->setValue(v);
}

void BuildHelpDlg::updateFamCount()
{
	const PicBucket& bucket = PicBucket::instance();

	for (int f = 0; f < bucket.families.size(); ++f)
	{
		const PicFamily &fam = bucket.families[f];
		m_fams[f].num->setCompoundValue(fam.nSetsSelected);
		m_fams[f].count->setText(QString("%1").arg(fam.nSelected));
	}
}

void BuildHelpDlg::setActionLabel(bool enabled)
{
	if (enabled)
	{
		if (m_curPressedId == 1)
			m_actHelpLabel->setText("<font color=\"#1100B6\">Double Click a tile to Add a Cube");
		else
			m_actHelpLabel->setText("<font color=\"#B60009\">Double Click a tile to Remove a Cube");
	}
	else
		m_actHelpLabel->setText("<font color=\"#505050\">Can't edit while trying to solve design.");

}

void BuildHelpDlg::actionPressed(int id)
{
	if (m_curPressedId == id)
		return;

	m_curPressedId = id;
	m_buildBots->button(id)->setChecked(true);
	setActionLabel(true);
	emit changedAction(id == 2);

}

void BuildHelpDlg::changeAction(bool remove)
{
	actionPressed(remove?2:1);
}


void BuildHelpDlg::setTilesCount(int count)
{
	m_tilesText = QString("%1").arg(count);
}

void BuildHelpDlg::setPieceCount(int count)
{
	m_picText = QString("%1").arg(count);
	updateFamCount();
}

enum EIconSelect { ICON_NONE, ICON_ERROR, ICON_WARN, ICON_OK };

/// display the most appropriate message in the message widget
/// choose an icon and set the tooltip text
/// called from statsUpdate()
void BuildHelpDlg::updateWarnMsgs()
{
	QString txt, tool, selptool;
	EIconSelect icon = ICON_NONE;
	m_picPre.clear();

	if (m_doc->isSlvEngineRunning())
	{
		switch (m_doc->getWarning())
		{
		case CubeDoc::WARN_NONE:
			txt = "<b>Running...";
			break;
		case CubeDoc::WARN_VERY_LITTLE_PIECES:
			txt = "<b>Running...";
			//m_picPre = "<font color=\"#FF6D01\">"; // it is disabled anyway..
			break;
		default:
			txt = "<font color=\"#E200B5\"><b>running? how could that be?? BUG.";
			break;
		}
		m_animLabel->setMovie(m_cubeAnim);
	}
	else
	{
		switch (m_doc->getWarning())
		{
		case CubeDoc::WARN_SHAPE_SIDES:
			icon = ICON_ERROR;
			txt = "<b>This design cannot be built, watch for the red markings for the problem";
			tool = "this design can't be build because 4 tiles can't fit on the same line.\nAdd or remove blocks from the design in order to solve this problem";
			break;
		case CubeDoc::WARN_SHAPE_NOT_CONNECT:
			icon = ICON_ERROR;
			txt = "<b>I can't work on more then one shape at a time. Remove one of the shapes.";
			tool = "the design contains several disconnected shapes,\nRemove some of the blocks and leave only one solid shape.";
			break;
		case CubeDoc::WARN_NOT_ENOUGH_PIECES:
			icon = ICON_ERROR;
			txt = "<b>There aren't enough pieces to build this design. Select more pieces";
			tool = "The number of selected pieces needs to be greater then the number of tiles\nin the design, select more piece";
			if (m_doc->getBuild().tilesCount() > 6)
			{
				char *add = " or remove blocks from the design.";
				txt += add; tool += add;
			}
			else
			{
				txt += "."; tool += ".";
			}

			m_picPre = "<font color=\"#F01700\">";
			selptool = tool;
			break;
		case CubeDoc::WARN_VERY_LITTLE_PIECES:
			icon = ICON_WARN;
			// can't happen if tileCount == 6, so no need to check
			txt = "<b>It is unlikely a solution can be found with this few pieces. Select more pieces or remove blocks from the design.";
			tool = "If you select some more pieces, finding a solution for the design\nwill be faster and more probable.";
			m_picPre = "<font color=\"#FF6D01\">";
			selptool = tool;
			break;
		case CubeDoc::WARN_SHAPE_NO_START:
			icon = ICON_ERROR;
			txt = "<font color=\"#E200B5\"><b>No yellow tile? how could that be?? BUG.";
			break;
		case CubeDoc::WARN_FULL_ENUM_NO_SOLVES:
			icon = ICON_ERROR;
			txt = "<b>I searched all the possibilities and couldn't find a solution. Change pieces selection.";
			tool = "It's not possible to build this design with the selected pieces\nselect more or other pieces and try again.";
			m_picPre = "<font color=\"#00A413\">";
			break;
		case CubeDoc::WARN_NONE:
			icon = ICON_OK;
			txt = "<b>Press \"Solve It!\" to find solutions.";
			m_picPre = "<font color=\"#00A413\">";
			break;
		}
	}

	switch (icon)
	{
	case ICON_OK: m_animLabel->setPixmap(QPixmap(":/images/run_ok.png")); break;
	case ICON_WARN: m_animLabel->setPixmap(QPixmap(":/images/run_warn.png")); break; 
	case ICON_ERROR: m_animLabel->setPixmap(QPixmap(":/images/run_error.png")); 
//		m_warnLabel->setMinimumSize(0, 100);
		break;
	}

	//m_goBot->setVisible(m_doc->isGoEnabled());
	m_warnLabel->setText(txt);
	m_warnLabel->setToolTip(tool);

	m_selPieces->setText("Total: " + m_picPre + m_picText);
	m_buildTilesLabel->setText("Needed pieces: " + m_picPre + m_tilesText);

	m_selPieces->setToolTip(selptool);


}

/// edit widgets are disabled when the solution engine is running.
/// called from statsUpdate()
void BuildHelpDlg::enableEditWidgets(bool v)
{
	m_picSel->setEnabled(v);
	m_addBot->setEnabled(v);
	m_removeBot->setEnabled(v);
}


void BuildHelpDlg::statsUpdate(int hint, int data)
{
	switch (hint)
	{
	case SHINT_WARNING:
	case SHINT_START:
	case SHINT_STOP:
	case SHINT_STATUS:
		updateWarnMsgs();
		if (!m_doc->isSlvEngineRunning())
		{
			enableEditWidgets(true);
			m_picSel->setToolTip(QString());
//			m_picsBot->setToolTip(QString());
			setActionLabel(true);
			if (m_cubeAnim->state() == QMovie::Running)
				m_cubeAnim->stop();
		}
		else
		{
			enableEditWidgets(false);
			m_picSel->setToolTip("Can't change piece selection while solving");
//			m_picsBot->setToolTip("Can't change piece selection while solving");
			setActionLabel(false);

			if (m_cubeAnim->state() != QMovie::Running)
				m_cubeAnim->start();
		}

	break;
	}

}
