#include "cubemap.h"

// =================================================================================================
// Load cubemap textures from file and generate an OpenGL cubemap 

void CCubemap::SetParams(void) {
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}


void CCubemap::Deploy(int bufferIndex) {
    if (Available()) {
        Bind();
        SetParams();
        int i, l = int (m_buffers.Length());
        // put the available textures on the cubemap as far as possible and put the last texture on any remaining cubemap faces
        // Reguar case six textures: One texture for each cubemap face
        // Special case one textures: all cubemap faces bear the same texture
        // Special case two textures: first texture goes to first 5 cubemap faces, 2nd texture goes to 6th cubemap face. Special case for smileys with a uniform skin and a face                    
        CTextureBuffer* texBuf = nullptr;
        for (i = 0; i < l; i++) {
            texBuf = &m_buffers [i];
            glTexImage2D(GLenum (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, texBuf->m_internalFormat, texBuf->m_width, texBuf->m_height, 0, texBuf->m_format, GL_UNSIGNED_BYTE, texBuf->m_data);
        }
        for (; i < 6; i++)
            glTexImage2D(GLenum(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, texBuf->m_internalFormat, texBuf->m_width, texBuf->m_height, 0, texBuf->m_format, GL_UNSIGNED_BYTE, texBuf->m_data);
        Release ();
    }
}

// =================================================================================================
