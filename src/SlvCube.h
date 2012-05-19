#pragma once

#include "general.h"
#include "SlvPainter.h"
#include "CubeAcc.h"


class PicDef;
class PicGroupDef;
class Shape;
class MyFile;

/** SlvCube represents a single solution of a design. SlvCube does not contain any information
	about the structure of the Shape it based on and hence depends completly on living in the
	context of the Shape it was created upon. Essentially SlvCube is just a series of pieces
	layed out in the order of the faces of the Shape.
	An SlvCube \b Does contain complete information of the pieces it is made of an can live
	independantly from the PicsSet which lead to its creation.
*/
class SlvCube
{
public:
	/// SlvPiece holds the date of a single tile in a solution.
	/// An array of objects of this class, ordered by the order of the Shape tiles 
	/// make up a single solution to the shape.
	struct SlvPiece
	{
		SlvPiece(int _sc = -1, int _rt = -1, int _abs_sc = -1, int _abs_rt = -1) 
			: sc(_sc), rt(_rt), abs_sc(_abs_sc), abs_rt(_abs_rt), sdef(NULL)
		{}
		// loaded data
		int sc; ///< the piece, from the compressed pics
		int rt; ///< the rotation, from the compressed pics
		int abs_sc; ///< the piece, from the 'added' pics set
		int abs_rt; ///< absolute rotation [0,7]

		// inferred data, using PicsSet
		const PicDef *sdef;
		//int rtindx; // absolute rotation index [0,7]
	};

	/// SlvPic holds the data of a single piece that was selected when the solution was generated.
	/// the array picdt is essentially a snapshot of the PicsSet taken when the solution was
	/// generated. It is important as a reference point to the SlvPiece data.
	struct SlvPic
	{
		SlvPic(int _gind = -1, int _pind = -1) : gind(_gind), pind(_pind) {}
		int gind; // what group
		int pind; // which one in the group
	};

	/// called when the solution engine find a solution
	SlvCube(const vector<ShapePlace>& plc, const vector<ShapePlace>& abs_plc, const PicsSet *picsc, const Shape *shape);
	/// called when opening a file
	SlvCube(const Shape *shape); 
	
	/// prepare the painter to do its job
	void genPainter();

	bool saveTo(MyFile *wrfl, int index);
	bool readFrom(MyFile *rdfl, int _slvsz);
	void transform(const TTransformVec &moveTo);
	
	int numPieces() const { return dt.size(); }
	const PicGroupDef* getPieceGrpDef(int n) const;

	//int slvsz; // (size of data) keep it close. will always be equal to that of the Solutions
	vector<SlvPiece> dt; 

	//int picssz;
	vector<SlvPic> picdt; // used to construct PicsSet
	bool bConsidersSym; // the solve needs to know if it was symmetric or not to know how to build the pics. this is bad. TBD

	SlvPainter painter;
	const Shape* shape; ///< the shape this solution is based on. the shape is held by CubeDoc

private:
	void makeDt(ShapePlace *plc);

};