#ifndef __PARAMBASE_H_INCLUDED__
#define __PARAMBASE_H_INCLUDED__

#include <QString>
#include <QVariant>
#include <QDialog>
#include <QSettings>
#include <QMetaEnum>

class MyInputDlg;
class MySettings;
class WidgetLine;
class ParamBase;

#pragma warning (disable :4250) // inherits via dominance
#pragma warning (disable :4100) // unreferenced formal parameter


class Prop : public QObject
{
	Q_OBJECT;
protected:
	Prop(const QString& mName, const QString& dName)
		:m_memberName(mName), m_displayName(dName), m_changed(false), 
		 m_userCheckChanged(false), m_shouldStore(true), m_addEndSpacer(false), m_index(-1)
	{}

	QString m_memberName;
	QString m_displayName;

	bool m_shouldStore;
	bool m_addEndSpacer;
	int m_index; // if this Prop is in a list, this can be used as the index of this instance in the list

private:
	bool m_changed; // was changed since last store to reg.
	mutable bool m_userCheckChanged; //  was changed since last time the user checked

public:
	int& index() { return m_index; }
	const QString& mname() { return m_memberName; }
	const QString& displayName() { return m_displayName; }
	virtual bool isChanged() const { return m_changed; }
	virtual bool isUserCheckChanged() const { return m_userCheckChanged; }
	virtual void setChanged(bool v) { m_changed = v; }
	virtual void setUserCheckedChanged(bool v) const { m_userCheckChanged = v; }
	void shouldStore(bool v) { m_shouldStore = v; }
	void addSpacer() { m_addEndSpacer = true; }
	virtual QString typname() = 0;

	// for XML serialization
	// XML serialization doesn't work for TypePropN
	virtual QString toString() const { return QString(); };
	// may return false but still has to set to some valid value.
	virtual bool fromString(const QString& s) { return false; };

	virtual void addSelfTo(ParamBase* param, const QString& type);
	virtual void fireChanged();

	virtual void storeIn(MySettings& set) { }
	virtual void loadFrom(MySettings& set) { }
	virtual void addWidget(MyInputDlg* dlg); // default implementation creates a line and calls addInnerWidget()
	virtual void addInnerWidget(WidgetLine& wl) { };
	virtual void reset() {}
	virtual void copyFrom(const Prop* op) {} // default does nothing

	const bool checkChanged() const 
	{ 
		bool ret = isUserCheckChanged();
		setUserCheckedChanged(false);
		return ret; // return the value before
	}

	// try to  treat this Prop as a TypeProp<T> and assign a value to it.
	template<typename T>
	bool tryAssignTypeVal(const T& v);

public slots:
	virtual void externUpdate() {} // for children use.
signals:
	void changed();
	void updateWidget();

	friend class ParamBase;
};

class ParamBase;

enum EWidgetType
{
	WIDGET_EDIT,
	WIDGET_SLIDER
};

template<typename T>
class Initor
{
public:
	Initor(const T& defv) 
		:d(defv), mn(defv), mx(defv), wtype(WIDGET_EDIT) {}
	Initor(const QString& dispName, const T& defv, EWidgetType _wtype = WIDGET_EDIT) 
		:d(defv), mn(defv), mx(defv), disp(dispName), wtype(_wtype) {}
	Initor(const T& defv, const T& minv, const T& maxv) 
		:d(defv), mn(minv), mx(maxv), wtype(WIDGET_SLIDER) {}
	Initor(const QString& dispName, const T& defv, const T& minv, const T& maxv) 
		:d(defv), mn(minv), mx(maxv), wtype(WIDGET_SLIDER), disp(dispName) {}

	operator const T&() { return d; }
	bool hasDispName() const { return !disp.isNull(); }

	const T& d, mn, mx;
	bool hasMnMx;
	QString disp;
	EWidgetType wtype;
};


