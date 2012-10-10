#include "Mesh.h"

#include <QGLWidget>


void Mesh::paint(bool names) const
{
	if (m_vtx.size() == 0)
		return;

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &m_vtx[0]);

	if (m_hasNormals) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, &m_normals[0]);
	}
	else {
		glDisableClientState(GL_NORMAL_ARRAY);
	}

	if (m_hasTexCoord) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, &m_texCoord[0]);
	}
	else {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	if (names) {
		if (m_hasNames) {
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(3, GL_UNSIGNED_BYTE, 0, &m_name[0]);
		}
	}
	else {
		if (m_hasColors) {
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(4, GL_FLOAT, 0, &m_color4[0]);
		}
		else {
			glDisableClientState(GL_COLOR_ARRAY);
			if (m_uniformColor) {
				glColor4fv(m_uColor.v);	
			}
		}
	}

	if (m_hasIdx) {
		glDrawElements(m_type, m_idx.size(), GL_UNSIGNED_INT, &m_idx[0]);
	}
	else {
		glDrawArrays(m_type, 0, m_vtx.size());
	}

}
