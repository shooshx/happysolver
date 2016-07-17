#include "CubeDocBase.h"
#include "general.h"
#include "Cube.h"
#include "SlvCube.h"
#include "BinWriter.h"
#include <sstream>

CubeDocBase* CubeDocBase::s_instance = nullptr; // singleton for use of SlvPainter access of m_flagPiece


void CubeDocBase::openSimple(const char* str)
{
    SlvCube* slv = new SlvCube(m_shp.get());
    istringstream ss(str);
    while (ss.good()) {
        string c;
        int sc = 0, rt = 0;
        ss >> sc >> rt;
        slv->dt.push_back(SlvCube::SlvPiece(sc, rt));
    }
    m_slvs->addBackCommon(slv);
}

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

    unique_ptr<BuildWorld> newbuild(new BuildWorld);

    if (!newbuild->loadFrom(&rdfl))
    { // no build, we can't do nothing.
        rdfl.close();
        m_lastMsg = "Error opening solution file.\nfile: " + name + "\n\nUnable to read shape information";
        return false;
    }

    // generate the shape from the build. this is the shape we're going to use in the end
    unique_ptr<Shape> gendshape(new Shape);
    gendshape->generate(newbuild.get()); // get the return value? no real need to.

    unique_ptr<Solutions> newslvs;
    unique_ptr<Shape> loadedshp(new Shape);

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

        newslvs.reset(new Solutions(gendshape->fcn));
        if (!newslvs->readFrom(&rdfl, gendshape.get()))
        {
            m_lastMsg = "failed reading";
            newslvs.reset();
            return false;
        }
        else
        { // has solutions
            if (newslvs->slvsz != gendshape->fcn)
            { // solutions has errors
                m_lastMsg = "Error opening solution file.\nfile: " + name + "\n\nFile contains shape faces inconsistency\nSolutions not loaded";
                newslvs.reset();
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

    m_build.reset(newbuild.release());

    m_shp.reset(gendshape.release());

    // directly initialize the step indicator now.
    m_nUpToStep = m_shp->fcn;

    // what to do with solutions?
    if (hasSolutions)
    {
        m_slvs.reset(newslvs.release());
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
    EGenResult ret = shape->generate(m_build.get());
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
            complain("The Shape is made of several disconnected parts. it must be a single volume or surface for the solver to work");
            break;
        case GEN_RESULT_ILLEGAL_SIDE:
            complain("Bad Shape - Illegal sides of cubes found");
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

bool CubeDocBase::onGenShape(bool resetSlv, const GenTemplate* temp, const OrderTemplate* orderTemp)
{
    unique_ptr<Shape> newshp(new Shape);
    // the generate process first fills as much of the template as possible, then new tiles
    newshp->m_genTemplate = temp; 
    newshp->m_orderTemplate = orderTemp;
    bool ret = callGenerate(newshp.get(), false);
    newshp->m_genTemplate = nullptr;
    newshp->m_orderTemplate = nullptr;

    if (ret)
    {
        m_build->justGen();

        m_shp.reset(newshp.release());

        m_sthread->haveNewShape(m_shp->fcn);

        clearRemoveFlags();
        m_flagPiece.resize(m_shp->fcn);

        if (resetSlv)
        {
            m_slvs->clear(m_shp->fcn);
            m_nCurSlv = -1;
        }
    }
    return ret;
}


void CubeDocBase::solveGo(SlvCube *starter, bool keepPrev)
{
    if (isSlvEngineRunning())	
    {
        m_sthread->fExitnow = 1;
        return;
    }

    cout << "solveGo!" << endl;

    m_sthread->m_keepPrevSlvs = keepPrev;
    dismissComplain();

    if (m_shp.get() == nullptr || m_build->getChangedFromGen())
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
  /*  if (pics->added.size() < m_shp->fcn)  removed since in incremental build we add pieces for the new addition
    {  TBD - replace with what? - check once starter is removed
        complain("Unable to complay, too few pieces for this shape");
        return;
    }*/

  //  if (pics->added.size() == m_shp->fcn + 42 + 1) // 42 is too easy to come by.
  //      easter();

    // we're all good to go!


    m_sthread->fExitnow = 0;
    m_sthread->setRuntime(m_slvs.get(), m_shp.get(), pics, &m_conf.engine, starter);
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
    if (m_shp.get() == nullptr)
        return;
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

// new shape would have the same faces order as the given vector, even thought this might 
// not be the natural order of the shape if it as generated from scratch
// the difference is due to accumulated generation of the original shape
void CubeDocBase::generateFromFaces(OrderTemplate& ort)
{
    SqrLimits ldBounds; // loaded bounds, not centered in the world
    ldBounds.Inverse(BUILD_SIZE * 4);
    Shape tmp;
    tmp.faces = new Shape::FaceDef[ort.faces.size()];
    tmp.fcn = ort.faces.size();
    for(int i = 0; i < ort.faces.size(); ++i) {
        const auto& t = ort.faces[i];
        auto& f = tmp.faces[i];
        f.ex = t.ex;
        f.dr = t.dr;
        ldBounds.MaxMin(f.ex.z, f.ex.x, f.ex.y);
    }
    tmp.initSizeAndBounds(ldBounds); // needed for unGenerate
    ort.bounds.Inverse(BUILD_SIZE * 4);
    m_build->unGenerate(&tmp, &ort.bounds); // ort.bounds is centered in the world properly
    
    ort.faceDefs = tmp.faces; // will move it to the new shape
    tmp.faces = nullptr; // prevent it from deallocating
    onGenShape(true, nullptr, &ort);
}

void CubeDocBase::addSlvMin(const vector<pair<int, int>>& sv)
{
    cout << "load-slv " << sv.size() << endl;
    auto newslvs = new Solutions(sv.size());
    SlvCube* slv = new SlvCube(m_shp.get());
    newslvs->addBackCommon(slv);
    slv->dt.resize(sv.size());
    for(int i = 0; i < sv.size(); ++i) {
        slv->dt[i].abs_sc = sv[i].first;
        slv->dt[i].abs_rt = sv[i].second;
    }
    m_slvs.reset(newslvs);
    m_nCurSlv = 0; 
}

void CubeDocBase::loadMinText(const string& s)
{
    istringstream is(s);
    int fcn = 0, x, y, z, dr, sc, rt;
    is >> fcn;
    OrderTemplate ort;
    //vector<tuple<Vec3i, int>> faces;
    for(int i = 0; i < fcn; ++i) {
        is >> x >> y >> z >> dr;
        if (!is.good()) {
            cout << "unexpected end read faces " << i << endl;
            return;
        }
        ort.faces.push_back(OrderTemplate::LoadedFace(Vec3i(x, y, z), (EPlane)dr));
    }
    vector<pair<int, int>> slv; // sc, dt
    for(int i = 0; i < fcn; ++i) {
        is >> sc >> rt;
        if (!is.good()) {
            cout << "unexpected end read solution " << i << endl;
            return;
        }
        slv.push_back(make_pair(sc, rt));
    }

    generateFromFaces(ort);
    addSlvMin(slv);
}

string CubeDocBase::serializeMinText()
{
    ostringstream os;
    os << m_shp->fcn << "\n";
    for (int i = 0; i < m_shp->fcn; ++i) {
        const auto& face = m_shp->faces[i];
        os << face.ex.x << " " << face.ex.y << " " << face.ex.z << " " << face.dr << " ";
    }
    os << "\n";
    auto slv = getCurrentSolve();
    if (slv == nullptr)
        return os.str();
    M_ASSERT(slv->dt.size() == m_shp->fcn);
    for (int i = 0; i < m_shp->fcn; ++i) {
        os << slv->dt[i].abs_sc << " " << slv->dt[i].abs_rt << " ";
    }
    os << "\n";
    return os.str();
}


const uint8_t BinWriter::s_bitMasks[] = { 0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff };

string CubeDocBase::serializeMinBin()
{
    string ret;
    BinWriter wr(ret);
    wr.addBits(0x01, 8); // version
    wr.addBits(m_shp->fcn, 8);
    for (int i = 0; i < m_shp->fcn; ++i) {
        const auto& face = m_shp->faces[i];
        wr.addBits(face.ex.x / 4, 6);
        wr.addBits(face.ex.y / 4, 6);
        wr.addBits(face.ex.z / 4, 6);
        wr.addBits(face.dr, 2);
    }
    auto slv = getCurrentSolve();
    if (slv != nullptr)
    {
        M_ASSERT(slv->dt.size() == m_shp->fcn);
        for (int i = 0; i < m_shp->fcn; ++i) {
            wr.addBits(slv->dt[i].abs_sc, 8);
            wr.addBits(slv->dt[i].abs_rt, 3);
        }
    }
    wr.flush();
    return wr.m_buf;
}


bool CubeDocBase::loadMinBin(const string& s)
{
    BinWriter rd(const_cast<string&>(s));
    if (rd.readBits(8) != 0x01)
        return false;
    int fcn = rd.readBits(8);
    int x, y, z, dr, sc, rt;
    OrderTemplate ort;
    //vector<tuple<Vec3i, int>> faces;
    for (int i = 0; i < fcn; ++i) {
        x = rd.readBits(6) * 4;
        y = rd.readBits(6) * 4;
        z = rd.readBits(6) * 4;
        dr = rd.readBits(2);
        if (rd.m_reachedEnd) {
            cout << "unexpected end reading faces " << i << endl;
            return false;
        }
        ort.faces.push_back(OrderTemplate::LoadedFace(Vec3i(x, y, z), (EPlane)dr));
    }

    vector<pair<int, int>> slv; // sc, dt
    for (int i = 0; i < fcn; ++i) {
        sc = rd.readBits(8);
        rt = rd.readBits(3);
        if (rd.m_reachedEnd) {
            cout << "unexpected end reading slv " << i << endl;
            return false;
        }
        slv.push_back(make_pair(sc, rt));
    }

    generateFromFaces(ort);
    addSlvMin(slv);
    return true;
}



void CubeDocBase::pushState()
{
    auto& bucket = PicBucket::mutableInstance();
    cout << "SSS " << &m_stateStack << " " << m_stateStack.size() << "," << m_stateStack.capacity() << endl;
    m_stateStack.emplace_back();
    auto& s = m_stateStack.back();
    s.picSelect.reserve(bucket.pdefs.size());
    for(auto& pd: bucket.pdefs) {
        s.picSelect.push_back(pd.getSelected());
        pd.setSelected(0);
    }
    s.shape = m_shp;
    m_shp.reset();

    s.bld.reset(m_build.release());
    m_build.reset(new BuildWorld);
    m_build->initializeNew(true);

    s.slvs.reset(m_slvs.release());
    m_slvs.reset(new Solutions);
    s.curSlv = m_nCurSlv;
    m_nCurSlv = -1;
    m_nUpToStep = -1;
}

void CubeDocBase::popState()
{
    M_ASSERT(m_stateStack.size() > 0);
    auto& bucket = PicBucket::mutableInstance();
    auto& s = m_stateStack.back();
    for(int i = 0; i < bucket.pdefs.size(); ++i) {
        int count = 0;
        if (i < s.picSelect.size())
            count = s.picSelect[i]; // the new pdefs may be bigger than the old
        bucket.pdefs[i].setSelected(count);
    }

    m_shp = s.shape;
 
    m_build.reset(s.bld.release());

    m_slvs.reset(s.slvs.release());
    m_nCurSlv = s.curSlv;
    m_nUpToStep = m_shp->fcn;

    m_stateStack.pop_back();
}