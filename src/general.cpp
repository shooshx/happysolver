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

#include "general.h"

#include <QString>
#include <QCoreApplication>

QString humanCount(qint64 n)
{
	QString init = QString("%1").arg(n);
	
	if (init.length() < 4) return init;

	QByteArray buf = init.toAscii();
	int len = buf.count();
	
	char rbuf[50] = {0};
	int ind = len - 1;
	int x = 0;
	for (int i = len + (len-1)/3 - 1; i >= 0; --i)
	{
		if (++x % 4 == 0) rbuf[i] = ',';
		else rbuf[i] = buf[ind--];
	}
	return rbuf;
}


qint64 powOf10[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, Q_INT64_C(1000000000), Q_INT64_C(10000000000), Q_INT64_C(100000000000) };

QString humanCount(double n, int pers)
{
	qint64 dn = (qint64)n;
	qint64 pwr = powOf10[qMin(pers, 11)];
	qint64 fl = (qint64)((n - dn) * pwr);
	QString sfl = QString("%1").arg(fl).toAscii();

	return humanCount(dn) + "." + sfl;

}

void flushAllEvents()
{
	QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents); // process the hide event. otherwise the 3d view is moved and it doesn't look nice.
	QCoreApplication::sendPostedEvents(NULL, 0);
	QCoreApplication::flush();
}

