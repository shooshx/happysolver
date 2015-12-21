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

#include "ActFocusComboBox.h"
#include <QAction>
#include <QActionEvent>

 
void ActFocusComboBox::updateList()
{
	SlvData dat = m_defaultAction->data().value<SlvData>();
	int slvn = dat.size;
	if (count() != slvn) //there's something to update
	{ // TBD: there is room for optimization here, if we only need to add, just add
		clear();
		QString st("%1");
		for(int i = 0; i < slvn; ++i)
		{
			addItem(st.arg(i + 1), i);
		}
	}
	setCurrentIndex(dat.index);
}

void ActFocusComboBox::actionEvent(QActionEvent *event)
{
    QAction *action = event->action();
    switch (event->type()) {
    case QEvent::ActionChanged:
     //   if (action == m_defaultAction)
     //       setDefaultAction(action); // update button state
		updateList();
		setEnabled(action->isEnabled());
        break;
    case QEvent::ActionAdded:
     //   connect(action, SIGNAL(triggered()), this, SLOT(actionTriggered()));
        break;
    case QEvent::ActionRemoved:
        if (m_defaultAction == action) {
            m_defaultAction = 0;
        }
        action->disconnect(this);
        break;
    default:
        ;
    }
    QComboBox::actionEvent(event);
}

void ActFocusComboBox::setDefaultAction(QAction *action)
{
	m_defaultAction = action;
    if (!action)
        return;
    if (!actions().contains(action))
        addAction(action);

    setToolTip(action->toolTip());
    setStatusTip(action->statusTip());
    setWhatsThis(action->whatsThis());
    setEnabled(action->isEnabled());
	connect(this, SIGNAL(activated(int)), this, SLOT(setActIndex(int)));
}

void ActFocusComboBox::setActIndex(int ind)
{
	SlvData data(ind, m_defaultAction->data().value<SlvData>().size);
	m_defaultAction->setData(QVariant::fromValue(data)); // needed because setData is not a slot and takes a variant
}
