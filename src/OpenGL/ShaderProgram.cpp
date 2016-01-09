#include "ShaderProgram.h"
#include "glGlob.h"
#include <iostream>

using namespace std;


int ShaderProgram::g_users = 0;
ShaderProgram *ShaderProgram::g_current = nullptr;


ShaderParam::ShaderParam(const string& name, ShaderProgram* prog) :m_uid(-1), m_name(name) {
    prog->addParam(this);
}

void ShaderProgram::shadersInit() {
    static bool did = false;
    if (did) {
        return;
    }

#ifdef _WINDOWS
    uint x = glewInit();
    if (x != GLEW_OK) {
        cout << "Error: " << glewGetErrorString(x) << endl;
        throw HCException("glew init");
    }
#endif
    did = true;
}

bool ShaderProgram::printShaderInfoLog(uint obj)
{
    int infologLength = 0, charsWritten  = 0;
    string infoLog;

    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
    if (infologLength > 1)
    {
        infoLog.resize(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, (char*)infoLog.data());
        cout << "Shader Info Log:\n" << infoLog << endl;
    }
    int ret;
    glGetShaderiv(obj, GL_COMPILE_STATUS, &ret);
    if (!ret)
        cout << "Shader compile failed" << endl;
    return ret;
}

bool ShaderProgram::printProgramInfoLog(uint obj)
{
    int infologLength = 0, charsWritten  = 0;
    string infoLog;

    glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
    if (infologLength > 1)
    {
        infoLog.resize(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, (char*)infoLog.data());
        cout << "Program Info Log:\n" << infoLog << endl;
    }
    int ret;
    glGetProgramiv(obj, GL_LINK_STATUS, &ret);
    if (!ret)
        cout << "Program compile failed" << endl;
    return ret;
}

void ShaderProgram::use() const
{
    if (g_users++ != 0)
        return;
    if (!isOk()) {

        return;
    }
    mglCheckErrorsC("buse");
    glUseProgram(m_progId);
    mglCheckErrorsC("use");
    g_current = const_cast<ShaderProgram*>(this);
}
void ShaderProgram::unuse() const 
{
    if (--g_users != 0)
        return;
    glUseProgram(0);
    g_current = nullptr;
}

void ShaderProgram::clear() 
{
    m_vtxprog.clear();
    m_fragprog.clear();
    m_geomprog.clear();

    glDeleteProgram(m_progId);
    for(auto it = m_createdShaders.begin(); it != m_createdShaders.end(); ++it) {
        glDeleteShader(*it);
    }
    m_createdShaders.clear();
    m_isOk = false; 
} 

bool ShaderProgram::init(const ProgCompileConf& conf)
{
    shadersInit();

    mglCheckErrorsC("clear errors");
    const char* name = typeid(this).name();

    m_progId = glCreateProgram();

    //ShaderDefines defines;
    string defines; // not used

    getCodes(); // populate the lists, set m_type.
    m_isOk = false;

#ifdef QT_CORE_LIB
    if (!m_geomprog.empty())
    { // has geometry shaders
        glProgramParameteriEXT(m_progId, GL_GEOMETRY_INPUT_TYPE_EXT, conf.geomInput);
        glProgramParameteriEXT(m_progId, GL_GEOMETRY_OUTPUT_TYPE_EXT, conf.geomOutput);
        glProgramParameteriEXT(m_progId, GL_GEOMETRY_VERTICES_OUT_EXT, conf.geomVtxCount);	
    }
#endif
    mglCheckErrorsC(string("codes ") + name);


    for (int i = 0; i < m_vtxprog.size(); ++i)
    {
        uint vso = glCreateShader(GL_VERTEX_SHADER);
        const char *srcs[2] = { defines.c_str(), m_vtxprog[i].c_str() };
        glShaderSource(vso, 2, srcs, nullptr);
        glCompileShader(vso);
        glAttachShader(m_progId, vso);
        printShaderInfoLog(vso);
        m_createdShaders.push_back(vso);
    }
    mglCheckErrorsC(string("vtx ") + name);

#ifdef QT_CORE_LIB
    for (int i = 0; i < m_geomprog.size(); ++i)
    {
        uint gso = glCreateShader(GL_GEOMETRY_SHADER_EXT);
        const char *srcs[2] = { defines.c_str(), m_geomprog[i].c_str() };
        glShaderSource(gso, 2, srcs, nullptr);
        glCompileShader(gso);
        glAttachShader(m_progId, gso);
        printShaderInfoLog(gso);
        m_createdShaders.push_back(gso);
    }
    mglCheckErrorsC(string("geom ") + name);
#endif

    for (int i = 0; i < m_fragprog.size(); ++i)
    {
        uint fso = glCreateShader(GL_FRAGMENT_SHADER);
        const char *srcs[2] = { defines.c_str(), m_fragprog[i].data() };
        glShaderSource(fso, 2, srcs, nullptr);
        glCompileShader(fso);
        glAttachShader(m_progId, fso);
        printShaderInfoLog(fso);
        m_createdShaders.push_back(fso);
    }
    mglCheckErrorsC(string("frag ") + name);


    if (m_createdShaders.size() == 0)
        return m_isOk;

    glLinkProgram(m_progId);
    mglCheckErrorsC(string("link ") + name);

    m_isOk = printProgramInfoLog(m_progId);
    if (m_isOk)
    {
        cout << "Compiled OK " << m_progId << endl;
    }
    mglCheckErrorsC(string("proginfo ") + name);


    successLink();

    mglCheckErrorsC(string("vars ") + name);
    return m_isOk;

}

