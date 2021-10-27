#include "mesh.h"
#include "textureHandler.h"

// =================================================================================================

void CMesh::Init(GLenum shape, CTexture* texture, CList<CString> textureNames, GLenum textureType, CVector color) {
    m_shape = shape;
    m_shapeSize = ShapeSize();
    m_indices.m_componentCount = m_shapeSize;
    PushColor(color);
    SetupTexture(texture, textureNames, textureType);
}


void CMesh::CreateVAO(void) {
    m_vao.Init (m_shape, GetTexture(), GetColor());
    m_vao.Enable();
    if (m_vertices.AppDataLength()) {
        m_vertices.Setup();
        m_vao.AddVertexBuffer("Vertex", m_vertices.GLData(), m_vertices.GLDataLength() * 4, GL_FLOAT, 3);
    }
    if (m_normals.AppDataLength()) {
        m_normals.Setup();
        // in the case of an icosphere, the vertices also are the vertex normals
        m_vao.AddVertexBuffer("Normal", m_vertices.GLData(), m_vertices.GLDataLength() * 4, GL_FLOAT, 3);
    }
    if (m_texCoords.AppDataLength()) {
        m_texCoords.Setup();
        m_vao.AddVertexBuffer("TexCoord", m_texCoords.GLData(), m_texCoords.GLDataLength() * 4, GL_FLOAT, 2);
    }
    if (m_vertexColors.AppDataLength()) {
        m_vertexColors.Setup();
        m_vao.AddVertexBuffer("Color", m_vertexColors.GLData(), m_vertexColors.GLDataLength() * 4, GL_FLOAT, 4);
    }
    if (m_indices.AppDataLength()) {
        m_indices.Setup();
        m_vao.AddIndexBuffer(m_indices.GLData(), m_indices.GLDataLength() * 4, GL_UNSIGNED_INT);
    }
    m_vao.Disable();
}


void CMesh::SetupTexture(CTexture* texture, CList<CString> textureNames, GLenum textureType) {
    if (!textureNames.Empty())
        textureHandler->CreateByType (textureNames, textureType);
    else if (texture != nullptr)
        m_textures.Append (texture);
}


void CMesh::PushTexture(CTexture* texture) {
    if (texture != nullptr)
        m_textures.Append(texture);
}


void CMesh::PopTexture(void) {
    if (!m_textures.Empty())
        m_textures.Pop(-1);
}


CTexture* CMesh::GetTexture(void) {
    if (m_textures.Length())
        return m_textures[-1];
    return nullptr;
}


void CMesh::PushColor(CVector color) {
    m_colors.Append(color);
}


void CMesh::PopColor(void) {
    m_colors.Pop(-1);
}


CVector CMesh::GetColor(void) {
    if (!m_colors.Empty())
        return m_colors[-1];
    return CVector(1, 1, 1);
}


bool CMesh::EnableTexture(void) {
    CTexture* texture = GetTexture();
    if (!texture)
        return false;
    texture->Enable();
    return true;
}


void CMesh::DisableTexture(void) {
    CTexture* texture = GetTexture();
    if (texture)
        texture->Disable();
}


void CMesh::Render(void) {
    if (m_vao.IsValid()) {
        SetTexture();
        SetColor();
        m_vao.Render();
    }
}


void CMesh::SetTexture(void) {
    m_vao.SetTexture(GetTexture());
}


void CMesh::SetColor(void) {
    m_vao.SetColor(GetColor());
}


void CMesh::Destroy (void) {
    m_vertices.Destroy ();
    m_normals.Destroy ();
    m_texCoords.Destroy ();
    m_vertexColors.Destroy ();
    m_indices.Destroy ();
    m_textures.Destroy ();
    m_colors.Destroy ();
    m_vao.Destroy ();
}

// =================================================================================================
