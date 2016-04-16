#pragma once


typedef unsigned short TPicBits;

/** PicArr is the most basic definition of the layout a piece.
	it defines a single 5x5 piece which can be turned, reversed, copied, changed and accessed
*/
class PicArr
{
public:
	PicArr() :turned(false), rtnindx(-1) {
		for(int i = 0; i < 25; ++i)
			v[i] = 0;
	} // array uninitialized
 
	void turn();
	void revY();
	void copyTo(PicArr &dest) const;
	bool equalTo(const PicArr &dest, bool bSym = false) const;
	void makeRtns(PicArr rtns[8]) const;
    void makeRtnsBits(TPicBits rtbits[8]) const;

	int& set(int x, int y) { 
        return v[x * 5 + y]; 
    }
	int axx(int x, int y) const { 
        return v[(x << 2)+x + y]; 
    }

	TPicBits getBits() const;
    void fromBits(TPicBits v);

    void clear() {
        for (int i = 0; i < 25; ++i)
            v[i] = 0;
        turned = false;
        rtnindx = -1;
    }

	int v[25];
	
	bool turned; // is this rtn turned up-side-down or not. (used in genIFS)
	int rtnindx;	 // original rotation index, always [0,7]
};

const int frameX[] = { 0, 1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1, 0, 0, 0, 0 };
const int frameY[] = { 0, 0, 0, 0, 0, 1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1 };