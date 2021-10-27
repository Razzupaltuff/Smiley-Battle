#pragma once

#include "vector.h"
#include "texcoord.h"
#include "vao.h"
#include "vertexdatabuffers.h"
#include "texturehandler.h"

// =================================================================================================

class CQuad {
    public:
        CVertexBuffer   m_vertices;
        CTexCoordBuffer m_texCoords;
        CTexture*       m_texture;
        CVector         m_color;
        CVAO            m_vao;

    CQuad() : m_texture (nullptr) {}

    CQuad(std::initializer_list<CVector> vertices, CTexture* texture = nullptr, CVector color = CVector(1, 1, 1)) {
        m_vertices.m_appData = vertices;
        m_texture = texture;
        m_color = color;
    }


    CQuad operator= (CQuad other) {
        m_vertices = other.m_vertices;
        m_texCoords = other.m_texCoords;
        m_texture = other.m_texture;
        m_color = other.m_color;
        m_vao = other.m_vao;
        return *this;
    }


    void Init (std::initializer_list<CVector> vertices, CTexture* texture = nullptr, CVector color = CVector (1, 1, 1)) {
        m_vertices.m_appData = vertices;
        m_texture = texture;
        m_color = color;
    }


    void Create(void) {
        CreateTexCoords();
        CreateVAO();
    }

    void Destroy(void) {
        m_vao.Destroy();
        textureHandler->Remove (m_texture);
    }

    void CreateTexCoords(void);

    void CreateVAO(void);

    inline void SetTexture(CTexture* texture) {
        m_texture = texture;
    }

    void SetColor(CVector color) {
        m_color = color;
    }

    void Render(void);

    void Fill(CVector color, float alpha = 1.0);

};

// =================================================================================================
