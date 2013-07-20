#ifndef MYCHECKDLG_H
#define MYCHECKDLG_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QMetaEnum>
#include "ui_MyInputDlg.h"

#include "ParamBase.h"
#include "MyDialog.h"

class QRadioButton;


QVariant fromString(const QString& v, const type_info& ti);

inline QString toString(const float& v) 
{ 
	if (qAbs(v) > 0.1)
		return QString().sprintf("%1.3f", v); 
	else
		return QString().sprintf("%f", v);
}
inline QString toString(const QString& v) { return v; }
inline QString toString(const int& v) { return QString().sprintf("%d", v); }
inline QString toString(const uint& v) { return QString().sprintf("%d", v); }
inline QString toString(const QStringList& v) { return v.join(" "); }

class WidgetIn : public QObject
{
	Q_OBJECT
public:
	WidgetIn(Prop* v, QWidget* w) :pv(v), pw(w), inCommit(false)
	{ // TBD - this causes reload() right after every commit() because commit() changes the value.
		connect(pv, SIGNAL(changed()), this, SLOT(reload()));
		if (!v->displayName().isEmpty())
			w->setToolTip(v->displayName() + " (" + v->mname() + ")");
		else
			w->setToolTip(v->mname());
	}
	virtual ~WidgetIn() 
	{
		delete pw; // normally never called because Ins are created and not desctoyed.
	}

	virtual void commitImp() = 0;
	virtual void reloadImp() = 0;

	bool isInCommit() const { return inCommit; }

	enum EFlags { None, DoModal }; // for ColorSelIn (DoModal is infact DoModalLess)
signals:
	void commitedChange();

public slots:
	void commit()
	{
		inCommit = true;
		commitImp();
		inCommit = false;
	}
	void reload()
	{
		if (!inCommit)
			reloadImp();
	}

	// this needs to be here because the SliderIn is a template and moc doesn't support it.
	// none of these are connected by default.
	virtual void residualUpdate() {} // an update that happens after a commit
	virtual void widgetUpdate() {} // update the apperance of the widget from outside


private:
	Prop *pv;
	QWidget *pw;
protected:
	bool inCommit;
};

class NoDataIn : public WidgetIn
{
public:
	NoDataIn(Prop* v, QWidget* w) :WidgetIn(v, w) {}
	virtual void commitImp() {}
	virtual void reloadImp() {}
};

class CheckBoxIn : public WidgetIn
{
public:
	CheckBoxIn(TypeProp<bool>* _v, QAbstractButton *_w, bool autoup = true) 
		:WidgetIn(_v, _w), v(_v), w(_w) 
	{
		w->setCheckable(true);
		if (autoup)
		{
			connect(w, SIGNAL(toggled(bool)), this, SLOT(commit()));
		}
	}

	virtual void commitImp()
	{
		*v = w->isChecked();
		emit commitedChange();
	}
	virtual void reloadImp()
	{
		w->setChecked(*v);
	}
private:
	QAbstractButton *w;
	TypeProp<bool> *v;
};


template<class T, class TW>
struct TextIn : public WidgetIn
{
public:
	TextIn(TypeProp<T>* _v, TW *_w, bool autoup = true) 
		:WidgetIn(_v, _w), v(_v), w(_w)
	{
		if (autoup)
			connect(w, SIGNAL(textEdited(const QString&)), this, SLOT(commit()));
	}
	virtual void commitImp()
	{
		*v = qvariant_cast<T>(fromString(w->text(), typeid(T)));
	}
	virtual void reloadImp()
	{
		w->setText(toString(*v));
	}

private:
	TW *w;
	TypeProp<T> *v;
};

// Hack because can't template typedefs
template<class T>
class WidgetTIn
{
public:
	typedef TextIn<T, QLineEdit> LineEditIn;
	typedef TextIn<T, QLabel> LabelIn; // don't pass autoup true
private:
	WidgetTIn();
};


template<class T>
struct SliderIn : public WidgetIn
{
public:
	SliderIn(TypeProp<T>* _v, QSlider *_w, QLabel* _numl, T _mult = 1, bool autoup = true) 
		:WidgetIn(_v, _w), v(_v), w(_w), numl(_numl), mult(_mult)
	{
		widgetUpdate();
		if (autoup)
		{
			//connect(w, SIGNAL(sliderReleased()), this, SLOT(commit())); 
			connect(w, SIGNAL(valueChanged(int)), this, SLOT(commit()));
		}
		connect(w, SIGNAL(valueChanged(int)), this, SLOT(residualUpdate()));
		connect(v, SIGNAL(updateWidget()), this, SLOT(widgetUpdate()));
	}
	virtual void commitImp()
	{
		*v = (T)w->value() / mult;
	}
	virtual void reloadImp()
	{
		w->setValue(v->val() * mult);
		residualUpdate();
	}

