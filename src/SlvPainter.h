#pragma once

#include "Configuration.h"
#include "general.h"
#include "MyObject.h"
#include "Mesh.h"

#include <QTextStream>

class SlvCube;
class GLWidget;


/** LinesCollection draws the lines seperating the pieces in the 3D solution display.
	The lines objects in this structure exist in parallel to the PicPainter
	which draws the actual pieces. They are invoked seperatly.
*/
class LinesCollection
{
public:
	LinesCollection() {}
	void resize(int sz) {
		m_bufs.resize(sz);
	}

	Mesh& operator[](uint i) 
	{ 
		Q_ASSERT(i < m_bufs.size());
		return m_bufs[i];
	}

	const Mesh& operator[](uint i) const
	{
		Q_ASSERT(i < m_bufs.size());
		return m_bufs[i];
	}

	vector<Mesh> m_bufs;
};


/** SlvPainter paints an entire solution scene on the given GLWidget.
	It is responsible for arranging the various pieces objects in their
	right position and orientation according to a specific SlvCube which lives
	in the context of a specific Shape.
	An instance of SlvPainter is contained in every SlvCube created. Upon demand
	it would paint the pieces of that specific SlvCube.
	Notice, there is some preperation that should be done before the painter
	can do its job. this preperation is performed in SlvCube::genPainter()
	\see SlvCube
*/
class SlvPainter
{
public:
	// contains a pointer to its parent
	SlvPainter(const SlvCube* _scube) :scube(_scube) {}
	void paint(GLWidget* context, bool fTargets, int singleChoise, int upToStep, ELinesDraw cfgLines) const;

	bool exportToObj(QTextStream& meshFile, QTextStream& materialsFiles) const;

	bool isNull() const { return scube == NULL; }
	void setSlvCube(const SlvCube *sc) { scube = sc; }


	Vec3 qmin, qmax; // 2 opposites for bounding box

	LinesCollection m_linesIFS;

private:
	void paintPiece(int f, GLWidget* context, bool fTargets) const;
	void paintLines(int f, bool singleChoise, GLWidget *context, ELinesDraw cfgLines) const;

	bool exportPieceToObj(QTextStream& meshFile, QTextStream& materialsFiles, int i, uint& numVerts,
						  uint &numTexVerts, uint &numNormals, uint &numObjs) const;

	const SlvCube* scube;

};

