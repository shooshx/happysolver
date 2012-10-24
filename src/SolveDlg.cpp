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

#include "SolveDlg.h"

#include <QTimer>
#include <QDateTime>

#include "Configuration.h"
#include "CubeDoc.h"

#include <QtDebug>


SolveDlg::SolveDlg(QWidget *parent, CubeDoc *doc)
	:QDialog(parent), m_doc(doc)
{
	for (int i = 0; i < SPEED_AVG_SIZE; ++i) m_speed[i] = -1;
	m_elapsedtime = 0;

	ui.setupUi(this);

	setMinimumSize(size());
	setMaximumSize(size());
	connect(ui.goButton, SIGNAL(clicked()), this, SIGNAL(go())); // doesn't use the action...
	connect(ui.resetButton, SIGNAL(clicked()), this, SIGNAL(resetSolves()));

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(displayTimeout()));

	updateStatus();
	updateSol();
	updatePart();

}


void SolveDlg::start()
{
	m_last_tms = 0;
	m_speedind = 0;
	for (int i = 0; i < SPEED_AVG_SIZE; ++i) m_speed[i] = -1;

	m_starttime = QDateTime::currentDateTime().toTime_t();
	m_timer->start(1000);
	statsUpdate(SHINT_ALL, -1);
}

void SolveDlg::stop()
{
	m_timer->stop();
}


void SolveDlg::displayTimeout()
{
	mint64 tms = m_doc->getRunningStats()->tms; // sample it once

	m_speed[m_speedind++] = tms - m_last_tms;
	if (m_speedind == SPEED_AVG_SIZE) m_speedind = 0;
	m_last_tms = tms;
	m_elapsedtime = QDateTime::currentDateTime().toTime_t() - m_starttime;

	statsUpdate(SHINT_ALL, -1);
}


void SolveDlg::updateSol(int n)
{
	if (n == -1)
	{
		n = m_doc->getSolvesNumber();
	}

	ui.solutions->setText(humanCount(n).c_str());
}


void SolveDlg::updatePart(int argn)
{
	// sample it.
	auto* state = m_doc->getRunningStats();
	volatile bool lucky = state->lucky;
	// dear compiler, don't optimize me out...
	volatile mint64 tms = state->tms;
	volatile int n = argn;
	volatile mint64 show = (n != -1)?n:tms;
	volatile int maxp = state->maxp;

	ui.changes->setText(humanCount(show).c_str());
	
	float avg = 0;
	int howmany = 0;
	// assume the intervals are 1 second
	for (int i = 0; i < SPEED_AVG_SIZE; ++i) 
		if (m_speed[i] != -1) { 
			avg += m_speed[i]; ++howmany; 
		}
	if (howmany != 0) 
		avg /= (float)howmany;
	QString spd = QString(humanCount(avg, 1).c_str()) + "  ch/s";
	if (lucky) 
		spd = "<font color=\"#CF1111\">" + spd; // luck parameter changes color of speed to red

	ui.speed->setText(spd);

	time_t thetime = m_elapsedtime;
	int seconds = thetime % 60; thetime /= 60;
	int minutes = thetime % 60; thetime /= 60;
	int hours = thetime % 24;
	int days = thetime /= 24;
	QString str = QString("%1:%2:%3:%4").arg(days).arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
	ui.CPUTime->setText(str);

	ui.maxPlace->setText(QString("%1").arg(maxp));

}

void SolveDlg::updateStatus()
{
	ui.resetButton->setEnabled(m_doc->solvesExist());

	if (!m_doc->isSlvEngineRunning())
	{
		ui.goButton->setText("Go");
		ui.goButton->setEnabled(m_doc->isGoEnabled());
		ui.status->setText("Ready");
		if (m_doc->getRunningStats()->tms == 0)
			ui.groupBoxStats->setTitle("Run Statistics");
		else
			ui.groupBoxStats->setTitle("Last Run Statistics");
	}
	else
	{
		ui.goButton->setText("Stop");
		ui.status->setText("Running");
		ui.groupBoxStats->setTitle("Run Statistics");
	}
}

void SolveDlg::statsUpdate(int hint, int data)
{
	switch (hint)
	{
	case SHINT_ALL: 
		updateSol(data);
		updatePart();
		updateStatus();
		break;
	case SHINT_SOLUTIONS:
		updateSol(data);
		updatePart();
		updateStatus();
		break;
	case SHINT_PARTCHN:
		updatePart(data);
		break;
	case SHINT_WARNING:
	case SHINT_STATUS:
		updateStatus();
		break;
	case SHINT_START:
		start();
		updateStatus();
		break;
	case SHINT_STOP:
		updateSol();
		updatePart();
		updateStatus();
		stop();
		break;
	}

}