	virtual void residualUpdate() 
	{
		if (numl != NULL)
			numl->setText(toString(w->value() / mult));
	}
	virtual void widgetUpdate()
	{
		w->setRange(v->minVal * mult, v->maxVal * mult);
	}

private:
	QSlider *w;
	TypeProp<T> *v;
	QLabel *numl;
	T mult;
};

template<class T>
struct SpinBoxIn : public WidgetIn
{
public:
	SpinBoxIn(TypeProp<T>* _v, QSpinBox *_w, bool autoup = true) 
		:WidgetIn(_v, _w), v(_v), w(_w)
	{
		widgetUpdate();
		if (autoup)
		{
			connect(w, SIGNAL(valueChanged(int)), this, SLOT(commit()));
		}
		connect(v, SIGNAL(updateWidget()), this, SLOT(widgetUpdate()));
	}
	virtual void commitImp()
	{
		*v = (T)w->value();
	}
	virtual void reloadImp()
	{
		w->setValue(v->val());
	}

	virtual void widgetUpdate()
	{
		w->setRange(v->minVal, v->maxVal);
	}

private:
	QSpinBox *w;
	TypeProp<T> *v;
};



template<class T>
struct ComboBoxIn : public WidgetIn
{
public:
	ComboBoxIn(TypeProp<T>* _v, QComboBox *_w, bool autoup = true, QObject* altContainer = NULL) 
		:WidgetIn(_v, _w), v(_v), w(_w)
	{
		me = getMetaEnum<T>(altContainer?altContainer:v->container); 

		const char *vname;
		int index = 0;
		while ((vname = me.key(index++)) != 0)
			w->addItem(vname);

		w->setEditable(false);
		if (autoup)
			connect(w, SIGNAL(currentIndexChanged(int)), this, SLOT(commit()));
	}
	virtual void commitImp()
	{
		*v = (T)me.value(w->currentIndex());
	}
	virtual void reloadImp()
	{
		int index = 0;
		while (((T)me.value(index) != *v) && (index < me.keyCount()))
			++index;
		if (index != me.keyCount())
			w->setCurrentIndex(index);
		else
			w->setCurrentIndex(0);
	}
private:
	QComboBox *w;
	QMetaEnum me;
	TypeProp<T> *v;
};

// specialization
template<>
struct ComboBoxIn<StringSelect> : public WidgetIn
{
public:
	ComboBoxIn(TypeProp<StringSelect>* _v, QComboBox *_w, bool autoup = true) 
		:WidgetIn(_v, _w), v(_v), w(_w)
	{
		w->setEditable(false);
		if (autoup)
			connect(w, SIGNAL(currentIndexChanged(int)), this, SLOT(commit()));
	}
	virtual void commitImp()
	{
		*v = w->currentIndex();
	}
	virtual void reloadImp();

	virtual void widgetUpdate(); // defined in the cpp
	
	QComboBox* widget() { return w; }
private:
	QComboBox *w;
	TypeProp<StringSelect> *v;
};

class MyColorPicker;

struct ColorSelIn : public WidgetIn
{
	Q_OBJECT
public:
	ColorSelIn(TypeProp<QColor> *_v, QAbstractButton *_bw, bool withAlpha = false, EFlags flags = None, bool _autoup = true);

	virtual void commitImp()
	{
		*v = selected;
	}
	virtual void reloadImp()
	{
		selected = *v;
		updateLabel();
	}

	virtual void residualUpdate();

	void updateLabel();
private slots:
	void valueUpdate(const QColor& c); // for proof color online change
	void accepted();
	void rejected();
	void updateDlg();

private:
	QColor selected;
	QAbstractButton *bw;
	QLabel *sw;
	TypeProp<QColor> *v;
	bool autoup;

	MyColorPicker *modellessDlg;
	QColor original;
	bool doModal;
	bool withAlpha;

};


class WidgetLine
{
public:
	void removeLabel()
	{	
		if (keepLabel)
			return;
		layout->removeItem(layout->itemAt(0));
		hasLabel = false;
	}
	bool isFirst()
	{
		return ((layout->count() == 1) && hasLabel); // only the label is there
	}
	void addSpacer()
	{
		layout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
	}

	MyInputDlg *dlg;
	bool keepLabel;
private:
	WidgetLine(QBoxLayout* _layout, MyInputDlg* _dlg) 
		: layout(_layout), dlg(_dlg), hasLabel(true), keepLabel(false) {}

	QBoxLayout *layout;
	friend class MyInputDlg;
	bool hasLabel;
};


class MyInputDlg : public MyDialog
{
	Q_OBJECT

public:
	MyInputDlg(QWidget *parent = 0);
	virtual ~MyInputDlg() 
	{}

	static int getRadio(QWidget *parent, const QString& o1, const QString& o2, const QString& caption);
	static int getRadio(QWidget *parent, const QString& o1, const QString& o2, const QString& o3, const QString& caption);
	
