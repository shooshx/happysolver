#include "MyInputDlg.h"

#include <QCheckBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QColorDialog>
#include <QPalette>

#include "MyColorPicker.h"

MyInputDlg::MyInputDlg(QWidget *parent)
	: MyDialog(parent), m_autoCommit(false)
{
	ui.setupUi(this);
	ui.captionLabel->hide();
	ui.applyBot->setHidden(m_autoCommit);
}

void MyInputDlg::setCaptions(const QString& caption)
{
	if (!caption.isEmpty())
	{
		ui.captionLabel->show();
		ui.captionLabel->setText(caption);
	}
	else
	{
		ui.captionLabel->hide();
	}
}

void MyInputDlg::setApplyButton(bool shown)
{
	m_autoCommit = !shown;
	ui.applyBot->setVisible(shown);
}




void MyInputDlg::addRadio(const QString& o)
{
	QRadioButton *c = new QRadioButton(o);
	if (m_radios.isEmpty())
		c->setChecked(true);
	else
		c->setChecked(false);
	m_radios.append(c);

	ui.optLayout->addWidget(c);
	connect(c, SIGNAL(toggled(bool)), this, SLOT(changedSel()));
}






int MyInputDlg::getRadioResult()
{
	for(int i = 0; i < m_radios.size(); ++i)
	{
		if (m_radios[i]->isChecked())
			return i;
	}
	return -1;
}


QVariant fromString(const QString& v, const std::type_info& ti)
{
	if (ti == typeid(float))
		return v.toFloat();
	else if (ti == typeid(int))
		return v.toInt();
	else if (ti == typeid(double))
		return v.toDouble();
	else if (ti == typeid(QString))
		return v;
	else if (ti == typeid(uint))
		return v.toUInt();
	else
		throw "UNKNOWN!";
}	



void MyInputDlg::commit()
{
	foreach(WidgetIn *w, m_widgets)
		w->commit();
	emit commitedChange();
}
void MyInputDlg::reload()
{
	foreach(WidgetIn *w, m_widgets)
		w->reload();
}


bool MyInputDlg::getValues(QWidget *parent, Prop &p1, Prop &p2, const QString caption)
{
	MyInputDlg *dlg = new MyInputDlg(parent);

	dlg->setCaptions(caption);
	dlg->setWindowTitle("Selection");
	dlg->setModal(true);
	dlg->setApplyButton(false);

	p1.addWidget(dlg);
	p2.addWidget(dlg);

	dlg->reload();
	QDialog::DialogCode ret = (QDialog::DialogCode)dlg->exec();
	if (ret == QDialog::Rejected)
		return false;

	return true;
}
bool MyInputDlg::getValues(QWidget *parent, Prop &p1, Prop &p2, Prop &p3, const QString caption)
{
	MyInputDlg *dlg = new MyInputDlg(parent);

	dlg->setCaptions(caption);
	dlg->setWindowTitle("Selection");
	dlg->setModal(true);
	dlg->setApplyButton(false);

	p1.addWidget(dlg);
	p2.addWidget(dlg);
	p3.addWidget(dlg);

	dlg->reload();
	QDialog::DialogCode ret = (QDialog::DialogCode)dlg->exec();
	if (ret == QDialog::Rejected)
		return false;

	return true;
}
bool MyInputDlg::getValues(QWidget *parent, Prop &p1, Prop &p2, Prop &p3, Prop &p4, const QString caption)
{
	MyInputDlg *dlg = new MyInputDlg(parent);

	dlg->setCaptions(caption);
	dlg->setWindowTitle("Selection");
	dlg->setModal(true);
	dlg->setApplyButton(false);

	p1.addWidget(dlg);
	p2.addWidget(dlg);
	p3.addWidget(dlg);
	p4.addWidget(dlg);

	dlg->reload();
	QDialog::DialogCode ret = (QDialog::DialogCode)dlg->exec();
	if (ret == QDialog::Rejected)
		return false;

	return true;
}

bool MyInputDlg::getValue(QWidget *parent, Prop &p1, const QString caption)
{
	MyInputDlg *dlg = new MyInputDlg(parent);

	dlg->setCaptions(caption);
	dlg->setWindowTitle("Selection");
	dlg->setModal(true);
	dlg->setApplyButton(false);

	p1.addWidget(dlg);

	dlg->reload();
	QDialog::DialogCode ret = (QDialog::DialogCode)dlg->exec();
	if (ret == QDialog::Rejected)
		return false;

	return true;
}



int MyInputDlg::getRadio(QWidget *parent, const QString& o1, const QString& o2, const QString& caption)
{
	MyInputDlg dlg(parent);
	dlg.addRadio(o1);
	dlg.addRadio(o2);
	dlg.setCaptions(caption);

	QDialog::DialogCode ret = (QDialog::DialogCode)dlg.exec();
	if (ret == QDialog::Rejected)
		return -1;
	return dlg.getRadioResult();
}

int MyInputDlg::getRadio(QWidget *parent, const QString& o1, const QString& o2, const QString& o3, const QString& caption)
{
	MyInputDlg dlg(parent);
	dlg.addRadio(o1);
	dlg.addRadio(o2);
	dlg.addRadio(o3);
	dlg.setCaptions(caption);
	QDialog::DialogCode ret = (QDialog::DialogCode)dlg.exec();
	if (ret == QDialog::Rejected)
		return -1;
	return dlg.getRadioResult();
}

void MyInputDlg::on_okBot_clicked()
{
	commit();
	accept();
}

void MyInputDlg::on_cancelBot_clicked()
{
	reject();
}

void MyInputDlg::on_applyBot_clicked()
{
	commit();
}


