#include "Mesh.h"
#include "OpenGL/Shaders.h"
#include "OpenGL/glGlob.h"


static uint glType(Mesh::Type t) {
    switch (t){
    case Mesh::LINES: return GL_LINES;
    case Mesh::TRIANGLES: return GL_TRIANGLES;
#ifdef QT_CORE_LIB
    case Mesh::QUADS: return GL_QUADS;
#endif
    case Mesh::TRI_STRIP: return GL_TRIANGLE_STRIP;
    case Mesh::TRI_FAN: return GL_TRIANGLE_FAN;
    default: throw HCException("bad mesh type");
    }
}

void Mesh::calcTrianglesNormals() {
    m_normals.clear();
    m_normals.resize(m_vtx.size());
    for(int i = 0, ni = 0; i < m_idx.size(); i += 3) {
        Vec3 fn = Vec3::triangleNormal(m_vtx[m_idx[i + 2]], m_vtx[m_idx[i + 1]], m_vtx[m_idx[i]]);
        fn.unitize();
        m_normals[m_idx[i]] += fn;
        m_normals[m_idx[i + 1]] += fn;
        m_normals[m_idx[i + 2]] += fn;
    }
    for(int i = 0; i < m_normals.size(); ++i)
        m_normals[i].unitize();
}


void Mesh::paint(bool names) const
{
    const vector<Vec3> *vtx = &m_vtx, *normals = &m_normals;
    //const vector<Vec2> *texCoord = &m_texCoord;
    if (m_common) {
        vtx = &m_common->vtx;   
        normals = &m_common->normals;
    //    texCoord = &m_common->texCoord;
    }
    if (vtx->size() == 0) {
#ifdef EMSCRIPTEN
        cout << "vtx->size()==0 " << m_common << endl;
        throw HCException("mesh is empty");
#endif
        return;
    }

    BaseProgram* bprog = ShaderProgram::currentt<BaseProgram>();
    NoiseSlvProgram* nprog = ShaderProgram::currenttTry<NoiseSlvProgram>();
    BuildProgram* lprog = ShaderProgram::currenttTry<BuildProgram>(); 

    mglCheckErrors("set-uni");
    m_vtxBo.setData(&(*vtx)[0], vtx->size());
    bprog->vtx.setArr(m_vtxBo);

    if (nprog != nullptr) {
        if (m_hasNormals) {
            m_normBo.setData(&(*normals)[0], normals->size());
            nprog->normal.setArr(m_normBo);
        }
        else 
            nprog->normal.disableArr();
    }
    if (names) {
        if (m_hasNames) {
            m_colBo.setData(&m_name[0], m_name.size());
            bprog->colorAatt.setArr<Vec4b>(m_colBo);
        }
        else {
            bprog->colorAatt.disableArr();
        }
        if (lprog != nullptr) {
            lprog->tag.set(0);
            lprog->tag.disableArr();
        }
    }
    else {
        if (m_hasColors) {
            m_colBo.setData(&m_name[0], m_name.size());
            bprog->colorAatt.setArr<Vec4b>(m_colBo);
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

        if (lprog != nullptr) {
            if (m_hasTag) {
                m_tagBo.setData(&m_tag[0], m_tag.size());
                lprog->tag.setArr(m_tagBo);
            }
            else {
                lprog->tag.set(0);
                lprog->tag.disableArr();
            }
        }

    }

    mglCheckErrors("bufs");

    uint gltype = glType(m_type);
    if (m_hasIdx) {
        if (m_idxBuf == 0) {
            glGenBuffers(1, const_cast<uint*>(&m_idxBuf));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxBuf);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_idx.size() * sizeof(ushort), &m_idx[0], GL_STATIC_DRAW);
        }
        else {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxBuf);

        }
        glDrawElements(gltype, m_idx.size(), GL_UNSIGNED_SHORT, 0);
    }
    else {
        glDrawArrays(gltype, 0, vtx->size());
    }

/*
    for(int i = 0; i < m_addIdx.size(); ++i) {
        const IdxBuf& idx = m_addIdx[i];
        if (!idx.m_enabled)
            continue;
        glDrawElements(glType(idx.m_type), idx.m_idx.size(), GL_UNSIGNED_SHORT, &idx.m_idx[0]);
    }
    */

    mglCheckErrors("draw");

}


int Mesh::elemSize() {
    switch (m_type){
    case LINES: return 2; break;
    case TRIANGLES: return 3; break;
    case QUADS: return 4; break;
    default: throw HCException("bad mesh type");
    }
}
int Mesh::numElem() {
    int es = elemSize();
    if ((m_idx.size() % es) != 0)
        throw HCException("bad idx size");
    return m_idx.size() / es;
}