void AttribParam::disableArr() {
    if (m_uid != -1) {
        glDisableVertexAttribArray(m_uid);
        glVertexAttribPointer(m_uid, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, nullptr);
    }
}
void AttribParam::enableArr() {
    if (m_uid != -1) {
        glEnableVertexAttribArray(m_uid);
        glVertexAttribPointer(m_uid, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, nullptr);
    }
}

template<typename T>
uint glType();
template<> uint glType<Vec3b>() { return GL_UNSIGNED_BYTE; }
template<> uint glType<Vec4b>() { return GL_UNSIGNED_BYTE; }
template<> uint glType<Vec4>() { return GL_FLOAT; }
template<> uint glType<Vec3>() { return GL_FLOAT; }
template<> uint glType<int>() { return GL_UNSIGNED_INT; }
template<> uint glType<float>() { return GL_FLOAT; }

template<typename T>
uint getElemCount();
template<> uint getElemCount<Vec3b>() { return 3; }
template<> uint getElemCount<Vec4b>() { return 4; }
template<> uint getElemCount<Vec4>() { return 4; }
template<> uint getElemCount<Vec3>() { return 3; }
template<> uint getElemCount<int>() { return 1; }
template<> uint getElemCount<float>() { return 1; }

template<typename T>
uint shouldNorm(); 
template<> uint shouldNorm<Vec3b>() { return GL_TRUE; } // normalize to [0-1] since we give values at [0-255]
template<> uint shouldNorm<Vec4b>() { return GL_TRUE; }
template<> uint shouldNorm<Vec4>() { return GL_FALSE; }
template<> uint shouldNorm<Vec3>() { return GL_FALSE; }
template<> uint shouldNorm<int>() { return GL_FALSE; }
template<> uint shouldNorm<float>() { return GL_FALSE; }


template<typename T> 
void AttribParam::setArr(const GlArrayBuffer& bo) const {
    if (m_uid == -1) 
        return;

    glBindBuffer(GL_ARRAY_BUFFER, bo.m_buf);

    glEnableVertexAttribArray(m_uid);
    glVertexAttribPointer(m_uid, getElemCount<T>(), glType<T>(), shouldNorm<T>(), 0, 0);

}

template void AttribParam::setArr<Vec4>(const GlArrayBuffer& bo) const;
template void AttribParam::setArr<Vec3>(const GlArrayBuffer& bo) const;
template void AttribParam::setArr<Vec3b>(const GlArrayBuffer& bo) const;
template void AttribParam::setArr<Vec4b>(const GlArrayBuffer& bo) const;
template void AttribParam::setArr<float>(const GlArrayBuffer& bo) const;

