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

#include "AssembleStepDlg.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QPushButton>


AssembleStepDlg::AssembleStepDlg(QWidget *parent) :QDialog(parent)
{
	setWindowTitle("Step By Step");
	QVBoxLayout *vlay = new QVBoxLayout;
	setLayout(vlay);
	m_slider = new QSlider(Qt::Horizontal, this);
	m_slider->setTickPosition(QSlider::TicksBelow);
	m_slider->setRange(1, 6); // bug in QT4.3.0 makes this not look even
	m_slider->setValue(6);
	connect(m_slider, SIGNAL(valueChanged(int)), this, SIGNAL(stepChanged(int)));

	vlay->addWidget(m_slider);
	QHBoxLayout *hlay = new QHBoxLayout;
	vlay->addLayout(hlay);
	m_prev = new QPushButton("prev", this);
	m_prev->setAutoRepeat(true);
	connect(m_prev, SIGNAL(clicked()), this, SLOT(prev()));
	m_next = new QPushButton("next", this);
	m_next->setAutoRepeat(true);
	connect(m_next, SIGNAL(clicked()), this, SLOT(next()));
	hlay->addWidget(m_prev);
	hlay->addStretch();
	hlay->addWidget(m_next);

}

void AssembleStepDlg::setTilesCount(int n)
{
	m_slider->setMaximum(n); // maximum needs to be equal to the maximum face number which is zero based
	m_slider->setValue(n);
}

void AssembleStepDlg::prev()
{
	m_slider->setValue(m_slider->value() - 1);
}

void AssembleStepDlg::next()
{
	m_slider->setValue(m_slider->value() + 1);
}
