#include "ObjExport.h"
#include "Pieces.h"
#include "Mat.h"



static float filterZero(float x) // don't allow numbers like 1.1e-16 to the output
{
    if (abs(x) < EPSILON)
        return 0.0f;
    return x;
}

void ObjExport::addMesh(const PicGroupDef *def, Mesh& mesh, const Mat4& fMatrix)
{
    bool didMtl = false;
    ++numObjs;
    if (def != nullptr && mtl != nullptr) {
        //if (def->drawtype == DRAW_COLOR)
        {
            (*mtl) << "newmtl material" << numObjs << "\n";
            (*mtl) << "  Ns 32\n  d 1\n  Tr 1\n  Tf 1 1 1\n  illum 2\n  Ka 0.0000 0.0000 0.0000\n  Ks 0.3500 0.3500 0.3500\n";
            (*mtl) << "  Kd " << def->color.r << " " << def->color.g << " " << def->color.b << "\n";
            didMtl = true;
        }
    }

    meshout << "g Object" << numObjs << "\n";

    if (didMtl) {
        meshout << "usemtl material" << numObjs << "\n";
    } 


    // We found unique vertices. Now let's put them to file
    for (int i = 0; i < mesh.m_vtx.size(); ++i) {
        Vec3 v = mesh.m_vtx[i];
        v = fMatrix.transformVec(v);
        meshout << "v " << filterZero(v.x) << " " << filterZero(v.y) << " " << filterZero(v.z) << "\n";
    }

    /*
    addTexCoord |= mesh.m_hasTexCoord;
    if (addTexCoord)	{
        for (int i = 0; i < mesh.m_texCoord.size(); ++i) {
            Vec2& t = mesh.m_texCoord[i];
            meshout << "vt " << t.x << " " << t.y << "\n";
        }
    }
    */

    int elemSize = mesh.elemSize();
    for(int idxi = 0; idxi < mesh.m_idx.size(); idxi += elemSize) 
    {
        if (elemSize == 4) {
            int a = mesh.m_idx[idxi + 0] + numVerts,
                b = mesh.m_idx[idxi + 1] + numVerts,
                c = mesh.m_idx[idxi + 2] + numVerts,
                d = mesh.m_idx[idxi + 3] + numVerts;

            if (asTriangles) {
                if (addTexCoord) {
                    meshout << "f " << a << "/" << a << " " << b << "/" << b << " " << c << "/" << c << "\n";
                    meshout << "f " << a << "/" << a << " " << c << "/" << c << " " << d << "/" << d << "\n";
                }
                else {
                    meshout << "f " << a << " " << b << " " << c << "\n";
                    meshout << "f " << a << " " << c << " " << d << "\n";
                }
            }
            else {
                meshout << "f " << a << " " << b << " " << c << " " << d << "\n";
            }
        }
        else if (elemSize == 3) {
            meshout << "f " << mesh.m_idx[idxi + 0] + numVerts << " " <<
                               mesh.m_idx[idxi + 1] + numVerts << " " <<
                               mesh.m_idx[idxi + 2] + numVerts << "\n";
        }
    }

    numVerts += mesh.m_vtx.size();
    //numTexVerts += realNumTexVers;

}

/*
    addTexCoord |= mesh.m_hasTexCoord;
    map<string, int> dedTex; // string is serialization of the coordinates
    vector<int> mi2fi(mesh.m_texCoord.size()+1); // mesh index to file index
    int fi = 1;
    if (addTexCoord)	{
        // deduplicate 
        for (int i = 0; i < mesh.m_texCoord.size(); ++i) {
            stringstream ss;
            Vec2& t = mesh.m_texCoord[i];
            ss << t.x << " " << t.y;
            string st = ss.str();
            auto it = dedTex.find(st);
            if (it != dedTex.end()) {
                mi2fi[i + 1] = it->second;
                continue; // it's already there
            }
            dedTex[st] = fi;
            mi2fi[i+1] = fi;
            ++fi;
            meshout << "vt " << st << "\n";
        }
    }
    */