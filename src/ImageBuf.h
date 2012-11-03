#pragma once

#include "general.h"

class ImageBuf
{
public:
	ImageBuf(const string& name);
	~ImageBuf() {
		delete[] buf;
	}
	int width() {
		return m_width;
	}
	int height() {
		return m_height;
	}

private:
	int m_width, m_height;
	byte *buf;

};