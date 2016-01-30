#pragma once
#include "Solutions.h"
#include "BuildWorld.h"

#include <memory>
using namespace std;

class Shape;
class Solutions;
class BuildWorld;
class SlvCube;

// hints for the SolveDlg
#define SHINT_SOLUTIONS	1000 // data is the number of solutions
#define SHINT_PARTCHN	1001 // data is the value tms (times part was changed
#define SHINT_STATUS	1002 // data is 0
#define SHINT_ALL		1003 // data is 0
#define SHINT_JUSTGEN	1004 // exactly like IDS_ALL and in addition informs that a gen was just succesful
#define SHINT_START		1005 // like STATUS, when the thread starts
#define SHINT_STOP		1006 // like STATUS, when the thread stops
#define SHINT_WARNING	1007 // warning changed

class CubeDocBase 
{
public:
    CubeDocBase() {
        m_build = new BuildWorld;
        m_slvs = new Solutions;
    }
    virtual ~CubeDocBase()
    {}

  	const Shape* getCurrentShape() { 
        return m_shp.get(); 
    }
    BuildWorld& getBuild() { 
        return *m_build; 
    }

    SlvCube *getCurrentSolve() 
    { 
        if (m_slvs == nullptr) 
            return nullptr;
        return ((m_slvs->size() == 0)?(nullptr):(m_slvs->at(m_nCurSlv))); 
    }

    int getCurrentSolveIndex() { 
        return m_nCurSlv; 
    }
    bool solvesExist() const { 
        return ((m_slvs != nullptr) && (m_slvs->size() > 0)); 
    }
    int getSolvesNumber() const 
    { 
        if (m_slvs == nullptr) 
            return 0;
        return m_slvs->size(); 
    }

    int getUpToStep() const { 
        return m_nUpToStep; 
    }

    // returns false with m_lastMsg set on error
    bool realOpen(const string& name, bool* gotSolutions);

    virtual void transferShape();
    virtual bool onGenShape(bool resetSlv, GenTemplate* temp);
    bool callGenerate(Shape *shape, bool bSilent);

public:
    Configuration m_conf;
    string m_lastMsg;

protected:

    shared_ptr<Shape> m_shp;
    Solutions *m_slvs;

    /// the current design
    BuildWorld *m_build; 

    /// the index of the current solution. -1 means there are none.
    int m_nCurSlv; 

    /// show solution and design up to this step. -1 mean show all.
    /// up to step, not including. the m_nUpToStep piece is not shown.
    /// to show all, m_nUpToStep should be the number of faces in the shape
    int m_nUpToStep; 



};