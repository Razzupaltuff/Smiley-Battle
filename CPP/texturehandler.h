#pragma once

#include "glew.h"
#include "texture.h"
#include "cubemap.h"
#include "clist.h"

// =================================================================================================
// Very simply class for texture tracking
// Main purpose is to keep track of all texture objects in the game and return them to OpenGL in
// a well defined and controlled way at program termination without having to bother about releasing
// textures at a dozen places in the game

class CTextureHandler {
    public:
        CList<CTexture*>    m_textures;

        typedef CTexture* (*tGetter) (void);

        CTextureHandler() {}

        ~CTextureHandler () {
            Destroy ();
        }

        void Destroy(void);

        CTexture* GetTexture(void);

        CCubemap* GetCubemap(void);

        bool Remove(CTexture* texture);

        CList<CTexture*> Create(CList<CString>& textureNames, GLenum textureType);

        CList<CTexture*> CreateTextures(CList<CString>& textureNames);

        CList<CTexture*> CreateCubemaps(CList<CString>& textureNames);

        CList<CTexture*> CreateByType(CList<CString>& textureNames, GLenum textureType);

};

extern CTextureHandler* textureHandler;

// =================================================================================================
