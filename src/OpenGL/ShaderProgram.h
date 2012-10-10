#pragma once
#ifndef SHADERPROGRAM_H_INCLUDED
#define SHADERPROGRAM_H_INCLUDED

#include <qtglobal>
#include <QVector>
#include <QString>

#include "GLTexture.h"

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

	bool init(const ProgCompileConf& conf = ProgCompileConf());
	bool isOk() const { return m_isOk; }

	uint progId() const { return m_progId; }

	typedef QList<QString> TCodesList;
	
	// populate m_vtxprog, m_geomprog, m_fragprog
	virtual void getCodes() = 0;
	// called after successfull linkage
	virtual void successLink() = 0;

protected: 
	void use() const;
	void unuse() const;

	static bool printShaderInfoLog(uint obj);
	static bool printProgramInfoLog(uint obj);

	uint m_progId;
	EProgType m_type;
	TCodesList m_vtxprog, m_geomprog, m_fragprog;
	bool m_isOk;
	QList<uint> m_createdShaders; // for later deletion.

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



class ShaderParam 
{
public:
	ShaderParam(const QString& name) :m_uid(-1), m_name(name)
	{}
	virtual void getLocation(uint progId) = 0;
	QString& name() { return m_name; }

protected:
	uint m_uid;
	QString m_name;
};


class UniformParam : public ShaderParam
{
public:
	UniformParam(const QString& name) : ShaderParam(name) {}
	virtual void getLocation(uint progId);

	template<typename T>
	void set(const T& v) const;
};


class AttribParam : public ShaderParam
{
public:
	AttribParam(const QString& name) : ShaderParam(name) {}
	virtual void getLocation(uint progId);

	template<typename T>
	void set(const T& v) const;
};




class FloatAttrib : public AttribParam
{
public:
	FloatAttrib(const char* name) : AttribParam(name) {}
	void set(float v) const;
};

class Vec3Uniform : public UniformParam
{
public:
	Vec3Uniform(const char* name) : UniformParam(name) {}
	void set(const Vec3& v) const;
};

class Vec2Uniform : public UniformParam
{
public:
	Vec2Uniform(const char* name) : UniformParam(name) {}
	void set(const Vec2& v) const;
};

class IntUniform : public UniformParam
{
public:
	IntUniform(const char* name) : UniformParam(name) {}
	void set(int v) const;
};


void mglActiveTexture(int i);

#if 0

class GenericShader : public ShaderProgram
{
public:
	GenericShader() {}

	TCodesList& vtxprogs() { return m_vtxprog; }
	TCodesList& fragprogs() { return m_fragprog; }
	TCodesList& geomprogs() { return m_geomprog; }

	virtual void clear() 
	{
		ShaderProgram::clear();
		foreach(ShaderParam* p, m_params)
			delete p;
		m_params.clear();
	}

	void addParam(ShaderParam* p, int index)
	{
		if (m_params.size() <= index)
			m_params.resize(index + 1);
		m_params[index] = p;
	}

	template<typename T>
	bool setUniform(const T& val, int index)
	{
		if (!isOk())
			return false;
		if (index >= m_params.size())
			return false;
		UniformParam* p = dynamic_cast<UniformParam*>(m_params[index]);
		if (p == NULL)
		{
			printf("Param %s is not a uniform\n", m_params[index]->name().toAscii().data());
			return false;
		}
		p->set(val);
		return true;
	}

	template<typename T>
	bool setAttrib(const T& val, int index)
	{
		if (!isOk())
			return false;
		if (index >= m_params.size())
			return false;
		AttribParam* p = dynamic_cast<AttribParam*>(m_params[index]);
		if (p == NULL)
		{
			printf("Param %s is not an attribute\n", m_params[index]->name().toAscii().data());
			return false;
		}
		p->set(val);
		return true;
	}


protected:
	virtual void getCodes() {}
	virtual void successLink() 
	{
		foreach(ShaderParam* p, m_params)
			p->getLocation(m_progId);
	}

private: 
	QVector<ShaderParam*> m_params; // memory leaks

};



class AllShader;
typedef QVector<AllShader*> TShaderList;

class ShadersManager
{
public:
	ShadersManager();
	~ShadersManager();
	void init();

	AllShader* operator[](int p)
	{
		if (p >= m_prog.size())
			return NULL;
		return m_prog[p];
	}
	void append(AllShader* s)
	{
		m_prog.append(s);
	}

	void bindTexture(int index, const float* data, int size);

	void bindTexture(int index, const QVector<float>& dat)
	{
		bindTexture(index, dat.data(), dat.size());
	}

	TShaderList& lst() { return m_prog; }
	const GlTexture* tex(int index) const { return m_texEng[index]; }

private:
	TShaderList m_prog;
	QVector<GlTexture*> m_texEng; // map the index of the texture engine to the Texture bound.
};
#endif





#endif // SHADERPROGRAM_H_INCLUDED
