#ifndef MYCOLORPICKER_H
#define MYCOLORPICKER_H

#include <QDialog>
#include "ui_MyColorPicker.h"

class MyColorPicker : public QDialog
{
	Q_OBJECT

public:
	MyColorPicker(QWidget *parent = 0, const QColor& init = Qt::white);
	~MyColorPicker() {}

	static QColor getColor(const QColor& init, QWidget* parent);
	
	QColor getCol() { return m_col; }

protected:
	virtual void closeEvent(QCloseEvent *event);

private:
	Ui::MyColorPickerClass ui;
	QColor m_col;
	QColor m_init;

	void setLabelC();
	void setTextC();
	void setHtmlC();
	void setTriangC();
	void emitProof();

public slots:
	void setColor(const QColor& col);

private slots:
	void triangleChanged(const QColor &col);
	void textChanged();

	void on_invalidBot_clicked();
	void on_proofCheck_stateChanged(int);
	void on_htmlColEdit_textEdited(const QString &);
	void on_oldBut_clicked();

signals:
	void colorProof(const QColor& c);
};

#endif // MYCOLORPICKER_H
