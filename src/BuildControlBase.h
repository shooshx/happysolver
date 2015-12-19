#pragma once
#include "BaseGLWidget.h"
#include "Mesh.h"
#include "BuildWorld.h"
#include "OpenGL/Shaders.h" // BuildProgram
#include "Vec.h"

class CubeDocBase;

class BuildControlBase : public GLHandler
{
public:
    BuildControlBase(BaseGLWidget* gl, CubeDocBase* doc);

    /// the action a double click would cause
    enum EActStatus 
    { 
        ADD, ///< a double click would add a box
        REMOVE,  ///< a double click would remove a box
        CANT_REMOVE, ///< a double click can't remove a box because it is the only box in the design
        EDIT_DISABLE ///< a double click would do nothing because edits are disable which solution engine is running.
    };

protected: 
    virtual void initialized();
    virtual void drawTargets(bool inChoise);
    virtual void myPaintGL();
    virtual void switchIn();
    virtual void switchOut();

    virtual bool scrDblClick(int x, int y);
    virtual bool scrMove(bool rightButton, bool ctrlPressed, int x, int y);

    // implemeted by inheriting
    virtual void emitTilesCount(int n) = 0;
    virtual void emitTileHover(int tile, EActStatus act) {}

    bool doMouseMove(int x, int y, bool makeBufs = true);
    bool isInRemove();
    void checkSides();
    void reCalcBldMinMax();

    void makeBuffers();

private:
    bool tiledDblClick(int choise);
    bool boxedDblClick(int choise, int x, int y);

    bool getChoiseTiles(int choise, bool remove, CoordBuild bb[6], Vec3i &g);

    //void drawTargetsPart(bool fTrans, bool fLines);
    void drawErrorCyliders();


protected:
    CubeDocBase *m_doc;

    bool m_bEditEnabled;
    bool m_fSetStrtMode;
    bool m_fUnSetBlueMode;

    bool m_bBoxedMode;

    bool m_bInternalBoxRemove; // internal, from ctrl
    bool m_bBoxRemove; // from the GUI button. this represents the external status, not including Ctrl button

    bool m_bLastBoxRemove; ///< used in mouseMoveEvent()
    int m_lastChoise; ///< used in mouseMoveEvent()
    Vec3i m_lastCubeChoise; ///< used in mouseMoveEvent()

    /// number of valid tiles in m_curMarkedTiles
    int m_nMarkedTiles;
    bool m_inFade;
    float m_fadeFactor;
    /// the tiles marked with blue or red under the mouse pointer
    CoordBuild m_curMarkedTiles[6];

    /// when a D2 error occur the offending sides are marked by blinking cylinders
    /// to make the blinking effect we need to remember theie current alpha and
    /// the direction where that alpha was going.
    float m_errCylindrAlpha;
    float m_errCylindrAlphaDt; ///< advancing of the alpha


   // bool m_bDoneUpdate; ///< optmization. avoiding multiple updates in mouseMoveEvent()

    Mesh m_realTiles;
    Mesh m_transTiles;
    Mesh m_realLines;
    Mesh m_transLines;

    Mesh m_cylinder;

    BuildProgram m_prog;
    Vec3 m_buildmin, m_buildmax;

};