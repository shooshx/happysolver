#include "Shaders.h"

#include "cubes.vtx.glsl.h"
#include "cubes.frag.glsl.h"
#include "build.vtx.glsl.h"
#include "build.frag.glsl.h"
#include "flat.vtx.glsl.h"
#include "flat.frag.glsl.h"
#include "cubesNoise.vtx.glsl.h"
#include "cubesNoise.frag.glsl.h"


void SlvProgram::getCodes() {
	m_vtxprog += code_cubes_vtx_glsl;
	m_fragprog += code_cubes_frag_glsl;
}


void BuildProgram::getCodes() {
	m_vtxprog += code_build_vtx_glsl;
	m_fragprog += code_build_frag_glsl;
}


void FlatProgram::getCodes() {
	m_vtxprog += code_flat_vtx_glsl;
	m_fragprog += code_flat_frag_glsl;
}


NoiseSlvProgram::NoiseSlvProgram()
	:noisef("noisef"), colorA("colorA"), colorB("colorB"), drawtype("drawtype")
{}

void NoiseSlvProgram::getCodes() {
	m_vtxprog += code_cubesNoise_vtx_glsl;
	m_fragprog += code_cubesNoise_frag_glsl;
}

void NoiseSlvProgram::successLink() {
	noisef.getLocation(progId());
	colorA.getLocation(progId());
	colorB.getLocation(progId());
	drawtype.getLocation(progId());
}
