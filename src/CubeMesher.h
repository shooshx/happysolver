#pragma once

#include "PicArr.h"

class MyAllocator;
class Mesh;

// takes a 2d array and create a mesh that mimics the subdivided mesh but do it without subdivision
template<int W, int H, typename ArrT>
class CubeMesher
{
public:
    CubeMesher(Mesh* outmesh, PicArr& arr) : m_mesh(outmesh), m_arr(arr)
    {}

    void create(MyAllocator* alloc);
private:
    bool uncub(int x, int y) const;
    bool uncubex(int x, int y) const;

private:
    Mesh* m_mesh;
    ArrT& m_arr;
};

typedef CubeMesher<5,5,PicArr> PicCubeMesher;