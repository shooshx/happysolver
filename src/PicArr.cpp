#include "PicArr.h"


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
	for(int x = 0; x < 5; ++x)
	{ 
		for(int y = 0; y < 5; ++y)
		{
			dest.set(x,y) = axx(x,y);
		}
	}
	dest.turned = turned;
	dest.rtnindx = rtnindx;
}

const int frameX[] = {0, 1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1, 0, 0, 0, 0 };
const int frameY[] = {0, 0, 0, 0, 0, 1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1 };



TPicBits PicArr::getBits() const
{
	TPicBits b = 0;
	for (int i = 15; i >= 0; --i) {
		b <<= 1;
		b |= (axx(frameX[i], frameY[i]) != 0)?1:0;
	}
	return b;
}