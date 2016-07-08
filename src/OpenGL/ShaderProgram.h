#pragma once
#ifndef SHADERPROGRAM_H_INCLUDED
#define SHADERPROGRAM_H_INCLUDED


#include <vector>

using namespace std;

#include "GLTexture.h"
#include "../Mat.h"



class ProgCompileConf
{
public:
    ProgCompileConf() :geomInput(0),geomOutput(0),geomVtxCount(0) {}
    ProgCompileConf(uint gIn, uint gOut, uint gCnt)
        :geomInput(gIn), geomOutput(gOut), geomVtxCount(gCnt)
    {}
    uint geomInput;
    uint geomOutput;
    uint geomVtxCount;
};

class ShaderProgram;

class GlBuffer
{
public:
    GlBuffer(int type) : m_type(type) {}
    void bind();
    template<typename T>
    bool setData(const vector<T>& v);

    uint m_buf = 0;
    uint m_type;
    uint m_size = 0; // number of elements of type T in the buffer

};

class GlArrayBuffer : public GlBuffer
{
public:
    GlArrayBuffer() : GlBuffer(GL_ARRAY_BUFFER) {}
};
class GlElementArrayBuffer : public GlBuffer
{
public:
    GlElementArrayBuffer() : GlBuffer(GL_ELEMENT_ARRAY_BUFFER) {}
};



class ShaderParam 
{
public:
    ShaderParam(const string& name, ShaderProgram* prog);
    virtual void getLocation(uint progId) = 0;
    const string& name() { return m_name; }
    bool isValid() const {
        return m_uid != -1; 
    }
    int id() { return m_uid; }

protected:
    int m_uid;
    string m_name;
};


class UniformParam : public ShaderParam
{
public:
    UniformParam(const string& name, ShaderProgram* prog) : ShaderParam(name, prog) {}
    virtual void getLocation(uint progId);

    template<typename T> 
    void set(const T& v) const;
};


class AttribParam : public ShaderParam
{
public:
    AttribParam(const string& name, ShaderProgram* prog) : ShaderParam(name, prog) 
    {
       
    }
    virtual void getLocation(uint progId);

    template<typename T>
    void set(const T& v) const;

    template<typename T> 
    void setArr(const GlArrayBuffer& bo) const;
    void disableArr();
    void enableArr();

};


void shadersInit();

class ShaderProgram
{
public:
    ShaderProgram() : m_progId(0), m_isOk(false)
    {}
    virtual ~ShaderProgram() 
    {
        clear();
    }

    virtual void clear();
    virtual int getClass() const = 0; // used for avoiding dynamic_cast

    static bool hasCurrent() { return g_current != nullptr; }
    static ShaderProgram* current() { return g_current; };
    template<typename T>
    static T* currentt() { 
        if (g_current == nullptr)
            return nullptr;
        //T* c = dynamic_cast<T*>(g_current);
        if (g_current->getClass() != T::tClass()) {
            cout << "ERROR: Wrong program type!" << endl;
            //DebugBreak();
            throw HCException("Wrong program type");
        }
        return static_cast<T*>(g_current); 
    };
    template<typename T>
    static T* currenttTry() { 
        if (g_current == nullptr || g_current->getClass() != T::tClass())
            return nullptr;
        T* c = static_cast<T*>(g_current);
        return c; 
    };

    bool init(const ProgCompileConf& conf = ProgCompileConf());
    bool isOk() const { 
        return m_isOk; 
    }

    uint progId() const { return m_progId; }
    void addParam(ShaderParam* p) {
        m_params.push_back(p);
    }

    typedef vector<string> TCodesList;
    
    // populate m_vtxprog, m_geomprog, m_fragprog
    virtual void getCodes() = 0;
    // called after successfull linkage
    virtual void successLink() {
        for (auto it = m_params.begin(); it != m_params.end(); ++it)
            (*it)->getLocation(progId());
    }

    static void shadersInit();

protected: 
    void use() const;
    void unuse() const;

    static bool printShaderInfoLog(uint obj);
    static bool printProgramInfoLog(uint obj);

    uint m_progId;

    TCodesList m_vtxprog, m_geomprog, m_fragprog;
    bool m_isOk;
    vector<uint> m_createdShaders; // for later deletion.
    vector<ShaderParam*> m_params; // members params that were registerd

    static int g_users; // global. can't use more than one program at a time
    static ShaderProgram *g_current;

    friend class ProgramUser;
};


class ProgramUser
{
public:
    ProgramUser(const ShaderProgram *prog = nullptr) : m_prog(prog) 
    {
        if (m_prog != nullptr)
            m_prog->use();
    }
    void use(const ShaderProgram *prog)
    {
        if (prog == nullptr)
        {
            dispose();
            return;
        }
        m_prog = prog;
        m_prog->use();
    }

    ~ProgramUser()
    {
        if (m_prog != nullptr)
            m_prog->unuse();
    }
    void dispose()
    {
        if (m_prog != nullptr)
            m_prog->unuse();
        m_prog = nullptr;
    }

private:
    const ShaderProgram *m_prog;
};

// add definitions to a shader source (#define)
/*
class ShaderDefines {
public:
    ShaderDefines() {}
    void add(const string& d) {
        m_source += string("#define ") + d + "\n";
    }
    const char* c_str() const  { 
        return m_source.c_str(); 
    }
private:
    string m_source;
};
*/


class FloatAttrib : public AttribParam {
public:
    FloatAttrib(const char* name, ShaderProgram* prog) : AttribParam(name, prog) {}
    void set(float v) const;
    void setArr(const GlArrayBuffer& bo) const {
        AttribParam::setArr<float>(bo);
    }
};
class Vec3Attrib : public AttribParam {
public:
    Vec3Attrib(const char* name, ShaderProgram* prog) : AttribParam(name, prog) {}
    void setArr(const GlArrayBuffer& bo) const {
        AttribParam::setArr<Vec3>(bo);
    }
};
class IntAttrib : public AttribParam {
public:
    IntAttrib(const char* name, ShaderProgram* prog) : AttribParam(name, prog) {}
    void setArr(const GlArrayBuffer& bo) const {
        AttribParam::setArr<int>(bo);
    }
};



class Vec3Uniform : public UniformParam {
public:
    Vec3Uniform(const char* name, ShaderProgram* prog) : UniformParam(name, prog) {}
    void set(const Vec3& v) const;
};

class Vec2Uniform : public UniformParam {
public:
    Vec2Uniform(const char* name, ShaderProgram* prog) : UniformParam(name, prog) {}
    void set(const Vec2& v) const;
};

class IntUniform : public UniformParam {
public:
    IntUniform(const char* name, ShaderProgram* prog) : UniformParam(name, prog) {}
    void set(int v) const;
};
class FloatUniform : public UniformParam {
public:
    FloatUniform(const char* name, ShaderProgram* prog) : UniformParam(name, prog) {}
    void set(float v) const;
};

class Mat4Uniform : public UniformParam {
public:
    Mat4Uniform(const char* name, ShaderProgram* prog) : UniformParam(name, prog) {}
    void set(const Mat4& v) const;
};

class Mat3Uniform : public UniformParam {
public:
    Mat3Uniform(const char* name, ShaderProgram* prog) : UniformParam(name, prog) {}
    void set(const Mat3& v) const;
};

class Mat2Uniform : public UniformParam {
public:
    Mat2Uniform(const char* name, ShaderProgram* prog) : UniformParam(name, prog) {}
    void set(const Mat2& v) const;
};

void mglActiveTexture(int i);







#endif // SHADERPROGRAM_H_INCLUDED
