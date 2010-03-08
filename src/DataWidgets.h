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

#ifndef __DATAWIDGETS_H__INCLUDED__
#define __DATAWIDGETS_H__INCLUDED__

#include <QPushButton>
#include <QSpinBox>
#include <QLabel>
#include <QResizeEvent>

/** \file
	Declares various widgets which carry with them a data item which holds information about their identity.
	Declares the DataSpinBox and DataSpinBox classes which are data widgets.
	Declares SizedWidget which is not really related to this file but is here anyway.
	Declares ActionPushButton which is an action oriented push button.
*/

/** DataPushButton is a push button with some memory about its identity.
	It is needed due to a misfeature in QT which doesn't allow PushButtons 
	to have attached data. The big buttons with the piece drawing in the
	advanced piece selection window are of this class.
*/
class DataPushButton : public QPushButton
{
	Q_OBJECT
public:
	DataPushButton(int d, const QIcon & icon, const QString & text, QWidget* parent);
	DataPushButton(int d, const QString & text, QWidget* parent);
	virtual ~DataPushButton () {}

	int getData() { return m_data; }
	void setData(int d) { m_data = d; }

private slots:
	void doPtoggled(bool checked);
	void doPclicked(bool checked);

signals:
	void ptoggled(int d, bool checked);
	void pclicked(int d, bool checked);

private:
	int m_data; 

};

/** DataSpinBox is a spin box with memory which can be set to middle values.
	This widget doubles as
	- a spin box with data
	- a "Compound" spinbox, with intelligent special value which can be thought
	at as a 'middle' value between two values.

	sometimes only one feature is needed, sometimes both.
*/
class DataSpinBox :public QSpinBox
{
	Q_OBJECT
public:
	DataSpinBox(int d, QWidget* parent = NULL);
	virtual ~DataSpinBox() {}

	int getData() { return m_data; }
	void setData(int d) { m_data = d; }
	void setEditEnabled(bool en);

public slots:
	void setCompoundValue(int v);

private slots:
	void doDvalueChanged(int v);

signals:
	void dvalueChanged(int v, int d);

protected:
	virtual QAbstractSpinBox::StepEnabled stepEnabled() const;
	virtual void stepBy(int steps);

private:
	int m_data; // just indexes, use for various things
	int m_backval; // the value that is there virtually instead of 'X'
};


class QAction;

/**	ActionPushButton is a QPushButton which takes its characteristics from a QAction.
	this class is needed due to a deficiency in QPushButtons. most of the functionality
	is copied from QToolButton. This class is needed becuase QToolButton doesn't
	know how to look like a regular QPushButton in a satisfactory manner.
*/
class ActionPushButton : public QPushButton
{
	Q_OBJECT
public:
	ActionPushButton(QAction *action, QWidget *parent = NULL) : QPushButton(parent), m_defaultAction(NULL), m_showText(false)
	{ setDefaultAction(action); }

	virtual ~ActionPushButton() {}

	void setDefaultAction(QAction *action);
	void showText(bool v) { m_showText = v; setDefaultAction(m_defaultAction); /* update */ }
	void setAltText(const QString& v) {  m_altText = v; setDefaultAction(m_defaultAction);  }

protected:
	virtual void actionEvent(QActionEvent *event);
	virtual void nextCheckState();

private:
	QAction *m_defaultAction;
	QString m_altText;
	bool m_showText;
};



/** SizedWidget is a normal QWidget which impements sizeHint().
	it is required in the sizing scheme of the sidebars.
*/
class SizedWidget : public QWidget
{
public:
	SizedWidget(QSize size, QWidget* parent) :QWidget(parent), m_size(size) {}
	virtual ~SizedWidget() {}
	virtual QSize sizeHint() const { return m_size;	}

protected:
	QSize m_size;
};


#endif
