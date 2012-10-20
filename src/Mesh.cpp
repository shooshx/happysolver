#include "Mesh.h"
#include "OpenGL/Shaders.h"
#include "OpenGL/glGlob.h"

#include <QGLWidget>


void Mesh::paint(bool names) const
{
	if (m_vtx.size() == 0)
		return;

	BaseProgram* bprog = ShaderProgram::currentt<BaseProgram>();
	NoiseSlvProgram* nprog = ShaderProgram::currenttTry<NoiseSlvProgram>();

	bprog->vtx.setArr(&m_vtx[0]);

	if (nprog != NULL) {
		if (m_hasNormals) {
			nprog->normal.setArr(&m_normals[0]);
		}
		else {
			nprog->normal.disableArr();
		}
	}

// 	if (m_hasTexCoord) {
// 		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
// 		glTexCoordPointer(2, GL_FLOAT, 0, &m_texCoord[0]);
// 	}
// 	else {
// 		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
// 	}

	if (names) {
		if (m_hasNames) {
			bprog->colorAatt.setArr(&m_name[0]);
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
	}

	mglCheckErrors("bufs");

	if (m_hasIdx) {
		glDrawElements(m_type, m_idx.size(), GL_UNSIGNED_INT, &m_idx[0]);
	}
	else {
		glDrawArrays(m_type, 0, m_vtx.size());
	}

	mglCheckErrors("draw");

}



