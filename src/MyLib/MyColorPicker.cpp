#include "MyColorPicker.h"
#include <QColorDialog>

MyColorPicker::MyColorPicker(QWidget *parent, const QColor& init)
	: QDialog(parent), m_col(init), m_init(init)
{
	ui.setupUi(this);
	connect(ui.colortriangle, SIGNAL(colorChanged(const QColor &)), this, SLOT(triangleChanged(const QColor &)));
	connect(ui.redEdit, SIGNAL(textEdited(const QString &)), this, SLOT(textChanged()));
	connect(ui.greenEdit, SIGNAL(textEdited(const QString &)), this, SLOT(textChanged()));
	connect(ui.blueEdit, SIGNAL(textEdited(const QString &)), this, SLOT(textChanged()));
	connect(ui.alphaEdit, SIGNAL(textEdited(const QString &)), this, SLOT(textChanged()));
	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	setTextC();
	setLabelC();
	setTriangC();
	setHtmlC();
}

void MyColorPicker::closeEvent(QCloseEvent *event) 
{ 
	if (ui.proofCheck->isChecked())
		accept(); 
}

void MyColorPicker::setColor(const QColor& col)
{
	m_col = col;
	setTextC();
	setLabelC();
	setTriangC();
	setHtmlC();
	// don't emit proof
}

void MyColorPicker::setLabelC()
{
	ui.viewCol->setStyleSheet(QString("QLabel { background-color : %1; }").arg(m_col.name()));
}
void MyColorPicker::setTextC()
{
	ui.redEdit->setText(QString("%1").arg(m_col.red()));
	ui.greenEdit->setText(QString("%1").arg(m_col.green()));
	ui.blueEdit->setText(QString("%1").arg(m_col.blue()));
	ui.alphaEdit->setText(QString("%1").arg(m_col.alpha()));
}
void MyColorPicker::setTriangC()
{
	ui.colortriangle->setColor(m_col);
}
void MyColorPicker::setHtmlC()
{
	ui.htmlColEdit->setText(QString().sprintf("%02X%02X%02X", m_col.red(), m_col.green(), m_col.blue()));
}

void MyColorPicker::emitProof()
{
	if (ui.proofCheck->isChecked())
		emit colorProof(m_col);
}

void MyColorPicker::triangleChanged(const QColor &col)
{
	QColor rgbcol = col.convertTo(QColor::Rgb); // the triangle returns a color in Hsv
	if (rgbcol == m_col)
		return;
	m_col = rgbcol;
	setLabelC();
	setTextC();
	setHtmlC();
	emitProof();
}
void MyColorPicker::textChanged()
{
	int r = ui.redEdit->text().toInt();
	int g = ui.greenEdit->text().toInt();
	int b = ui.blueEdit->text().toInt();
	int a = ui.alphaEdit->text().toInt();

	QColor col = QColor(r,g,b,a);
	if (col == m_col)
		return;
	m_col = col;

	setLabelC();
	setTriangC();
	setHtmlC();
	emitProof();
}

void MyColorPicker::on_oldBut_clicked()
{
	QColor col = QColorDialog::getColor(m_col, this);
	if (!col.isValid())
		return;
	if (col == m_col)
		return;

	m_col = col;
	setTextC();
	setLabelC();
	setTriangC();
	setHtmlC();
	emitProof();
}

void MyColorPicker::on_htmlColEdit_textEdited(const QString &)
{
	QString text = ui.htmlColEdit->text();
	m_col.setRed(text.mid(0, 2).toInt(NULL, 16));
	m_col.setGreen(text.mid(2, 2).toInt(NULL, 16));
	m_col.setBlue(text.mid(4, 2).toInt(NULL, 16));

	setTextC();
	setLabelC();
	setTriangC();
	emitProof();
}

QColor MyColorPicker::getColor(const QColor& init, QWidget* parent)
{
	MyColorPicker dlg(parent, init);
	QDialog::DialogCode ret = (QDialog::DialogCode)dlg.exec();
	if (ret == QDialog::Rejected)
		return QColor(); // return invalid;
	return dlg.getCol();
}



void MyColorPicker::on_proofCheck_stateChanged(int)
{
	if (ui.proofCheck->isChecked()) // just turned on
		emit colorProof(m_col);
	else // just turned off
		emit colorProof(m_init);
}

void MyColorPicker::on_invalidBot_clicked()
{
	m_col = QColor();
	accept();
}