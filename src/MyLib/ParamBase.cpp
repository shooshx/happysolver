#include "ParamBase.h"

#include "MyInputDlg.h"
#include "paramSpecialization.h"

#include <cfloat>
#include "../Vec.h"
// no where else to put it
const Vec3 MAX_VEC(FLT_MAX, FLT_MAX, FLT_MAX), MIN_VEC(-FLT_MAX, -FLT_MAX, -FLT_MAX);
const Vec3 INVALID_COLOR = MAX_VEC;


void ParamBase::reset()
{
	foreach(Prop* p, m_props)
	{
		p->reset();
	}
}

void ParamBase::fireChanged()
{
	foreach(Prop* p, m_props)
	{
		p->fireChanged();
	}
}

void Prop::fireChanged()
{
	if (isChanged()) 
		emit changed();
}

#if 0
void TypePropN::fireChanged()
{
	foreach(Prop* p, m_props)
		p->fireChanged();
}
void TypePropN::addWidget(MyInputDlg* dlg)
{
	WidgetLine wl = dlg->startLine(this);
	wl.keepLabel = keepLabel;
	foreach(Prop* p, m_props)
		p->addInnerWidget(wl);
	dlg->endLine(wl);
}
void TypePropN::addSelfTo(ParamBase* param, const QString& type)
{
	foreach(Prop* p, m_props)
		p->addSelfTo(param, type);

}

#endif

void ParamBase::copyFrom(const ParamBase *other)
{
	foreach(Prop* p, m_props)
	{
		const Prop *op = other->propByName(p->mname());
		p->copyFrom(op);
	}
}


bool ParamBase::execDialog(QWidget *parent, const QString caption)
{
	MyInputDlg *dlg = createDialog(parent, true);
	dlg->setCaptions(caption);
	dlg->setApplyButton(false);

	//reset(); don't reset it to the default since it probably has some more recent value
	QDialog::DialogCode ret = (QDialog::DialogCode)dlg->exec();
	delete dlg;
	if (ret == QDialog::Rejected)
		return false;
	return true;
}



MyInputDlg* ParamBase::createDialog(QWidget *parent, bool modal, bool hasApply)
{
	MyInputDlg *dlg = new MyInputDlg(parent);
	dlg->setWindowTitle(getGroupName());
	dlg->setApplyButton(hasApply);
	dlg->setModal(modal);

	foreach(Prop* p, m_props)
	{
		p->addWidget(dlg);
	}
	dlg->reload();

	return dlg;
}


void ParamBase::storeToReg(MySettings &set, bool force)
{
	if (set.isOffline())
		return;
	set.beginGroup(getGroupName());
	foreach(Prop *p, m_props)
	{
		if (force || p->isChanged())
		{
			p->storeIn(set);
			p->setChanged(false); // don't save it next times.
		}
	}
	set.endGroup();
}

void ParamBase::loadFromReg(MySettings &set, bool setUnChange)
{
	set.beginGroup(getGroupName());
	foreach(Prop *p, m_props)
	{
		p->loadFrom(set);
		if (setUnChange)
		{
			p->setChanged(false);
			p->setUserCheckedChanged(false);
		}
	}
	set.endGroup();
}

void Prop::addWidget(MyInputDlg* dlg)
{
	WidgetLine wl = dlg->startLine(this);
	addInnerWidget(wl);
	if (m_addEndSpacer)
		wl.addSpacer();
	dlg->endLine(wl);
}





template<> void TypeProp<QColor>::addInnerWidget(WidgetLine& wl)
{
	wl.dlg->addValueColor(this, wl);
}


template<> void TypeProp<QStringList>::addInnerWidget(WidgetLine& wl)
{ // there is no widget yet for a string list.
}

template<> void TypeProp<bool>::addInnerWidget(WidgetLine& wl)
{
	if (wl.isFirst())
	{
		wl.removeLabel();
	}

	wl.dlg->addOpt(this, wl);
}


template<> void TypeProp<StringSelect>::addInnerWidget(WidgetLine& wl)
{
	ComboBoxIn<StringSelect> *c = wl.dlg->addComboBox(this, wl);
	c->reload();
	if (value.m_addBot)
	{
		QPushButton *addbot = wl.dlg->addPushButton(this, "+", wl);
		addbot->setMaximumWidth(20);
		connect(addbot, SIGNAL(clicked()), c, SLOT(widgetUpdate()));
	}
}

template<class T>
void TypeProp<T>::addNumericWidget(WidgetLine& wl)
{
	if (wtype == WIDGET_EDIT)
		wl.dlg->addValue(this, wl);
	else if (wtype == WIDGET_SLIDER)
		wl.dlg->addValueSlider(this, wl);
}

