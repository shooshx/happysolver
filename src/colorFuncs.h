#pragma once

#include <QColor>
#include <QString>
#include "Vec.h"

inline QColor toCol(const Vec3& v) {
	return QColor(v.r * 255.0f, v.g * 255.0f, v.b * 255.0f);
}
inline Vec3 toVec(const QColor& c) {
	return Vec3(c.red() / 255.0f, c.green() / 255.0f, c.blue() / 255.0f);
}
inline QString colToText(const QColor& c) {
	return QString("%1, %2, %3").arg(c.red()).arg(c.green()).arg(c.blue());
}
inline Vec3 vec3FromText(const QString& t) {
	QStringList l = t.split(QRegExp("[, ]"), QString::SkipEmptyParts);
	if (l.size() != 3)
		return Vec3(0.0f, 0.0f, 0.0f);
	return Vec3(l[0].toFloat(), l[1].toFloat(), l[2].toFloat());
}
inline QString vec3ToText(const Vec3& c) {
	return QString("%1, %2, %3").arg(c.r).arg(c.g).arg(c.b);
}