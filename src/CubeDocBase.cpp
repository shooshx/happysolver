#include "CubeDocBase.h"
#include "general.h"
#include "Cube.h"

// TBD - check messages.
bool CubeDocBase::realOpen(const string& name, bool* gotSolutions)
{
    MyFile rdfl;
#ifdef XX_QT_CORE_LIB
    if (!rdfl.openRead(name.c_str())) {
        m_lastMsg = "failed opening file " + name;
        return false;
    }
#else
    rdfl.openBuf(name.c_str());
#endif
    bool hasSolutions = false;

    BuildWorld *newbuild = new BuildWorld;

    if (!newbuild->loadFrom(&rdfl))
    { // no build, we can't do nothing.
        delete newbuild; newbuild = nullptr;
        rdfl.close();
        m_lastMsg = "Error opening solution file.\nfile: " + name + "\n\nUnable to read shape information";
        return false;
    }

    // generate the shape from the build. this is the shape we're going to use in the end
    auto_ptr<Shape> gendshape(new Shape);
    gendshape->generate(newbuild); // get the return value? no real need to.

    Solutions *newslvs = nullptr;
    auto_ptr<Shape> loadedshp(new Shape);

    if (!loadedshp->loadFrom(&rdfl))
    { // no shape (hence no solutions) but there is a build
        loadedshp.reset(); // get rid of it, its no good.
        // newbuild - no call to justGen (stay with ctor values)

    }
    else // there is a shape loaded (you can't have solutions without a shape in the file.)
    {
        // don't use the shape just loaded, instead, generate the build, and draw a transform
        // from the loaded shape for the genereted one.
        // TBD MOVE THIS. only needed if there are solutions.
        TTransformVec movedTo(loadedshp->fcn);
        bool trivialTransform = false;
        if (!loadedshp->createTrasformTo(gendshape.get(), movedTo, &trivialTransform))
        {
            m_lastMsg = "failed shape transform, bug.";
            return false;
        }

        loadedshp.reset(); // we're done with it.

        newbuild->justGen(); // shouldn't gen it next time.

        newslvs = new Solutions(gendshape->fcn);
        if (!newslvs->readFrom(&rdfl, gendshape.get()))
        {
            m_lastMsg = "failed reading";
            delete newslvs; newslvs = nullptr;
            return false;
        }
        else
        { // has solutions
            if (newslvs->slvsz != gendshape->fcn)
            { // solutions has errors
                m_lastMsg = "Error opening solution file.\nfile: " + name + "\n\nFile contains shape faces inconsistency\nSolutions not loaded";
                delete newslvs; newslvs = nullptr;
                return false;
            }
            else
            {
                if (!trivialTransform)
                    newslvs->transform(movedTo);
                hasSolutions = true;
            }
        }
    }

    // don't do anything before all the gui is done (messageboxes etc')
    // because a paint to the build would reveal something half baked.

    delete m_build;
    m_build = newbuild;

    m_shp.reset(gendshape.release());

    // directly initialize the step indicator now.
    m_nUpToStep = m_shp->fcn;

    // what to do with solutions?
    if (hasSolutions)
    {
        delete m_slvs;
        m_slvs = newslvs;
        m_nCurSlv = 0; // re-set it from -1
        *gotSolutions = true;
    }
    else // an open without solutions
    {
        if (m_shp.get() != nullptr)
            m_slvs->clear(m_shp->fcn);
        else
            m_slvs->clear(); // size will get set when generated

        m_nCurSlv = 0;
        *gotSolutions = false;
    }


    rdfl.close();
    return true;
}


bool CubeDocBase::callGenerate(Shape *shape, bool bSilent)
{
    EGenResult ret = shape->generate(m_build);
    if (ret == GEN_RESULT_OK)
        return true;
    else if (!bSilent)
    {
        switch (ret)
        {
        case GEN_RESULT_NO_START:
            complain("Generation of shape failed.\nMake sure your shape has a yellow starting piece.");
            break;
        case GEN_RESULT_NOT_CONNECT:
            complain("Generation of shape failed.\nShape is made of several disconnected parts.\nThe shape must be a single volume or surface.");
            break;
        case GEN_RESULT_ILLEGAL_SIDE:
            complain("Generation of shape failed.\nIllegal side discovered.");
            break;
        case GEN_RESULT_UNKNOWN:
            complain("Generation of shape failed.\nUnknown error, this is a bug!!!");
            break;
        case GEN_RESULT_OK: // shut gcc up.
            break;
        }
    }
    return false;
}

bool CubeDocBase::onGenShape(bool resetSlv, GenTemplate* temp)
{
    unique_ptr<Shape> newshp(new Shape);
    newshp->m_genTemplate = temp;
    bool ret = callGenerate(newshp.get(), false);
    newshp->m_genTemplate = nullptr;

    if (ret)
    {
        m_build->justGen();
        m_shp.reset(newshp.release());
        if (resetSlv)
        {
            m_slvs->clear(m_shp->fcn);
            m_nCurSlv = 0;
        }
    }
    return ret;
}


void CubeDocBase::solveGo()
{
    if (isSlvEngineRunning())	
    {
        m_sthread->fExitnow = 1;
        return;
    }

    cout << "solveGo!" << endl;
    if ((m_shp.get() == nullptr) || (m_build->getChangedFromGen()))
    {
        if (!onGenShape())
            return;
    }

    // now build the Pics. it's important that the PicsSet will be built in this thread
    // since it involves the bucket.
    // here, the snapshot of the bucket piece selection
    // for any solutions to be produced in this run
    // the snapshot of the EngineConf is captured in Cube::Cube()
    // in the thread itself. (is that too late?)
    PicsSet *pics = new PicsSet(m_conf.engine.nAsym != ASYM_REGULAR);
    if (pics->added.size() < m_shp->fcn)
    {
        complain("Unable to complay, too few pieces for this shape");
        return;
    }

  //  if (pics->added.size() == m_shp->fcn + 42 + 1) // 42 is too easy to come by.
  //      easter();

    // we're all good to go!


    m_sthread->fExitnow = 0;
    m_sthread->setRuntime(m_slvs, m_shp.get(), pics, &m_conf.engine, getCurrentSolve());
    m_sthread->doStart();

}

void CubeDocBase::solveStop()
{
    if (!isSlvEngineRunning())
        return;

    m_sthread->fExitnow = 1;
    m_sthread->doWait();
}

bool CubeDocBase::isSlvEngineRunning() {
    return (m_sthread != nullptr) && (m_sthread->fRunning);
}

const RunStats* CubeDocBase::getRunningStats()
{
    static RunStats dummy;
    if (m_sthread == nullptr)
        return &dummy;
    return &m_sthread->m_stats;
}



void CubeDocBase::transferShape()
{
    shared_ptr<Shape> oldshp = m_shp; // save it so it won't be deleted yet
    GenTemplate temp;
    for (int i = 0; i < m_shp->fcn; ++i) {
        const auto& face = m_shp->faces[i];
        // ex is can change but ex+bound-min is always the same place
        temp.add(face.ex + m_shp->buildBounds.getMin(), face.dr, face.facing);
    }

    if (!onGenShape(false, &temp))
        return;

    m_slvs->toNewShape(m_shp.get());
}