// need to be specialized so we could initialize it from const char*
template<> class Initor<QString>
{
public:
	Initor(const char* dispName, const char* defv) :d(defv), disp(dispName), wtype(WIDGET_EDIT) {}
	Initor(const char* defv) :d(defv), wtype(WIDGET_EDIT) {}
	Initor(const QString& dispName, const QString& defv) :d(defv), disp(dispName), wtype(WIDGET_EDIT) {}
	Initor(const QString& defv) :d(defv), wtype(WIDGET_EDIT) {}

	operator const QString&() { return d; }
	bool hasDispName() const { return !disp.isNull(); }

	QString d, mn, mx;
	QString disp;
	EWidgetType wtype;
};

template<typename T>
Initor<T> init(const QString& dispName, const T& defv)
{
	return Initor<T>(dispName, defv);
}
inline Initor<QString> init(const QString& dispName, const char* c)
{
	return Initor<QString>(dispName, c);
}
template<typename T>
Initor<T> init(const QString& dispName, const T& defv, const T& minv, const T& maxv)
{
	return Initor<T>(dispName, defv, minv, maxv);
}
template<typename T>
Initor<T> init(const T& defv, const T& minv, const T& maxv)
{
	return Initor<T>(defv, minv, maxv);
}

// those who gather Props
// it's a QObject because of its double use also as the guy with the enums, sometimes.
class IPropGather : public QObject
{
public:
	virtual void addProperty(Prop *p) = 0;
};

// not something you would like to copy around
template<class T>
class TypeProp : public Prop
{
public:
	TypeProp(IPropGather* _container, const QString& memberName, const QString& dispName, Initor<T> iv)
		:Prop(memberName, iv.hasDispName()?iv.disp:dispName), container(_container), 
		 defaultVal(iv.d), minVal(iv.mn), maxVal(iv.mx), 
		 wtype(iv.wtype)
	{
		init(_container);
	}
	TypeProp(IPropGather* _container, QObject* _enumCont, const QString& memberName, const QString& dispName, Initor<T> iv)
		:Prop(memberName, iv.hasDispName()?iv.disp:dispName), container(_enumCont),
		 defaultVal(iv.d), minVal(iv.mn), maxVal(iv.mx), 
	 	 wtype(iv.wtype)
	{
		init(_container);
	}
	TypeProp(IPropGather* _container, const QString& memberName, Initor<T> iv)
		:Prop(memberName, iv.hasDispName()?iv.disp:memberName), container(_container), 
		 defaultVal(iv.d), minVal(iv.mn), maxVal(iv.mx), 
		 wtype(iv.wtype)
	{
		init(_container);
	}
	TypeProp(IPropGather* _container, QObject* _enumCont, const QString& memberName, Initor<T> iv)
		:Prop(memberName, iv.hasDispName()?iv.disp:memberName), container(_enumCont), 
		 defaultVal(iv.d), minVal(iv.mn), maxVal(iv.mx), 
		 wtype(iv.wtype)
	{
		init(_container);
	}

	void init(IPropGather* _container)
	{
		if (_container != NULL)
			_container->addProperty(this);
		else // initialized with NULL, set the default value because you are not going to get called anymore.
			value = defaultVal;
	}

	void copyFrom(const Prop* op)
	{
		const TypeProp<T> *top = static_cast<const TypeProp<T>*>(op); // needs to succeed
		(*this) = top->val();
	}

	//operator T() { return value; }
	operator const T&() const { return value; }
	const T& val() const { return value; }
	// there is no non-const version because all changes must be done using the operator=
	T& hackVal() { return value; } // don't call this unless you know what's going on.
	void hackChanged() // after changing something with hackVal(), call this.
	{
		setChanged(true);
		setUserCheckedChanged(true);
		emit changed();
	}


	template<class TO> // T and TO need to have op= and op==
	TypeProp<T>& operator=(const TO& v) 
	{
		if (value == v) // new value at the right
			return *this;
		value = v;
		hackChanged();
		return *this;
	}

