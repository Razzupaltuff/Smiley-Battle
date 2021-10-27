#pragma once

#include "vbo.h"
#include "shaders.h"

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

class CVAO {
    public:

        CList<CVBO> m_dataBuffers;
        CVBO        m_indexBuffer;
        GLuint      m_handle;
        GLuint      m_shape;
        CTexture*   m_texture;
        CVector     m_color;
        float       m_minBrightness;

        CVAO() : m_handle (0), m_shape (0), m_texture (nullptr), m_color (CVector (1, 1, 1)), m_minBrightness (1.0) {}

        void Init (GLuint shape, CTexture* texture = nullptr, CVector color = CVector(1, 1, 1));

        ~CVAO () {
            Destroy ();
        }

        CVAO(CVAO const& other) {
            Copy (other);
        }

        CVAO& Copy (CVAO const& other);

        void Destroy(void);

        void Reset (void);

        inline bool IsValid(void) {
            return m_handle != 0;
        }

        inline void Enable(void) {
            glBindVertexArray(m_handle);
        }

        inline void Disable(void) {
            glBindVertexArray(0);
        }

        inline void SetTexture(CTexture * texture) {
            m_texture = texture;
        }

        inline void SetColor(CVector color) {
            m_color = color;
        }

        inline void SetMinBrightness(float minBrightness) {
            m_minBrightness = minBrightness;
        }

        inline void ResetMinBrightness(void) {
            m_minBrightness = 0;
        }

        inline CTexture* EnableTexture(void) {
            if (m_texture == nullptr)
                return nullptr;
            m_texture->Enable();
            return m_texture;
        }

        inline void DisableTexture(void) {
            if (m_texture != nullptr)
                m_texture->Disable();
        }

        // add a vertex or index data buffer
        void AddBuffer(const char* type, void* data, size_t dataSize, size_t componentType, size_t componentCount = 0);

        void AddVertexBuffer(const char* type, void* data, size_t dataSize, size_t componentType, size_t componentCount);

        void AddIndexBuffer(void* data, size_t dataSize, size_t componentType);

        void Render(bool useShader = true);

};

// =================================================================================================
