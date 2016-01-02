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

#ifndef __MYFILE_H__INCLUDED__
#define __MYFILE_H__INCLUDED__

#include <string>
using namespace std;

#define STATE_CLOSED 0
#define STATE_OPEN_READ 1
#define STATE_OPEN_WRITE 2

/** \file
	Declares the MyFile class.
*/

/** MyFile is a silly abstraction to a text file which contains data saved by the application.
	MyFile is the format of the solution or shape files saved by the application. it is
	supposed to be hierarchical in nature.
	Implementing a hierarchical data format from scratch turned out to be not such
	a great idea. The saved files should be moved to XML format and this class should be removed.
	\see CubeDoc
*/
class MyFile  
{
public:
	MyFile()
	{}

	~MyFile()
	{
		close();
	}

	bool openRead(const char *filename);
	bool openWrite(const char *filename);
    bool openBuf(const char* buf);
	void close();

	// header and value strings should not contain ":" or "<" or ">"!
	int seekString(const char *str, const char *stopat);
	bool seekHeader(const char *hName); // through the entire file
	bool seekValue(const char *vName, int level = 0, bool bFromStart = false);  // from current header to next header
	int readNumsBuf(int cnt, int *buffer);
	int readNums(int cnt, ...);

	bool writeHeader(const char* hName);
	bool writeValue(const char *vName, bool endCRLF, int level = 0); // sequential!!!
	bool writeNumsBuf(int cnt, int *buffer, bool endCRLF = true); // cnt == 0: only CRLF
	bool writeNums(int cnt, bool endCRLF, ...); 

	bool writeStr(const char* st); // this is used only for debug purposes.

	int getState() { return state; }
	
private:
    char getChar();
    bool atEof();
    void setPos(int p);
    int getPos();
    bool mscanint(int* to);

private:

	string name;
    int state = STATE_CLOSED;
    int curHeader = -1;

	FILE *fl = nullptr;
	
    const char* buf = nullptr;
    const char* bufptr = nullptr;
    bool reachedEnd = false;
};

#endif // __MYFILE_H__INCLUDED__
