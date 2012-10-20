#pragma once
#include "Mat.h"
#include <vector>
using namespace std;

#define MAX_MATSTACK (20)

class MatStack
{
private:
	struct MatEntry {
		MatEntry() : msg(NULL) {}
		MatEntry(const Mat4 _m, const char* _msg) : m(_m), msg(_msg) {}
		Mat4 m;
		const char* msg;
	};

public:
	MatStack() {}
	Mat4& cur() {
		return m_cur.m;
	}

	void push(const char* msg = NULL) {
		if (m_s.size() > MAX_MATSTACK)
			throw exception("matrix stack overflow");
		m_s.push_back(MatEntry(m_cur.m, msg));
	}

	void pop() {
		if (m_s.size() <= 0)
			throw exception("matrix stack underflow");
		m_cur = m_s.back();
		m_s.pop_back();
	}

	void translate(float x, float y, float z) {
		m_cur.m.translate(x, y, z);
	}
	void rotate(float angle, float x, float y, float z) {
		m_cur.m.rotate(angle, x, y, z);
	}
	void scale(float x, float y, float z) {
		m_cur.m.scale(x, y, z);
	}
	void mult(const Mat4& o) {
		m_cur.m.mult(o);
	}
	void identity() {
		m_cur.m.identity();
	}

private:
	MatEntry m_cur;

	vector<MatEntry> m_s;
};