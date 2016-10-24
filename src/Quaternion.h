#pragma once

#include "Mat.h"

class Quaternion 
{
public:
    float w, x, y, z;
    
    Quaternion()
        : w(1), x(0), y(0), z(0)
    {}
    Quaternion(float fW, float fX, float fY, float fZ)
        : w(fW), x(fX), y(fY), z(fZ)
    {}

    float dot(const Quaternion& b) const {
        return x*b.x + y*b.y + z*b.z + w*b.w;
    }

    Quaternion operator*(float b) const {
        Quaternion ret;
        ret.x = x * b;
        ret.y = y * b;
        ret.z = z * b;
        ret.w = w * b;
        return ret;
    }

    Quaternion operator+(const Quaternion& b) const {
        Quaternion ret;
        ret.x = x + b.x;
        ret.y = y + b.y;
        ret.z = z + b.z;
        ret.w = w + b.w;
        return ret;
    }
    Quaternion operator-() const
    {
        return Quaternion(-w, -x, -y, -z);
    }

    float norm() const
    {
        return w*w + x*x + y*y + z*z;
    }
    float normalise()
    {
        float len = norm();
        float factor = 1.0f / sqrt(len);
        *this = *this * factor;
        return len;
    }

    // from Ogre https://bitbucket.org/sinbad/ogre/src/7bc3436adeec80d49d981a65b5a01b4f99f552b2/OgreMain/src/OgreQuaternion.cpp?at=default&fileviewer=file-view-default
    static Quaternion slerp(const Quaternion& rkP, const Quaternion& rkQ, float t)
    {
        float fCos = rkP.dot(rkQ);
        Quaternion rkT;

        // Do we need to invert rotation?
        if (fCos < 0.0f) // for shortest path
        {
            fCos = -fCos;
            rkT = -rkQ;
        }
        else
        {
            rkT = rkQ;
        }

        if (abs(fCos) < 1 - 1e-03)
        {
            // Standard case (slerp)
            float fSin = sqrt(1 - fCos*fCos);
            float fAngle = atan2(fSin, fCos); // in radians
            float fInvSin = 1.0f / fSin;
            float fCoeff0 = sin((1.0f - t) * fAngle) * fInvSin;
            float fCoeff1 = sin(t * fAngle) * fInvSin;
            return rkP * fCoeff0 + rkT * fCoeff1;
        }
        else
        {
            // There are two situations:
            // 1. "rkP" and "rkQ" are very close (fCos ~= +1), so we can do a linear
            //    interpolation safely.
            // 2. "rkP" and "rkQ" are almost inverse of each other (fCos ~= -1), there
            //    are an infinite number of possibilities interpolation. but we haven't
            //    have method to fix this case, so just use linear interpolation here.
            Quaternion ln = rkP * (1.0f - t) + rkT * t;
            // taking the complement requires renormalisation
            ln.normalise();
            return ln;
        }

    }


    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/
    Mat4 toMatrix()
    {
        Quaternion& q = *this;
        Mat4 m;
        float& m00 = m.m[0*4+0];
        float& m01 = m.m[0*4+1];
        float& m10 = m.m[1*4+0];
        float& m11 = m.m[1*4+1];
        float& m02 = m.m[0*4+2];
        float& m20 = m.m[2*4+0];
        float& m22 = m.m[2*4+2];
        float& m21 = m.m[2*4+1];
        float& m12 = m.m[1*4+2];
        m.m[15] = 1.0;

        double sqw = q.w*q.w;
        double sqx = q.x*q.x;
        double sqy = q.y*q.y;
        double sqz = q.z*q.z;

        // invs (inverse square length) is only required if quaternion is not already normalised
        double invs = 1 / (sqx + sqy + sqz + sqw);
        m00 = (sqx - sqy - sqz + sqw)*invs; // since sqw + sqx + sqy + sqz =1/invs*invs
        m11 = (-sqx + sqy - sqz + sqw)*invs;
        m22 = (-sqx - sqy + sqz + sqw)*invs;

        double tmp1 = q.x*q.y;
        double tmp2 = q.z*q.w;
        m10 = 2.0 * (tmp1 + tmp2)*invs;
        m01 = 2.0 * (tmp1 - tmp2)*invs;

        tmp1 = q.x*q.z;
        tmp2 = q.y*q.w;
        m20 = 2.0 * (tmp1 - tmp2)*invs;
        m02 = 2.0 * (tmp1 + tmp2)*invs;
        tmp1 = q.y*q.z;
        tmp2 = q.x*q.w;
        m21 = 2.0 * (tmp1 + tmp2)*invs;
        m12 = 2.0 * (tmp1 - tmp2)*invs;

        return m;
    }

    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
    static Quaternion fromMat(const Mat4& m) 
    {
        float m00 = m.m[0*4+0];
        float m01 = m.m[0*4+1];
        float m10 = m.m[1*4+0];
        float m11 = m.m[1*4+1];
        float m02 = m.m[0*4+2];
        float m20 = m.m[2*4+0];
        float m22 = m.m[2*4+2];
        float m21 = m.m[2*4+1];
        float m12 = m.m[1*4+2];

        float tr = m00 + m11 + m22;
        Quaternion q;

        if (tr > 0) {
            float S = sqrt(tr + 1.0) * 2; // S=4*qw 
            q.w = 0.25 * S;
            q.x = (m21 - m12) / S;
            q.y = (m02 - m20) / S;
            q.z = (m10 - m01) / S;
        }
        else if ((m00 > m11)&(m00 > m22)) {
            float S = sqrt(1.0 + m00 - m11 - m22) * 2; // S=4*qx 
            q.w = (m21 - m12) / S;
            q.x = 0.25 * S;
            q.y = (m01 + m10) / S;
            q.z = (m02 + m20) / S;
        }
        else if (m11 > m22) {
            float S = sqrt(1.0 + m11 - m00 - m22) * 2; // S=4*qy
            q.w = (m02 - m20) / S;
            q.x = (m01 + m10) / S;
            q.y = 0.25 * S;
            q.z = (m12 + m21) / S;
        }
        else {
            float S = sqrt(1.0 + m22 - m00 - m11) * 2; // S=4*qz
            q.w = (m10 - m01) / S;
            q.x = (m02 + m20) / S;
            q.y = (m12 + m21) / S;
            q.z = 0.25 * S;
        }    
        return q;
    }



    
};
