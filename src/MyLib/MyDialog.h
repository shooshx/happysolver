#ifndef MYDIALOG_H_INCLUDED
#define MYDIALOG_H_INCLUDED

#include <QDialog>
#include <QAction>

class MyDialog : public QDialog
{
	Q_OBJECT;
public:
	MyDialog(QWidget *parent)
		: QDialog(parent)
	{}

	virtual ~MyDialog() {}
	void connectAction(QAction *action)
	{
		action->setCheckable(true);
		action->setChecked(isVisible());
		connect(action, SIGNAL(toggled(bool)), this, SLOT(setVisible(bool)));
		connect(this, SIGNAL(shownChanged(bool)), action, SLOT(setChecked(bool)));
	}

protected:
	virtual void hideEvent(QHideEvent *) { if (isHidden()) emit shownChanged(false); }
	virtual void showEvent(QShowEvent *) { emit shownChanged(true); }

signals:
	void shownChanged(bool shown);

	
};

#endif // MYDIALOG_H_INCLUDED
