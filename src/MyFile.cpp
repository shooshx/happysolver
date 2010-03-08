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

#include "MyFile.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


bool MyFile::openRead(const char *filename)
{
	fl =  fopen(filename, "r");
	if (fl == NULL) 
		return false;

	name = filename;
	state = STATE_OPEN_READ;
	return true;
}

bool MyFile::openWrite(const char *filename)
{
	fl = fopen(filename, "w");
	if (fl == NULL)
		return false;

	name = filename;
	state = STATE_OPEN_WRITE;
	return true;
}

void MyFile::close()
{
	if (fl != NULL)
	{
		fclose(fl);
		fl = NULL;
	}
}



int MyFile::seekString(const char *str, const char *stopat)
{
	if (strlen(str) == 0) return FALSE;

	unsigned int mi = 0;
	int offs = -1;
	int stpn = (stopat == NULL)?0:strlen(stopat);

	while (!feof(fl))
	{
		char tmp[2];
		fread(tmp, 1, 1, fl);
		++offs;

		if ((mi < strlen(str)) && (tmp[0] == str[mi]))
		{
			++mi;
			if (mi == strlen(str))
				return offs+1;
		}
		else
		{
			for(int st = 0; st < stpn; ++st)
			{
				if (tmp[0] == stopat[st])
					return -1;
			}
			mi = 0;
			if (tmp[0] == str[mi]) ++mi;
		}

	}
	return -1;
}


int MyFile::seekHeader(const char *hName) // through the entire file
{
	if (state != STATE_OPEN_READ) 
		return FALSE;
	fseek(fl, 0, SEEK_SET);

	QString sstr = QString("<") + hName + ">";
	int ret = seekString(sstr.toAscii(), NULL);

	if (ret != -1) curHeader = ftell(fl); // plus the '>'
	else return FALSE;

	return TRUE;
}

int MyFile::seekValue(const char *vName, int level, bool bFromStart)  // from current header to the next header
{
	if (state != STATE_OPEN_READ) 
		return FALSE;
	
	if (bFromStart)
		fseek(fl, curHeader, SEEK_SET);

	if (strlen(vName) > 255) return FALSE;
	char sstr[260];
	sprintf(sstr, "%c%s", ':'+level, vName);

	int ret = seekString(sstr, "<");
	if (ret == -1) return FALSE;


	if (seekString("=", "<:") == -1)
		return FALSE;

	return TRUE;
}

int MyFile::readNumsBuf(int cnt, int *buffer) // returns the number of numbers read
{
	if (state != STATE_OPEN_READ) 
		return FALSE;

	int i;
	for(i = 0; i < cnt; ++i)
	{
		int tmp;
		if (fscanf(fl, "%d", &tmp) < 1)
			return i;
		buffer[i] = tmp;
	}
	return i+1;
}

int MyFile::readNums(int cnt, ...)
{
	if (state != STATE_OPEN_READ) 
		return FALSE;

	va_list marker;
	int *tmp;

	va_start(marker, cnt);

	int i;
	for(i = 0; i < cnt; ++i)
	{
		tmp = va_arg(marker, int*);
		if (fscanf(fl, "%d", tmp) < 1)
			return i;

	}
	va_end(marker);
	return i;
}



int MyFile::writeHeader(const char* hName)
{
	if (state != STATE_OPEN_WRITE) 
		return FALSE;

	fprintf(fl, "<%s>\n", hName);
	return TRUE;
}

int MyFile::writeValue(const char *vName, bool endCRLF, int level)
{
	if (state != STATE_OPEN_WRITE) 
		return FALSE;

	if (strlen(vName) > 255) return FALSE;

	fprintf(fl, "%c%s = ", ':'+level, vName);
	if (endCRLF) fprintf(fl, "\n");

	return TRUE;
}

int MyFile::writeNumsBuf(int cnt, int *buffer, bool endCRLF) // cnt == 0: only CRLF
{
	if (state != STATE_OPEN_WRITE) 
		return FALSE;

	for(int i = 0; i < cnt; ++i)
	{
		fprintf(fl, "%d ", buffer[i]);
	}
	if (endCRLF)
	{
		fprintf(fl, "\n");
	}
	return TRUE;

}

int MyFile::writeNums(int cnt, bool endCRLF, ...)
{
	if (state != STATE_OPEN_WRITE) 
		return FALSE;

	va_list marker;
	int tmp;

	va_start(marker, endCRLF);

	for(int i = 0; i < cnt; ++i)
	{
		tmp = va_arg(marker, int);
		fprintf(fl, "%d ", tmp);
	}
	va_end(marker);

	if (endCRLF)
	{
		fprintf(fl, "\n");
	}

	return TRUE;
}


int MyFile::writeStr(const char* st)
{
	if (state != STATE_OPEN_WRITE) 
		return FALSE;

	fprintf(fl, "%s", st);
	return TRUE;
}
