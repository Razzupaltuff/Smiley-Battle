#include <stdio.h>
#include "texture.h"
#include "SDL_image.h"
#include "renderer.h"

// =================================================================================================

CTextureBuffer::CTextureBuffer (SDL_Surface * image) {
    Create (image);
}


CTextureBuffer::CTextureBuffer (CTextureBuffer const& other) {
    Copy (const_cast<CTextureBuffer&> (other));
}


CTextureBuffer::CTextureBuffer (CTextureBuffer&& other) noexcept {
    Copy (other, CTextureBuffer::eMove);
}


CTextureBuffer& CTextureBuffer::Create (SDL_Surface* image) {
    m_width = image->w;
    m_height = image->h;
    m_componentCount = image->pitch / image->w;
    if (m_componentCount < 3) {
        SDL_Surface* h = image;
        image = SDL_ConvertSurfaceFormat (image, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface (h);
        m_componentCount = image->pitch / image->w;
    }
    m_internalFormat = (m_componentCount == 4) ? GL_RGBA : GL_RGB;
    m_format = (m_componentCount == 4) ? GL_RGBA : GL_RGB;
    m_dataSize = m_width * m_height * m_componentCount;
    m_data = malloc (m_dataSize);
    memcpy (m_data, image->pixels, m_dataSize);
    SDL_FreeSurface (image);
    return *this;
}


CTextureBuffer& CTextureBuffer::operator= (CTextureBuffer other) {
    return Copy (other);
}

#if 0
CTextureBuffer& CTextureBuffer::operator= (CTextureBuffer&& other) {
    return Copy (other, 1);
}
#endif

CTextureBuffer& CTextureBuffer::Copy (CTextureBuffer& other, eCopyMode mode) {
    m_width = other.m_width;
    m_height = other.m_height;
    m_componentCount = other.m_componentCount;
    m_internalFormat = other.m_internalFormat;
    m_format = other.m_format;
    m_dataSize = other.m_dataSize;
    if (mode == eClone) {
        m_data = malloc (m_dataSize);
        memcpy (m_data, other.m_data, m_dataSize);
    }
    else {
        m_data = other.m_data;
        if (mode == eMove)
            other.m_data = nullptr;
        else
            m_isAlias = true;
    }
    return *this;
}

// =================================================================================================

bool CTexture::Create (void) {
    Destroy ();
    glGenTextures (1, &m_handle);
    return (m_handle > 0);
}


void CTexture::Destroy (void) {
    if (m_handle != 0) {
        Release();
        glDeleteTextures (1, &m_handle);
        m_handle = 0;
    }
}

bool CTexture::Available (void) {
    return (m_handle > 0) && (m_buffers.Length () > 0);
}


void CTexture::Bind (void) {
    if (Available ()) {
        glBindTexture (m_type, m_handle);
    }
}


void CTexture::Release (void) {
    if (Available ())
        glBindTexture (m_type, 0);
    }


void CTexture::SetParams (void) {
    if (m_useMipMaps) {
        glTexParameteri (m_type, GL_GENERATE_MIPMAP, GL_TRUE);
        glTexParameteri (m_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri (m_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        glTexParameteri (m_type, GL_GENERATE_MIPMAP, GL_FALSE);
        glTexParameteri (m_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri (m_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}


void CTexture::Wrap (void) {
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapMode);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapMode);
}


void CTexture::Enable (int tmu) {
    glActiveTexture (GL_TEXTURE0 + tmu);
    glEnable (m_type);
    Bind ();
    SetParams ();
    Wrap ();
}


void CTexture::Disable (void) {
    Release ();
    glDisable (m_type);
}


void CTexture::Deploy (int bufferIndex) {
    if (Available ()) {
        Bind ();
        SetParams ();
        CTextureBuffer& texBuf = m_buffers [bufferIndex];
        glTexImage2D (m_type, 0, texBuf.m_internalFormat, texBuf.m_width, texBuf.m_height, 0, texBuf.m_format, GL_UNSIGNED_BYTE, texBuf.m_data);
        Release ();
    }
}

// --------------------------------------------------------------------------------
// Load loads textures from file. The texture filenames are given in filenames
// An empty filename ("") means that the previously loaded texture should be used here as well
// This makes sense e.g. for cubemaps if several of its faces share the same texture, like e.g. spherical smileys,
// which have a face on one side and flat color on the remaining five sides of the cubemap used to texture them.
// So a smiley cubemap texture list could be specified here like this: ("skin.png", "", "", "", "", "face.png")
// This would cause the skin texture to be reused for in the place of the texture data buffers at positions 2, 3, 4
// and 5. You could also do something like ("skin.png", "", "back.png", "", "face.png") or just ("color.png", "", "", "", "", "")
// for a uniformly textured sphere. The latter case will however be also taken into regard by the cubemap class.
// It allows to pass a single texture which it will use for all faces of the cubemap

bool CTexture::Load (CList<CString>& fileNames, bool flipVertically) {
    // load texture from file
    m_fileNames = fileNames;
    CTextureBuffer * texBuf = nullptr;
    for (auto [i, fileName] : fileNames) {
        if (fileName.Empty ())
            m_buffers.Add (-1)->Copy (*texBuf, CTextureBuffer::eCopy);
        else {
            SDL_Surface * image = IMG_Load (fileName.Buffer ());
            if (!image) {
                fprintf (stderr, "Couldn't find '%s'\n", (char*) (fileName));
                return false;
            }
            texBuf = m_buffers.Add (-1);
            texBuf->Create (image);
        }
    }
    return true;
}


bool CTexture::CreateFromFile (CList<CString>& fileNames, bool flipVertically) {
    if (!Create ())
        return false;
    if (fileNames.Empty ())
        return true;
    if (!Load (fileNames, flipVertically))
        return false;
    Deploy ();
    return true;
}


 bool CTexture::CreateFromSurface(SDL_Surface* surface) {
    if (!Create ())
        return false;
    m_buffers.Append (CTextureBuffer (surface));
    return true;
}

// =================================================================================================
