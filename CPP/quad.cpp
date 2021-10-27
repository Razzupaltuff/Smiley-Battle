
#include "quad.h"

// =================================================================================================

void CQuad::CreateTexCoords(void) {
    if (m_texture && (m_texture->WrapMode() == GL_REPEAT)) {
        for (auto [i, v] : m_vertices.m_appData)
            m_texCoords.Append (CTexCoord(v.X(), v.Z()));
    }
    else {
        m_texCoords.Append (CTexCoord(0, 1));
        m_texCoords.Append (CTexCoord(0, 0));
        m_texCoords.Append (CTexCoord(1, 0));
        m_texCoords.Append (CTexCoord(1, 1));
    }
}


void CQuad::CreateVAO(void) {
    if (m_vertices.m_appData.Length()) {
        m_vao.Init (GL_QUADS, m_texture, m_color);
        m_vertices.Setup();
        m_texCoords.Setup();
        m_vao.Enable();
        m_vao.AddVertexBuffer("Vertex", m_vertices.GLData(), m_vertices.GLDataLength () * 4, GL_FLOAT, 3);
        m_vao.AddVertexBuffer("TexCoord", m_texCoords.GLData(), m_texCoords.GLDataLength () * 4, GL_FLOAT, 2);
        m_vao.Disable();
    }
}


void CQuad::Render(void) {
    if (m_vao.IsValid()) {
        m_vao.SetTexture(m_texture);
        m_vao.SetColor(m_color);
        m_vao.Render(true);
    }
}


// fill 2D area defined by x and y components of vertices with color color
void CQuad::Fill(CVector color, float alpha) {
#if 0
    m_vao.SetTexture(nullptr);
    m_vao.SetColor(color);
    m_vao.Render();
#else
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    for (auto [i, v] : m_vertices.m_appData) {
        glColor4f(color.X(), color.Y(), color.Z(), alpha);
        glVertex2f(v.X(), v.Y());
    }
    glEnd();
#endif       
}

// =================================================================================================