	template<typename T>
	static bool getValue(const QString& o1, T& v1, const QString caption, QWidget *parent)
	{
		ParamTemp prm("Selection");
		TypeProp<T> tv1(&prm, o1, o1, v1);
		prm.reset(); // set the values
		if (!prm.execDialog(parent, caption))
			return false;
		v1 = tv1;
		return true;
	}

	template<typename T1, typename T2>
	static bool getValues(const QString& o1, const QString& o2, T1& v1, T2& v2, const QString caption, QWidget *parent)
	{
		ParamTemp prm("Selection");
		TypeProp<T1> tv1(&prm, o1, o1, v1);
		TypeProp<T2> tv2(&prm, o2, o2, v2);
		prm.reset(); // set the values
		if (!prm.execDialog(parent, caption))
			return false;
		v1 = tv1;
		v2 = tv2;
		return true;
	}

	template<typename T1, typename T2, typename T3>
	static bool getValues(const QString& o1, const QString& o2, const QString& o3, T1 &v1, T2 &v2, T3 &v3, const QString& caption, QWidget *parent)
	{
		ParamTemp prm("Selection");
		TypeProp<T1> tr1(&prm, o1, o1, v1);
		TypeProp<T2> tr2(&prm, o2, o2, v2);
		TypeProp<T3> tr3(&prm, o3, o3, v3);
		prm.reset(); // set the values
		if (!prm.execDialog(parent, caption))
			return false;
		v1 = tr1;
		v2 = tr2;
		v3 = tr3;
		return true;
	}


	static bool getValues(QWidget *parent, Prop &p1, Prop &p2, Prop &p3, Prop &p4, const QString caption);
	static bool getValues(QWidget *parent, Prop &p1, Prop &p2, Prop &p3, const QString caption);
	static bool getValues(QWidget *parent, Prop &p1, Prop &p2, const QString caption);
	static bool getValue(QWidget *parent, Prop &p1, const QString caption);

	void setCaptions(const QString& caption);

	// must be called before any widgets are added.
	// if false, then any change effect is immediate.
	void setApplyButton(bool shown);

	void addRadio(const QString& o);



	WidgetLine startLine(Prop* prop)
	{
		QLabel *l = new QLabel(prop->displayName());
		QHBoxLayout *layout = new QHBoxLayout();
		layout->addWidget(l);
		return WidgetLine(layout, this);
	}
	void endLine(const WidgetLine& wl)
	{
		ui.optLayout->insertLayout(ui.optLayout->count() - 1, wl.layout);
	}

	void addOpt(TypeProp<bool>* v, const WidgetLine& wl);

	template<class T>
	void addValue(TypeProp<T>* v, const WidgetLine& wl)
	{
		QLineEdit *c = new QLineEdit();
		c->setMinimumWidth(0);
		WidgetTIn<T>::LineEditIn *w = new WidgetTIn<T>::LineEditIn(v, c, m_autoCommit);
		wl.layout->addWidget(c);
		m_widgets.append(w);
	}

	template<class T>
	void addValueLabel(TypeProp<T>* v, const WidgetLine& wl)
	{
		QLabel *c = new QLabel();
		c->setFrameShadow(QFrame::Sunken);
		c->setFrameShape(QFrame::Panel);
		c->setAlignment(Qt::AlignRight);
		WidgetTIn<T>::LabelIn *w = new WidgetTIn<T>::LabelIn(v, c, false);
		wl.layout->addWidget(c);
		m_widgets.append(w);
	}

	template<class T>
	void addValueSlider(TypeProp<T>* v, const WidgetLine& wl)
	{
		QSlider *c = new QSlider(Qt::Horizontal);
		QLabel *numl = new QLabel();
		SliderIn<T> *w = new SliderIn<T>(v, c, numl, m_autoCommit);
		wl.layout->addWidget(c);
		wl.layout->addWidget(numl);

		m_widgets.append(w);
	}

	void addValueColor(TypeProp<QColor>* v, const WidgetLine& wl);

	template<class T>
	ComboBoxIn<T>* addComboBox(TypeProp<T>* v, const WidgetLine& wl)
	{
		QComboBox *c = new QComboBox();
		ComboBoxIn<T> *w = new ComboBoxIn<T>(v, c, m_autoCommit);
		wl.layout->addWidget(c);
		m_widgets.append(w);
		return w;
	}

	void addSeparator();
	QPushButton* addPushButton(Prop *v, const WidgetLine& wl);
	QPushButton* addPushButton(Prop *v, const QString& text, const WidgetLine& wl);

	int getRadioResult();



private:
	Ui::MyCheckDlgClass ui;


	QVector<QRadioButton*> m_radios;

	QVector<WidgetIn*> m_widgets;
	bool m_autoCommit; // is the change immediate or after apply

public slots:
	void commit();
	void reload();

private slots:
	void on_applyBot_clicked();
	void on_cancelBot_clicked();
	void on_okBot_clicked();

signals:
	void commitedChange();

};




#endif // MYCHECKDLG_H
