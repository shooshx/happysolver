#include <emscripten.h>
#include <html5.h>
#include <trace.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <iostream>
#include <exception>

#include "../src/CubeDocBase.h"
#include "../src/BaseGLWidget.h"
#include "../src/ModelControlBase.h"
#include "../src/SlvCube.h"
#include "../src/Cube.h"
#include "../src/BinWriter.h"

using namespace std;

int makeShader(int type, const char* text) 
{
    auto s = glCreateShader(type);
    glShaderSource(s, 1, &text, nullptr);
    glCompileShader(s);
    int statusOk = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &statusOk);
    if (!statusOk) {
        int sz = 0;
        glGetShaderInfoLog(s, 0, &sz, nullptr);
        string err;
        err.resize(sz);
        glGetShaderInfoLog(s, sz, &sz, (char*)err.data());
        cout << err;
        throw std::runtime_error("FAILED compile shader");
    }
    return s;
}

void complain(const char* msg) {
    cout << "ERROR: " << msg << endl;
}
void sendIsRunning();

class RunContext : public SolveContext
{
public:
    virtual void notifyLastSolution(bool firstInGo) override;
    virtual void notifyFullEnum() override;
    virtual void notifyNotEnoughPieces() override;

    // called from CubeDocBase::solveGo
    virtual void doStart() override {
        init();
        sendIsRunning(); // was set in init above
        EM_ASM(requestSlvRun());
    }
    virtual void doWait() override {
        runAll(); // in solveStop() this will detect that we turned fExitnow on and will stop the run 
    }
    
    bool runSome() {
        doRun(10000);
        return fRunning;
    }
    void runAll() {
        doRun(-1);
    }
};


class MainCtrl
{
public:
    MainCtrl() : m_modelGl(&m_gl, &m_doc)
    {
        m_gl.m_handlers.push_back(&m_modelGl);
        m_handler = &m_modelGl;
        m_doc.m_sthread = &m_runctx;
        //cout << "mainctrl ctor" << endl;
        //cout << "SSS0 " << this << " " << &m_doc.m_stateStack << " " << m_doc.m_stateStack.size() << "," << m_doc.m_stateStack.capacity() << endl;
    }
    CubeDocBase m_doc;
    BaseGLWidget m_gl;
    ModelControlBase m_modelGl;
    RunContext m_runctx;
    
    GLHandler *m_handler;
    
    bool m_requested = false;
    void requestDraw()
    {
        if (m_requested)
            return;
        m_requested = true;
        EM_ASM(requestAnim());
    }
    
    void draw(float delta) {
        m_requested = false;
        try {
            m_requested = m_modelGl.m_buildCtrl.fadeTimeout(delta);
            m_gl.paint(false);
        }
        catch(const std::exception& e) {
            cout << "GOT-EXCEPTION " << e.what() << endl;
        }    
    }
    
    /*void slvReady()
    {
        //m_doc.getCurrentSolve()->genPainter();
        m_modelGl.reCalcSlvMinMax();
    }*/
};


MainCtrl *g_ctrl = nullptr;

void sendSlvStat() {
    EM_ASM_( slvStat($0, $1), g_ctrl->m_doc.getCurrentSolveIndex(), g_ctrl->m_doc.getSolvesNumber());
}
void sendIsRunning() {
    EM_ASM_( updateIsRunning($0), g_ctrl->m_runctx.fRunning);
}

void dispFirstSlv()
{
   // g_ctrl->slvReady();
    g_ctrl->m_doc.getCurrentSolve()->genPainter(); // needed for minMax
    g_ctrl->m_modelGl.reCalcSlvMinMax();
    
    g_ctrl->m_gl.switchHandler(&g_ctrl->m_modelGl);
    g_ctrl->m_gl.reset();
    sendSlvStat();
    g_ctrl->requestDraw();
}


void RunContext::notifyLastSolution(bool firstInGo)
{
    //cout << "Slv-notify" << endl;
    g_ctrl->m_doc.setCurSlvToLast();
    //dispFirstSlv(); resets the view
    sendSlvStat();
    g_ctrl->requestDraw();
}

