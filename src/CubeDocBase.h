#pragma once
#include "Solutions.h"
#include "BuildWorld.h"

#include <memory>
using namespace std;

class Shape;
class Solutions;
class BuildWorld;
class SlvCube;

class CubeDocBase 
{
public:
    CubeDocBase() {
        m_build = new BuildWorld;
        m_slvs = new Solutions;
    }

  	const Shape* getCurrentShape() { 
        return m_shp.get(); 
    }
    BuildWorld& getBuild() { 
        return *m_build; 
    }

    SlvCube *getCurrentSolve() 
    { 
        if (m_slvs == NULL) 
            return NULL;
        return ((m_slvs->size() == 0)?(NULL):(m_slvs->at(m_nCurSlv))); 
    }

    int getCurrentSolveIndex() { 
        return m_nCurSlv; 
    }
    bool solvesExist() const { 
        return ((m_slvs != NULL) && (m_slvs->size() > 0)); 
    }
    int getSolvesNumber() const 
    { 
        if (m_slvs == NULL) 
            return 0;
        return m_slvs->size(); 
    }

    int getUpToStep() const { 
        return m_nUpToStep; 
    }


public:
    Configuration m_conf;

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