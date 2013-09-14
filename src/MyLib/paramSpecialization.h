#include <QMetaEnum>

#include "ParamBase.h"
#include "MyInputDlg.h"

// specialization
template<> void TypeProp<QColor>::addInnerWidget(WidgetLine& wl);
template<> void TypeProp<QStringList>::addInnerWidget(WidgetLine& wl);
template<> void TypeProp<bool>::addInnerWidget(WidgetLine& wl);
template<> void TypeProp<StringSelect>::addInnerWidget(WidgetLine& wl);
template<> void TypeProp<int>::addInnerWidget(WidgetLine& wl);
template<> void TypeProp<uint>::addInnerWidget(WidgetLine& wl);
template<> void TypeProp<float>::addInnerWidget(WidgetLine& wl);

template void TypeProp<QString>::addInnerWidget(WidgetLine& wl);


template<class T>
void TypeProp<T>::addInnerWidget(WidgetLine& wl) 
{
	if (container != NULL)
	{
		const QMetaObject* mo = container->metaObject();
		const std::type_info& ti = typeid(T);
		QString n(ti.name());

		if (n.startsWith("enum") && (mo->enumeratorCount() != 0))
		{
			int ie = mo->indexOfEnumerator(n.section(':', -1).toLatin1());
			if (ie == -1)
				return;
			QMetaEnum me = mo->enumerator(ie);
			
			wl.dlg->addComboBox(this, wl);
			return;

		}
	}
}


template<> bool TypeProp<bool>::mySetValue(MySettings& sett, const QString& key);
template<> bool TypeProp<bool>::myGetValue(MySettings& sett, const QString& key);
template<> bool TypeProp<QColor>::mySetValue(MySettings& sett, const QString& key);
template<> bool TypeProp<QColor>::myGetValue(MySettings& sett, const QString& key);
template<> bool TypeProp<StringSelect>::mySetValue(MySettings& sett, const QString& key);
template<> bool TypeProp<StringSelect>::myGetValue(MySettings& sett, const QString& key);

template bool TypeProp<QString>::mySetValue(MySettings& sett, const QString& key);
template bool TypeProp<QString>::myGetValue(MySettings& sett, const QString& key);

template<class T>
bool TypeProp<T>::mySetValue(MySettings& sett, const QString& key)
{
	return false;
}

template<class T>
bool TypeProp<T>::myGetValue(MySettings& sett, const QString& key)
{
	return false;
}


template<> QString TypeProp<QColor>::toString() const;
template<> bool TypeProp<QColor>::fromStringImp(const QString& s);
template<> QString TypeProp<QString>::toString() const;
template<> bool TypeProp<QString>::fromStringImp(const QString& s);
template<> bool TypeProp<float>::fromStringImp(const QString& s);

template<> QString TypeProp<StringSelect>::toString() const;
template<> bool TypeProp<StringSelect>::fromStringImp(const QString& s);
template<> QString TypeProp<QStringList>::toString() const;
template<> bool TypeProp<QStringList>::fromStringImp(const QString& s);


template<typename T> // for anything int-derived
QString TypeProp<T>::toString() const
{
	QMetaEnum me = getMetaEnum<T>(container);
	if (me.isValid())
	{
		return QString(me.key(value)); // string name
	}
	return QString("%1").arg(value);
}

template<typename T> // for anything int-derived (enums)
bool TypeProp<T>::fromStringImp(const QString& s)
{
	// parse as enum string
	QMetaEnum me = getMetaEnum<T>(container);
	if (me.isValid())
	{
		QString us = s.toLower().trimmed();
		int index = 0;
		while (index < me.keyCount() && us != QString(me.key(index)).toLower())
			++index;
		if (index < me.keyCount())
		{
			int v = me.value(index);
			value = *(T*)&v;
			return true;
		}
	}

	// parse as int
	bool ok = false;
	int i = s.toInt(&ok);
	value = *(T*)&i; // old ugly friend...
	return ok;
}
