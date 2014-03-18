#pragma once

#include "general.h"

class ImgBuf 
{
public:
    ImgBuf() : m_width(-1), m_height(-1) {}
    ImgBuf(int width, int height, const uchar* bits) 
        : m_width(width), m_height(height), m_buf(width * height)
    {
        memcpy(&m_buf[0], bits, m_width * m_height * sizeof(uint));
    }

    ImgBuf(int width, int height) 
        : m_width(width), m_height(height), m_buf(width * height)
    {}

    int width() const { return m_width; }
    int height() const { return m_height; }
    const uchar* bits() const { 
        return (const uchar*)&m_buf[0]; 
    }
    uchar* bits() { 
        return (uchar*)&m_buf[0]; 
    }
    uint* buf() {
        return &m_buf[0];
    }
    ImgBuf *copy(int xoffs, int yoffs, int width, int height) 
    {
        M_ASSERT(xoffs + width < m_width && yoffs + height < m_height);
        ImgBuf *ret = new ImgBuf(width, height);
        uint *rbuf = ret->buf(), *sbuf = buf();
        for(int y = 0; y < height; ++y) 
            for(int x = 0; x < width; ++x) 
                rbuf[y * width + x] = sbuf[ (yoffs + y)*m_width + xoffs + x ];
        return ret;
    }

private:
    DISALLOW_COPY(ImgBuf);

    int m_width;
    int m_height;
    vector<uint> m_buf;
};