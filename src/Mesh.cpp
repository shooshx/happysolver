#include "Mesh.h"
#include "OpenGL/Shaders.h"
#include "OpenGL/glGlob.h"


static uint glType(Mesh::Type t) {
    switch (t){
    case Mesh::LINES: return GL_LINES;
    case Mesh::TRIANGLES: return GL_TRIANGLES;
//#ifdef QT_CORE_LIB
//    case Mesh::QUADS: return GL_QUADS;
//#endif
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

void Mesh::makeSelfBos() {
    if (!m_vtxBo.setData(m_vtx))
        return;

    if (m_hasNormals)
        m_normBo.setData(m_normals);
    if (m_hasNames)
        m_namesBo.setData(m_name);
    if (m_hasColors) 
        m_colBo.setData(m_color4);
    if (m_hasTag) 
        m_tagBo.setData(m_tag);
    if (m_hasIdx)
        m_idxBo.setData(m_idx);
}


void Mesh::paint(bool names) const
{
    const vector<Vec3> *vtx = &m_vtx, *normals = &m_normals;
    const GlArrayBuffer* vbo = &m_vtxBo;
    const GlArrayBuffer* nbo = &m_normBo;
    //const vector<Vec2> *texCoord = &m_texCoord;
    if (m_common) {
        vtx = &m_common->vtx;   
        normals = &m_common->normals;
        vbo = &m_common->m_vtxBo;
        nbo = &m_common->m_normBo;
    //    texCoord = &m_common->texCoord;
    }
    if (vbo->m_size == 0) {
#ifdef EMSCRIPTEN
      //  cout << "vtx->size()==0 " << m_common << endl;
      //  throw HCException("mesh is empty");
#endif
        return;
    }

    BaseProgram* bprog = ShaderProgram::currentt<BaseProgram>();
    NoiseSlvProgram* nprog = ShaderProgram::currenttTry<NoiseSlvProgram>();
    BuildProgram* lprog = ShaderProgram::currenttTry<BuildProgram>(); 

    mglCheckErrors("set-uni");
    bprog->vtx.setArr(*vbo);

    if (nprog != nullptr) {
        if (m_hasNormals) {
            nprog->normal.setArr(*nbo);
        }
        else 
            nprog->normal.disableArr();
    }
    if (names) {
        if (m_hasNames) {
            bprog->colorAatt.setArr<Vec4b>(m_namesBo);
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
            bprog->colorAatt.setArr<Vec4>(m_colBo);
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

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxBo.m_buf);
        glDrawElements(gltype, m_idxBo.m_size, GL_UNSIGNED_SHORT, 0);
    }
    else {
        glDrawArrays(gltype, 0, vbo->m_size);
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
