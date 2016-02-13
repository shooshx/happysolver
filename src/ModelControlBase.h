#pragma once

#include "BaseGLWidget.h"
#include "CubeDocBase.h"
#include "OpenGL/Shaders.h"
#include "BuildControlBase.h"

class SlvCube;

class ModelControlBase : public GLHandler
{
public:
    ModelControlBase(BaseGLWidget* gl, CubeDocBase *doc);
    void initTex();

    void reCalcSlvMinMax();
    virtual void switchIn() override;

protected:
    CubeDocBase *m_doc;

    int m_nSingleChoise; // right click, show a single piece
    int m_nHoverChoise, m_nLastHoveChs;

    NoiseSlvProgram m_progNoise;
    FlatProgram m_progFlat; // for choice selection
    Vec3 m_modelmin, m_modelmax;

public:
    BuildControlBase m_buildCtrl;

protected:
    virtual void initialized() override;
    virtual void myPaintGL(bool inChoise) override;
    virtual void drawTargets(bool inChoise);

    // events
    virtual void scrPress(bool rightButton, int x, int y) override;
    virtual void scrRelease(bool rightButton) override;
    virtual bool scrMove(bool rightButton, bool ctrlPressed, int x, int y) override;
    virtual bool scrDblClick(bool ctrlPressed, int x, int y) override;

    // implemented by child
    virtual void emitChosenPiece(int p) {}
    virtual void emitHoverPiece(int p) {}

public:
    //static void drawIFSPolygons(BaseGLWidget *that, const MyObject &obj, bool fTargets);
   // static void drawIFSLines(const MyObject &obj, bool singleChoise);

    /// do the actual painting the scene. This is a static method because it is used
    /// also in PieceGLWidget for drawing.
    /// \arg \c that the GL context in which to draw. (not really needed because we use display lists.)
    /// \arg \c doc the CubeDoc is not a part of the GLWidget so it needs to be sent along to know the shape
    /// \arg \c scube the actual specific solution to draw
    /// \arg \c fTargets are we drawing plain targets or actual fully textured and lit pieces.
    /// \arg \c singleChoise when drawing the whole solution this is -1 otherwise it is
    /// the index of the specific piece to draw.
    /// \arg \c upToStep up to what step to draw the solution or -1 for the entire solution.
    /// currently, -1 is never used and in the normal situation this would be the count of the
    /// tiles in the shape.
    static void paint(BaseGLWidget* that, CubeDocBase *doc, SlvCube *scube, bool fTargets, int singleChoise, int upToStep);
    

};