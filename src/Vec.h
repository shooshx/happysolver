#pragma once

#include "general.h"
#include <cmath>

typedef unsigned char byte;
typedef unsigned int uint;

#define PI (3.141592653589793)
#define TWOPI (2.0 * PI)
#define HALFPI (0.5 * PI)
#define SQRT_2 (1.414213562373)

struct Vec4b
{
    Vec4b() :x(0), y(0), z(0), w(0) {}
    Vec4b(byte dx, byte dy, byte dz, byte dw) :x(dx), y(dy), z(dz), w(dw) {}
    Vec4b(const Vec4b& src) :x(src.x), y(src.y), z(src.z), w(src.w) {};

    static Vec4b fromName(uint name) {
        return Vec4b(name & 0xFF, (name >> 8) & 0xFF, (name >> 16) & 0xFF, 0xFF);
    }

    union {
        byte v[4];
        struct {
            byte x,y,z,w;
        };
    };
};

struct Vec3b
{
    Vec3b() :x(0), y(0), z(0) {}
    Vec3b(byte dx, byte dy, byte dz) :x(dx), y(dy), z(dz) {}
    Vec3b(const Vec3b& src) :x(src.x), y(src.y), z(src.z) {};

    static Vec3b fromName(uint name) {
        return Vec3b(name & 0xFF, (name >> 8) & 0xFF, (name >> 16) & 0xFF);
    }

    union {
        byte v[3];
        struct {
            byte x,y,z;
        };
    };
};



struct Vec2i
{
    Vec2i() :x(0), y(0) {}
    Vec2i(int dx, int dy) :x(dx), y(dy) {}

    union {
        struct {
            int x, y;
        };
        struct {
            int width, height;
        };
    };
};


/** Vec3i is a general purpose three dimentional vertor of ints.
    It is used in various places where there is no need for a float vector
    such as in the processes of BuildWorld
*/
struct Vec3i
{
public:
    Vec3i() :x(0), y(0), z(0) {}
    Vec3i(int dx, int dy, int dz) :x(dx), y(dy), z(dz) {}
    Vec3i(const Vec3i& src) :x(src.x), y(src.y), z(src.z) {};
    void clear() {
        x = 0; y = 0; z = 0;
    }

    int x, y, z;

    Vec3i& operator+=(const Vec3i &a) { x += a.x; y += a.y; z += a.z; return *this; }
    Vec3i& operator-=(const Vec3i &a) { x -= a.x; y -= a.y; z -= a.z; return *this; }

};

