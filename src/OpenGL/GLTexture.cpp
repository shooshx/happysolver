#include "GLTexture.h"

#define GL_TEXTURE_BASE_LEVEL             0x813C
#define GL_TEXTURE_MAX_LEVEL              0x813D

void GlTexture::init(GLenum target, const Vec2i& size, int depth, GLenum internal_format, 
                   GLenum format, GLenum type, const void* ptr, GLenum minFilter, GLenum magFilter, GLenum wrap)
{
    if (m_obj != -1)
        destroy();

    m_target = target;
    glGenTextures(1, &m_obj);
    //printf(":%d\n", m_obj);
    glBindTexture(target, m_obj);
    mglCheckErrorsC("bind");
    if (target == GL_TEXTURE_2D)
    {
#ifdef QT_CORE_LIB
        // see http://www.opengl.org/wiki/Common_Mistakes        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
#endif
        if (ptr != nullptr) {
            glTexImage2D(target, 0, internal_format, size.width, size.height, 0, format, type, ptr);
            mglCheckErrorsC("tex2d");
        }
    }
    else {
        throw HCException("texture not supported");
    }
    /*if (target == GL_TEXTURE_1D)
    {
        glTexImage1D(target, 0, internal_format, size.width, 0, format, type, ptr);
        mglCheckErrorsC("tex1d");
    }
    else if ((target == GL_TEXTURE_3D) || (target == GL_TEXTURE_2D_ARRAY_EXT))
    {
        glTexImage3D(target, 0, internal_format, size.width(), size.height(), depth, 0, format, type, ptr);
        mglCheckErrorsC("tex3d");
    }*/

    /*if (minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_NEAREST_MIPMAP_LINEAR ||
        minFilter == GL_LINEAR_MIPMAP_NEAREST || minFilter == GL_NEAREST_MIPMAP_NEAREST)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    }*/

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap);
    //glTexParameteri(target, GL_TEXTURE_WRAP_R, wrap);

    m_size = Vec3i(size.width, size.height, depth);
    mglCheckErrorsC("texture");
}


// GlTexture::GlTexture(const QGLContext* context, const QImage* img, uint target)
// {
// 	m_fromContext = const_cast<QGLContext*>(context);
// 	m_obj = m_fromContext->bindTexture(*img, target);
// 	m_target = target;
// 	m_size = Vec3i(img->width(), img->height(), 1.0);
// }

GlTexture::~GlTexture()
{
    unbind();
    destroy();
}

void GlTexture::destroy()
{
    //if (m_fromContext != nullptr)
//		m_fromContext->deleteTexture(m_obj);
    //cout << "Del-Tex " << m_obj << endl;
    if (m_obj != -1)
        glDeleteTextures(1, &m_obj);
    m_obj = -1;
    m_size.clear();
    m_target = -1;
}

void GlTexture::bind() const
{
    if (m_obj == -1)
        return;
    glBindTexture(m_target, m_obj);
}

void GlTexture::unbind() const
{
    if (m_obj == -1)
        return;
    glBindTexture(m_target, 0);
}


/////////////////////////
/*
void RenderBuffer::init(const Vec2i& size, uint internal_format, int numSamp)
{
    if (m_obj != -1)
        destroy();

    glGenRenderbuffersEXT(1, &m_obj);
    M_ASSERT(!glIsRenderbufferEXT(m_obj));
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_obj);
    M_ASSERT(glIsRenderbufferEXT(m_obj));
    
    glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, numSamp, internal_format, size.width(), size.height());
    mglCheckErrors("renderBuf");
}
RenderBuffer::~RenderBuffer()
{
    destroy();
}
void RenderBuffer::destroy()
{
    if (m_obj != -1)
        glDeleteRenderbuffersEXT(1, &m_obj);
    m_obj = -1;
}

void RenderBuffer::bind() const
{
    if (m_obj == -1)
        return;
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_obj);
}
void RenderBuffer::unbind() const
{
    if (m_obj == -1)
        return;
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
}
*/