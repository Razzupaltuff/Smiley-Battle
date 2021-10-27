#include "vao.h"
#include "renderer.h"

// =================================================================================================
// "Premium version of" OpenGL vertex array objects. CVAO instances offer methods to convert python
// lists into the corresponding lists of OpenGL items (vertices, normals, texture coordinates, etc)
// The current implementation requires a fixed order of array buffer creation to comply with the 
// corresponding layout positions in the shaders implemented here.
// Currently offers shaders for cubemap and regular (2D) texturing.
// Implements loading of varying textures, so an application item derived from or using a CVAO instance
// (e.g. an ico sphere) can be reused by different other application items that require different 
// texturing. This implementation basically allows for reusing one single ico sphere instance whereever
// a sphere is needed.
// Supports indexed and non indexed vertex buffer objects.
//
// // Due to the current shader implementation (fixed position layout), buffers need to be passed in a
// fixed sequence: vertices, colors, ...
// TODO: Expand shader for all kinds of inputs (texture coordinates, normals)
// See also https://qastack.com.de/programming/8704801/glvertexattribpointer-clarification

void CVAO::Init (GLuint shape, CTexture* texture, CVector color) {
    m_shape = shape;
    glGenVertexArrays (1, &m_handle);
    m_texture = texture;
    m_color = color;
    m_minBrightness = 0.0f;
}


void CVAO::Destroy(void) {
    for (auto [i, vbo] : m_dataBuffers)
        vbo.Destroy();
    if (m_handle > 0) {
        Disable();
        glDeleteVertexArrays(1, &m_handle);
        m_handle = 0;
    }
}


CVAO& CVAO::Copy (CVAO const& other) {
    m_dataBuffers = other.m_dataBuffers;
    m_indexBuffer = other.m_indexBuffer;
    m_handle = other.m_handle;
    m_shape = other.m_shape;
    m_texture = other.m_texture;
    m_color = other.m_color;
    m_minBrightness = other.m_minBrightness;
    return *this;
}


// add a vertex or index data buffer
void CVAO::AddBuffer(const char* type, void * data, size_t dataSize, size_t componentType, size_t componentCount) {
    if (!strcmp(type, "Index"))
        AddIndexBuffer(data, dataSize, componentType);
    else
        AddVertexBuffer(type, data, dataSize, componentType, componentCount);
}


void CVAO::AddVertexBuffer(const char* type, void * data, size_t dataSize, size_t componentType, size_t componentCount) {
    CVBO* buffer = m_dataBuffers.Add (-1);
    buffer->Create(type, GL_ARRAY_BUFFER, int (m_dataBuffers.Length() - 1), data, dataSize, componentType, componentCount);
}


void CVAO::AddIndexBuffer(void * data, size_t dataSize, size_t componentType) {
    m_indexBuffer.Create("Index", GL_ELEMENT_ARRAY_BUFFER, -1, data, dataSize, componentType);
}


void CVAO::Render(bool useShader) {
    int shaderId = shaderHandler->SelectShader(useShader, EnableTexture());
    shaderHandler->Shader (shaderId).SetUniformVector("fillColor", m_color);
    if (shaderId == 0) {
        shaderHandler->Shader (shaderId).SetUniformFloat ("minBrightness", m_minBrightness);
    }
    Enable();
    if (!m_indexBuffer.m_data)
        glDrawArrays(m_shape, 0, m_dataBuffers[0].m_itemCount); // draw non indexed arrays
    else
        glDrawElements(m_shape, m_indexBuffer.m_itemCount, m_indexBuffer.m_componentType, nullptr); // draw using an index buffer
    Disable();
    if (shaderId > -1)
        glUseProgram(0);
    DisableTexture();
}

// =================================================================================================
