#pragma once
#include "Solutions.h"
#include "BuildWorld.h"
#include "SlvCube.h"

#include <memory>
using namespace std;

class Shape;
class Solutions;
class BuildWorld;
class SlvCube;
class RunStats;
class SolveContext;

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
        m_build.reset(new BuildWorld);
        m_slvs.reset(new Solutions);

        M_ASSERT(s_instance == nullptr);
        s_instance = this;
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
        if (m_slvs.get() == nullptr || m_slvs->size() == 0 || m_nCurSlv == -1)
            return nullptr;

        return m_slvs->at(m_nCurSlv); 
    }
    SlvCube* getCurrentSolveAndClearSlvs()
    {
        if (m_slvs.get() == nullptr || m_slvs->size() == 0 || m_nCurSlv == -1)
            return nullptr;
        auto s = m_slvs->release(m_nCurSlv);
        m_nCurSlv = -1;
        m_slvs->clear();
        return s;
    }
    void clearSlvs() {
        m_nCurSlv = -1;
        m_slvs->clear();
    }

    int getCurrentSolveIndex() { 
        return m_nCurSlv; 
    }
    bool solvesExist() const { 
        return ((m_slvs.get() != nullptr) && (m_slvs->size() > 0)); 
    }
    int getSolvesNumber() const 
    { 
        if (m_slvs.get() == nullptr) 
            return 0;
        return m_slvs->size(); 
    }

    int getUpToStep() const { 
        return m_nUpToStep; 
    }

    // returns false with m_lastMsg set on error
    bool realOpen(const string& name, bool* gotSolutions);

    virtual void transferShape();
    virtual bool onGenShape(bool resetSlv = true, GenTemplate* temp = nullptr);
    bool callGenerate(Shape *shape, bool bSilent);

    bool isSlvEngineRunning();
    const RunStats* getRunningStats();

    /// starts and stops the solution engine thread.
    virtual void solveGo(SlvCube* starter = nullptr);
    /// stops the solve thread if its running. return only after it is stopped.
    virtual void solveStop();

    void setCurSlvToLast() {
        m_nCurSlv = m_slvs->size() - 1;
        auto s = getCurrentSolve();
        if (s)
            cout << "SLV " << s->debug_prn();
        m_nUpToStep = m_shp->fcn;
    }

    void clearRemoveFlags() {
        fill(m_flagPiece.begin(), m_flagPiece.end(), 0);
    }

    string serializeMinText();
    void loadMinText(const string& s);
    string serializeMinBin();
    bool loadMinBin(const string& s);

    void generateFromFaces(const vector<tuple<Vec3i, int>>& faces);
    void addSlvMin(const vector<pair<int, int>>& sv);

    void openSimple(const char* str);

    void pushState();
    void popState();


public:
    Configuration m_conf;
    string m_lastMsg;

    SolveContext *m_sthread;

    vector<int> m_flagPiece; // int for every face in the shape, makes SlvPainter flag pieces that are going to be deleted

    static CubeDocBase* s_instance; 

protected:

    struct DocState {
        DocState() {}
        DocState(DocState&& o) 
            : picSelect(std::move(o.picSelect))
            , shape(std::move(o.shape))
            , slvs(std::move(o.slvs))
            , bld(std::move(o.bld))
            , curSlv(o.curSlv)
        {}
        vector<int> picSelect;
        shared_ptr<Shape> shape;
        unique_ptr<Solutions> slvs; // should actually by unique_ptr but that's too much of a hassle
        unique_ptr<BuildWorld> bld;
        int curSlv = -1;

        DISALLOW_COPY(DocState);
    };
    vector<DocState> m_stateStack;

    shared_ptr<Shape> m_shp;
    unique_ptr<Solutions> m_slvs;

    /// the current design
    unique_ptr<BuildWorld> m_build; 
    
    /// the index of the current solution. -1 means there are none.
    int m_nCurSlv = -1;

    /// show solution and design up to this step. -1 mean show all.
    /// up to step, not including. the m_nUpToStep piece is not shown.
    /// to show all, m_nUpToStep should be the number of faces in the shape
    int m_nUpToStep = -1; 

};