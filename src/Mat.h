#pragma once

#include "Vec.h"
#include <cstring>

#define MY_PI (3.14159265358979323846)

template<typename T>
inline void mswap(T& a, T& b) {
    T t = a;
    a = b;
    b = t;
}

class Mat3
{
public:
    Mat3 invert() const;
    void transpose() {
        mswap(m[2], m[6]);
        mswap(m[1], m[3]);
        mswap(m[5], m[7]);
    }
    float operator()(int x, int y) const {
        return m[x + y*3];
    }
    float& operator()(int x, int y) {
        return m[x + y*3];
    }


    float m[9];
};

class Mat4
{
public:
    Mat4() {
        zero();
    }
    Mat4(const Mat4& from) {
        memcpy(m, from.m, 16*sizeof(float));
    }
    Mat4& operator=(const Mat4& from) {
        if (&from != this) {
            memcpy(m, from.m, 16*sizeof(float));
        }
        return *this;
    }

    static Mat4 rotation(float angle, float x, float y, float z)
    {
        Mat4 m;
        float c = cos(angle / 180.0f * MY_PI);
        float s = sin(angle / 180.0f * MY_PI);
        m.m[0] = x * x * (1 - c) + c    ; m.m[4] = x * y * (1 - c) - z * s; m.m[8 ] = x * z * (1 - c) + y * s; m.m[12] = 0.0f;
        m.m[1] = y * x * (1 - c) + z * s; m.m[5] = y * y * (1 - c) + c;		m.m[9 ] = y * z * (1 - c) - x * s; m.m[13] = 0.0f;
        m.m[2] = x * z * (1 - c) - y * s; m.m[6] = y * z * (1 - c) + x * s; m.m[10] = z * z * (1 - c) + c;	 m.m[14] = 0.0f;
        m.m[3] = 0.0f;					  m.m[7] = 0.0f;					m.m[11] = 0.0f;					 m.m[15] = 1.0f;
        return m;
    }

    static Mat4 translation(float x, float y, float z)
    {
        Mat4 m;
        m.m[0] = 1.0f; m.m[4] = 0.0f; m.m[8 ] = 0.0f; m.m[12] = x;
        m.m[1] = 0.0f; m.m[5] = 1.0f; m.m[9 ] = 0.0f; m.m[13] = y;
        m.m[2] = 0.0f; m.m[6] = 0.0f; m.m[10] = 1.0f; m.m[14] = z;
        m.m[3] = 0.0f; m.m[7] = 0.0f; m.m[11] = 0.0f; m.m[15] = 1.0f;
        return m;
    }

    static Mat4 scalation(float x, float y, float z)
    {
        Mat4 m;
        m.m[0] = x;    m.m[4] = 0.0f; m.m[8 ] = 0.0f; m.m[12] = 0.0f;
        m.m[1] = 0.0f; m.m[5] = y;    m.m[9 ] = 0.0f; m.m[13] = 0.0f;
        m.m[2] = 0.0f; m.m[6] = 0.0f; m.m[10] = z;    m.m[14] = 0.0f;
        m.m[3] = 0.0f; m.m[7] = 0.0f; m.m[11] = 0.0f; m.m[15] = 1.0f;
        return m;
    }

    // ret = A * B
    static Mat4 multt(const Mat4 matA, const Mat4 matB)
    {
        Mat4 m;
        for (unsigned int i = 0; i < 4; ++i)
        {
            for (unsigned int j = 0; j < 4; ++j)
            {
                float res = 0.0f;
                for (unsigned int k = 0; k < 4; ++k)
                {
                    res += matA.m[i + k * 4] * matB.m[k + j * 4];
                }
                m.m[i + j * 4] = res;
            }
        }
        return m;
    }
    static Mat4 ident() {
        Mat4 m;
        m.identity();
        return m;
    }

    Vec3 transformVec(const Vec3& vec) const
    { // bug - ignoring w.
        Vec3 outVec;
        outVec.v[0] = m[0] * vec[0] + m[4] * vec[1] + m[8 ] * vec[2] + m[12];
        outVec.v[1] = m[1] * vec[0] + m[5] * vec[1] + m[9 ] * vec[2] + m[13];
        outVec.v[2] = m[2] * vec[0] + m[6] * vec[1] + m[10] * vec[2] + m[14];
        return outVec;
    }

    void translate(float x, float y, float z) {
        *this = multt(*this, translation(x, y, z));
    }
    void rotate(float angle, float x, float y, float z) {
        *this = multt(*this, rotation(angle, x, y, z));
    }
    void lrotate(float angle, float x, float y, float z) {
        *this = multt(rotation(angle, x, y, z), *this);
    }
    void scale(float x, float y, float z) {
        *this = multt(*this, scalation(x, y, z));
    }

    void mult(const Mat4& o) {
        *this = multt(*this, o);
    }

    void zero() {
        memset(m, 0, 16*sizeof(float));
    }

    void identity() {
        zero();
        m[0] = 1.0;
        m[5] = 1.0;
        m[10] = 1.0;
        m[15] = 1.0;
    }

    Mat3 toNormalsTrans();

    float m[16];
};