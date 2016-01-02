#include <emscripten.h>
#include <html5.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <iostream>
#include <exception>

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


extern "C" {

unsigned int vtxBuf = 0;
int attrVtx = 0;

void cpp_start()
{
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    auto ctx = emscripten_webgl_create_context("mycanvas", &attr);
    auto ret = emscripten_webgl_make_context_current(ctx);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepthf(1.0);
    glEnable(GL_DEPTH_TEST);
    
    const char* vshader = R"(
precision highp float;
attribute vec3 aVtxPos;

void main(void) {
    // vec4 tpos = uMVMatrix * vec4(aVtxPos, 1.0);
    gl_Position = vec4(aVtxPos, 1.0); //uPMatrix * tpos;
}    
    )";
    const char* fshader = R"(
precision highp float;
void main(void) {
    gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}    
    )";
    
    auto prog = glCreateProgram();
    glAttachShader(prog, makeShader(GL_VERTEX_SHADER, vshader));
    glAttachShader(prog, makeShader(GL_FRAGMENT_SHADER, fshader));
    glLinkProgram(prog);
    
    
    attrVtx = glGetAttribLocation(prog, "aVtxPos");
    glEnableVertexAttribArray(attrVtx);
    //prog.uMVMat = gl.getUniformLocation(prog, "uMVMatrix");    
    //prog.uPMat = gl.getUniformLocation(prog, "uPMatrix");
    glUseProgram(prog);
    
    // create model
    static float vtxData[] = { 0,0,0, 0.5,0,0, 0.5,0.5,0 };

    glGenBuffers(1, &vtxBuf);
    glBindBuffer(GL_ARRAY_BUFFER, vtxBuf);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), &vtxData, GL_STATIC_DRAW);
    //model.vtxBuf.numItems = vtxData.length / 3    
}
bool cpp_progress(float deltaSec) 
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glBindBuffer(GL_ARRAY_BUFFER, vtxBuf);
    glVertexAttribPointer(attrVtx, 3, GL_FLOAT, false, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);    
    return true;
}

} // extern "C"

