#pragma once

#include "glew.h"
#include <string.h>

// =================================================================================================
// OpenGL vertex buffer handling: Creation, sending attributes to OpenGL, binding for rendering

class CVBO {
    public:

        int         m_index;
        const char* m_type;
        GLenum      m_bufferType;
        void *      m_data;
        GLuint      m_handle;
        GLsizei     m_size;
        size_t      m_itemSize;
        GLsizei     m_itemCount;
        GLint       m_componentCount;
        GLenum      m_componentType;

        CVBO(const char* type = "", GLint bufferType = GL_ARRAY_BUFFER);

        CVBO(CVBO const& other) {
            Copy (other);
        }

        CVBO& operator=(CVBO const& other) {
            Copy (other);
            return *this;
        }

        CVBO& operator=(CVBO&& other) noexcept {
            Copy (other);
            return *this;
        }

        CVBO& Copy (CVBO const& other) {
            m_index = other.m_index;
            m_type = other.m_type;
            m_bufferType = other.m_bufferType;
            m_data = other.m_data;
            m_handle = other.m_handle;
            m_size = other.m_size;
            m_itemSize = other.m_itemSize;
            m_itemCount = other.m_itemCount;
            m_componentCount = other.m_componentCount;
            m_componentType = other.m_componentType;
            return *this;
        }

        inline void Bind(void) {
            glBindBuffer(m_bufferType, m_handle);
        }

        inline void Release(void) {
            glBindBuffer(m_bufferType, 0);
        }

        inline void EnableAttribs(void) {
            if (m_index > -1)
                glEnableVertexAttribArray(m_index);
        }

        inline void DisableAttribs(void) {
            if (m_index > -1)
                glDisableVertexAttribArray(m_index);
        }

        inline void Describe(void) {
            if (m_index > -1) {
                EnableAttribs();
                glVertexAttribPointer(m_index, m_componentCount, m_componentType, GL_FALSE, 0, nullptr);
            }
        }

        // data: buffer with OpenGL data (float or unsigned int)
        // dataSize: buffer size in bytes
        // componentType: OpenGL type of OpenGL data components (GL_FLOAT or GL_UNSIGNED_INT)
        // componentCount: Number of components of the primitives represented by the render data (3 for 3D vectors, 2 for texture coords, 4 for color values, ...)
        void Create(const char* type, GLint bufferType, int index, void* data, size_t dataSize, size_t componentType, size_t componentCount = 1);

        void Destroy(void);

        size_t ComponentSize (size_t componentType);

};

// =================================================================================================
