#include "Mesh.h"
#include "OpenGL/Shaders.h"
#include "OpenGL/glGlob.h"
#include <fstream>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

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

void Mesh::makeSelfBos(bool andDealloc) {
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

    if (andDealloc) {
        m_vtx.clear();
        m_vtx.shrink_to_fit();
        m_normals.clear();
        m_normals.shrink_to_fit();
        m_idx.clear();
        m_idx.shrink_to_fit();

        m_name.clear();
        m_name.shrink_to_fit();
        m_color4.clear();
        m_color4.shrink_to_fit();
        m_tag.clear();
        m_tag.shrink_to_fit();
    }
}

void Mesh::CommonData::makeSelfBos()
{
    m_vtxBo.setData(vtx);
    vtx.clear();
    vtx.shrink_to_fit();
    m_normBo.setData(normals);
    normals.clear();
    normals.shrink_to_fit();
}

void Mesh::makeIdxBo(bool dealloc)
{
    m_idxBo.setData(m_idx);
    m_idx.clear();
    m_idx.shrink_to_fit();
}


void Mesh::paint(bool names) const
{
    const GlArrayBuffer* vbo = &m_vtxBo;
    const GlArrayBuffer* nbo = &m_normBo;

    if (m_common) {
        vbo = &m_common->m_vtxBo;
        nbo = &m_common->m_normBo;
    }
    if (vbo->m_size == 0) {
#ifdef EMSCRIPTEN
      //  cout << "vtx->size()==0 " << m_common << endl;
      //  throw HCException("mesh is empty");
#endif
        return;
    }

    BaseProgram* bprog = static_cast<BaseProgram*>(ShaderProgram::current());
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
    //cout << "~1-----------" << endl;
    if (names) {
      //  cout << "~2" << endl;
        if (lprog != nullptr) 
        {
            if (m_hasNames) {
                lprog->colorAatt.setArr<Vec4b>(m_namesBo);
            }
            else {
                lprog->colorAatt.disableArr();
            }
            lprog->tag.set(0);
            lprog->tag.disableArr();
        }
    }
    else {
        if (m_hasColors) {
            if (lprog != nullptr) {
                lprog->colorAatt.setArr<Vec4>(m_colBo);
            }
        }
        else 
        {
            if (m_uniformColor) {
                if (bprog->colorAu.isValid()) {
                    bprog->colorAu.set(m_uColor);
                }
            }
            else if (m_singleAttColor) {
                // happens in build that needs to set a uniform color to the lines
                if (lprog != nullptr) {
                    lprog->colorAatt.disableArr(); // disable it as an array to get a single color below
                    lprog->colorAatt.set(m_singleColorForAtt);
                }
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


void Mesh::save(const string& path, bool asObj)
{
    ofstream f(path.c_str());
    if (!f.good()) {
        cout << "Failed opening path for mesh save " << path << endl;
        return;
    }
    M_ASSERT(m_vtx.size() > 0 && m_idx.size() > 0);

    // short hand format for unification
    for (int i = 0; i < m_vtx.size(); ++i) {
        Vec3& v = m_vtx[i];
        Vec3& n = m_normals[i];
        //Vec2& t = m_mesh.m_texCoord[i];
        if (asObj) {
            f << "v " << v.x << " " << v.y << " " << v.z << "\n"; // obj
            f << "vn " << n.x << " " << n.y << " " << n.z << "\n";
        }
        else {
            f << "v "  << v.x << " " << v.y << " " << v.z << " " << n.x << " " << n.y << " " << n.z << "\n"; // << " " << t.x << " " << t.y << "\n";
        }
        
    }

    if (m_type == QUADS)
    {
        if ((m_idx.size() % 4) != 0)
            throw HCException("bad size");
        for (int i = 0; i < m_idx.size(); i += 4) {
            if (asObj) {
                f << "f " << m_idx[i] + 1 << "//" << m_idx[i] + 1 << " " << m_idx[i + 1] + 1 << "//" << m_idx[i + 1] + 1 << " " << m_idx[i + 2] + 1 << "//" << m_idx[i + 2] + 1 << "\n";  // obj
                f << "f " << m_idx[i] + 1 << "//" << m_idx[i] + 1 << " " << m_idx[i + 2] + 1 << "//" << m_idx[i + 2] + 1 << " " << m_idx[i + 3] + 1 << "//" << m_idx[i + 3] + 1 << "\n";  // obj
               // f << "f " << m_idx[i] + 1 << " " << m_idx[i + 2] + 1 << " " << m_idx[i + 3] + 1 << "\n"; // obj
            }
            else {
                f << m_idx[i] << " " << m_idx[i + 1] << " " << m_idx[i + 2] << " " << m_idx[i + 3] << "\n";
            }
        }
    }
    else {
        throw HCException("can't save triangles");
    }
}


void Mesh::load(const char* name) 
{
#ifdef EMSCRIPTEN
    // this is rather uglu looking it up in window each time but there's not much choice...
    // possibly replace with emscripten::val

    m_vtxBo.bind();
    EM_ASM_(GLctx.bufferData(GLctx.ARRAY_BUFFER, window[Pointer_stringify($0)].vtx, GLctx.STATIC_DRAW), name);
    m_vtxBo.m_size = EM_ASM_INT(return window[Pointer_stringify($0)].vtx.length, name);

    m_normBo.bind();
    EM_ASM_(GLctx.bufferData(GLctx.ARRAY_BUFFER, window[Pointer_stringify($0)].norm, GLctx.STATIC_DRAW), name);
    m_normBo.m_size = EM_ASM_INT(return window[Pointer_stringify($0)].norm.length, name);

    m_idxBo.bind();
    EM_ASM_(GLctx.bufferData(GLctx.ELEMENT_ARRAY_BUFFER, window[Pointer_stringify($0)].idx, GLctx.STATIC_DRAW), name);
    m_idxBo.m_size = EM_ASM_INT(return window[Pointer_stringify($0)].idx.length, name);
    m_type = Mesh::TRIANGLES;
    m_hasIdx = m_hasNormals = true;
    
#endif
}

