#pragma once

#include "glew.h"
#include "texture.h"
#include "vao.h"
#include "vertexdatabuffers.h"

// =================================================================================================
// Mesh class definitions for basic mesh information, allowing to pass child classes to functions
// operating with or by meshes
// A mesh is defined by a set of faces, which in turn are defined by vertices, and a set of textures
// The only mesh used in Smiley Battle are ico spheres

class CAbstractMesh {
public:
    virtual void Create(int quality, CTexture* texture, CList<CString> textureNames) = 0;

    virtual void Destroy(void) = 0;

    virtual void PushTexture(CTexture * texture) = 0;

    virtual void PopTexture(void) = 0;

    virtual CTexture * GetTexture(void) = 0;

    virtual void PushColor(CVector color) = 0;

    virtual void PopColor(void) = 0;

    virtual CVector GetColor(void) = 0;

    virtual void Render(void) = 0;
};

// =================================================================================================

class CMesh : public CAbstractMesh {
    public:
        CList<CTexture*>    m_textures;
        CList<CVector>      m_colors;
        CVertexBuffer       m_vertices;
        CVertexBuffer       m_normals;
        CTexCoordBuffer     m_texCoords;
        CColorBuffer        m_vertexColors;
        CIndexBuffer        m_indices;
        CVAO                m_vao;
        GLenum              m_shape;
        size_t              m_shapeSize;

        CMesh() : m_shape (GL_TRIANGLES), m_shapeSize (3) {}

        ~CMesh () {
            Destroy ();
        }

        void Init (GLenum shape, CTexture* texture = nullptr, CList<CString> textureNames = CList<CString> (), GLenum textureType = GL_TEXTURE_2D, CVector color = CVector(1, 1, 1));

        virtual void Create(int quality, CTexture* texture, CList<CString> textureNames) {}

        virtual void Destroy (void);

        inline size_t ShapeSize(void) {
            if (m_shape == GL_QUADS)
                return 4;
            if (m_shape == GL_TRIANGLES)
                return 3;
            if (m_shape == GL_LINES)
                return 2;
            return 1;
        }

        void CreateVAO(void);

        inline CVAO& VAO(void) {
            return m_vao;
        }

        void SetupTexture(CTexture* texture, CList<CString> textureNames, GLenum textureType);

        virtual void PushTexture(CTexture* texture);

        virtual void PopTexture(void);

        virtual CTexture* GetTexture(void);

        virtual void PushColor(CVector color);

        virtual void PopColor(void);

        virtual CVector GetColor(void);

        bool EnableTexture(void);

        void DisableTexture(void);

        void SetTexture(void);

        void SetColor(void);

        virtual void Render(void);

};

// =================================================================================================
