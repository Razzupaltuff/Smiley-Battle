#pragma once 

#include "texture.h"

// =================================================================================================
// Load cubemap textures from file and generate an OpenGL cubemap 

class CCubemap : public CTexture {
    public:
        CCubemap() : CTexture(GL_TEXTURE_CUBE_MAP) {}

        virtual void SetParams (void);

        virtual void Deploy (int bufferIndex = 0);

};

// =================================================================================================