void RunContext::notifyFullEnum() { 
    cout << "Full-enum" << endl;
    g_ctrl->m_doc.setCurSlvToLast();

    sendSlvStat();
    g_ctrl->requestDraw();
}
void RunContext::notifyNotEnoughPieces() {
    cout << "Not enough pieces" << endl;
    g_ctrl->m_doc.clearSlvs();
    sendSlvStat();
    g_ctrl->requestDraw();
}

extern "C" { // ----------------------- interface to js -------------------------




// read a string with just sc,rt for th
void loadSlvSimple(const char* str)
{
    g_ctrl->m_doc.openSimple(str);

    g_ctrl->m_doc.setCurSlvToLast();
    //g_ctrl->slvReady();
    g_ctrl->requestDraw();
}

void solveGo() {
    try {
        g_ctrl->m_doc.solveGo();
        g_ctrl->m_runctx.runAll(); // find first solution
        dispFirstSlv();
    }
    catch(const std::exception& e) {
        cout << "GO-GOT-EXCEPTION " << e.what() << endl;
    }      
}

void populatePicsSide(PicBucket& bucket)
{
    for(auto& fam: bucket.families) 
    {
        string initials;
        char lc = 0;
        for(auto c: fam.name) {
            // first two letters of every word
            if ((c >= 'A' && c <= 'Z') || (lc >= 'A' && lc <= 'Z') )
                initials += c;
            lc = c;
        }
        fam.ctrlId = initials;
        EM_ASM_(picsAddFamily(Pointer_stringify($0), Pointer_stringify($1)), fam.name.c_str(), initials.c_str());
        for(int gi = 0; gi < fam.numGroups; ++gi)
        {
            int grpi = fam.startIndex + gi;
            auto& grp = bucket.grps[grpi];
            string dispName = grp.name.substr(grp.name.find('-') + 1);
            EM_ASM_(picsAddCube(Pointer_stringify($0), Pointer_stringify($1), $2), dispName.c_str(), grp.name.c_str(), grpi);
        }
    }
    
    for(const auto& fam: bucket.families) 
    {
        // need to do this after all the additions, otherwise it is reset for some reason
        if (fam.onResetSetCount > 0) {
            M_CHECK(fam.onResetSetCount == 1); // setting count not supported
            EM_ASM_(setFamCheck(Pointer_stringify($0), true), fam.ctrlId.c_str());
        }
    }    
}

bool initCubeEngine(const char* stdpcs, const char* unimesh)
{
    try {

        auto& bucket = PicBucket::mutableInstance();

      //  if (!bucket.loadXML(stdpcs))
      //      return false;

        populatePicsSide(bucket);
            
            
        return true;
    }
    catch(const std::exception& e) {
        cout << "INIT-GOT-EXCEPTION " << e.what() << endl;
        return false;
    }        
}

void resizeGl(int width, int height) 
{
    g_ctrl->m_gl.resize(width, height);
    g_ctrl->m_gl.invalidateChoice();
    g_ctrl->requestDraw();
}

bool cpp_draw(float deltaSec);

void mouseDown(int rightButton, int x, int y) {
    g_ctrl->m_gl.mousePress(rightButton, x, y);
    g_ctrl->requestDraw();
}
void mouseUp(int rightButton) {
    g_ctrl->m_gl.mouseRelease(rightButton);
    g_ctrl->requestDraw();
}
void mouseMove(int buttons, int ctrlPressed, int x, int y) {

    bool needUpdate = g_ctrl->m_gl.mouseMove(buttons, ctrlPressed, x, y);
    // always need draw since swapBuffers is automatic and doChoise clears the view
    if (needUpdate || g_ctrl->m_gl.m_screenNeedUpdate) {
        g_ctrl->requestDraw();
        cpp_draw(0);
        g_ctrl->m_gl.m_screenNeedUpdate = false;
    }
}
void mouseDblClick(int ctrlPressed, int x, int y) {
    bool needUpdate = g_ctrl->m_gl.mouseDoubleClick(ctrlPressed, x, y);
    sendIsRunning();
    if (needUpdate)
        g_ctrl->requestDraw();
        
#ifdef __EMSCRIPTEN_TRACING__
    emscripten_trace_report_memory_layout();
    emscripten_trace_report_off_heap_data();
#endif        
}
void mouseWheel(int delta) {
    g_ctrl->m_gl.mouseWheelEvent(delta);
    g_ctrl->requestDraw();
}

double getTms() {
    return (double)g_ctrl->m_runctx.m_stats.tms;
}


void cpp_start(int pcsCount)
{
    g_ctrl = new MainCtrl;
 
#ifdef __EMSCRIPTEN_TRACING__
    // C:\lib\Emscripten\emscripten-trace-collector-master
    emscripten_trace_configure("http://127.0.0.1:5000/", "HappyCubeSolver");
#endif    
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
//    attr.premultipliedAlpha = false;
    attr.alpha = false;
    auto ctx = emscripten_webgl_create_context("mycanvas", &attr);
    auto ret = emscripten_webgl_make_context_current(ctx);

    
    try {
        g_ctrl->m_gl.m_cullFace = false;
        g_ctrl->m_gl.init();

        
        auto& bucket = PicBucket::createSingleton();
        bucket.pdefs.reserve(pcsCount);
        bucket.loadUnifiedJs();
        
        g_ctrl->m_modelGl.initTex();        
        
    }
    catch(const std::exception& e) {
        cout << "START_GOT-EXCEPTION " << e.what() << endl;
    }        
}

void conf(bool rand) 
{
    g_ctrl->m_doc.m_conf.engine.fRand = rand;
    g_ctrl->m_doc.m_conf.engine.nPersist = PERSIST_ONLY_ONE; // otherwise solutions just stores past solutions which we don't care about
}

bool cpp_draw(float deltaSec) 
{
    if (g_ctrl->m_requested)
        g_ctrl->draw(deltaSec);
    return g_ctrl->m_requested;
}

// after restartSolve
bool cpp_slvrun()
{
    sendIsRunning();
    bool again = false;
    if (g_ctrl->m_runctx.fRunning) {
        again = g_ctrl->m_runctx.runSome();  
    }
    sendIsRunning();
    return again;
}

void setGrpCount(int grpi, int count)
{
    auto& bucket = PicBucket::mutableInstance();
    if (grpi < 0 || grpi >= bucket.grps.size()) {
        cout << "no-such-cube(sc) " << grpi << endl;
        return;
    }
    //cout << "PIC-SEL " << grpi << " " << count << endl;
    auto& grp = bucket.grps[grpi];
    for(auto pi: grp.picsi) {
        bucket.pdefs[pi].setSelected(count);
    }
}

void setPicCount(int grpi, int relIndex, int count)
{
    auto& bucket = PicBucket::mutableInstance();
    if (grpi < 0 || grpi >= bucket.grps.size()) {
        cout << "ERR no-such-cube(sp) " << grpi << endl;
        return;
    }
    auto& grp = bucket.grps[grpi];
    if (relIndex > grp.picsi.size()) {
        cout << "ERR relIndex too big " << relIndex << " " << grpi << endl;
        return;
    }
    cout << "SET-PIC grp=" << grpi << " defi=" << grp.picsi[relIndex] << " = " << count << endl;
    bucket.pdefs[grp.picsi[relIndex]].setSelected(count);
}


// a stack of pushd vectors, each the size of pdefs that holds the info about the pieces selection
// (used in editor to save the last selection
// two ops in the same func to save exported symbols and have the stack static
void stackState(int op) 
{
    if (op == 0) {
        g_ctrl->m_doc.pushState();
    }
    else {
        g_ctrl->m_doc.popState();
        g_ctrl->requestDraw();
    }
    g_ctrl->m_modelGl.m_buildCtrl.reloadWorld();
    
}

void setEditAction(int a)
{
    //cout << "EDIT "  << a << endl;
    g_ctrl->m_modelGl.m_buildCtrl.m_bEditEnabled = (a != 0);
    g_ctrl->m_modelGl.m_buildCtrl.m_bBoxRemove = (a == 2);
}

void runningRestart()
{
    // if its running it means it still did not find a solution
    if (g_ctrl->m_doc.isSlvEngineRunning()) {
        g_ctrl->m_modelGl.restartSolve(true, false); // with starter
    }
}

#define NR_KEEP_PREV 1

void newRestart(int flags) 
{
    g_ctrl->m_modelGl.restartSolve(false, (flags & NR_KEEP_PREV) != 0); // without starter template solution
}
void stopSlvRun() 
{
    g_ctrl->m_doc.solveStop();
}

int serializeCurrent()
{
    static string s;
    s = g_ctrl->m_doc.serializeMinBin();
    //cout << "**** " << s.size() << "::";
    for(int i = 0;i < s.size(); ++i) {
        //cout << (uint32_t)(uint8_t)s[i] << " ";
        EM_ASM_( scratchArr.push($0), (uint8_t)s[i] );
    }
    cout << endl;
    return 0;
}

// AQoAAAgAQAAAAARAAQAQAAEAEAQAAQAgCARBHXRogCOG8YptBGEKWzg=
void deserializeAndLoad(int len) // shape and solution
{
    string s;
    s.resize(len);
    for(int i = 0; i < s.size(); ++i) {
        s[i] = EM_ASM_INT( return scratchArr[$0], i);
    }
    g_ctrl->m_doc.loadMinBin(s);
    g_ctrl->m_modelGl.m_buildCtrl.reloadWorld();
    
    g_ctrl->m_doc.setCurSlvToLast();
    dispFirstSlv();
    g_ctrl->m_gl.reCalcProj(true);
    g_ctrl->requestDraw();
}

struct Tooth {
    Tooth(uint8_t _x, uint8_t _y, int8_t p0, int8_t p1, int8_t p2=-1, int8_t p3=-1) 
        :x(_x), y(_y), plen(0)
    {
        p[plen++] = p0; p[plen++] = p1;
        if (p2 != -1)
            p[plen++] = p2;
        if (p3 != -1)
            p[plen++] = p3;            
    }
    uint8_t x, y, plen;
    int8_t p[4];
} teeth[] = {
    Tooth(1, 1, 0,1),   Tooth(2, 1, 0,1), Tooth(3, 1, 0,1), Tooth(4, 1, 0,1), Tooth(5,1, 0,1,3),
    Tooth(6, 1, 0,3),   Tooth(7, 1, 0,3), Tooth(8, 1, 0,3), Tooth(9, 1, 0,3,5),  
    Tooth(10,1, 0,5),   Tooth(11,1, 0,5), Tooth(12,1, 0,5), Tooth(13,1, 0,5),    
                                                            
    Tooth(1, 5, 0,1,2), Tooth(2, 5, 1,2), Tooth(3, 5, 1,2), Tooth(4, 5, 1,2), Tooth(5,5, 1,2,3,4),
    Tooth(6, 5, 3,4),   Tooth(7, 5, 3,4), Tooth(8, 5, 3,4), Tooth(9, 5, 3,4,5,6),
    Tooth(10,5, 5,6),   Tooth(11,5, 5,6), Tooth(12,5, 5,6), Tooth(13,5, 5,6,0),    
                                                           
    Tooth(1, 9, 0,2),   Tooth(2, 9, 0,2), Tooth(3, 9, 0,2), Tooth(4, 9, 0,2), Tooth(5,9, 0,2,4), 
    Tooth(6, 9, 0,4),   Tooth(7, 9, 0,4), Tooth(8, 9, 0,4), Tooth(9, 9, 0,4,6), 
    Tooth(10,9, 0,6),   Tooth(11,9, 0,6), Tooth(12,9, 0,6), Tooth(13,9, 0,6),   
                                         
    Tooth(1, 2, 0,1),   Tooth(1, 3, 0,1), Tooth(1, 4, 0,1),
    Tooth(1, 6, 0,2),   Tooth(1, 7, 0,2), Tooth(1, 8, 0,2),    
                                         
    Tooth(5, 2, 1,3),   Tooth(5, 3, 1,3), Tooth(5, 4, 1,3),
    Tooth(5, 6, 2,4),   Tooth(5, 7, 2,4), Tooth(5, 8, 2,4),
                                         
    Tooth(9, 2, 3,5),   Tooth(9, 3, 3,5), Tooth(9, 4, 3,5),
    Tooth(9, 6, 4,6),   Tooth(9, 7, 4,6), Tooth(9, 8, 4,6),
                                         
    Tooth(13,2, 0,5),   Tooth(13,3, 0,5), Tooth(13,4, 0,5),
    Tooth(13,6, 0,6),   Tooth(13,7, 0,6), Tooth(13,8, 0,6),    
};

#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))

void getToothPossibilities(int x, int y)
{
    int tlen = ARRAY_SIZE(teeth);
    for(int i = 0; i < tlen; ++i) 
    {
        if (teeth[i].x == x && teeth[i].y == y) {
            for(int j = 0; j < teeth[i].plen; ++j) {
                EM_ASM_(toothScratch.push($0), teeth[i].p[j]);
            }
            return;
        }
    }
}

void readCubeToEditor(int grpi, const char* defaultCubeBits)
{
    auto& bucket = PicBucket::mutableInstance();
    if (grpi < 0) {
        cout << "no-such-cube(rde) " << grpi << endl;
        return;
    }
    string bits;
    BinWriter sigParse(bits);
    
    if (grpi >= bucket.grps.size() || bucket.grps[grpi].editorData.piecesFrame.empty()) 
    {// wasn't created yet, use default pieces cube
        string sbits(defaultCubeBits);
        M_CHECK(!sbits.empty());
        sigParse.unrepr(sbits);
    }
    else {
        bits = bucket.grps[grpi].editorData.piecesFrame;
    }
    
    cout << "GOTSIG " << sigParse.repr() << endl;
    M_CHECK(bits.size() == 9);

    int tlen = ARRAY_SIZE(teeth);
    for(int i = 0; i < tlen; ++i) 
    {
        Vec2i tpos(teeth[i].x, teeth[i].y);
        int plen = teeth[i].plen;
        uint valIndex = sigParse.readBits((plen == 2)?1:2);
        int pp = teeth[i].p[valIndex]; 
        EM_ASM_(cube[$0][$1] = $2, tpos.y, tpos.x, pp);
    }    
}


// relates to the pieces in the js `cube` array, 0 is the frame
Vec2i pieceOrigin[] = { Vec2i(-1,-1), Vec2i(1,1), Vec2i(1,5),
                                      Vec2i(5,1), Vec2i(5,5),
                                      Vec2i(9,1), Vec2i(9,5) };

// take the current cube edit data and update PicBucket
void readCubeFromEditor(int grpi) // fromEditor
{
    PicArr pcs[6];
    for(int i = 0; i < 6; ++i)
        pcs[i].fillCenter(); // just for better display
    
    string cubeSig;
    int countBits = 0;
    BinWriter sigWriter(cubeSig);

    int tlen = ARRAY_SIZE(teeth);
    for(int i = 0; i < tlen; ++i) 
    {
        Vec2i tpos(teeth[i].x, teeth[i].y);
        int val = EM_ASM_INT(return cube[$0][$1], tpos.y, tpos.x);
        int plen = teeth[i].plen;
        uint valIndex = 0; // what index in p was the value of this tooth found in
        int pp = -1;
        // search of the option we found in the cube among the options of the tooth
        for(int pi = 0; pi < plen; ++pi) // options in this tooth
        {
            pp = teeth[i].p[pi]; 
            valIndex = pi;
            if (val == pp)
                break;
        }
        M_CHECK(pp != -1); // found it, otherwise its missing from teeth?
        if (pp != 0) // 1 in the cube array is piece 0, etc', 0 means its the frame
        {
            Vec2i inpic = tpos - pieceOrigin[pp];
            // go to the piece referenced and set the tooth in it
            pcs[pp - 1].set(inpic.x, inpic.y) = 1;
        }
        countBits += (plen == 2)?1:2;
        sigWriter.addBits(valIndex, (plen == 2)?1:2); // there are either 2,3,4 values possible for the tooth so either 1 or 2 bits required
        //cout << "--" << valIndex << "," << ((plen == 2)?1:2) << "  =" << hex << cubeSig << dec << endl;
    }
    sigWriter.flush();
    cout << countBits << " SIG=" << sigWriter.repr() << endl;
    auto& bucket = PicBucket::mutableInstance();
    bucket.updateGrp(grpi, pcs, true); // from editor, we want to reCompress every time so that we'll have a working set
    bucket.grps[grpi].editorData.piecesFrame = cubeSig;
    
#ifdef __EMSCRIPTEN_TRACING__
    emscripten_trace_report_memory_layout();
    emscripten_trace_report_off_heap_data();
#endif
}

int readCubeFromSig(int grpi, const char* sig, const char* name)
{
    PicArr pcs[6];
    for(int i = 0; i < 6; ++i)
        pcs[i].fillCenter(); // just for better display
        
    string bits;
    BinWriter sigParse(bits);
  //  cout << "READ " << grpi << " " << sig << endl;
    sigParse.unrepr(sig);    

    M_CHECK(bits.size() == 9);

    int tlen = ARRAY_SIZE(teeth);
    for(int i = 0; i < tlen; ++i) 
    {
        Vec2i tpos(teeth[i].x, teeth[i].y);
        int plen = teeth[i].plen;
        uint valIndex = sigParse.readBits((plen == 2)?1:2);
        int pp = teeth[i].p[valIndex]; 
        if (pp != 0) // 1 in the cube array is piece 0, etc', 0 means its the frame
        {
            Vec2i inpic = tpos - pieceOrigin[pp];
            // go to the piece referenced and set the tooth in it
            pcs[pp - 1].set(inpic.x, inpic.y) = 1;
        }
    }   
    
    auto& bucket = PicBucket::mutableInstance();
    grpi = bucket.updateGrp(grpi, pcs, false); // do the meshes and compress only on the last one (no need to do each time)
    bucket.grps[grpi].editorData.piecesFrame = bits;
    bucket.grps[grpi].name = name;
    
    return grpi;
}

void postReadAllPics()
{
    auto& bucket = PicBucket::mutableInstance();
    bucket.doReCompress();
}

void bucketAddFam(const char* name, int startDefi, int count, int resetSelCount)
{
    auto& bucket = PicBucket::mutableInstance();
    bucket.families.resize(bucket.families.size() + 1);
    PicFamily& f = bucket.families.back();
    f.name = name;
    f.startIndex = startDefi;
    f.numGroups = count;
    f.onResetSetCount = resetSelCount;
}


void freeMeshAllocator()
{
    PicDisp::g_smoothAllocator.clear();
}

shared_ptr<GlTexture> g_lastTexture;

void textureParamCube(int grpi, int dtype, float r1, float g1, float b1, float r2, float g2, float b2, int isBlack, 
                      const char* backHex, const char* frontHex, const char* blackSelect, int rotate, const char* url)
{
    auto& bucket = PicBucket::mutableInstance();
    if (grpi < 0 || grpi >= bucket.grps.size()) {
        cout << "no-such-cube(tpc) " << grpi << endl;
        return;
    }

    PicGroupDef& cgrp = bucket.grps[grpi];
    auto prevDrawType = cgrp.drawtype;

    cgrp.color = Vec3(r1, g1, b1);
    cgrp.exColor = Vec3(r2, g2, b2);
    cgrp.drawtype = (EDrawType)dtype;
    cgrp.blackness = (isBlack == 0)?BLACK_NOT:BLACK_ONE;
    auto& ed = cgrp.editorData;

    switch(cgrp.drawtype) {
    case DRAW_COLOR: 
        cgrp.gtex.reset(); break;
    case DRAW_TEXTURE_BLEND:
    case DRAW_TEXTURE_MARBLE: 
        M_CHECK(bucket.gtexs.size() > 0);
        cgrp.gtex = bucket.gtexs[0]; break;
    case DRAW_TEXTURE_INDIVIDUAL_HALF:
        cgrp.gtex = g_lastTexture; 
        if (prevDrawType != DRAW_TEXTURE_INDIVIDUAL_HALF) {
            bucket.makeAllComp();
            // need to redo the compressed pics since the symmetry consideration changed for some pieces
        }
        break;
    default:
        cout << "Unexpected drawtype " << cgrp.drawtype << endl;    
    }
    
    ed.url = url; // save these even though drawType is not texture since we might get back to texture later
    ed.rotate = rotate;
    ed.backHex = backHex;
    ed.frontHex = frontHex; // without #
    ed.blackSelect = blackSelect; // "black", "white", "auto" from GUI
    
    
    g_ctrl->requestDraw();
}

void textureParamToEditor(int grpi) {
    auto& bucket = PicBucket::mutableInstance();
    if (grpi < 0 || grpi >= bucket.grps.size()) {
        cout << "no-such-cube(tpe) " << grpi << endl;
        return;
    }
    PicGroupDef& cgrp = bucket.grps[grpi];
    auto& ed = cgrp.editorData;
    EM_ASM_(jsgrp.colorBack.setColor('#'+Pointer_stringify($0)), ed.backHex.c_str());
    EM_ASM_(jsgrp.colorFront.setColor('#'+Pointer_stringify($0)), ed.frontHex.c_str());
    EM_ASM_(editBlackSel.value = Pointer_stringify($0), ed.blackSelect.c_str());
    EM_ASM_(jsgrp.drawType = $0, cgrp.drawtype);
    EM_ASM_(jsgrp.rotAngle = $0, ed.rotate);
    EM_ASM_(jsgrp.lastUrl = Pointer_stringify($0), ed.url.c_str());
    
    EM_ASM_(jsgrp.imgOffset.x = $0; jsgrp.imgOffset.y=$1, cgrp.editorData.imageOffset.x, cgrp.editorData.imageOffset.y);
    EM_ASM_(jsgrp.imgZoom = $0, cgrp.editorData.imageZoom);    
}

int getCubeTextureHandle(int grpi, int width, int height)
{
    auto& bucket = PicBucket::mutableInstance();
    if (grpi < 0 || grpi >= bucket.grps.size()) {
        cout << "no-such-cube " << grpi << endl;
        return 0;
    }
    PicGroupDef& cgrp = bucket.grps[grpi];
    
    if (cgrp.gtex.get() != nullptr) {
        const auto& sz = cgrp.gtex->size();
        if (sz.x == width && sz.y == height) { // exists with the same size, don't recreate it
            return cgrp.gtex->handle();
        }
    }
    cout << "creating New-Tex " << grpi << " " << cgrp.gtex.get() << " drawType=" << cgrp.drawtype << endl;
    auto curTex = make_shared<GlTexture>();
    g_lastTexture = curTex;
    curTex->init(GL_TEXTURE_2D, Vec2i(width, height), 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, nullptr, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);

    if (cgrp.drawtype == DRAW_TEXTURE_INDIVIDUAL_HALF)
        cgrp.gtex = curTex; // we might be updating it in the editor start but the drawType is something else
    g_ctrl->requestDraw();
    return curTex->handle();  
}

Vec2 baseFramePicPos[] = { {1,1}, {1,5}, {5,1}, {5,5}, {9,1}, {9,5} };

void readCubeTexCoord(int grpi, int imgOffsetX, int imgOffsetY, double imgZoom, int imgWidth, int imgHeight, int TSZ)
{
    auto& bucket = PicBucket::mutableInstance();
    if (grpi < 0 || grpi >= bucket.grps.size()) {
        cout << "no-such-cube (ctc)" << grpi << endl;
        return;
    }
    PicGroupDef& cgrp = bucket.grps[grpi];
    auto& ed = cgrp.editorData;

    ed.imageOffset = Vec2i(imgOffsetX, imgOffsetY);
    ed.imageZoom = imgZoom;

    double imgRatio = ((double)imgHeight/(double)imgWidth) / (11.0/15.0);

    for(int i = 0; i < 6; ++i) {
        auto& pic = cgrp.getPic(i);

        pic.texX = baseFramePicPos[i].x / (15.0 * imgZoom) - imgOffsetX / ((15.0*TSZ) * imgZoom);
        pic.texY = baseFramePicPos[i].y / (11.0 * imgRatio*imgZoom) - imgOffsetY / ((11.0*TSZ) * imgRatio*imgZoom);
            
        pic.texScaleX = 5.0/(15.0*imgZoom);
        pic.texScaleY = 5.0/(11.0*imgRatio*imgZoom); 
    }
    g_ctrl->requestDraw();
}

void goToSlv(int n) {
    g_ctrl->m_doc.setCurSlvTo(n);
    sendSlvStat();
    g_ctrl->requestDraw();
}


} // extern "C"

