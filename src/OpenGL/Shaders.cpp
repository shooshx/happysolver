#include "Shaders.h"
#include "../Mat.h"

#include "build.vtx.glsl.h"
#include "build.frag.glsl.h"
#include "flat.vtx.glsl.h"
#include "flat.frag.glsl.h"
#include "cubesNoise.vtx.glsl.h"
#include "cubesNoise.frag.glsl.h"


void BuildProgram::getCodes() {
	m_vtxprog.push_back(code_build_vtx_glsl);
	m_fragprog.push_back(code_build_frag_glsl);
}


void FlatProgram::getCodes() {
	m_vtxprog.push_back(code_flat_vtx_glsl);
	m_fragprog.push_back(code_flat_frag_glsl);
}

void NoiseSlvProgram::getCodes() {
	m_vtxprog.push_back(code_cubesNoise_vtx_glsl);
	m_fragprog.push_back(code_cubesNoise_frag_glsl);
}


void NoiseSlvProgram::setModelMat(const Mat4& m) {
    modelMat.set(m);
    normalMat.set(m.toNormalsTrans());
}