#pragma once

#include "BaseGLWidget.h"
#include "CubeDocBase.h"
#include "OpenGL/Shaders.h"
#include "BuildControlBase.h"
#include "Quaternion.h"

class SlvCube;

// enlarges the arrows button when mouse hovers above it
struct FloatProgress : public IProgressable
{
    FloatProgress(float totalTimeSec) :m_ttime(totalTimeSec) {}
    bool reset() {
        bool wasReset = m_value != 0.0;
        m_value = 0.0f;
        return wasReset;
    }
    virtual bool progress(float deltaSec) override;
    float m_value = 0.0; // [0-1]
    const float m_ttime = 0.0;
};

struct SlerpProgress : public FloatProgress
{
    SlerpProgress(float totalTimeSec, BaseGLWidget* bgl) : FloatProgress(totalTimeSec), m_bgl(bgl){}
    bool reset(const Mat4 startt, const Mat4& endt);
    virtual bool progress(float deltaSec) override;

    Quaternion m_qstart, m_qend;
    BaseGLWidget* m_bgl = nullptr;
};

class ModelControlBase : public GLHandler
{
public:
    ModelControlBase(BaseGLWidget* gl, CubeDocBase *doc);
    void initTex();

    void reCalcSlvMinMax();
    virtual void switchIn() override;

    void restartSolve(bool withCurrentAsStarter, bool keepPrev);

protected:
    CubeDocBase *m_doc;

    int m_nSingleChoise; // right click, show a single piece
    int m_nHoverChoise, m_nLastHoveChs;

    NoiseSlvProgram m_progNoise;
    FlatProgram m_progFlat; // for choice selection
    Vec3 m_modelmin, m_modelmax;
    Vec3 m_bldDiff;

    SlvCube* m_lastSlv = nullptr; // for knowing if we need to invalidate the choise
    int m_lastUpTo = -1;

    unique_ptr<Mesh> m_arrowMesh;
    unique_ptr<Mesh> m_sphereMesh; // arrows center
    Recti m_arrowPressArea; // rect around the arrows sphere
    bool m_arrowSpherePressed = false; // makes sure both press and release are on the the sphere to do the reset, make sphere oranger
    FloatProgress m_spherePopAnim; // sphere turns orange
    SlerpProgress m_resetPositionAnim; // slerp movement

public:
    BuildControlBase m_buildCtrl;
    bool m_addArrows = false; // rotation indicators arrows

protected:
    virtual void initialized() override;
    virtual void myPaintGL(bool inChoise) override;
    virtual void drawTargets(bool inChoise);

    // events
    virtual void scrPress(bool rightButton, int x, int y) override;
    virtual void scrRelease(bool rightButton, int x, int y) override;
    virtual bool scrMove(bool rightButton, bool ctrlPressed, int x, int y) override;
    virtual bool scrDblClick(bool ctrlPressed, int x, int y) override;

    // implemented by child
    virtual void emitChosenPiece(int p) {}
    virtual void emitHoverPiece(int p) {}

    virtual void clearChoise() override {
        m_buildCtrl.clearChoise();
    }


    bool shapeHoverForBuild(int choise, bool rightButton, bool ctrlPressed, int x, int y);
    void drawAxisArrows();


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