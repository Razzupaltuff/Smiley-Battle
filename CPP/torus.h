#pragma once

#include "vector.h"
#include "texcoord.h"
#include "vertexdatabuffers.h"
#include "mesh.h"

// =================================================================================================

class CTorus : public CMesh {
    public:
        CList<CTexCoord>    m_quadTexCoords;
        size_t              m_vertexCount;

        CTorus(CTexture* texture = nullptr, CList<CString> textureNames = CList<CString>(), CVector color = CVector(1, 1, 1)) : CMesh (), m_vertexCount(0) {
            m_quadTexCoords = { CTexCoord(0, 0), CTexCoord(1, 0), CTexCoord(1, 1), CTexCoord(0, 1) };
            CMesh::Init (GL_QUADS, texture, textureNames, GL_TEXTURE_2D, color);
        }

        void Create(int quality, float width, float height);

        size_t CreateVertices(int quality, float width, float height);

        size_t CreateCircle(size_t vertexCount, float r, float y);

        void CreateIndex(void);

        void CreateQuadIndex(size_t i, size_t j);
};

// =================================================================================================
