#include "CubeMesher.h"
#include "Mesh.h"
#include "MyObject.h"



static void addPolyMirror(MyObject& obj, const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, bool flip = false, bool addNormal = false)
{
    obj.addPoly(d, c, b, a, flip, addNormal);
    obj.addPoly(Vec3(a.x, a.y, 1.0f - a.z), Vec3(b.x, b.y, 1.0f - b.z), Vec3(c.x, c.y, 1.0f - c.z), Vec3(d.x, d.y, 1.0f - d.z), flip, addNormal);
}
static void addAcross(MyObject& obj, float x, float y, float z1, float z2,
    const Vec2& d1, const Vec2& d2, const Vec2& d3, const Vec2& d4, bool flip = false, bool flipD = false, bool addNormal = false) //flipD - flips x and y
{
    if (!flipD)
        obj.addPoly(Vec3(x + d1.x, y + d1.y, z1), Vec3(x + d2.x, y + d2.y, z1), Vec3(x + d3.x, y + d3.y, z2), Vec3(x + d4.x, y + d4.y, z2), flip, addNormal);
    else
        obj.addPoly(Vec3(x + d1.y, y + d1.x, z1), Vec3(x + d2.y, y + d2.x, z1), Vec3(x + d3.y, y + d3.x, z2), Vec3(x + d4.y, y + d4.x, z2), !flip, addNormal);
}
static void addPolyMirrorAc(MyObject& obj, float x, float y, float z1, float z2,
    const Vec2& d1, const Vec2& d2, const Vec2& d3, const Vec2& d4, bool flip = false, bool flipD = false)
{
    addAcross(obj, x, y, z1, z2, d1, d2, d3, d4, !flip, flipD);
    addAcross(obj, x, y, 1 - z1, 1 - z2, d1, d2, d3, d4, flip, flipD);
}

static void addPolyMirrorCorner(MyObject& obj, float x, float y, float z1, float z2,
    const Vec2& d1, const Vec2& d2, const Vec3& p, const Vec2& d4, bool flip = false, bool flipD = false)
{
    if (!flipD)
        obj.addPoly(Vec3(x + d1.x, y + d1.y, z1), Vec3(x + d2.x, y + d2.y, z1), p, Vec3(x + d4.x, y + d4.y, z2), !flip);
    else
        obj.addPoly(Vec3(x + d1.y, y + d1.x, z1), Vec3(x + d2.y, y + d2.x, z1), p, Vec3(x + d4.y, y + d4.x, z2), flip);

    if (!flipD)
        obj.addPoly(Vec3(x + d1.x, y + d1.y, 1 - z1), Vec3(x + d2.x, y + d2.y, 1 - z1), Vec3(p.x, p.y, 1 - p.z), Vec3(x + d4.x, y + d4.y, 1 - z2), flip);
    else
        obj.addPoly(Vec3(x + d1.y, y + d1.x, 1 - z1), Vec3(x + d2.y, y + d2.x, 1 - z1), Vec3(p.x, p.y, 1 - p.z), Vec3(x + d4.y, y + d4.x, 1 - z2), !flip);
}

static void addPolyMirrorCorner2(MyObject& obj, float x, float y, float z1, float z2,
    const Vec2& d1, const Vec3& p, const Vec2& d2, const Vec2& d4, bool flip = false, bool flipD = false)
{
    if (!flipD)
        obj.addPoly(Vec3(x + d1.x, y + d1.y, z1), p, Vec3(x + d2.x, y + d2.y, z1), Vec3(x + d4.x, y + d4.y, z2), !flip);
    else
        obj.addPoly(Vec3(x + d1.y, y + d1.x, z1), p, Vec3(x + d2.y, y + d2.x, z1), Vec3(x + d4.y, y + d4.x, z2), flip);

    if (!flipD)
        obj.addPoly(Vec3(x + d1.x, y + d1.y, 1 - z1), Vec3(p.x, p.y, 1 - p.z), Vec3(x + d2.x, y + d2.y, 1 - z1), Vec3(x + d4.x, y + d4.y, 1 - z2), flip);
    else
        obj.addPoly(Vec3(x + d1.y, y + d1.x, 1 - z1), Vec3(p.x, p.y, 1 - p.z), Vec3(x + d2.y, y + d2.x, 1 - z1), Vec3(x + d4.y, y + d4.x, 1 - z2), !flip);
}


