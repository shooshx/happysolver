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


class MainCtrl
{
public:
    MainCtrl() : m_modelGl(&m_gl, &m_doc)
    {
        m_gl.m_handlers.push_back(&m_modelGl);
        m_handler = &m_modelGl;
    }
    CubeDocBase m_doc;
    BaseGLWidget m_gl;
    ModelControlBase m_modelGl;
    
    GLHandler *m_handler;
    
    void draw() {
        try {
            m_gl.paint();
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

extern "C" {

unsigned int vtxBuf = 0;
int attrVtx = 0;

void loadSolution(const char* buf) {
    try {
        bool hasSlv = false;
        if (!g_ctrl.m_doc.realOpen(buf, &hasSlv)) {
            cout << "error: " << g_ctrl.m_doc.m_lastMsg << endl;
            cout << "doc realOpen failed" << endl;
        }
        g_ctrl.slvReady();
        
        g_ctrl.m_gl.switchHandler(&g_ctrl.m_modelGl);
        g_ctrl.m_gl.reset();
    }
    catch(const std::exception& e) {
        cout << "LOAD-GOT-EXCEPTION " << e.what() << endl;
    }        
}

bool initCubeEngine(const char* stdpcs, const char* unimesh)
{
    try {
        PicBucket::createSingleton();
        
        g_ctrl.m_modelGl.initTex();
        
        if (!PicBucket::mutableInstance().loadXML(stdpcs))
            return false;
            
        PicBucket::mutableInstance().loadUnifiedJs();    
        return true;
    }
    catch(const std::exception& e) {
        cout << "INIT-GOT-EXCEPTION " << e.what() << endl;
        return false;
    }        
}

void resize(int width, int height) 
{
    g_ctrl.m_gl.resize(width, height);
}

void mouseDown(int rightButton, int x, int y) {
    g_ctrl.m_gl.mousePress(rightButton, x, y);
    g_ctrl.draw();
}
void mouseUp(int rightButton) {
    g_ctrl.m_gl.mouseRelease(rightButton);
    g_ctrl.draw();
}
void mouseMove(int buttons, int ctrlPressed, int x, int y) {
    bool needUpdate = g_ctrl.m_gl.mouseMove(buttons, ctrlPressed, x, y);
    if (needUpdate)
        g_ctrl.draw();
}

void cpp_start()
{
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    auto ctx = emscripten_webgl_create_context("mycanvas", &attr);
    auto ret = emscripten_webgl_make_context_current(ctx);
    
    try {
        g_ctrl.m_gl.init();
        
    }
    catch(const std::exception& e) {
        cout << "START_GOT-EXCEPTION " << e.what() << endl;
    }        
}
bool cpp_progress(float deltaSec) 
{
    g_ctrl.draw();
    return true;
}

} // extern "C"