	virtual QString typname()
	{
		return QString(typeid(T).name());
	}

	virtual void storeIn(MySettings& set)
	{
		if (!m_shouldStore)
			return;
		const QString& key = mname();
		if (mySetValue(set, key))
			return;
		set.setValue(key, getVal());
	}
	virtual void loadFrom(MySettings& set)
	{
		if (!m_shouldStore)
		{
			reset(); // instead of loading, reset to default
			return;
		}
		const QString& key = mname();
		if (myGetValue(set, key))
			return;

		setVal(set.value(key, getDefVal()));
	}

	// default implementation do nothing. specific specializations will do something
	bool mySetValue(MySettings& sett, const QString& key);
	bool myGetValue(MySettings& sett, const QString& key);

	virtual void reset()
	{
		*this = defaultVal;
	}

	void setRange(const T& mn, const T& mx)
	{
		Q_ASSERT(wtype == WIDGET_SLIDER);
		minVal = mn; maxVal = mx;
		emit updateWidget();
	}

	T minVal, maxVal;
	QObject* container; // needed for enums QMetaObject

	// ugly cast for the benefit of MyInputDlg widgets.
	int toInt() const { return *(int*)&value; }

	void addInnerWidget(WidgetLine& wl);

	virtual QString toString() const;
	virtual bool fromString(const QString& s)
	{
		bool ret = fromStringImp(s);
		hackChanged();
		return ret;
	}
	bool fromStringImp(const QString& s);

private:
	virtual QVariant getVal() { return /*QVariant::fromValue(*/value/*)*/; }
	virtual QVariant getDefVal() { return /*QVariant::fromValue(*/defaultVal/*)*/; }
	virtual void setVal(const QVariant& v) 
	{ 
		if (v.canConvert<T>())
			value = v.value<T>(); 
		else // user defined enum
		{
			int vi = v.toInt();
			value = *(T*)&vi;
		}
	}
	void addNumericWidget(WidgetLine& wl);

private:
	T value;
	T defaultVal;
	EWidgetType wtype;

};

template<typename T>
QMetaEnum getMetaEnum(QObject *container)
{
	const type_info& ti = typeid(T);
	QString n(ti.name());
	const QMetaObject* mo = container->metaObject();
	int ie = mo->indexOfEnumerator(n.section(':', -1).toLatin1());
	if (ie == -1)
		return QMetaEnum(); // invalid;
	return mo->enumerator(ie);
}

template<typename T>
bool Prop::tryAssignTypeVal(const T& v)
{
	TypeProp<T> *cprop = dynamic_cast<TypeProp<T> *>(this);
	if (cprop != NULL)
	{
		*cprop = v; 
		return true;
	}
	return false;
}


#if 0 // multiple inheritancce not supported
class TypePropN : public Prop, public IPropGather
{
public:
	TypePropN(const QString& memberName, const QString& dispName)
		:Prop(memberName, dispName), keepLabel(false)
	{
	}
	void initConnect()
	{
		foreach(Prop* p, m_props)
			connect(p, SIGNAL(changed()), this, SIGNAL(changed()));
	}

	virtual void storeIn(MySettings& set)
	{
		foreach(Prop* p, m_props)
			if (p->isChanged()) 
				p->storeIn(set);
	}
	virtual void loadFrom(MySettings& set)
	{
		foreach(Prop* p, m_props)
			p->loadFrom(set);
	}
	virtual void addWidget(MyInputDlg* dlg); // overloaded from Prop

	virtual void reset()
	{
		foreach(Prop* p, m_props)
			p->reset();
	}
	virtual bool isChanged() const
	{
		foreach(Prop* p, m_props)
			if (p->isChanged())
				return true;
		return false;
	}
	virtual void fireChanged();
	virtual void setChanged(bool v)
	{
		foreach(Prop* p, m_props)
			p->setChanged(v);
	}
	virtual bool isUserCheckChanged() const
	{
		foreach(Prop* p, m_props)
			if (p->isUserCheckChanged())
				return true;
		return false;
	}
	virtual void setUserCheckedChanged(bool v) const
	{
		foreach(Prop* p, m_props)
			p->setUserCheckedChanged(v);
	}

