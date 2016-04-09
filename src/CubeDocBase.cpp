#include "CubeDocBase.h"
#include "general.h"
#include "Cube.h"
#include "SlvCube.h"
#include <sstream>

CubeDocBase* CubeDocBase::s_instance = nullptr; // singleton for use of SlvPainter access of m_flagPiece

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

    delete m_build;
    m_build = newbuild;

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
    // the generate process first fills as much of the template as possible, then new tiles
    newshp->m_genTemplate = temp; 
    bool ret = callGenerate(newshp.get(), false);
    newshp->m_genTemplate = nullptr;

    if (ret)
    {
        m_build->justGen();
        m_shp.reset(newshp.release());

        clearRemoveFlags();
        m_flagPiece.resize(m_shp->fcn);

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
  /*  if (pics->added.size() < m_shp->fcn)  removed since in incremental build we add pieces for the new addition
    {  TBD - replace with what? - check once starter is removed
        complain("Unable to complay, too few pieces for this shape");
        return;
    }*/

  //  if (pics->added.size() == m_shp->fcn + 42 + 1) // 42 is too easy to come by.
  //      easter();

    // we're all good to go!


    m_sthread->fExitnow = 0;
    m_sthread->setRuntime(m_slvs.get(), m_shp.get(), pics, &m_conf.engine, getCurrentSolve());
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


void CubeDocBase::generateFromFaces(const vector<tuple<Vec3i, int>>& faces)
{
    SqrLimits bounds;
    bounds.Inverse(BUILD_SIZE * 4);
    Shape tmp;
    tmp.faces = new Shape::FaceDef[faces.size()];
    tmp.fcn = faces.size();
    for(int i = 0; i < faces.size(); ++i) {
        const auto& t = faces[i];
        auto& f = tmp.faces[i];
        f.ex = get<0>(t);
        f.dr = (EPlane)get<1>(t);
        bounds.MaxMin(f.ex.z, f.ex.x, f.ex.y);
    }
    tmp.initSizeAndBounds(bounds);
    m_build->unGenerate(&tmp);
    onGenShape();
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
    vector<tuple<Vec3i, int>> faces;
    for(int i = 0; i < fcn; ++i) {
        is >> x >> y >> z >> dr;
        if (!is.good()) {
            cout << "unexpected end read faces " << i << endl;
            return;
        }
        faces.push_back(make_tuple(Vec3i(x, y, z), dr));
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

    generateFromFaces(faces);
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

//    00BBCCDD  cur=6, count=5
// AAAAA 
class BinWriter
{
public:
    BinWriter(string& s) : m_buf(s)
    {}

    void addBits(uint8_t v, int bitCount) {
        M_ASSERT((v & (~bitMasks[bitCount])) == 0); // check other bits not in bit count are 0
        if (m_curShift + bitCount > 8) {
            m_curByte |= v << m_curShift;
            m_buf.push_back(m_curByte);
            m_curByte = v >> (8 - m_curShift);
            m_curShift = bitCount - 8 + m_curShift;
        }
        else {
            m_curByte |= v << m_curShift;
            m_curShift += bitCount;
        }
    }
    void flush() {
        m_buf.push_back(m_curByte);
        m_curByte = 0;
        m_curShift = 0;
    }

    uint8_t readBits(int bitCount)
    {
        uint8_t r;
        if (m_curShift + bitCount > 8) {
            if (rdEnd())
                return 0;
            r = m_curByte >> m_curShift;
            m_curByte = m_buf[m_rdOffset++];
            r |= m_curByte << (8 - m_curShift);
            m_curShift = bitCount - 8 + m_curShift;
        }
        else {
            if (m_rdOffset == 0) {
                if (rdEnd())
                    return 0;
                m_curByte = m_buf[m_rdOffset++];
            }
            r = m_curByte >> m_curShift;
            m_curShift += bitCount;
        }
        return r & bitMasks[bitCount];
    }
    bool rdEnd() {
        m_reachedEnd = m_rdOffset >= m_buf.size();
        return m_reachedEnd;
    }


    static const uint8_t bitMasks[];
    string& m_buf;
    int m_curShift = 0;
    uint8_t m_curByte;
    int m_rdOffset = 0; // next byte to read
    bool m_reachedEnd = false;
};

const uint8_t BinWriter::bitMasks[] = { 0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff };

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
    vector<tuple<Vec3i, int>> faces;
    for (int i = 0; i < fcn; ++i) {
        x = rd.readBits(6) * 4;
        y = rd.readBits(6) * 4;
        z = rd.readBits(6) * 4;
        dr = rd.readBits(2);
        if (rd.m_reachedEnd) {
            cout << "unexpected end reading faces " << i << endl;
            return false;
        }
        faces.push_back(make_tuple(Vec3i(x, y, z), dr));
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

    generateFromFaces(faces);
    addSlvMin(slv);
    return true;
}


