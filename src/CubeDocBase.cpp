#include "CubeDocBase.h"



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