#pragma once
#include <string>
using namespace std;

typedef unsigned short TPicBits;

/** PicArr is the most basic definition of the layout a piece.
	it defines a single 5x5 piece which can be turned, reversed, copied, changed and accessed
*/
class PicArr
{
public:
	explicit PicArr(TPicBits bits = 0) :turned(false), rtnindx(-1) {
		for(int i = 0; i < 25; ++i)
			v[i] = 0;
        if (bits != 0)
            fromBits(bits);
	} 
 
	void turn();
	void revY();
	void copyTo(PicArr &dest) const;
	bool equalTo(const PicArr &dest, bool bSym = false) const;
	void makeRtns(PicArr rtns[8]) const;
    void makeRtnsBits(TPicBits rtbits[8]) const;

	char& set(int x, int y) { 
        return v[y * 5 + x]; 
    }
	int axx(int x, int y) const { 
        return v[(y << 2)+y + x]; 
    }

	TPicBits getBits() const;
    void fromBits(TPicBits v);

    void clear() {
        for (int i = 0; i < 25; ++i)
            v[i] = 0;
        turned = false;
        rtnindx = -1;
    }
    string prn(bool oneLine = false) const;
    void fillCenter(); // just for printing, not functional

	char v[25];
	
	bool turned; // is this rtn turned up-side-down or not. (used in genIFS)
	int rtnindx;	 // original rotation index, always [0,7]
};

const int frameX[] = { 0, 1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1, 0, 0, 0, 0 };
const int frameY[] = { 0, 0, 0, 0, 0, 1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1 };