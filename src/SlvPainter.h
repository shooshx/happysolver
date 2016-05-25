#pragma once

#include "Configuration.h"
#include "general.h"
//#include "MyObject.h"
#include "Mesh.h"

class SlvCube;
class BaseGLWidget;
class ObjExport;


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
        M_ASSERT(i < m_bufs.size());
        return m_bufs[i];
    }

    const Mesh& operator[](uint i) const
    {
        M_ASSERT(i < m_bufs.size());
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
    SlvPainter(const SlvCube* _scube) :m_scube(_scube) 
    {

    }
    void paint(BaseGLWidget* context, bool fTargets, int singleChoise, int upToStep, ELinesDraw cfgLines) const;

    bool exportToObj(ObjExport& oe) const;

    bool isNull() const { return m_scube == nullptr; }
    void setSlvCube(const SlvCube *sc) { m_scube = sc; }

public:
    Vec3 qmin, qmax; // 2 opposites for bounding box

    LinesCollection m_linesIFS;

private:
    void paintPiece(int f, BaseGLWidget* context, bool fTargets) const;
    void paintLines(int f, bool singleChoise, BaseGLWidget *context, ELinesDraw cfgLines) const;

    bool exportPieceToObj(ObjExport& oe, int i) const;

    const SlvCube* m_scube;
    mutable vector<Mat4> m_mats;
};

