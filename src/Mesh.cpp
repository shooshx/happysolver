#include "Mesh.h"
#include "OpenGL/Shaders.h"
#include "OpenGL/glGlob.h"

#include <QGLWidget>


static uint glType(Mesh::Type t) {
	switch (t){
	case Mesh::LINES: return GL_LINES;
	case Mesh::TRIANGLES: return GL_TRIANGLES;
	case Mesh::QUADS: return GL_QUADS;
	case Mesh::TRI_STRIP: return GL_TRIANGLE_STRIP;
	case Mesh::TRI_FAN: return GL_TRIANGLE_FAN;
	default: throw exception("bad mesh type");
	}
}


void Mesh::paint(bool names) const
{
	if (m_vtx.size() == 0)
		return;

	BaseProgram* bprog = ShaderProgram::currentt<BaseProgram>();
	NoiseSlvProgram* nprog = ShaderProgram::currenttTry<NoiseSlvProgram>();
	BuildProgram* lprog = ShaderProgram::currenttTry<BuildProgram>(); 

	bprog->vtx.setArr(&m_vtx[0]);

	if (nprog != NULL) {
		if (m_hasNormals) 
			nprog->normal.setArr(&m_normals[0]);
		else 
			nprog->normal.disableArr();
	}
	if (names) {
		if (m_hasNames) {
			//printf(" names %d\n", m_name.size());
			bprog->colorAatt.setArr(&m_name[0]);
		}
		else {
			bprog->colorAatt.disableArr();
		}
		if (lprog != NULL) {
			lprog->tag.set(0);
			lprog->tag.disableArr();
		}
	}
	else {
		if (m_hasColors) {
			bprog->colorAatt.setArr(&m_color4[0]);
		}
		else {
			bprog->colorAatt.disableArr();
			if (m_uniformColor) {
				if (bprog->colorAu.isValid()) 
					bprog->colorAu.set(m_uColor);
				else 
					bprog->colorAatt.set(m_uColor);
			}
		}
		if (lprog != NULL) {
			if (m_hasTag) 
				lprog->tag.setArr(&m_tag[0]);
			else {
				lprog->tag.set(0);
				lprog->tag.disableArr();
			}
		}
	}

	mglCheckErrors("bufs");

	uint gltype = glType(m_type);
	if (m_hasIdx) {
		glDrawElements(gltype, m_idx.size(), GL_UNSIGNED_INT, &m_idx[0]);
	}
	else {
		glDrawArrays(gltype, 0, m_vtx.size());
	}

	for(int i = 0; i < m_addIdx.size(); ++i) {
		const IdxBuf& idx = m_addIdx[i];
		if (!idx.m_enabled)
			continue;
		glDrawElements(glType(idx.m_type), idx.m_idx.size(), GL_UNSIGNED_INT, &idx.m_idx[0]);
	}

	mglCheckErrors("draw");

}


int Mesh::elemSize() {
	switch (m_type){
	case LINES: return 2; break;
	case TRIANGLES: return 3; break;
	case QUADS: return 4; break;
	default: throw exception("bad mesh type");
	}
}
int Mesh::numElem() {
	int es = elemSize();
	if ((m_idx.size() % es) != 0)
		throw exception("bad idx size");
	return m_idx.size() / es;
}
