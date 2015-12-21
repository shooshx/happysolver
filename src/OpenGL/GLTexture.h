#pragma once
#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED

//#include <QGLWidget>
#include "../general.h"
#include "../Vec.h"
#include "glGlob.h"
#ifndef _WINDOWS
#include <QtOpenGL>
#endif


#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

class GlTexture
{
public:
	GlTexture() 
		: m_obj(-1), m_target(-1)
	{}
	GlTexture(uint target, const Vec2i& size) 
		: m_obj(-1), m_target(-1)
	{
		init(target, size, 1);
	}
	GlTexture(uint target, const Vec2i& size, int depth) 
		: m_obj(-1), m_target(-1)
	{
		init(target, size, depth);
	}


	~GlTexture();

	void init(uint target, const Vec2i& size, int depth, uint internal_format = GL_RGBA8, 
		      uint format = GL_RGBA, uint type = GL_UNSIGNED_BYTE, const void* ptr = NULL,
			  uint minFilter = GL_NEAREST, uint magFilter = GL_NEAREST, uint wrap = GL_CLAMP_TO_EDGE);
	void destroy();

	uint handle() const { return m_obj; }
	// means we're not going to delete this texture. better take care of it yourself.
	void detach() { m_obj = -1; }

	const Vec3i& size() const { return m_size; }
	bool isValid() const { return m_obj != -1; }
	uint target() const { return m_target; }

	void bind() const;
	void unbind() const;

private:
	DISALLOW_COPY(GlTexture);
	uint m_obj;
	Vec3i m_size;
	uint m_target;
	
};


class RenderBuffer
{
public: 
	RenderBuffer() : m_obj(-1) {}
	RenderBuffer(const Vec2i& size, uint internal_format, int numSamp)
	{
		init(size, internal_format, numSamp);
	}
	~RenderBuffer();
	void init(const Vec2i& size, uint internal_format, int numSamp);
	void destroy();

	void bind() const;
	void unbind() const;

	uint handle() const { return m_obj; }
	bool isValid() const { return m_obj != -1; }

private:
	DISALLOW_COPY(RenderBuffer);
	uint m_obj;

};

#endif // TEXTURE_H_INCLUDED