inline bool operator==(const Vec3i &a, const Vec3i &b)
{
    return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
}
inline bool operator!=(const Vec3i &a, const Vec3i &b)
{
    return ((a.x != b.x) || (a.y != b.y) || (a.z != b.z));
}
inline Vec3i operator+(const Vec3i &a, const Vec3i &b)
{
    return Vec3i(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline Vec3i operator-(const Vec3i &a, const Vec3i &b)
{
    return Vec3i(a.x - b.x, a.y - b.y, a.z - b.z);
}

#define EPSILON (1e-5)

inline bool nearDist(float a, float b) {
    return abs(a - b) < EPSILON;
}

/** Coord3df is a general purpose three dimentional vector of floats.
    It is used mostly for purposes related to openGL coordinates system
*/
struct Vec3
{
    Vec3() { v[0] = 0.0; v[1] = 0.0; v[2] = 0.0; }
    Vec3(float x, float y, float z) { v[0] = x; v[1] = y; v[2] = z; }
    /// initialize from a Coord3d of ints.
    explicit Vec3(const Vec3i& a) { v[0] = (float)a.x; v[1] = (float)a.y; v[2] = (float)a.z; }
    void clear() { v[0] = 0.0; v[1] = 0.0; v[2] = 0.0; }

    Vec3& operator=(const Vec3i &a)
    {
        v[0] = a.x; v[1] = a.y; v[2] = a.z;
        return *this;
    }
    Vec3& operator+=(const Vec3& a)
    {
        v[0] += a.v[0]; v[1] += a.v[1]; v[2] += a.v[2];
        return *this;
    }
    Vec3& operator-=(float v)
    {
        x -= v; y -= v; z -= v;
        return *this;
    }
    Vec3& operator/=(float s)
    {
        v[0] /= s; v[1] /= s; v[2] /= s;
        return *this;
    }
    Vec3& operator*=(float s)
    {
        v[0] *= s; v[1] *= s; v[2] *= s;
        return *this;
    }

    float &operator[](int row) { M_ASSERT(row < 3); return v[row]; }
    const float &operator[](int row) const { M_ASSERT(row < 3); return v[row]; }
    float const* ptr() const { return &v[0]; }

    friend Vec3 operator+(const Vec3 &a, const Vec3 &b);
    friend Vec3 operator-(const Vec3 &a, const Vec3 &b);
    friend Vec3 operator/(const Vec3 &a, float s);
    friend Vec3 operator*(const Vec3 &a, float s);
    friend Vec3 operator*(float s, const Vec3 &a);
    friend bool operator==(const Vec3& a, const Vec3& b);

    void pmin(const Vec3 &a)
    {
        v[0] = mMin(v[0], a[0]);
        v[1] = mMin(v[1], a[1]);
        v[2] = mMin(v[2], a[2]);
    }

    static Vec3 crossProd(const Vec3 &a, const Vec3& b)
    {
        return Vec3(a.y * b.z - a.z * b.y, 
                   a.z * b.x - a.x * b.z, 
                   a.x * b.y - a.y * b.x);
    }

    static Vec3 triangleNormal(const Vec3& a, const Vec3& b, const Vec3& c) {
        return crossProd(a - b, c - b);
    }

    void pmax(const Vec3 &a)
    {
        v[0] = mMax(v[0], a[0]);
        v[1] = mMax(v[1], a[1]);
        v[2] = mMax(v[2], a[2]);
    }
    void unitize()
    {
        double len = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
        M_ASSERT(len != 0.0);
        v[0] /= len; v[1] /= len; v[2] /= len;
    }

    bool isNear(const Vec3& vert) const
    {
        return (abs(vert.x - x) < EPSILON) && 
               (abs(vert.y - y) < EPSILON) &&
               (abs(vert.z - z) < EPSILON);
    }


    union {
        float v[3];
        struct {
            float x,y,z;
        };
        struct {
            float r,g,b;
        };
    };
};

inline Vec3 operator+(const Vec3 &a, const Vec3 &b)
{
    return Vec3(a.v[0] + b.v[0], a.v[1] + b.v[1], a.v[2] + b.v[2]);
}
inline Vec3 operator-(const Vec3& a, const Vec3& b)
{
    return Vec3(a.v[0] - b.v[0], a.v[1] - b.v[1], a.v[2] - b.v[2]);
}
inline Vec3 operator/(const Vec3 &a, float s)
{
    return Vec3(a.v[0] / s, a.v[1] / s, a.v[2] / s);
}
inline Vec3 operator*(const Vec3 &a, float s)
{
    return Vec3(a.v[0] * s, a.v[1] * s, a.v[2] * s);
}
inline Vec3 operator*(float s, const Vec3 &a)
{
    return Vec3(a.v[0] * s, a.v[1] * s, a.v[2] * s);
}
inline bool operator==(const Vec3& a, const Vec3& b)
{
    return ((a.v[0] == b.v[0]) && (a.v[1] == b.v[1]) && (a.v[2] == b.v[2]));
}

struct Vec2
{
public:

    Vec2() : x(0.0f), y(0.0f) {}
    Vec2(float nx, float ny) : x(nx), y(ny) {}


    bool isNear(const Vec2& vert) const
    {
        return (abs(vert.x - x) < EPSILON) &&
               (abs(vert.y - y) < EPSILON);
    }

    union {
        float v[2];
        struct {
            float x, y;
        };
    };
};

struct Vec4
{
public:
    Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Vec4(float nx, float ny, float nz, float nw) : x(nx), y(ny), z(nz), w(nw) {}

    union {
        float v[4];
        struct {
            float x, y, z, w;
        };
    };
};