#define E 0.125
#define ER 0.03125
#define EP 0.055555555
#define ES 0.02 // for saddle point

template<int W, int H, typename ArrT>
bool CubeMesher<W,H,ArrT>::uncub(int x, int y) const {
    return m_arr.axx(x, y);
}

template<int W, int H, typename ArrT>
bool CubeMesher<W, H, ArrT>::uncubex(int x, int y) const {
    if (x < 0 || y < 0 || x > W-1 || y > H-1)
        return 0;
    return uncub(x, y) == 1 || (x > 0 && x < W-1 && y > 0 && y < H-1);
}

template<int W, int H, typename ArrT>
void CubeMesher<W, H, ArrT>::create(MyAllocator* alloc)
{
    MyObject obj(alloc);

    // central tiles
    for (int x = 0; x < W; ++x)
    {
        for (int y = 0; y < H; ++y)
        {
            if (uncubex(x, y)) {
                addPolyMirror(obj, Vec3(x + E, y + E, 0), Vec3(x + 1 - E, y + E, 0), Vec3(x + 1 - E, y + 1 - E, 0), Vec3(x + E, y + 1 - E, 0), false, true);
            }
        }
    }

    // slithers that connect central tiles - every two centrol tiles 
    for (int x = 0; x < W-1; ++x)
    {
        for (int y = 0; y < H; ++y)
        {
            if (uncubex(x, y) && uncubex(x + 1, y)) {
                addPolyMirror(obj, Vec3(x + 1, y + E, 0), Vec3(x + 1, y + 1 - E, 0), Vec3(x + 1 - E, y + 1 - E, 0), Vec3(x + 1 - E, y + E, 0), false, true);
                addPolyMirror(obj, Vec3(x + 1, y + E, 0), Vec3(x + 1, y + 1 - E, 0), Vec3(x + 1 + E, y + 1 - E, 0), Vec3(x + 1 + E, y + E, 0), true, true);
            }
        }
    }
    for (int y = 0; y < H-1; ++y)
    {
        for (int x = 0; x < W; ++x)
        {
            if (uncubex(x, y) && uncubex(x, y + 1)) {
                addPolyMirror(obj, Vec3(x + E, y + 1, 0), Vec3(x + 1 - E, y + 1, 0), Vec3(x + 1 - E, y + 1 - E, 0), Vec3(x + E, y + 1 - E, 0), true, true);
                addPolyMirror(obj, Vec3(x + E, y + 1, 0), Vec3(x + 1 - E, y + 1, 0), Vec3(x + 1 - E, y + 1 + E, 0), Vec3(x + E, y + 1 + E, 0), false, true);
            }
        }
    }

    // small rects that connect central tiles, every 4 that exist
    for (int x = 1; x < W; ++x)
    {
        for (int y = 1; y < H; ++y)
        {
            if (uncubex(x, y) && uncubex(x - 1, y) && uncubex(x, y - 1) && uncubex(x - 1, y - 1)) {
                addPolyMirror(obj, Vec3(x, y, 0), Vec3(x + E, y, 0), Vec3(x + E, y + E, 0), Vec3(x, y + E, 0), false, true);
                addPolyMirror(obj, Vec3(x, y, 0), Vec3(x - E, y, 0), Vec3(x - E, y + E, 0), Vec3(x, y + E, 0), true, true);
                addPolyMirror(obj, Vec3(x, y, 0), Vec3(x + E, y, 0), Vec3(x + E, y - E, 0), Vec3(x, y - E, 0), true, true);
                addPolyMirror(obj, Vec3(x, y, 0), Vec3(x - E, y, 0), Vec3(x - E, y - E, 0), Vec3(x, y - E, 0), false, true);
            }
        }
    }

    // side panels, facing outside
    //  check all sides in the piece, this is simpler than going around
    for (int x = 0; x <= W; ++x)
    {
        for (int y = 0; y <= H; ++y)
        {
            bool a = uncubex(x - 1, y), b = uncubex(x, y);
            bool c = uncubex(x, y - 1);
            auto sidePanel = [=](MyObject& obj, bool flipD) {
                // panel
                addAcross(obj, x, y, E, 1 - E, Vec2(0, E), Vec2(0, 1 - E), Vec2(0, 1 - E), Vec2(0, E), b != 0, flipD, true);
                auto ser = (b != 0) ? ER : -ER;
                auto se = (b != 0) ? E : -E;
                // side panels connectors to top and bottom panels
                addPolyMirrorAc(obj, x, y, E, ER, Vec2(0, E), Vec2(0, 1 - E), Vec2(ser, 1 - E), Vec2(ser, E), b != 0, flipD);
                addPolyMirrorAc(obj, x, y, 0, ER, Vec2(se, E), Vec2(se, 1 - E), Vec2(ser, 1 - E), Vec2(ser, E), b == 0, flipD);
            };
            if (a != b) {
                sidePanel(obj, false);
            }
            if (c != b) {
                sidePanel(obj, true);
            }
            // ------ 4 at a time conditions ------
            bool d;
            a = uncubex(x - 1, y - 1); b = uncubex(x, y - 1);
            c = uncubex(x - 1, y); d = uncubex(x, y);
            //    --- straights ---
            auto straightConnect = [=](MyObject& obj, bool flipD) {
                // slithers
                addAcross(obj, x, y, E, 1 - E, Vec2(0, 0), Vec2(0, E), Vec2(0, E), Vec2(0, 0), a == 0, flipD, true);
                addAcross(obj, x, y, E, 1 - E, Vec2(0, 0), Vec2(0, -E), Vec2(0, -E), Vec2(0, 0), a != 0, flipD, true);
                // round squares
                auto ser = (a == 0) ? ER : -ER;
                auto se = (a == 0) ? E : -E;
                addPolyMirrorAc(obj, x, y, E, ER, Vec2(0, 0), Vec2(0, E), Vec2(ser, E), Vec2(ser, 0), a == 0, flipD);
                addPolyMirrorAc(obj, x, y, 0, ER, Vec2(se, 0), Vec2(se, E), Vec2(ser, E), Vec2(ser, 0), a != 0, flipD);
                addPolyMirrorAc(obj, x, y, E, ER, Vec2(0, 0), Vec2(0, -E), Vec2(ser, -E), Vec2(ser, 0), a != 0, flipD);
                addPolyMirrorAc(obj, x, y, 0, ER, Vec2(se, 0), Vec2(se, -E), Vec2(ser, -E), Vec2(ser, 0), a == 0, flipD);
            };
            // 0,1 or 1,0
            // 0,1    1,0
            if ((!a && b && !c && d) || (a && !b && c && !d))  {
                straightConnect(obj, false);
            }
            // 0,0 or 1,1  it's like the above, x,y reversed
            // 1,1    0,0
            if ((a && b && !c && !d) || (!a && !b && c && d)) {
                straightConnect(obj, true);
            }
            //    --- corners ---
            auto straightCorner = [=](MyObject& obj, bool flipD, int sY, int sX, bool checkWhich) {
                // equals operator (==) does a XOR operation
                addAcross(obj, x, y, E, 1 - E, Vec2(sX*ER, sY*ER), Vec2(sX*E, 0), Vec2(sX*E, 0), Vec2(sX*ER, sY*ER), (checkWhich == 0) != flipD, flipD);
                addAcross(obj, x, y, E, 1 - E, Vec2(sX*ER, sY*ER), Vec2(0, sY*E), Vec2(0, sY*E), Vec2(sX*ER, sY*ER), (checkWhich != 0) != flipD, flipD);

                Vec3 p;
                if (checkWhich)
                    p = Vec3(x + sY*EP, y + sX*EP, EP);
                else
                    p = Vec3(x, y, ES);

                auto ser = (checkWhich == 0) ? ER : -ER;
                auto se = (checkWhich == 0) ? E : -E;

                addPolyMirrorCorner(obj, x, y, E, ER, Vec2(sX*E, 0), Vec2(sX*ER, sY*ER), p, Vec2(sX*E, -sY*ser), (checkWhich != 0) != flipD, flipD);
                addPolyMirrorCorner(obj, x, y, E, ER, Vec2(0, sY*E), Vec2(sX*ER, sY*ER), p, Vec2(-sX*ser, sY*E), (checkWhich == 0) != flipD, flipD);

                if (checkWhich) { // only for point
                    addPolyMirrorCorner2(obj, x, y, ER, 0, Vec2(sX*E, -sY*ser), p, Vec2(-sX*ser, sY*E), Vec2(sX*E, sY*E), !flipD, flipD);
                }
                else { // a saddle
                    addPolyMirrorCorner(obj, x, y, 0, ER, Vec2(sX*E, -sY*E), Vec2(0, -sY*E), p, Vec2(sX*E, -sY*ser), !flipD, flipD);
                    addPolyMirrorCorner(obj, x, y, 0, ER, Vec2(-sX*E, sY*E), Vec2(-sX*E, 0), p, Vec2(-sX*ser, sY*E), flipD, flipD);
                    addPolyMirrorCorner(obj, x, y, 0, 0, Vec2(-sX*E, -sY*E), Vec2(-sX*E, 0), p, Vec2(0, -sY*E), !flipD, flipD);
                }
            };
            // 0,1 or 1,0
            // 1,1    0,0
            if ((!a && b && c && d) || (a && !b && !c && !d)) {
                straightCorner(obj, false, -1, -1, a);
            }
            // 1,0 or 0,1  it's x mirror of the above offsets
            // 1,1    0,0
            if ((a && !b && c && d) || (!a && b && !c && !d)) {
                straightCorner(obj, true, 1, -1, b);
            }
            // now x,y reversed of the above two
            // 1,1 or 0,0
            // 0,1    1,0
            if ((a && b && !c && d) || (!a && !b && c && !d)) {
                straightCorner(obj, true, -1, 1, c);
            }
            // 1,1 or 1,1  it's x mirror of the above offsets
            // 1,0    1,0
            if ((a && b && c && !d) || (!a && !b && !c && d)) {
                straightCorner(obj, false, 1, 1, d);
            }

        }
    }


    obj.vectorify();
    // flip it to the correct piece orientation
    for (int pn = 0; pn < obj.nPoints; ++pn)
    {
        Vec3 v = obj.points[pn]->p;
        obj.points[pn]->p = Vec3(1 - v.z, v.y, v.x); // see same transformation in the individual half shader
        Vec3 n = obj.points[pn]->n; // normals
        obj.points[pn]->n = Vec3(-n.z, n.y, n.x);
    }

    obj.clacNormalsExceptTouched();
    // true for saving objs
    // false for memory vbo
    obj.toMesh(*m_mesh, false); // quads



    /*  static int cci = 0;
    if (cci++ == 2) {
    stringstream ss;
    ss << "c:/temp/nosub/piece_" << hex << m_arr.getBits() << "_" << rand() << ".obj";
    m_mesh.save(ss.str(), true);
    //   exit(1);
    }*/
}

#undef E
#undef ER
#undef EP
#undef ES


// explicit instantiate
template class CubeMesher<5,5,PicArr>;