ColorSelIn::ColorSelIn(TypeProp<QColor> *_v, QAbstractButton *_bw, bool _withAlpha, EFlags flags, bool _autoup)
 :WidgetIn(_v, _bw), v(_v), bw(_bw), sw(nullptr), autoup(_autoup), withAlpha(_withAlpha)
 ,modellessDlg(nullptr)
 ,doModal(flags == DoModal)
{
	connect(bw, SIGNAL(clicked(bool)), this, SLOT(residualUpdate()));
	if (sw != nullptr)
	{
 		sw->setAutoFillBackground(true);
 		sw->setLineWidth(1);
 		sw->setFrameShape(QFrame::Panel);
 		sw->setFrameShadow(QFrame::Plain);
		sw->setMaximumSize(QSize(50, 200));
	}

}

void ColorSelIn::valueUpdate(const QColor& c)
{
	selected = c;
	updateLabel();
	if (autoup)
		commit();
}

void ColorSelIn::residualUpdate() 
{
	if (doModal)
	{
		if (modellessDlg == nullptr)
		{
			modellessDlg = new MyColorPicker((QWidget*)bw->parent(), selected);
			modellessDlg->setWindowTitle(v->displayName());
			connect(modellessDlg, SIGNAL(colorProof(const QColor&)), this, SLOT(valueUpdate(const QColor&)));
			connect(modellessDlg, SIGNAL(accepted()), this, SLOT(accepted()));
			connect(modellessDlg, SIGNAL(rejected()), this, SLOT(rejected()));
			connect(v, SIGNAL(changed()), this, SLOT(updateDlg()));
		}
		else
		{
			modellessDlg->setColor(selected);
		}
		original = selected;
		modellessDlg->show();
	}
	else
	{
		original = selected; // be able to restore it after cancel and proof
		MyColorPicker dlg((QWidget*)bw->parent(), selected);
		dlg.setWindowTitle(v->displayName());
		connect(&dlg, SIGNAL(colorProof(const QColor&)), this, SLOT(valueUpdate(const QColor&)));
		QDialog::DialogCode ret = (QDialog::DialogCode)dlg.exec();
		if (ret == QDialog::Rejected)
		{
			valueUpdate(original);
			return;
		}
		valueUpdate(dlg.getCol());
	}
}

void ColorSelIn::accepted()
{
	valueUpdate(modellessDlg->getCol());
}
void ColorSelIn::rejected()
{
	valueUpdate(original);
}
void ColorSelIn::updateDlg()
{
	if (inCommit)
		return;
	modellessDlg->setColor(selected);
	original = selected;
}

void ColorSelIn::updateLabel()
{
//	QPalette p = sw->palette();
//	p.setColor(QPalette::Window, selected);
//	sw->setPalette(p);

	//QColor textColor = getIdealTextColor(selected);
	if (selected.isValid())
	{
		bw->setIcon(QIcon());
		int grey = 0.299 * selected.red() + 0.587 * selected.green() + 0.114 * selected.blue();

		QColor fore = (grey > 150)?Qt::black:Qt::white;
 
		bw->setStyleSheet(QString("QPushButton { background-color : %1; color: %2; }").arg(selected.name()).arg(fore.name())  );
	}
	else // invalid
	{
		bw->setStyleSheet("");
		bw->setIcon(QIcon(":/images/ex.png"));
	}

}

void MyInputDlg::addValueColor(TypeProp<QColor>* v, const WidgetLine& wl)
{
	QPushButton *c = new QPushButton();
	c->setMaximumSize(QSize(25, 200));

	ColorSelIn *w = new ColorSelIn(v, c, true, WidgetIn::None, m_autoCommit);

	//QHBoxLayout *layout = new QHBoxLayout();
	//wl.layout->addWidget(coll);
	wl.layout->addWidget(c);

	m_widgets.append(w);
}

QPushButton* MyInputDlg::addPushButton(Prop *v, const WidgetLine& wl)
{
	QPushButton *c = new QPushButton(v->displayName());
	NoDataIn *w = new NoDataIn(v, c);
	wl.layout->addWidget(c);
	m_widgets.append(w);
	return c;
}

QPushButton* MyInputDlg::addPushButton(Prop *v, const QString& text, const WidgetLine& wl)
{
	QPushButton *c = new QPushButton(text);
	NoDataIn *w = new NoDataIn(v, c);
	wl.layout->addWidget(c);
	m_widgets.append(w);
	return c;
}


void MyInputDlg::addOpt(TypeProp<bool>* v, const WidgetLine& wl)
{
	QCheckBox *c = new QCheckBox(v->displayName());
	CheckBoxIn *w = new CheckBoxIn(v, c, m_autoCommit);
	wl.layout->addWidget(c);
	m_widgets.append(w);
}


void MyInputDlg::addSeparator()
{
	QFrame *f = new QFrame(this);
	f->setFrameShape(QFrame::HLine);
	f->setFrameShadow(QFrame::Raised);
	ui.optLayout->insertWidget(ui.optLayout->count() - 1, f);
}
 

void ComboBoxIn<StringSelect>::widgetUpdate() 
{
	QString item;
	if (!MyInputDlg::getValue("item name", item, QString(), w))
		return;

	v->hackVal().addItem(item); // this is ok because it's going to be saved as soon as the selection changes.
	v->hackChanged();
	//reload();
}

void ComboBoxIn<StringSelect>::reloadImp()
{
	w->blockSignals(true);
	w->clear();
	foreach(const QString& st, v->val().opts())
		w->addItem(st);
	w->blockSignals(false);

	if (v->val().sel() < v->val().opts().size())
		w->setCurrentIndex(v->val().sel());
	else
		w->setCurrentIndex(0);
}