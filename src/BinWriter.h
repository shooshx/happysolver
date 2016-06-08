#pragma once
#include "general.h"
#include <string>
#include <iomanip>
#include <sstream>

using namespace std;

//    00BBCCDD  cur=6, count=5
// AAAAA 
class BinWriter
{
public:
    BinWriter(string& s) : m_buf(s)
    {}

    void addBits(uint8_t v, int bitCount) {
        M_ASSERT((v & (~s_bitMasks[bitCount])) == 0); // check other bits not in bit count are 0
        if (m_curShift + bitCount > 8) {
            m_curByte |= v << m_curShift;
            m_buf.push_back(m_curByte);
            m_curByte = v >> (8 - m_curShift);
            m_curShift = bitCount - 8 + m_curShift;
        }
        else {
            m_curByte |= v << m_curShift;
            m_curShift += bitCount;
        }
    }
    void flush() {
        m_buf.push_back(m_curByte);
        m_curByte = 0;
        m_curShift = 0;
    }

    uint8_t readBits(int bitCount)
    {
        uint8_t r;
        if (m_curShift + bitCount > 8) {
            if (rdEnd())
                return 0;
            r = m_curByte >> m_curShift;
            m_curByte = m_buf[m_rdOffset++];
            r |= m_curByte << (8 - m_curShift);
            m_curShift = bitCount - 8 + m_curShift;
        }
        else {
            if (m_rdOffset == 0) {
                if (rdEnd())
                    return 0;
                m_curByte = m_buf[m_rdOffset++];
            }
            r = m_curByte >> m_curShift;
            m_curShift += bitCount;
        }
        return r & s_bitMasks[bitCount];
    }
    bool rdEnd() {
        m_reachedEnd = m_rdOffset >= m_buf.size();
        return m_reachedEnd;
    }

    string repr() const {
        stringstream ss;
        ss << hex;
        for(int i = 0; i < m_buf.size(); ++i)
            ss << setfill('0') << setw(2) << ((int)m_buf[i] & 0xff);
        return ss.str();
    }

    static uint8_t charToNum(uint8_t c) {
        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        if (c >= '0' && c <= '9')
            return c - '0';
        throw HCException("unexpected character");
    }

    void unrepr(const string& in) {
        M_CHECK((in.size() % 2) == 0);
        m_buf.resize(in.size() / 2);
        for(int i = 0; i < m_buf.size(); ++i) {
            m_buf[i] = charToNum(in[i*2]) * 16 + charToNum(in[i*2+1]);
        }
    }


    static const uint8_t s_bitMasks[];
    string& m_buf;
    int m_curShift = 0;
    uint8_t m_curByte;
    int m_rdOffset = 0; // next byte to read
    bool m_reachedEnd = false;
};


/* test
    string a="aa11ff00ddaFfa121ff30330000d";
    string r;
    BinWriter(r).unrepr(a);
    cout << BinWriter(r).repr() << endl;
*/
