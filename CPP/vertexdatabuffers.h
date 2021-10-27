#pragma once

#include "glew.h"
#include "vector.h"
#include "texcoord.h"
#include "carray.h"
#include "clist.h"

// =================================================================================================
// Data buffer handling as support for vertex buffer operations.
// Interface classes between python and OpenGL representations of rendering data
// Supplies iterators, assignment and indexing operatores and transparent data conversion to OpenGL
// ready format (Setup() method)

template < class APPDATA_T, class GLDATA_T >
class CVertexDataBuffer {
    public:
        CList<APPDATA_T>    m_appData;
        CArray<GLDATA_T>    m_glData;
        size_t              m_componentCount;

        CVertexDataBuffer(size_t componentCount = 1) : m_componentCount (componentCount) {}

        CVertexDataBuffer& operator= (CVertexDataBuffer& other) {
            return Copy (other);
        }

        CVertexDataBuffer& Copy (CVertexDataBuffer const& other) {
            if (this != &other) {
                m_appData = other.m_appData;
                m_glData = other.m_glData;
                m_componentCount = other.m_componentCount;
            }
            return *this;
        }

        virtual CArray<GLDATA_T>& Setup(void) = 0;

        operator GLvoid* () {
            return (GLvoid*)m_glData.Buffer();
        }

        inline size_t AppDataLength(void) {
            return m_appData.Length();
        }

        inline void* GLData(void) {
            return (void*)m_glData.Buffer();
        }

        inline size_t GLDataLength(void) {
            return m_glData.Length();
        }

        inline bool Append(APPDATA_T data) {
            return m_appData.Append(data);
        }

        inline APPDATA_T& operator[] (const size_t i) {
            return m_appData[i];
        }

        void Destroy (void) {
            m_appData.Destroy ();
            m_glData.Destroy ();
        }

        ~CVertexDataBuffer () {
            Destroy ();
        }

};

// =================================================================================================
// Buffer for vertex data (4D xyzw vector of type numpy.float32). Also used for normal data.
// A pre-populated data buffer can be passed to the constructor

class CVertexBuffer : public CVertexDataBuffer < CVector, GLfloat > {
    public:
        CVertexBuffer() : CVertexDataBuffer(3) {}

        // Create a densely packed numpy array from the vertex data
        virtual CArray<GLfloat>& Setup(void) {
            m_glData.Create(m_appData.Length() * 3);
            size_t n = 0;
            for (auto [i, v] : m_appData) {
                m_glData[n++] = v.X();
                m_glData[n++] = v.Y();
                m_glData[n++] = v.Z();
            }
            return m_glData;
        }
};

// =================================================================================================
// Buffer for texture coordinate data (2D uv vector). Also used for color information
// A pre-populated data buffer can be passed to the constructor

class CTexCoordBuffer : public CVertexDataBuffer < CTexCoord, GLfloat > {
    public:
        CTexCoordBuffer() : CVertexDataBuffer(2) {}

        // Create a densely packed numpy array from the vertex data
        virtual CArray<GLfloat>& Setup(void) {
            m_glData.Create(m_appData.Length() * 2);
            size_t n = 0;
            for (auto [i, v] : m_appData) {
                m_glData[n++] = v.m_u;
                m_glData[n++] = v.m_v;
            }
            return m_glData;
        }
};

// =================================================================================================
// Buffer for color data (4D rgba vector of type numpy.float32). 
// A pre-populated data buffer can be passed to the constructor

class CColorBuffer : public CVertexDataBuffer < CVector, GLfloat > {
public:
    CColorBuffer() : CVertexDataBuffer(4) {}

    // Create a densely packed numpy array from the vertex data
    virtual CArray<GLfloat>& Setup(void) {
        m_glData.Create(m_appData.Length() * 4);
        size_t n = 0;
        for (auto [i, v] : m_appData) {
            m_glData[n++] = v.X();
            m_glData[n++] = v.Y();
            m_glData[n++] = v.Z();
            m_glData[n++] = v.W();
        }
        return m_glData;
    }
};

// =================================================================================================
// Buffer for index data (n-tuples of integer values). 
// Requires an additional componentCount parameter, as index count depends on the vertex count of the 
// primitive being rendered (quad: 4, triangle: 3, line: 2, point: 1)

class CIndexBuffer : public CVertexDataBuffer < CArray<size_t>, GLuint > {
public:
    CIndexBuffer(size_t componentCount = 1) : CVertexDataBuffer(componentCount) {}

    // Create a densely packed numpy array from the vertex data
    virtual CArray<GLuint>& Setup(void) {
        m_glData.Create(m_appData.Length() * m_componentCount);
        size_t n = 0;
        for (auto [i, v] : m_appData)
            for (auto const& j : v) 
                m_glData[n++] = GLuint (*j);
        return m_glData;
    }

    CIndexBuffer operator= (CIndexBuffer const& other) {
        Copy (other);
        return *this;

    }

};

// =================================================================================================