template<> void TypeProp<int>::addInnerWidget(WidgetLine& wl) { addNumericWidget(wl); }
template<> void TypeProp<uint>::addInnerWidget(WidgetLine& wl) { addNumericWidget(wl); }
template<> void TypeProp<float>::addInnerWidget(WidgetLine& wl) { addNumericWidget(wl); }


void SeparatorProp::addWidget(MyInputDlg* dlg)
{
	dlg->addSeparator(); // doesn't work because it doesn't have a WidgetIn which calls it.
}
 
void PushButtomProp::addInnerWidget(WidgetLine& wl)
{
	m_but = wl.dlg->addPushButton(this, wl);
}


void ParamBase::addPropWatcher(const QString& propTypeName, ParamBase* watcher)
{
	foreach(Prop* p, m_props)
	{
		p->addSelfTo(watcher, propTypeName);
	}

}

void Prop::addSelfTo(ParamBase* param, const QString& type)
{
	if (typname() == type || (type.isEmpty() && type != "StringSelect"))
		param->addProperty(this);
}



MySettings::MySettings(const QString& group, const QString& name)
	:QSettings(group, name), m_offline(false)
{
}


template<>
bool TypeProp<StringSelect>::mySetValue(MySettings& sett, const QString& key) 
{
	sett.setValue(key, (value.sel() >=0)?value.opts()[value.sel()]:QString());
	return true;
}

template<>
bool TypeProp<StringSelect>::myGetValue(MySettings& sett, const QString& key) 
{ // the code should be taking care of populating the options because calling load
	QString sel = sett.value(key, QString()).toString();
	value.initSel(value.opts().indexOf(sel));
	return true;
}


// my own serialization to stuff

template<>
bool TypeProp<bool>::mySetValue(MySettings& sett, const QString& key) 
{ 
	sett.setValue(key, (int)value);
	return true; 
}

template<>
bool TypeProp<bool>::myGetValue(MySettings& sett, const QString& key) 
{ 
	value = sett.value(key, (int)defaultVal).toInt();
	return true; 
}



template<> 
bool TypeProp<QColor>::mySetValue(MySettings& sett, const QString& key) 
{ 
	sett.setValue(key, toString());
	return true; 
}
template<> 
bool TypeProp<QColor>::myGetValue(MySettings& sett, const QString& key) 
{ 
	QString s = sett.value(key, QString()).toString();
	fromStringImp(s);
	return true;
}



void StringSelect::addItem(const QString& item)
{
	m_sel = m_opts.indexOf(item);
	if (m_sel == -1)
	{
		m_opts.append(item);
		m_sel = m_opts.size() - 1; // select the one just added.
	}
	emit addedItem(item);
}


template<> QString TypeProp<QString>::toString() const
{
	return value;
}
template<> bool TypeProp<QString>::fromStringImp(const QString& s)
{
	value = s;
	return true;
}

template<> QString TypeProp<QColor>::toString() const
{
	if (value.isValid())
		return QString("%1 %2 %3 %4").arg(value.red()).arg(value.green()).arg(value.blue()).arg(value.alpha());
	else
		return "X";
}
template<> bool TypeProp<QColor>::fromStringImp(const QString& s)
{
	if (!s.isEmpty())
	{
		if (s == "X")
		{ // invalid color
			value = QColor();
			return true;
		}
		QStringList sl = s.split(' ', QString::SkipEmptyParts);
		if (sl.size() >= 3)
		{
			QColor c(sl[0].toInt(), sl[1].toInt(), sl[2].toInt());
			if (sl.size() >= 4)
				c.setAlpha(sl[3].toInt());
			value = c;
			return true;
		}
	}
	value = defaultVal;
	return false;
}


template<> bool TypeProp<float>::fromStringImp(const QString& s)
{
	bool ok = false;
	value = s.toFloat(&ok);
	return ok;
}

template<> QString TypeProp<StringSelect>::toString() const { return QString(); }
template<> bool TypeProp<StringSelect>::fromStringImp(const QString& s) { return false; }
template<> QString TypeProp<QStringList>::toString() const { return QString(); }
template<> bool TypeProp<QStringList>::fromStringImp(const QString& s) { return false; }


// complete MySettings defs

void MySettings::addParam(ParamBase *pr, const QString& groupName)
{
    pr->m_groupName = groupName;
    connect(pr, SIGNAL(changed()), this, SIGNAL(changed()));
    m_params.append(pr);
}

void MySettings::storeToReg()
{
    foreach(ParamBase *pr, m_params)
    pr->storeToReg(*this);
}
void MySettings::loadFromReg(bool setUnChange)
{
    foreach(ParamBase *pr, m_params)
    pr->loadFromReg(*this, setUnChange);
}
void MySettings::reset()
{
    foreach(ParamBase *pr, m_params)
    pr->reset();
}


