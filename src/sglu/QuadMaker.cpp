#include <iostream>
#include <map>
#include "../Vec.h"
using namespace std;


#define GL_QUAD_STRIP 0x0008
#define GL_TRIANGLE_FAN 0x0006
#define GL_POINTS 0x0000
#define GL_LINE_STRIP 0x0003
#define GLU_INVALID_VALUE 0xffff // don't know the value


float toZero(float a) {
    if (std::abs(a) < 0.000001)
        return 0;
    return a;
}

class GluTriStripAdder
{
public:
    GluTriStripAdder(ostream* os) :m_os(os) {}
    void addVtx(float x, float y, float z) 
    {
        x += m_translate.x;
        y += m_translate.y;
        z += m_translate.z;
        Vec3 v(toZero(x), toZero(y), toZero(z));

        m_prevprev = m_prev;
        m_prev = m_index;

        auto it = m_addedVtx.find(v);
        if (true) //it == m_addedVtx.end()) 
        {
            (*m_os) << "v " << v.x << " " << v.y << " " << v.z << "\n";
            m_index = m_vtxs.size() + 1;
            m_addedVtx.insert(make_pair(v, m_index));   
            m_vtxs.push_back(v);

            (*m_os) << "vn " << m_lastNormal.x << " " << m_lastNormal.y << " " << m_lastNormal.z << "\n";
            m_normals.push_back(m_lastNormal);
        }
        else {
            m_index = it->second;
        }

        m_objVtx++;
        if (m_objVtx == 1) 
            m_firstIndex = m_index; // first of this mesh
        if (m_objVtx != 1 && m_objVtx != 2)
        {
            if (m_type == GL_QUAD_STRIP) 
            {
                if ((m_objVtx % 2) != 0) // order of last two needs to change since its a quad strip (draw and understand)
                    face(m_prevprev, m_prev, m_index);
                else
                    face(m_prev, m_prevprev, m_index);
            }
            else // fan 
            {
                face(m_firstIndex, m_prev, m_index);
            }
        }
    }
    void addNormal(float x, float y, float z)
    {
        m_lastNormal = Vec3(x, y, z);
    }
    void face(int a, int b, int c) {
        if (a == b || b == c || c == a)
            return;
        (*m_os) << "f " << a << "//" << a << " " << b << "//" << b << " " << c << "//" << c << "\n";
        m_idxs.push_back(a);
        m_idxs.push_back(b);
        m_idxs.push_back(c);
    }
    void endStrip() {
        m_objVtx = 0;
    }

    void printJs(ostream& os);

    int m_type = 0; // fan or strip
    int m_index = 0; // obj starts from 1 - may accumulate multiple meshes
    int m_objVtx = 0; // count the vertices in each object added - restarted for every mesh
    int m_prev = 0, m_prevprev = 0, m_firstIndex = 0;
    ostream* m_os = nullptr;
    Vec3 m_translate;
    map<Vec3, int> m_addedVtx;
    Vec3 m_lastNormal;

    vector<Vec3> m_vtxs;
    vector<Vec3> m_normals;
    vector<int> m_idxs;
};

void GluTriStripAdder::printJs(ostream& os)
{
    os << "  vtx: new Float32Array([";
    for (const auto& v : m_vtxs)
        os << v.x << "," << v.y << "," << v.z << ",";
    os << "]),\n";

    os << "  norm: new Float32Array([";
    for (const auto& v : m_normals)
        os << v.x << "," << v.y << "," << v.z << ",";
    os << "]),\n";


    os << "  idx: new Int16Array([";
    for (const auto& v : m_idxs)
        os << v - 1 << ","; // VBO indexes start at 0
    os << "]),\n";
}

GluTriStripAdder *g_adder = nullptr;



void addNormal3f(float x, float y, float z) {
    g_adder->addNormal(x, y, z);
}
void addVertex(float x, float y, float z) {
    g_adder->addVtx(x, y, z);
}
void addTexCoord2f(float x, float y) {

}
void myBegin(int type) {
    if (type != GL_QUAD_STRIP && type != GL_TRIANGLE_FAN)
        throw runtime_error("unexpected type");
    g_adder->m_type = type;
}
void myEnd() {
    g_adder->endStrip();
}

void gluQuadricError(void* p, int v) {
    cout << "ERROR!" << endl;
}


#define glNormal3f addNormal3f
#define glVertex3f addVertex
#define glTexCoord2f addTexCoord2f
#define glBegin myBegin
#define glEnd myEnd

#include "myobj_quad.cpp"

void makeArrowObject(ostream& os)
{
    GluTriStripAdder adder(&os);
    g_adder = &adder;
    //float barRadius = 0.7, barLen = 15.0, triStartRadius = 1.5, triLen = 5.0;
    //float barRadius = 1.5, barLen = 12.0, triStartRadius = 2.4, triLen = 8.0;
    float barRadius = 1, barLen = 13.0, triStartRadius = 2, triLen = 7.0;

    int slices = 20;

    GLUquadric q;
    q.orientation = GLU_OUTSIDE;
    sgluCylinder(&q, barRadius, barRadius, barLen, slices, 1);

    q.orientation = GLU_INSIDE;
    sgluDisk(&q, 0, barRadius, slices, 1);

    adder.m_translate.z = barLen;

    q.orientation = GLU_OUTSIDE;
    sgluCylinder(&q, triStartRadius, 0, triLen, slices, 1);

    q.orientation = GLU_INSIDE;
    sgluDisk(&q, barRadius, triStartRadius, slices, 1);

    os << "\nVTX " << adder.m_addedVtx.size() << endl;

    adder.printJs(os);

    os.flush();

    g_adder = nullptr;
}

void makeSphere(ostream& os)
{
    GluTriStripAdder adder(&os);
    g_adder = &adder;

    GLUquadric q;
    q.orientation = GLU_OUTSIDE;

    sgluSphere(&q, 1.0, 14, 14);


    os << "\nVTX " << adder.m_addedVtx.size() << endl;

    adder.printJs(os);

    os.flush();
    g_adder = nullptr;
}
