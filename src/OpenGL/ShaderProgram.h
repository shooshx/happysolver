#pragma once
#ifndef SHADERPROGRAM_H_INCLUDED
#define SHADERPROGRAM_H_INCLUDED


#include <vector>

using namespace std;

#include "GLTexture.h"
#include "../Mat.h"

enum EProgType
{
	PTYPE_NOLIMIT,
	PTYPE_POINTS,
	PTYPE_TRIANGLES
};

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

class ShaderParam 
{
public:
	ShaderParam(const QString& name, ShaderProgram* prog);
	virtual void getLocation(uint progId) = 0;
	QString& name() { return m_name; }
	bool isValid() const {
		return m_uid != -1; 
	}
	uint id() { return m_uid; }

protected:
	uint m_uid;
	QString m_name;
};


class UniformParam : public ShaderParam
{
public:
	UniformParam(const QString& name, ShaderProgram* prog) : ShaderParam(name, prog) {}
	virtual void getLocation(uint progId);

	template<typename T> 
	void set(const T& v) const;
};


class AttribParam : public ShaderParam
{
public:
	AttribParam(const QString& name, ShaderProgram* prog) : ShaderParam(name, prog) {}
	virtual void getLocation(uint progId);

	template<typename T>
	void set(const T& v) const;

	template<typename T> 
	void setArr(const T* v) const;
	void disableArr();
	void enableArr();
};


void shadersInit();

class ShaderProgram
{
public:
	ShaderProgram() : m_progId(0), m_type(PTYPE_NOLIMIT), m_isOk(false)
	{}
	virtual ~ShaderProgram() 
	{
		clear();
	}

	virtual void clear();
	
	//bool isCurrent() const { return g_current == this; }
	virtual int type() const { return m_type; }

	static bool hasCurrent() { return g_current != NULL; }
	static ShaderProgram* current() { return g_current; };
	template<typename T>
	static T* currentt() { 
		if (g_current == NULL)
			return NULL;
		T* c = dynamic_cast<T*>(g_current);
		if (c == NULL) {
			printf("ERROR: Wrong program type!");
			DebugBreak();
		}
		return c; 
	};
	template<typename T>
	static T* currenttTry() { 
		if (g_current == NULL)
			return NULL;
		T* c = dynamic_cast<T*>(g_current);
		return c; 
	};

	bool init(const ProgCompileConf& conf = ProgCompileConf());
	bool isOk() const { return m_isOk; }

	uint progId() const { return m_progId; }
	void addParam(ShaderParam* p) {
		m_params.push_back(p);
	}

	typedef QList<QString> TCodesList;
	
	// populate m_vtxprog, m_geomprog, m_fragprog
	virtual void getCodes() = 0;
	// called after successfull linkage
	virtual void successLink() {
		for (auto it = m_params.begin(); it != m_params.end(); ++it)
			(*it)->getLocation(progId());
	}

protected: 
	void use() const;
	void unuse() const;

	static bool printShaderInfoLog(uint obj);
	static bool printProgramInfoLog(uint obj);

	uint m_progId;
	EProgType m_type;
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
	ProgramUser(const ShaderProgram *prog = NULL) : m_prog(prog) 
	{
		if (m_prog != NULL)
			m_prog->use();
	}
	void use(const ShaderProgram *prog)
	{
		if (prog == NULL)
		{
			dispose();
			return;
		}
		m_prog = prog;
		m_prog->use();
	}

	~ProgramUser()
	{
		if (m_prog != NULL)
			m_prog->unuse();
	}
	void dispose()
	{
		if (m_prog != NULL)
			m_prog->unuse();
		m_prog = NULL;
	}

private:
	const ShaderProgram *m_prog;
};

// add definitions to a shader source (#define)

class ShaderDefines
{
public:
	ShaderDefines()
	{
		m_source = QString().toAscii();
	}
	void add(const QString& d)
	{
		m_source += QString("#define %1\n").arg(d);
	}
	const char* c_str() const 
	{ 
		return m_source.data(); 
	}
private:
	QByteArray m_source;
};



class FloatAttrib : public AttribParam {
public:
	FloatAttrib(const char* name, ShaderProgram* prog) : AttribParam(name, prog) {}
	void set(float v) const;
};
class Vec3Attrib : public AttribParam {
public:
	Vec3Attrib(const char* name, ShaderProgram* prog) : AttribParam(name, prog) {}
	void setArr(const Vec3* v) const;
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


void mglActiveTexture(int i);







#endif // SHADERPROGRAM_H_INCLUDED
