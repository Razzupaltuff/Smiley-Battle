#include "vbo.h"

// =================================================================================================
// OpenGL vertex buffer handling: Creation, sending attributes to OpenGL, binding for rendering

// data: buffer with OpenGL data (float or unsigned int)
// dataSize: buffer size in bytes
// componentType: OpenGL type of OpenGL data components (GL_FLOAT or GL_UNSIGNED_INT)
// componentCount: Number of components of the primitives represented by the render data (3 for 3D vectors, 2 for texture coords, 4 for color values, ...)
CVBO::CVBO(const char* type, GLint bufferType) {
    m_index = -1;
    m_type = type;
    m_bufferType = bufferType;
    m_data = nullptr;
    m_handle = 0;
    m_size = 0;
    m_itemSize = 0;
    m_itemCount = 0;
    m_componentCount = 0;
    m_componentType = 0;
    glGenBuffers(1, &m_handle);
}


size_t CVBO::ComponentSize (size_t componentType) {
    switch (componentType) {
        case GL_FLOAT:
            return 4;
        case GL_UNSIGNED_INT:
            return 4;
        case GL_UNSIGNED_SHORT:
            return 2;
        default:
            return 4;
    }
}


void CVBO::Create(const char* type, GLint bufferType, int index, void* data, size_t dataSize, size_t componentType, size_t componentCount) {
    m_type = type;
    m_bufferType = bufferType;
    m_index = index;
    m_data = data;
    m_size = GLsizei (dataSize);
    m_itemSize = ComponentSize (componentType) * componentCount;
    m_itemCount = GLsizei(dataSize / m_itemSize);
    m_componentType = GLenum (componentType);
    m_componentCount = GLint (componentCount);
    Bind();
    glBufferData(m_bufferType, dataSize, m_data, GL_STATIC_DRAW);
    Describe();
}


void CVBO::Destroy(void) {
    if (m_handle > 0) {
        Release();
        glDeleteBuffers(1, &m_handle);
        m_handle = 0;
    }
}

// =================================================================================================
