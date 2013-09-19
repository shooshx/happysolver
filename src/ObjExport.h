#pragma once
#include "general.h"
#include <ostream>

class PicGroupDef;
class Mesh;
class Mat4;

// responsible for exporting meshes to obj file
class ObjExport {
public:
    ObjExport(ostream& _obj, ostream* _mtl) 
        : numVerts(1), numTexVerts(1), numObjs(0), meshout(_obj), mtl(_mtl), asTriangles(false), addTexCoord(false)
    {}

    void addMesh(const PicGroupDef *def, Mesh& obj, const Mat4& fMatrix);

    bool asTriangles;
    bool addTexCoord;

private:
    DISALLOW_COPY(ObjExport);

    uint numVerts;
    uint numTexVerts; 
    //uint numNormals;
    uint numObjs;

    ostream& meshout;
    ostream* mtl;

};