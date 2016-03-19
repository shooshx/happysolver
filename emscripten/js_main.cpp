#include <emscripten.h>
#include <html5.h>

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
    virtual void notifyFullEnum() override { 
        cout << "Full-enum" << endl;
    }
    virtual void notifyNotEnoughPieces() override {
        cout << "Not enough pieces" << endl;
    }

    virtual void doStart() override {
        init();
        EM_ASM(requestSlvRun());
    }
    virtual void doWait() override {
        runAll();
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
    cout << "Slv-notify" << endl;
    g_ctrl.m_doc.setCurSlvToLast();
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
            M_ASSERT(fam.onResetSetCount == 1); // setting count not supported
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
    //cout << "PIC-SEL " << grpi << " " << count << endl;
    auto& bucket = PicBucket::mutableInstance();
    auto& grp = bucket.grps[grpi];
    for(auto pi: grp.picsi) {
        bucket.pdefs[pi].setSelected(count);
    }
    

}

void setEditAction(int a)
{
    //cout << "EDIT "  << a << endl;
    g_ctrl.m_modelGl.m_buildCtrl.m_bEditEnabled = (a != 0);
    g_ctrl.m_modelGl.m_buildCtrl.m_bBoxRemove = (a == 2);
}

void restartSolve()
{
    g_ctrl.m_modelGl.restartSolve();
}

} // extern "C"

