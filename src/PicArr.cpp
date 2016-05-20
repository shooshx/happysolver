#include "PicArr.h"
#include <sstream>
using namespace std;

void PicArr::turn()
{
	int tx = 4 ,ty = 0;
	PicArr tmp;
	tmp.turned = turned;

	for(int y = 0; y < 5; ++y)
	{
		for(int x = 0; x < 5; ++x) 
		{
			tmp.set(tx,ty) = axx(x,y);
			ty = ty+1;
		}
		ty = 0;
		tx = tx-1;
	}
	tmp.copyTo(*this);
}

void PicArr::revY()
{
	int ty = 4;
	PicArr tmp;
	tmp.turned = turned;

	for(int y = 0; y < 5; ++y) 
	{
		for(int x = 0; x < 5; ++x)
			tmp.set(x,ty) = axx(x,y);
		ty = ty-1;
	}
	tmp.copyTo(*this);
}

bool PicArr::equalTo(const PicArr &dest, bool bSym) const
{
	for(int x = 0; x < 5; ++x)
	{
		for(int y = 0; y < 5; ++y)
		{
			if (dest.axx(x,y) != axx(x,y)) return false;
		}
	}

	if (bSym && (turned != dest.turned))
		return false;

	return true;
}



void PicArr::copyTo(PicArr &dest) const
{
	for(int i = 0; i < 25; ++i)
		dest.v[i] = v[i];
	dest.turned = turned;
	dest.rtnindx = rtnindx;
}


TPicBits PicArr::getBits() const
{
	TPicBits b = 0;
	for (int i = 15; i >= 0; --i) {
		b <<= 1;
		b |= (axx(frameX[i], frameY[i]) != 0)?1:0;
	}
	return b;
}

void PicArr::fromBits(TPicBits v)
{
    TPicBits b = 1;
    for (int i = 0; i < 16; ++i) {
        set(frameX[i], frameY[i]) = ((v & b) != 0) ? 1 : 0;
        b <<= 1;
    }
}


void PicArr::makeRtns(PicArr rtns[8]) const
{
	PicArr tmppic;
	copyTo(tmppic);

	tmppic.turned = false;
	for(int i = 0; i < 8; ++i)
	{ // make all it's rotations
		tmppic.copyTo(rtns[i]);
		rtns[i].rtnindx = i;
		if (i != 3) 
			tmppic.turn();
		else 
		{
			tmppic.turned = true;
			tmppic.revY();
		}
	}
}

void PicArr::makeRtnsBits(TPicBits rtbits[8]) const {
    PicArr rtns[8];
    makeRtns(rtns);
    for(int i = 0; i < 8; ++i)
        rtbits[i] = rtns[i].getBits();
}

string PicArr::prn(bool oneLine) const {
    stringstream ss;
    for(int y = 0; y < 5; ++y)
	{
		for(int x = 0; x < 5; ++x)
		{
			ss << axx(x,y);
		}
        if (oneLine)
            ss << " ";
        else
            ss << endl;            
	}
    return ss.str();
}

void PicArr::fillCenter()
{
    set(1,1) = 1; set(2,1) = 1; set(3,1) = 1;
    set(1,2) = 1; set(2,2) = 1; set(3,2) = 1;
    set(1,3) = 1; set(2,3) = 1; set(3,3) = 1;
}