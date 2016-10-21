#pragma once

#include <memory>
#include <vector>
#include <map>
#include "Vec.h"
#include "OpenGL/ShaderProgram.h"

using namespace std;

typedef unsigned short ushort;
typedef unsigned int uint;

class AttribParam;



class Mesh
{
public:
    Mesh() 
    {}

    void clear() {
        m_common.reset();
        m_vtx.clear();
        m_normals.clear();
        //m_texCoord.clear();
        m_color4.clear();
        m_name.clear();
        m_tag.clear();
        m_idx.clear();
        m_uColor = Vec3();
        m_addIdx.clear();
        m_singleColorForAtt = Vec4();
    }

    void paint(bool names = false) const;

    int elemSize();
    int numElem();
    void save(const string& path, bool asObj);

    enum Type { NONE, LINES, TRIANGLES, QUADS, TRI_STRIP, TRI_FAN };
    struct IdxBuf {
        IdxBuf(Type t) :m_type(t), m_enabled(true) {}
        vector<ushort> m_idx;
        Type m_type;
        bool m_enabled;
    };

    IdxBuf& addIdx(Type t) {
        m_addIdx.push_back(IdxBuf(t));
        return m_addIdx.back();
    }
    void calcTrianglesNormals();

    struct CommonData {
        vector<Vec3> vtx;
        vector<Vec3> normals;
        mutable GlArrayBuffer m_vtxBo; // buffer object
        mutable GlArrayBuffer m_normBo;
       
        void makeSelfBos();
    };

    void makeSelfBos(bool andDealloc);
    void makeIdxBo(bool dealloc);

    void load(const char* name);

public:
    shared_ptr<CommonData> m_common;
    vector<Vec3> m_vtx;

    vector<Vec3> m_normals;  // same size as vtx
    //vector<Vec2> m_texCoord; // same size as vtx NOT USED, using vertex position instead
    vector<Vec4> m_color4;
    vector<Vec4b> m_name;
    vector<float> m_tag; // general purpose data used by the shaders - type of build tile
    vector<ushort> m_idx;
    Vec3 m_uColor;
    Vec4 m_singleColorForAtt;

    bool m_hasNormals = false;
    //bool m_hasTexCoord;
    bool m_hasColors = false;
    bool m_hasNames = false;
    bool m_hasIdx = false; // drawElements or drawArrays
    bool m_uniformColor = false;
    bool m_hasTag = false;
    bool m_singleAttColor = false;

    Type m_type = NONE;

    vector<IdxBuf> m_addIdx;

    GlArrayBuffer m_vtxBo; // buffer object
    GlArrayBuffer m_normBo;
    GlArrayBuffer m_colBo; 
    GlArrayBuffer m_namesBo;
    GlArrayBuffer m_tagBo;
    GlElementArrayBuffer m_idxBo;
};



class VecRep
{
public:
    VecRep(vector<Vec3>* arr) :m_arr(arr) {}

    struct VecWrap {
        VecWrap(const Vec3& _v) :v(_v) {}
        bool operator==(const VecWrap& o) const {
            return v.isNear(o.v);
        }
        bool operator<(const VecWrap& o) const {
            if (nearDist(v.x, o.v.x)) {
                if (nearDist(v.y, o.v.y)) {
                    if (nearDist(v.z, o.v.z))
                        return false;
                    return v.z < o.v.z;
                }
                return v.y < o.v.y;
            }
            return v.x < o.v.x;
        }

        Vec3 v;
    };

    bool add(const Vec3& v, int* index) {
        auto it = m_add.find(VecWrap(v));
        if (it == m_add.end()) {
            m_arr->push_back(v);
            *index = m_arr->size() - 1;
            m_add[VecWrap(v)] = *index;
            return true;
        }
        *index = it->second;
        return false;
    }

    map<VecWrap, int> m_add;
    vector<Vec3>* m_arr;
};