	virtual QString typname()
	{
		return "PropN";
	}
	virtual void addSelfTo(ParamBase* param, const QString& type);

	virtual void addProperty(Prop* p) 
	{ 
		m_props.append(p); 
	}
	void insertProperty(Prop* p, int index)
	{
		m_props.insert(index, p);
	}

	bool keepLabel; // means this should beep the line label even though there's a check-box.
protected:
	QList<Prop*> m_props;
};

#endif

// for interface completion, let there be 1 as well...
#define DTypeProp1(T1, NAME1) \
class TypeProp1_1##T1##_##NAME1 : public TypePropN \
{ \
public:\
	TypeProp1_1##T1##_##NAME1(ParamBase* _container, const QString& memberName, const QString& dispName, Initor<T1> deft1)\
		:TypePropN(memberName, dispName),\
		 NAME1 (_container?this:NULL, _container, memberName + "_" + #NAME1, dispName, deft1)\
	{\
		initConnect();\
		if (_container != NULL)\
			_container->addProperty(this);\
	}\
	TypeProp1_1##T1##_##NAME1(ParamBase* _container, const QString& memberName, Initor<T1> deft1)\
		:TypePropN(memberName, memberName),\
		 NAME1 (_container?this:NULL, _container, memberName + "_" + #NAME1, deft1)\
	{\
		initConnect();\
		if (_container != NULL)\
			_container->addProperty(this);\
	}\
	TypeProp<T1> NAME1 ;\
}


// this is just crazy. using defines for templates? HA!
#define DTypeProp2(T1, NAME1, T2, NAME2) \
class TypeProp2_1##T1##_##NAME1##_2##T2##_##NAME2 : public TypePropN \
{ \
public:\
	TypeProp2_1##T1##_##NAME1##_2##T2##_##NAME2(ParamBase* _container, const QString& memberName, const QString& dispName, Initor<T1> deft1, Initor<T2> deft2)\
		:TypePropN(memberName, dispName),\
		 NAME1 (_container?this:NULL, _container, memberName + "_" + #NAME1, dispName, deft1),\
		 NAME2 (_container?this:NULL, _container, memberName + "_" + #NAME2, deft2)\
	{\
		initConnect();\
		if (_container != NULL)\
			_container->addProperty(this);\
	}\
	TypeProp2_1##T1##_##NAME1##_2##T2##_##NAME2(ParamBase* _container, const QString& memberName, Initor<T1> deft1, Initor<T2> deft2)\
		:TypePropN(memberName, memberName),\
		 NAME1 (_container?this:NULL, _container, memberName + "_" + #NAME1, deft1),\
		 NAME2 (_container?this:NULL, _container, memberName + "_" + #NAME2, deft2)\
	{\
		initConnect();\
		if (_container != NULL)\
			_container->addProperty(this);\
	}\
	TypeProp<T1> NAME1 ;\
	TypeProp<T2> NAME2 ;\
}


#define DTypeProp3(T1, NAME1, T2, NAME2, T3, NAME3) \
class TypeProp3_1##T1##_##NAME1##_2##T2##_##NAME2##_3##T3##_##NAME3 : public TypePropN\
{\
public:\
	TypeProp3_1##T1##_##NAME1##_2##T2##_##NAME2##_3##T3##_##NAME3(ParamBase* _container, const QString& memberName, const QString& dispName, Initor<T1> deft1, Initor<T2> deft2, Initor<T3> deft3)\
		:TypePropN(memberName, dispName),\
		 NAME1 (_container?this:NULL, _container, memberName + "_" + #NAME1, dispName, deft1),\
		 NAME2 (_container?this:NULL, _container, memberName + "_" + #NAME2, deft2),\
		 NAME3 (_container?this:NULL, _container, memberName + "_" + #NAME3, deft3)\
	{\
		initConnect();\
		if (_container != NULL)\
			_container->addProperty(this);\
	}\
	TypeProp<T1> NAME1 ;\
	TypeProp<T2> NAME2 ;\
	TypeProp<T3> NAME3 ;\
}

#define DTypeProp4(T1, NAME1, T2, NAME2, T3, NAME3, T4, NAME4) \
class TypeProp4_1##T1##_##NAME1##_2##T2##_##NAME2##_3##T3##_##NAME3##_4##T4##_##NAME4 : public TypePropN\
{\
public:\
	TypeProp4_1##T1##_##NAME1##_2##T2##_##NAME2##_3##T3##_##NAME3##_4##T4##_##NAME4(ParamBase* _container, const QString& memberName, const QString& dispName, \
		Initor<T1> deft1, Initor<T2> deft2, Initor<T3> deft3, Initor<T4> deft4)\
		:TypePropN(memberName, dispName),\
		 NAME1 (_container?this:NULL, _container, memberName + "_" + #NAME1, dispName, deft1),\
		 NAME2 (_container?this:NULL, _container, memberName + "_" + #NAME2, deft2),\
		 NAME3 (_container?this:NULL, _container, memberName + "_" + #NAME3, deft3),\
		 NAME4 (_container?this:NULL, _container, memberName + "_" + #NAME4, deft4)\
	{\
		initConnect();\
		if (_container != NULL)\
			_container->addProperty(this);\
	}\
	TypeProp<T1> NAME1 ;\
	TypeProp<T2> NAME2 ;\
	TypeProp<T3> NAME3 ;\
	TypeProp<T4> NAME4 ;\
}

#define DTypeProp5(T1, NAME1, T2, NAME2, T3, NAME3, T4, NAME4, T5, NAME5) \
class TypeProp5_1##T1##_##NAME1##_2##T2##_##NAME2##_3##T3##_##NAME3##_4##T4##_##NAME4##_5##T5##_##NAME5 : public TypePropN\
{\
public:\
	TypeProp5_1##T1##_##NAME1##_2##T2##_##NAME2##_3##T3##_##NAME3##_4##T4##_##NAME4##_5##T5##_##NAME5(ParamBase* _container, const QString& memberName, const QString& dispName, \
		Initor<T1> deft1, Initor<T2> deft2, Initor<T3> deft3, Initor<T4> deft4, Initor<T5> deft5)\
		:TypePropN(memberName, dispName),\
		 NAME1 (_container?this:NULL, _container, memberName + "_" + #NAME1, dispName, deft1),\
		 NAME2 (_container?this:NULL, _container, memberName + "_" + #NAME2, deft2),\
		 NAME3 (_container?this:NULL, _container, memberName + "_" + #NAME3, deft3),\
		 NAME4 (_container?this:NULL, _container, memberName + "_" + #NAME4, deft4),\
		 NAME5 (_container?this:NULL, _container, memberName + "_" + #NAME5, deft5)\
	{\
		initConnect();\
		if (_container != NULL)\
			_container->addProperty(this);\
	}\
	TypeProp<T1> NAME1 ;\
	TypeProp<T2> NAME2 ;\
	TypeProp<T3> NAME3 ;\
	TypeProp<T4> NAME4 ;\
	TypeProp<T5> NAME5 ;\
}

#define DTypeProp6(T1, NAME1, T2, NAME2, T3, NAME3, T4, NAME4, T5, NAME5, T6, NAME6) \
class TypeProp6_1##T1##_##NAME1##_2##T2##_##NAME2##_3##T3##_##NAME3##_4##T4##_##NAME4##_5##T5##_##NAME5##_6##T6##_##NAME6 : public TypePropN\
{\
public:\
	TypeProp6_1##T1##_##NAME1##_2##T2##_##NAME2##_3##T3##_##NAME3##_4##T4##_##NAME4##_5##T5##_##NAME5##_6##T6##_##NAME6(ParamBase* _container, const QString& memberName, const QString& dispName, \
		Initor<T1> deft1, Initor<T2> deft2, Initor<T3> deft3, Initor<T4> deft4, Initor<T5> deft5, Initor<T6> deft6)\
		:TypePropN(memberName, dispName),\
		 NAME1 (_container?this:NULL, _container, memberName + "_" + #NAME1, dispName, deft1),\
		 NAME2 (_container?this:NULL, _container, memberName + "_" + #NAME2, deft2),\
		 NAME3 (_container?this:NULL, _container, memberName + "_" + #NAME3, deft3),\
		 NAME4 (_container?this:NULL, _container, memberName + "_" + #NAME4, deft4),\
		 NAME5 (_container?this:NULL, _container, memberName + "_" + #NAME5, deft5),\
		 NAME6 (_container?this:NULL, _container, memberName + "_" + #NAME5, deft6)\
	{\
		initConnect();\
		if (_container != NULL)\
			_container->addProperty(this);\
	}\
	TypeProp<T1> NAME1 ;\
	TypeProp<T2> NAME2 ;\
	TypeProp<T3> NAME3 ;\
	TypeProp<T4> NAME4 ;\
	TypeProp<T5> NAME5 ;\
	TypeProp<T6> NAME6 ;\
}


#define DTypeProp7(T1, NAME1, T2, NAME2, T3, NAME3, T4, NAME4, T5, NAME5, T6, NAME6, T7, NAME7) \
class TypeProp7_1##T1##_##NAME1##_2##T2##_##NAME2##_3##T3##_##NAME3##_4##T4##_##NAME4##_5##T5##_##NAME5##_6##T6##_##NAME6##_7##T7##_##NAME7 : public TypePropN\
{\
public:\
	TypeProp7_1##T1##_##NAME1##_2##T2##_##NAME2##_3##T3##_##NAME3##_4##T4##_##NAME4##_5##T5##_##NAME5##_6##T6##_##NAME6##_7##T7##_##NAME7(ParamBase* _container, const QString& memberName, const QString& dispName, \
		Initor<T1> deft1, Initor<T2> deft2, Initor<T3> deft3, Initor<T4> deft4, Initor<T5> deft5, Initor<T6> deft6, Initor<T7> deft7)\
		:TypePropN(memberName, dispName),\
		 NAME1 (_container?this:NULL, _container, memberName + "_" + #NAME1, dispName, deft1),\
		 NAME2 (_container?this:NULL, _container, memberName + "_" + #NAME2, deft2),\
		 NAME3 (_container?this:NULL, _container, memberName + "_" + #NAME3, deft3),\
		 NAME4 (_container?this:NULL, _container, memberName + "_" + #NAME4, deft4),\
		 NAME5 (_container?this:NULL, _container, memberName + "_" + #NAME5, deft5),\
		 NAME6 (_container?this:NULL, _container, memberName + "_" + #NAME6, deft6),\
		 NAME7 (_container?this:NULL, _container, memberName + "_" + #NAME7, deft7)\
	{\
		initConnect();\
		if (_container != NULL)\
			_container->addProperty(this);\
	}\
	TypeProp<T1> NAME1 ;\
	TypeProp<T2> NAME2 ;\
	TypeProp<T3> NAME3 ;\
	TypeProp<T4> NAME4 ;\
	TypeProp<T5> NAME5 ;\
	TypeProp<T6> NAME6 ;\
	TypeProp<T7> NAME7 ;\
}

#define DTypeProp8(T1, NAME1, T2, NAME2, T3, NAME3, T4, NAME4, T5, NAME5, T6, NAME6, T7, NAME7, T8, NAME8) \
class TypeProp8_1##T1##_##NAME1##_2##T2##_##NAME2##_3##T3##_##NAME3##_4##T4##_##NAME4##_5##T5##_##NAME5##_6##T6##_##NAME6##_7##T7##_##NAME7##_8##T8##_##NAME8 : public TypePropN\
{\
public:\
	TypeProp8_1##T1##_##NAME1##_2##T2##_##NAME2##_3##T3##_##NAME3##_4##T4##_##NAME4##_5##T5##_##NAME5##_6##T6##_##NAME6##_7##T7##_##NAME7##_8##T8##_##NAME8(ParamBase* _container, const QString& memberName, const QString& dispName, \
		Initor<T1> deft1, Initor<T2> deft2, Initor<T3> deft3, Initor<T4> deft4, Initor<T5> deft5, Initor<T6> deft6, Initor<T7> deft7, Initor<T8> deft8)\
		:TypePropN(memberName, dispName),\
		 NAME1 (_container?this:NULL, _container, memberName + "_" + #NAME1, dispName, deft1),\
		 NAME2 (_container?this:NULL, _container, memberName + "_" + #NAME2, deft2),\
		 NAME3 (_container?this:NULL, _container, memberName + "_" + #NAME3, deft3),\
		 NAME4 (_container?this:NULL, _container, memberName + "_" + #NAME4, deft4),\
		 NAME5 (_container?this:NULL, _container, memberName + "_" + #NAME5, deft5),\
		 NAME6 (_container?this:NULL, _container, memberName + "_" + #NAME6, deft6),\
		 NAME7 (_container?this:NULL, _container, memberName + "_" + #NAME7, deft7),\
		 NAME8 (_container?this:NULL, _container, memberName + "_" + #NAME8, deft8)\
	{\
		initConnect();\
		if (_container != NULL)\
			_container->addProperty(this);\
	}\
	TypeProp<T1> NAME1 ;\
	TypeProp<T2> NAME2 ;\
	TypeProp<T3> NAME3 ;\
	TypeProp<T4> NAME4 ;\
	TypeProp<T5> NAME5 ;\
	TypeProp<T6> NAME6 ;\
	TypeProp<T7> NAME7 ;\
	TypeProp<T8> NAME8 ;\
}


class SeparatorProp : public Prop
{
public:
	SeparatorProp() : Prop("_sep","_sep") {}
	virtual void addWidget(MyInputDlg* dlg);
};


class ParamBase : public IPropGather
{
	Q_OBJECT
public:
	virtual ~ParamBase() {}

	virtual void addProperty(Prop *p)
	{
		m_props.append(p);
		m_byname.insert(p->m_memberName, p);
		connect(p, SIGNAL(changed()), this, SLOT(paramChanged()));
	}
	Prop* propByName(const QString& name) // by member name mname()
	{
		TByNameMap::iterator it = m_byname.find(name);
		if (it == m_byname.end())
			return NULL;
		return *it;
	}
	const Prop* propByName(const QString& name) const
	{
		return const_cast<ParamBase*>(this)->propByName(name);
	}

	void copyFrom(const ParamBase *other);

	bool execDialog(QWidget *parent, const QString caption);

	virtual QString getGroupName() { return m_groupName; }
		
	void reset();
	MyInputDlg *createDialog(QWidget* parent, bool modal = false, bool hasApply = false);

	virtual void storeToReg(MySettings &set, bool force = false);
	virtual void loadFromReg(MySettings &set, bool setUnChange = true);

	// if anything changed, fire the changed signals (in case they are not connected automatically)
	void fireChanged();

	void addPropWatcher(const QString& propTypeName, ParamBase* watcher);

public slots:
	virtual void paramChanged()
	{
		emit changed();
	}
	virtual void helperSlot() {}

signals:
	void changed();

protected:
	QString m_groupName;

	QList<Prop*> m_props;
	typedef QMap<QString, Prop*> TByNameMap;
	TByNameMap m_byname;

	friend class MySettings;
};

// used as a temporary parent to parameters on the stack.
class ParamTemp : public ParamBase
{
public:
	ParamTemp(const QString& title)
	{
		m_groupName = title;
	}

};

// the type for a TypeProp which is a QComboBox with a few dynamic options
class StringSelect : public QObject
{
	Q_OBJECT;
public:
	StringSelect() : m_sel(-1), m_addBot(true) {}
	StringSelect(const StringSelect& b)  // for the sake of QObject
		:m_sel(b.m_sel), m_opts(b.m_opts), m_addBot(true)
	{}

	bool operator==(const int s) const 
	{
		return (m_sel == s);
	}
	bool operator==(const StringSelect& b) const 
	{
		return (m_sel == b.m_sel) && (m_opts == b.m_opts);
	}

	void operator=(const int s)
	{
		if ((s == m_sel) || (s >= m_opts.size()))
			return;
		m_sel = s;
		if (s != -1)
			emit changedTo(m_opts[m_sel]);
	}
	void operator=(const StringSelect& b)
	{
		throw "no";
	}

	void addItem(const QString& item);

	const QStringList& opts() const { return m_opts; }
	void initOpts(const QStringList& list) { m_opts = list; }
	void initSel(int sel) { m_sel = sel; }

	int sel() const { return m_sel; }

	mutable bool m_addBot;

private:
	int m_sel;
	QStringList m_opts;

signals:
	void addedItem(const QString& item);
	void changedTo(const QString& item);
};

Q_DECLARE_METATYPE(StringSelect);


class MySettings : public QSettings
{
	Q_OBJECT;
public:
	MySettings(const QString& group, const QString& name);

	void setOffline(bool off) { m_offline = off; }
	bool isOffline() const { return m_offline; }

	void addParam(ParamBase *pr, const QString& groupName)
	{
		pr->m_groupName = groupName;
		connect(pr, SIGNAL(changed()), this, SIGNAL(changed()));
		m_params.append(pr);
	}

public slots:
	// returns true if we're ok to proceed.
	void storeToReg()
	{
		foreach(ParamBase *pr, m_params)
			pr->storeToReg(*this);
	}
	void loadFromReg(bool setUnChange = true)
	{
		foreach(ParamBase *pr, m_params)
			pr->loadFromReg(*this, setUnChange);
	}
	void reset()
	{
		foreach(ParamBase *pr, m_params)
			pr->reset();
	}

signals:
	void changed();

private:
	bool m_offline;
	QList<ParamBase*> m_params;

};

class PushButtomProp : public Prop
{
public:
	PushButtomProp(IPropGather* _container, const QString text) : Prop("_push", text), m_but(NULL) 
	{
		if (_container != NULL)
			_container->addProperty(this);
	}
	virtual void addInnerWidget(WidgetLine& wl);

	virtual QString typname() { return "PushButton"; }

	QPushButton *m_but;
};



// specialization of just these problematic method from TypeProp.
// use the serialization of StringSelect
// although we're not using the QVariant serialization, so this is just here to prevent a compile error.
template<> QVariant TypeProp<StringSelect>::getVal() { return QVariant::fromValue(value); }
template<> QVariant TypeProp<StringSelect>::getDefVal() { return QVariant::fromValue(defaultVal); }
template<> QString TypeProp<StringSelect>::typname() { return "StringSelect"; }

// used to display some result in the dialog
template<class T>
class ResultProp : public Prop
{
public:
	ResultProp(ParamBase* _container, const QString text) 
		: Prop("_result", text),
		m_text(NULL, "_result", text, T())
	{
		if (_container != NULL)
			_container->addProperty(this);
	}

	virtual void addInnerWidget(WidgetLine& wl)
	{
		wl.dlg->addValueLabel(&m_text, wl);
	}
	virtual QString typname() { return "res"; }

protected:
	TypeProp<T> m_text;

};


#endif // __PARAMBASE_H_INCLUDED__
