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

class RunContext : public SolveContext
{
public:
    virtual void notifyLastSolution(bool firstInGo) override;
    virtual void notifyFullEnum() override;
    virtual void notifyNotEnoughPieces() override;

    virtual void doStart() override {
        init();
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
    
    void slvReady()
    {
        m_doc.getCurrentSolve()->genPainter();
        m_modelGl.reCalcSlvMinMax();
    }
};

MainCtrl g_ctrl;

void RunContext::notifyLastSolution(bool firstInGo)
{
    //cout << "Slv-notify" << endl;
    g_ctrl.m_doc.setCurSlvToLast();
    g_ctrl.requestDraw();
}

void RunContext::notifyFullEnum() { 
    cout << "Full-enum" << endl;
    g_ctrl.m_doc.setCurSlvToLast();
    //g_ctrl.m_doc.clearSlvs();
    g_ctrl.requestDraw();
}
void RunContext::notifyNotEnoughPieces() {
    cout << "Not enough pieces" << endl;
    g_ctrl.m_doc.clearSlvs();
    g_ctrl.requestDraw();
}

extern "C" { // ----------------------- interface to js -------------------------

unsigned int vtxBuf = 0;
int attrVtx = 0;

void dispFirstSlv()
{
    g_ctrl.slvReady();
    g_ctrl.m_gl.switchHandler(&g_ctrl.m_modelGl);
    g_ctrl.m_gl.reset();
    g_ctrl.requestDraw();
}

void loadSolution(const char* buf) {
    try {
        bool hasSlv = false;
        if (!g_ctrl.m_doc.realOpen(buf, &hasSlv)) {
            cout << "error: " << g_ctrl.m_doc.m_lastMsg << endl;
            cout << "doc realOpen failed" << endl;
        }
        
        dispFirstSlv();
    }
    catch(const std::exception& e) {
        cout << "LOAD-GOT-EXCEPTION " << e.what() << endl;
    }        
}

// read a string with just sc,rt for th
void loadSlvSimple(const char* str)
{
    g_ctrl.m_doc.openSimple(str);

    g_ctrl.m_doc.setCurSlvToLast();
    //g_ctrl.slvReady();
    g_ctrl.requestDraw();
}

void solveGo() {
    try {
        g_ctrl.m_doc.solveGo();
        g_ctrl.m_runctx.runAll(); // find first solution
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
        auto& bucket = PicBucket::createSingleton();
    
        g_ctrl.m_modelGl.initTex();
           
        if (!bucket.loadXML(stdpcs))
            return false;

        populatePicsSide(bucket);
            
        bucket.loadUnifiedJs();    
        return true;
    }
    catch(const std::exception& e) {
        cout << "INIT-GOT-EXCEPTION " << e.what() << endl;
        return false;
    }        
}

void resizeGl(int width, int height) 
{
    g_ctrl.m_gl.resize(width, height);
    g_ctrl.m_gl.invalidateChoice();
    g_ctrl.requestDraw();
}

bool cpp_draw(float deltaSec);

void mouseDown(int rightButton, int x, int y) {
    g_ctrl.m_gl.mousePress(rightButton, x, y);
    g_ctrl.requestDraw();
}
void mouseUp(int rightButton) {
    g_ctrl.m_gl.mouseRelease(rightButton);
    g_ctrl.requestDraw();
}
void mouseMove(int buttons, int ctrlPressed, int x, int y) {

    bool needUpdate = g_ctrl.m_gl.mouseMove(buttons, ctrlPressed, x, y);
    // always need draw since swapBuffers is automatic and doChoise clears the view
    if (needUpdate || g_ctrl.m_gl.m_screenNeedUpdate) {
        g_ctrl.requestDraw();
        cpp_draw(0);
        g_ctrl.m_gl.m_screenNeedUpdate = false;
    }
}
void mouseDblClick(int ctrlPressed, int x, int y) {
    bool needUpdate = g_ctrl.m_gl.mouseDoubleClick(ctrlPressed, x, y);
    if (needUpdate)
        g_ctrl.requestDraw();
        
#ifdef __EMSCRIPTEN_TRACING__
    emscripten_trace_report_memory_layout();
    emscripten_trace_report_off_heap_data();
#endif        
}
void mouseWheel(int delta) {
    g_ctrl.m_gl.mouseWheelEvent(delta);
    g_ctrl.requestDraw();
}

double getTms() {
    return (double)g_ctrl.m_runctx.m_stats.tms;
}

void cpp_start()
{
#ifdef __EMSCRIPTEN_TRACING__
    emscripten_trace_configure("http://127.0.0.1:5000/", "HappyCubeSolver");
#endif    
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
//    attr.premultipliedAlpha = false;
    attr.alpha = false;
    auto ctx = emscripten_webgl_create_context("mycanvas", &attr);
    auto ret = emscripten_webgl_make_context_current(ctx);
    
    try {
        g_ctrl.m_gl.m_cullFace = false;
        g_ctrl.m_gl.init();
    }
    catch(const std::exception& e) {
        cout << "START_GOT-EXCEPTION " << e.what() << endl;
    }        
}

void conf(bool rand) 
{
    g_ctrl.m_doc.m_conf.engine.fRand = rand;
    g_ctrl.m_doc.m_conf.engine.nPersist = PERSIST_ONLY_ONE; // otherwise solutions just stores past solutions which we don't care about
}

bool cpp_draw(float deltaSec) 
{
    if (g_ctrl.m_requested)
        g_ctrl.draw(deltaSec);
    return g_ctrl.m_requested;
}

bool cpp_slvrun()
{
    if (g_ctrl.m_runctx.fRunning) {
        bool again = g_ctrl.m_runctx.runSome();  
        return again;
    }
    return false;
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



// a stack of pushd vectors, each the size of pdefs that holds the info about the pieces selection
// (used in editor to save the last selection
// two ops in the same func to save exported symbols and have the stack static
void stackState(int op) 
{
    if (op == 0) {
        g_ctrl.m_doc.pushState();
    }
    else {
        g_ctrl.m_doc.popState();
        g_ctrl.requestDraw();
    }
    g_ctrl.m_modelGl.m_buildCtrl.reloadWorld();
    
}

void setEditAction(int a)
{
    //cout << "EDIT "  << a << endl;
    g_ctrl.m_modelGl.m_buildCtrl.m_bEditEnabled = (a != 0);
    g_ctrl.m_modelGl.m_buildCtrl.m_bBoxRemove = (a == 2);
}

void runningRestart()
{
    // if its running it means it still did not find a solution
    if (g_ctrl.m_doc.isSlvEngineRunning()) {
        g_ctrl.m_modelGl.restartSolve(true); // with starter
    }
}

void newRestart() 
{
    g_ctrl.m_modelGl.restartSolve(false); // without starter
}

int serializeCurrent()
{
    static string s;
    s = g_ctrl.m_doc.serializeMinBin();
    cout << "**** " << s.size() << "::";
    for(int i = 0;i < s.size(); ++i) {
        cout << (uint32_t)(uint8_t)s[i] << " ";
        EM_ASM_( scratchArr.push($0), (uint8_t)s[i] );
    }
    cout << endl;
    return 0;
}

// AQoAAAgAQAAAAARAAQAQAAEAEAQAAQAgCARBHXRogCOG8YptBGEKWzg=
void deserializeAndLoad(int len)
{
    string s;
    s.resize(len);
    for(int i = 0; i < s.size(); ++i) {
        s[i] = EM_ASM_INT( return scratchArr[$0], i);
    }
    g_ctrl.m_doc.loadMinBin(s);
    g_ctrl.m_modelGl.m_buildCtrl.reloadWorld();
    
    g_ctrl.m_doc.setCurSlvToLast();
    g_ctrl.requestDraw();
}

// relates to the pieces in the js `cube` array, 0 is the frame
Vec2i pieceOrigin[] = { Vec2i(-1,-1), Vec2i(1,1), Vec2i(1,5),
                                      Vec2i(5,1), Vec2i(5,5),
                                      Vec2i(9,1), Vec2i(9,5) };

// the the current cube edit data and update PicBucket
void readCube(int grpi)
{
    PicArr pcs[6];
    for(int i = 0; i < 6; ++i)
        pcs[i].fillCenter(); // just for better display
    int tlen = EM_ASM_INT_V(return teeth.length);
    for(int i = 0; i < tlen; ++i) {
        Vec2i tpos(EM_ASM_INT(return teeth[$0].x, i), EM_ASM_INT(return teeth[$0].y, i));
        int plen = EM_ASM_INT(return teeth[$0].p.length, i);
        for(int pi = 0; pi < plen; ++pi) {
            int pp = EM_ASM_INT(return teeth[$0].p[$1], i, pi); 
            if (pp == 0) // 1 in the cube array is piece 0, etc'
                continue;
            Vec2i inpic = tpos - pieceOrigin[pp];
            int val = EM_ASM_INT(return cube[$0][$1], tpos.y, tpos.x);
            //cout << "--" << tpos.x << "," << tpos.y << "  " << inpic.x << "," << inpic.y << "  " << val << " " << ((val == pp) ? 1:0) << endl;
            pcs[pp - 1].set(inpic.x, inpic.y) = ((val == pp) ? 1:0);
        }
    }
    auto& bucket = PicBucket::mutableInstance();
    bucket.updateGrp(grpi, pcs);
    
#ifdef __EMSCRIPTEN_TRACING__
    emscripten_trace_report_memory_layout();
    emscripten_trace_report_off_heap_data();
#endif
}

void freeMeshAllocator()
{
    PicDisp::g_smoothAllocator.clear();
}

void textureParamCube(int grpi, int dtype, float r1, float g1, float b1, float r2, float g2, float b2)
{
    auto& bucket = PicBucket::mutableInstance();
    if (grpi < 0 || grpi >= bucket.grps.size()) {
        cout << "no-such-cube " << grpi << endl;
        return;
    }

    PicGroupDef& cgrp = bucket.grps[grpi];
    cgrp.color = Vec3(r1, g1, b1);
    cgrp.exColor = Vec3(r2, g2, b2);
    cgrp.drawtype = (EDrawType)dtype;
    g_ctrl.requestDraw();
}

int getCubeTextureHandle(int grpi, int width, int height)
{
    auto& bucket = PicBucket::mutableInstance();
    if (grpi < 0 || grpi >= bucket.grps.size()) {
        cout << "no-such-cube " << grpi << endl;
        return 0;
    }
    PicGroupDef& cgrp = bucket.grps[grpi];
    cgrp.drawtype = DRAW_TEXTURE_INDIVIDUAL_WHOLE;
    
    if (cgrp.gtex.get() != nullptr) {
        const auto& sz = cgrp.gtex->size();
        if (sz.x == width && sz.y == height) { // exists with the same size, don't recreate it
            return cgrp.gtex->handle();
        }
    }
    cgrp.gtex = make_shared<GlTexture>();
    cgrp.gtex->init(GL_TEXTURE_2D, Vec2i(width, height), 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, nullptr, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    g_ctrl.requestDraw();
    return cgrp.gtex->handle();  
}

void readCubeTexCoord(int grpi)
{
    auto& bucket = PicBucket::mutableInstance();
    if (grpi < 0 || grpi >= bucket.grps.size()) {
        cout << "no-such-cube " << grpi << endl;
        return;
    }
    PicGroupDef& cgrp = bucket.grps[grpi];
    for(int i = 0; i < 6; ++i) {
        auto& pic = cgrp.getPic(i);
 
        pic.texX = EM_ASM_DOUBLE(return cubeTexInfo[$0].x, i);
        pic.texY = EM_ASM_DOUBLE(return cubeTexInfo[$0].y, i);
        pic.texScaleX = EM_ASM_DOUBLE(return cubeTexInfo[$0].scaleX, i);
        pic.texScaleY = EM_ASM_DOUBLE(return cubeTexInfo[$0].scaleY, i);
    }
    g_ctrl.requestDraw();
}

} // extern "C"

