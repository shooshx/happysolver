#ifndef POINTSIZERENDERE_H__INCLUDEDE
#define POINTSIZERENDERE_H__INCLUDEDE


#include "ShaderProgram.h"


class BaseProgram : public ShaderProgram {
public:
	BaseProgram()
        :trans("trans", this), colorAatt("colorA", this), colorAu("colorA", this), vtx("vtx", this), fadeFactor("fadeFactor", this)
	{}

	Mat4Uniform trans;
	UniformParam colorAu;
	AttribParam colorAatt;
	Vec3Attrib vtx;
    FloatUniform fadeFactor; 
};


class BuildProgram : public BaseProgram {
public:
	BuildProgram()
		:tag("tag", this)
	{}
	virtual void getCodes();

	FloatAttrib tag;
	
};

class FlatProgram : public BaseProgram {
public:
	virtual void getCodes();
};

class NoiseSlvProgram : public BaseProgram {
public:
	NoiseSlvProgram()
		:noisef("noisef", this), colorB("colorB", this), drawtype("drawtype", this)
		,modelMat("modelMat", this), normalMat("normalMat", this), normal("normal", this)
		,texOffset("texOffset", this), lightPos("lightPos", this), flag("flag", this)
        ,texScale("texScale", this), texTrans("texTrans", this), isBlackSign("isBlackSign", this)
	{}
	virtual void getCodes();

	IntUniform noisef; 

	Vec3Uniform colorB;
	IntUniform drawtype;

	Mat4Uniform modelMat;
	Mat3Uniform normalMat;
	Vec3Attrib normal;
	Vec3Uniform texOffset;
	Vec3Uniform lightPos;
    IntUniform flag; // mark with red for remove
    Vec2Uniform texScale;
    Mat2Uniform texTrans;
    FloatUniform isBlackSign; // 0 for not black, -1 or 1 depending on the side of the piece facing outside
};


#if 0


class Mat4ArrUniform : public UniformParam
{
public:
	Mat4ArrUniform(const char* name) : UniformParam(name) {}
	void set(float mats[6 * 16]) const
	{
		if (m_uid != -1)
			glUniformMatrix4fv(m_uid, 6, false, mats);
	}
};


// a 1d array of data sent to the shader
class Shader1DData
{
public:
	Shader1DData(ShadersManager* mgr, const char* datName, const char* sizeName) 
		: m_dat(datName), m_datSize(sizeName), m_mgr(mgr)
	{}

	void getLocations(int progId);
	// get an index in the ShaderManages which is also the index of the texture engine
	void setData(int index) const;

	IntUniform m_dat;
	IntUniform m_datSize;

	ShadersManager *m_mgr;
};

class SLNotifiable;

class AllShader : public ShaderProgram
{
public:
	AllShader(ShadersManager* mgr = nullptr) :
		m_mats("cubeRot"),
		m_psize("psize"),
		m_polyData(mgr, "polyData", "polySize"),
		m_sphPolyData(mgr, "sphPolyData", nullptr),
		m_externP("external_p"),
		m_externS("external_s"),
		m_coneOrig("coneOrigin"),
		m_sheetData("sheet")
	{}


	void setProjMatrices(float mats[6 * 16]) const // an array with the 6 4x4 matrics
	{
		m_mats.set(mats);
	}
	void setPointSize(float size) const
	{
		m_psize.set(size);
	}
	void setExternalPoint(const Vec3& p) const
	{
		m_externP.set(p);
		VecS2 s(p);
		m_externS.set(s);
	}
	void setConeOrigin(const Vec3& p) const
	{
		m_coneOrig.set(p);
	}
	void setSheetData(int texind) const
	{
		m_sheetData.set(texind);
	}

	Shader1DData m_polyData;
	Shader1DData m_sphPolyData;

public:

	Mat4ArrUniform m_mats;

	FloatAttrib m_psize;

	Vec3Uniform m_externP;
	Vec2Uniform m_externS;

	Vec3Uniform m_coneOrig;
	IntUniform m_sheetData;

private:
	QList<SLNotifiable*> m_params;
public:
	void registerNotify(SLNotifiable* prm)
	{
		m_params.append(prm);
	}
	virtual void successLink();

	static void instantiate(ShadersManager* mgr);
};



class SLNotifiable // Successful Link Notifiable
{
public:
	SLNotifiable() {}
	SLNotifiable(AllShader* prog)
	{
		prog->registerNotify(this);
	}

	virtual void notifySL(AllShader* prog) = 0;
};


#endif

#endif // POINTSIZERENDERE_H__INCLUDEDE
