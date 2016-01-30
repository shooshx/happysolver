#pragma once
#include "general.h"
#include "PicArr.h"

class PicGroupDef;
class PicDef;

/**	PicType represents a piece when it is a part of an active piece set.
	when a piece is selected to take part in a construction and the 
	construction engine begins, it is loaded into a PicType instance
	as a PicType the piece has some more information which has to do 
	with piece repetition and rotation

	Represents all the pieces that look the same
	\see PicSet
*/
class PicType
{	
public:
	PicType() : rtnnum(0), isSym(false) {
		for(int i = 0; i < 8; ++i)
			bits[i] = 0;
	}

	/// load a piece from the bucket to this instance
	/// \arg bCsym the what-to-do-with-Asymmetric-pieces option
	void load(const PicDef& thedef, bool bCsym);
	
	int rtnnum; // how many rtns we actually have
	PicArr rtns[8];
	TPicBits bits[8];
	// does this type represent a non symmetric piece.
	// normal pieces and non-symmetric pieces are represented by different types even though with the same shape
	bool isSym; 

	int count() const { // how many instances of this pieces exist
		return addedInds.size();
	}

	// a reference for the defs this type represents
	class AddedRef {
	public:
		AddedRef(int _addedInd = -1, int _defRot = -1) : addedInd(_addedInd), defRot(_defRot) {}
		int addedInd; // index to the added list
		int defRot; // our rtn which is the base shape refers to this rtn in the def. [0,7]
	};

	vector<AddedRef> addedInds; ///< the index of the PicDef in the bucket

};


class AddedPic
{
public:
    AddedPic(int _defInd = -1, int _compInd = -1) : defInd(_defInd), compInd(_compInd) {}

	int defInd; // index in the bucket
	// possibly add the rtns of the added pic? good for lines.
    int compInd; // index of the the unified pic in comp (not used currently)
};

class SlvCube;

// reference to a specific rtn in a PicType
class TypeRef {
public:
	TypeRef() : typeInd(-1), rtnInd(-1) {}
	TypeRef(int _typeInd, int _rtnInd) : typeInd(_typeInd), rtnInd(_rtnInd) {}
	int typeInd; // index in the comp array
	int rtnInd;  // index in the rtns of a PicType (not absolute rotation)
};

/** PicsSet holds all the pieces of an active solving session.
	When the user hits "Solve It" a PicsSet is constructed with the currently selected
	pieces and the solution engine is ran using it.
*/
class PicsSet
{	
public:
	/// this ctor takes the selection from the Solution
	PicsSet(const SlvCube *scube); 

	///	this ctor takes the selection from the bucket
	///	with bSym == true, there are more rtns for every pic.
	/// \see CubeDoc::solveGo()
	PicsSet(bool cSym);

	/// used in the initialization of PicBucket
	PicsSet() : totalRtnCount(0), considerSymetric(false)
	{}
						
	const PicDef* getDef(int abs_sc) const;

	vector<PicType> comp;   // compressed list of distinct pieces, with references to the added list
	vector<AddedPic> added; // all pieces added, by the order they were added

	int totalRtnCount; // needed as the maximum size of tryd array
	bool considerSymetric;

public:
	void add(int defInd, bool considerSym);
};
