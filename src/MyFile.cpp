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
#include <stdlib.h>
#include <stdarg.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


bool MyFile::openRead(const char *filename)
{
	fl =  fopen(filename, "r");
	if (fl == nullptr) 
		return false;

	name = filename;
	state = STATE_OPEN_READ;
	return true;
}

bool MyFile::openBuf(const char* _buf)
{
    buf = _buf;
    bufptr = buf;
    state = STATE_OPEN_READ;
    return true;
}
char MyFile::getChar() 
{
    if (fl != nullptr) {
        char tmp[2];
        fread(tmp, 1, 1, fl);
        return tmp[0];
    }
    else if (bufptr != nullptr) {
        char c = *(bufptr++);
        reachedEnd = (c == 0);
        return c;
    }
    return 0;
}

bool MyFile::atEof() {
    if (fl != nullptr) {
        return feof(fl);
    }
    else if (bufptr != nullptr) {
        return reachedEnd;
    }
    return true;
}

void MyFile::setPos(int p) {
    if (fl != nullptr) {
        fseek(fl, 0, SEEK_SET);
    }
    else if (bufptr != nullptr) {
        bufptr = buf + p;
        reachedEnd = false;
    }
}

int MyFile::getPos() {
    if (fl != nullptr) {
        return ftell(fl);
    }
    else if (bufptr != nullptr) {
        return bufptr - buf;
    }
    return 0;
}

bool MyFile::openWrite(const char *filename)
{
	fl = fopen(filename, "w");
	if (fl == nullptr)
		return false;

	name = filename;
	state = STATE_OPEN_WRITE;
	return true;
}

void MyFile::close()
{
	if (fl != nullptr)
	{
		fclose(fl);
		fl = nullptr;
	}
}


// if we detect anything from stopAt, stop the search
int MyFile::seekString(const char *str, const char *stopat)
{
    int slen = strlen(str);
    if (slen == 0)
		return 0;

	unsigned int mi = 0;
	int offs = -1;
	int stpn = (stopat == nullptr)?0:strlen(stopat);

	while (!atEof())
	{
		char tmp = getChar();
		
		++offs;

        if ((mi < slen) && (tmp == str[mi]))
		{
			++mi;
            if (mi == slen)
				return offs+1;
		}
		else
		{
			for(int st = 0; st < stpn; ++st)
			{
				if (tmp == stopat[st])
					return -1;
			}
			mi = 0;
			if (tmp == str[mi]) ++mi;
		}

	}
	return -1;
}


bool MyFile::seekHeader(const char *hName) // through the entire file
{
	if (state != STATE_OPEN_READ) 
		return false;
    setPos(0);

	string sstr = string("<") + hName + ">";
	int ret = seekString(sstr.c_str(), nullptr);

	if (ret != -1) 
		curHeader = getPos(); // plus the '>'
	else 
		return false;

	return true;
}

bool MyFile::seekValue(const char *vName, int level, bool bFromStart)  // from current header to the next header
{
	if (state != STATE_OPEN_READ) 
		return false;
	
	if (bFromStart)
        setPos(curHeader);

	if (strlen(vName) > 255) 
		return false;
	char sstr[260];
	sprintf(sstr, "%c%s", ':'+level, vName);

	int ret = seekString(sstr, "<");
	if (ret == -1) 
		return false;


	if (seekString("=", "<:") == -1)
		return false;

	return true;
}

bool MyFile::mscanint(int* to)
{
    if (fl != nullptr) {
        if (fscanf(fl, "%d", to) < 1)
            return false;
    }
    else if (bufptr != nullptr) {
        char* endp = nullptr;
        *to = strtoul(bufptr, &endp, 10);
        if (endp == bufptr) // nothing read
            return false;
        bufptr = endp;
    }
    return true;
}

int MyFile::readNumsBuf(int cnt, int *buffer) // returns the number of numbers read
{
	if (state != STATE_OPEN_READ) 
		return 0;

	int i;
	for(i = 0; i < cnt; ++i)
	{
		int tmp;
        if (!mscanint(&tmp))
            return i;
		buffer[i] = tmp;
	}
	return i+1;
}

int MyFile::readNums(int cnt, ...)
{
	if (state != STATE_OPEN_READ) 
		return 0;

	va_list marker;
	int *tmp;

	va_start(marker, cnt);

	int i;
	for(i = 0; i < cnt; ++i)
	{
		tmp = va_arg(marker, int*);
        if (!mscanint(tmp))
            return i;
	}
	va_end(marker);
	return i;
}



bool MyFile::writeHeader(const char* hName)
{
	if (state != STATE_OPEN_WRITE) 
		return false;

	fprintf(fl, "<%s>\n", hName);
	return true;
}

bool MyFile::writeValue(const char *vName, bool endCRLF, int level)
{
	if (state != STATE_OPEN_WRITE) 
		return false;

	if (strlen(vName) > 255) 
		return false;

	fprintf(fl, "%c%s = ", ':'+level, vName);
	if (endCRLF) 
		fprintf(fl, "\n");

	return true;
}

bool MyFile::writeNumsBuf(int cnt, int *buffer, bool endCRLF) // cnt == 0: only CRLF
{
	if (state != STATE_OPEN_WRITE) 
		return false;

	for(int i = 0; i < cnt; ++i)
	{
		fprintf(fl, "%d ", buffer[i]);
	}
	if (endCRLF)
	{
		fprintf(fl, "\n");
	}
	return true;

}

bool MyFile::writeNums(int cnt, bool endCRLF, ...)
{
	if (state != STATE_OPEN_WRITE) 
		return false;

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

	return true;
}


bool MyFile::writeStr(const char* st)
{
	if (state != STATE_OPEN_WRITE) 
		return false;

	fprintf(fl, "%s", st);
	return true;
}