template<> 
void UniformParam::set(const float& v) const {
    // when calling this, a program should be in use
    if (m_uid != -1)
        glUniform1f(m_uid, v);
}

template<> 
void UniformParam::set(const Vec3& v) const {
    if (m_uid != -1)
        glUniform3fv(m_uid, 1, v.v);
}

template<> 
void UniformParam::set(const Vec2& v) const {
    if (m_uid != -1)
        glUniform2fv(m_uid, 1, v.v);
}

template<> 
void UniformParam::set(const Vec4& v) const {
    if (m_uid != -1)
        glUniform4fv(m_uid, 1, v.v);
}

template<> 
void UniformParam::set(const int& v) const {
    if (m_uid != -1)
        glUniform1i(m_uid, v);
}

template<>
void UniformParam::set(const Mat4& v) const {
    if (m_uid != -1)
        glUniformMatrix4fv(m_uid, 1, false, v.m);
}

template<>
void UniformParam::set(const Mat3& v) const {
    if (m_uid != -1)
        glUniformMatrix3fv(m_uid, 1, false, v.m);
}


template<> 
void AttribParam::set(const float& v) const {
    if (m_uid != -1)
        glVertexAttrib1f(m_uid, v);
}

template<> 
void AttribParam::set(const Vec3& v) const {
    if (m_uid != -1)
        glVertexAttrib3fv(m_uid, v.v);
}

template<> 
void AttribParam::set(const Vec2& v) const {
    if (m_uid != -1)
        glVertexAttrib2fv(m_uid, v.v);
}

template<> 
void AttribParam::set(const Vec4& v) const {
    if (m_uid != -1)
        glVertexAttrib4fv(m_uid, v.v);
}

#ifdef QT_CORE_LIB
template<> 
void AttribParam::set(const int& v) const {
    if (m_uid != -1)
        glVertexAttrib1s(m_uid, v); // EH?
}
#endif

void FloatAttrib::set(float v) const {
    AttribParam::set(v);
}


void Vec3Uniform::set(const Vec3& v) const {
    UniformParam::set(v);
}
void Vec2Uniform::set(const Vec2& v) const {
    UniformParam::set(v);
}
void IntUniform::set(int v) const {
    UniformParam::set(v);
}
void FloatUniform::set(float v) const {
    UniformParam::set(v);
}
void Mat4Uniform::set(const Mat4& v) const {
    UniformParam::set(v);
}
void Mat3Uniform::set(const Mat3& v) const {
    UniformParam::set(v);
}



void UniformParam::getLocation(uint progId)
{
    if (m_name.empty())
        return;
    m_uid = glGetUniformLocation(progId, m_name.c_str());
    if (m_uid == -1)
        cout << "WARNING: uniform '" << m_name << "' location is -1!" << endl;
}



void AttribParam::getLocation(uint progId)
{
    if (m_name.empty())
        return;
    m_uid = glGetAttribLocation(progId, m_name.c_str());
    if (m_uid == -1)
        cout << "WARNING: attribute '" << m_name << "' location is -1!" << endl;
}


void mglActiveTexture(int i) {
    glActiveTexture(GL_TEXTURE0 + i);
}

void GlBuffer::bind()
{
    if (m_buf == 0)
        glGenBuffers(1, const_cast<uint*>(&m_buf));

    glBindBuffer(m_type, m_buf);
}

template<typename T>
bool GlBuffer::setData(const vector<T>& v)
{
    m_size = v.size();
    if (v.size() == 0) {
        return false;
    }
    if (m_buf == 0)
        glGenBuffers(1, const_cast<uint*>(&m_buf));

    glBindBuffer(m_type, m_buf);
    glBufferData(m_type, v.size() * sizeof(T), &v[0], GL_STATIC_DRAW);
    return true;
}

template bool GlBuffer::setData(const vector<Vec3>& v);
template bool GlBuffer::setData(const vector<Vec4>& v);
template bool GlBuffer::setData(const vector<float>& v);
template bool GlBuffer::setData(const vector<Vec4b>& v);
template bool GlBuffer::setData(const vector<ushort>& v);

