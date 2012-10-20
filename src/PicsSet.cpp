#include "PicsSet.h"
#include "Pieces.h"
#include "SlvCube.h"

//const PicGroupDef* PicType::thegrp() const{
//	return thedef->mygrp();
//}


void PicType::load(const PicDef& thedef, bool bCsym)
{
	//defInd =_defInd;
	//thedef = &(PicBucket::instance().pdefs[defInd]);

	bool bSym = false; //(bCsym && isIndividual(thegrp()->drawtype)); // determine if it's symetric according to if it's half-texturized
		
	PicArr tmppic;
	thedef.v.copyTo(tmppic);
		
	tmppic.turned = true;
	for(int i = 0; i < 8; ++i)
	{ // make all it's rotations
		tmppic.copyTo(rtns[i]);
		rtns[i].rtnindx = i;
		if (i != 3) 
			tmppic.turn();
		else 
		{
			tmppic.turned = false;
			tmppic.revY();
		}
	}
	rtnnum = 8;

	// should delete?
	bool dflag[8] = { false };
	for(int i = 0; i < 7; ++i)
	{
		for(int j = i + 1; j < 8; ++j)
		{
			if (rtns[i].equalTo(rtns[j], bSym))
				dflag[j] = true;
		}
	}
	for(int i = 7; i >= 0; --i)
	{
		if (dflag[i]) 
		{
			rtnnum--;
			for(int j = i; j < 7; j++)
				rtns[j + 1].copyTo(rtns[j]);
		}
	}

	for(int i = 0; i < 8; ++i) {
		bits[i] = 0;
		if (i < rtnnum)
			bits[i] = rtns[i].getBits();
	}
}

void PicsSet::add(int defInd) 
{

	const PicDef& thedef = PicBucket::instance().pdefs[defInd];
	// search if we already know a piece that looks like this
	int ri = -1;
	bool found = false;
	for (int pi = 0; pi < comp.size() && !found; ++pi) 
	{
		PicType& pt = comp[pi];
		for(ri = 0; ri < pt.rtnnum; ++ri) 
		{
			if (pt.rtns[ri].equalTo(thedef.v))
				break;
		}
		if (ri < pt.rtnnum) 
		{
			pt.addedInds.push_back(PicType::AddedRef(added.size(), pt.rtns[ri].rtnindx));
			++pt.count;
			found = true;
		}
	}

	// not found in set, a piece we haven't seen yet
	if (!found) 
	{
		// add it as a new type of pieces
		PicType pt;
		  // TBD symmetric?
		pt.load(thedef, false); // create its rtns (possible rotations)
		pt.addedInds.push_back(PicType::AddedRef(added.size(), 0));
		++pt.count;
		int typeIndex = comp.size();
		comp.push_back(pt);
		// add to allRtn repository
		totalRtnCount += pt.rtnnum;
// 		for(int ri = 0; ri < pt.rtnnum; ++ri) 
// 		{
// 			RtnInfo rinf;
// 			rinf.bits = pt.rtns[ri].getBits();
// 			rinf.ref = TypeRef(typeIndex, ri);
// 			//rinf.nrtns = pt.rtnnum;
// 			allRtn.push_back(rinf);
// 		}
	}

	added.push_back( AddedPic(defInd) );

}


PicsSet::PicsSet(const SlvCube *scube)
	: bConsiderSymetric(scube->bConsidersSym), totalRtnCount(0)
{
	for (int i = 0; i < scube->picdt.size(); ++i)
	{	
		int defInd = PicBucket::instance().getPicInd(scube->picdt[i].gind, scube->picdt[i].pind);
		add(defInd);
	}
}


PicsSet::PicsSet(bool bSym) 
	: bConsiderSymetric(bSym), totalRtnCount(0)
{
	const PicBucket &bucket = PicBucket::instance();
	for(int idef = 0; idef < bucket.pdefs.size(); ++idef) 
	{
		// if the piece is selected a number of times, load it that number of times in to the set.
		for (int i = 0; i < bucket.pdefs[idef].getSelected(); ++i)
		{
			add(idef);
		}
	}

}

const PicDef* PicsSet::getDef(int abs_sc) const {
	return &PicBucket::instance().pdefs[added[abs_sc].defInd];
}


#if 0
///	Construct the repetitions of pieces.
/// construct the "rep" member of PicType. rep is an array of containing the indexes of
/// all other pics that are similar in shape to the current one.
void PicsSet::makereps()
{
	// temp repetitions
	int *trep = new int[PicBucket::instance().sumPics];

	for(int i = 0; i < size(); ++i)
	{
		// setting the repetitions of i
		pics[i].repnum = 0;
		int j;
		for(j = 0; j < size(); ++j)
		{
			// checking if j is the same
			// we need to have the comparison with 'turned' only if both i and j are 
			// asymmetric since only in this case pieces can't cover for one another
			bool bSym = bConsiderSymetric && 
				isIndividual(pics[i].thegrp()->drawtype) &&
				isIndividual(pics[j].thegrp()->drawtype);
				
			int rt = 0;
			bool found = false;
			while ((rt < pics[i].rtnnum) && (!found))
			{
				found = pics[j].rtns[rt].equalTo(pics[i].rtns[0], bSym); // sure there is a need for turned
				rt++;
			}
			if (found) 
			{
				trep[pics[i].repnum++] = j;
			}
		}

		pics[i].rep = new int[pics[i].repnum];
		for(j = 0; j < pics[i].repnum; ++j)
		{
			pics[i].rep[j] = trep[j];
		}
	}

	delete trep;
}

#endif