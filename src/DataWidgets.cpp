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

#include "DataWidgets.h"

#include <QLineEdit>
#include <QAction>

DataPushButton::DataPushButton(int d, const QIcon & icon, const QString & text, QWidget * parent)
	:QPushButton(icon, text, parent), m_data(d)
{
	connect(this, SIGNAL(toggled(bool)), this, SLOT(doPtoggled(bool)));
	connect(this, SIGNAL(clicked(bool)), this, SLOT(doPclicked(bool)));
}

DataPushButton::DataPushButton(int d, const QString & text, QWidget* parent)
	:QPushButton(text, parent), m_data(d)
{
	connect(this, SIGNAL(toggled(bool)), this, SLOT(doPtoggled(bool)));
	connect(this, SIGNAL(clicked(bool)), this, SLOT(doPclicked(bool)));
}


void DataPushButton::doPtoggled(bool checked) 
{ 
	emit ptoggled(m_data, checked); 
}

void DataPushButton::doPclicked(bool checked)
{
	emit pclicked(m_data, checked);
}


DataSpinBox::DataSpinBox(int d, QWidget* parent)
	:QSpinBox(parent), m_data(d), m_backval(0)
{
	QObject::connect(this, SIGNAL(valueChanged(int)), this, SLOT(doDvalueChanged(int)));
	lineEdit()->setReadOnly(true);
	setAlignment(Qt::AlignHCenter);
}

void DataSpinBox::setEditEnabled(bool en) 
{ 
	lineEdit()->setEnabled(en); 
}



void DataSpinBox::doDvalueChanged(int v)
{
	emit dvalueChanged(v, m_data);
}

QAbstractSpinBox::StepEnabled DataSpinBox::stepEnabled() const
{
	int v = value();
	if (v == -1)
		return QAbstractSpinBox::StepDownEnabled | QAbstractSpinBox::StepUpEnabled; // don't gray arrows for X

	QAbstractSpinBox::StepEnabled en = 0;
	if (v > 0)  
		en |= QAbstractSpinBox::StepDownEnabled;
	if (v < maximum()) 
		en |= QAbstractSpinBox::StepUpEnabled;
	return en;
}

void DataSpinBox::stepBy(int steps)
{
	if (value() == -1)
	{
		if ((m_backval == 0) && (steps < 0))
			setValue(1);
		else if ((m_backval == maximum()) && (steps > 0))
			setValue(maximum() - 1);
		else
			setValue(m_backval);
	}
	QSpinBox::stepBy(steps);
	lineEdit()->deselect(); // make the edit line white
}

void DataSpinBox::setCompoundValue(int v)
{
	if (v != -1)
	{
		setValue(v);
		setEditEnabled(true);
	}
	else
	{
		if (value() != -1)
			m_backval = value();
		setValue(-1);
		setEditEnabled(false);
	}
}

// intelligent special value functionality:
// if the spinbox has a -1 then this is the special value. range and special value text
// should be set from outside. (tbd?)
// when value is set to -1 then the X appears, and the edit is grayed
// when pressed up or down, the value before or after the last value before the X apprears
// for instance. we had 2, then we set to -1, then pressed Up. => 3 will appear.


void ActionPushButton::actionEvent(QActionEvent *event)
{
    QAction *action = event->action();
    switch (event->type()) {
    case QEvent::ActionChanged:
        if (action == m_defaultAction)
            setDefaultAction(action); // update button state
        break;
    case QEvent::ActionAdded:
    //    connect(action, SIGNAL(triggered()), this, SLOT(actionTriggered())); not needed because no one connects to this
        break;
    case QEvent::ActionRemoved:
        if (m_defaultAction == action) {
            m_defaultAction = 0;
#ifndef QT_NO_MENU
            if (action->menu() == menu())
                setMenu(0);
#endif
        }
        action->disconnect(this);
        break;
    default:
        ;
    }
    QPushButton::actionEvent(event);
}

void ActionPushButton::setDefaultAction(QAction *action)
{
    if (m_defaultAction && menu() == m_defaultAction->menu())
        setMenu(0);

    m_defaultAction = action;
    if (!action)
        return;
    if (!actions().contains(action))
        addAction(action);
	if (m_showText)
	{
		if (!m_altText.isEmpty())
			setText(m_altText);
		else
			setText(action->iconText());
	}
    setIcon(action->icon());
    setToolTip(action->toolTip());
    setStatusTip(action->statusTip());
    setWhatsThis(action->whatsThis());

    if (QMenu *menu = action->menu()) {
        // new 'default' popup mode defined introduced by tool bar. We
        // should have changed QToolButton's default instead.
        setMenu(menu);
    }
    setCheckable(action->isCheckable());
    setChecked(action->isChecked());
    setEnabled(action->isEnabled());
    setFont(action->font());
}

// activate the action
void ActionPushButton::nextCheckState()
{
    if (m_defaultAction == NULL)
        QAbstractButton::nextCheckState();
    else
        m_defaultAction->trigger();
}
