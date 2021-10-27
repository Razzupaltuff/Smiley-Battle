#include "torus.h"

// =================================================================================================

void CTorus::Create(int quality, float width, float height) {
    m_quadTexCoords = { CTexCoord(0,0), CTexCoord(1,0), CTexCoord(1,1), CTexCoord(0,1) };
    m_vertexCount = CreateVertices(quality, width, height);
    // we now have four vertex rings of m_vertexCount vertices each in the vertex buffer:
    // @ 0: lower outer vertices
    // @ vertexCount: lower inner vertices
    // @ 2 * vertexCount: upper outer vertices
    // @ 3 * vertexCount: upper inner vertices
    // now create quad indices #include "that. We will need four quad stripes:
    // outer vertical, inner vertical, lower horizontal, upper horizontal
    CreateIndex();
    CreateVAO();
}


// create four vertex rings. Resolution depends on quality ()
size_t CTorus::CreateVertices(int quality, float width, float height) {
    // compute lower outer vertex ring
    size_t vertexCount = CreateCircle(4 * size_t(pow(2, quality)), 0.5f, -height / 2.0f);
    // compute lower inner vertex ring
    float r = 1.0f - width;
    for (size_t i = 0; i < vertexCount; i++) {
        CVector& v = m_vertices[i];
        m_vertices.Append(CVector(v.X() * r, v.Y(), v.Z() * r));
    }
    // compute upper vertex rings by copying the lower vertex rings and replacing their y coordinates
    float y = height / 2.0f;
    for (size_t i = 0; i < 2 * vertexCount; i++) {
        CVector& v = m_vertices[i];
        m_vertices.Append(CVector(v.X(), y, v.Z()));
    }
    return vertexCount;
}


// create circular vertex coordinates    
size_t CTorus::CreateCircle(size_t vertexCount, float r, float y) {
    // Rad = lambda a : a / 180.0 * np.pi
    for (size_t i = 0; i < vertexCount; i++) {
        float a = float (i) / float (vertexCount) * 2.0f * float (M_PI);
        m_vertices.Append(CVector(cos(a) * r, y, sin(a) * r));
    }
    return vertexCount;
}


void CTorus::CreateIndex(void) {
    // create quads between lower and upper outer rings
    CreateQuadIndex(0, m_vertexCount * 2);
    // create quads between lower and upper inner rings
    CreateQuadIndex(m_vertexCount, m_vertexCount * 3);
    // create quads between lower outer and inner rings
    CreateQuadIndex(0, m_vertexCount);
    // create quads between upper outer and inner rings
    CreateQuadIndex(m_vertexCount * 2, m_vertexCount * 3);
}


// construct vertex indices for a quad stripe
// This fully depends on the sequence of vertices in the vertex buffer
void CTorus::CreateQuadIndex(size_t i, size_t j) {
    size_t k = 0;
    while (true) {
        size_t h = k;
        k = (h + 1) % m_vertexCount;
        CArray<size_t>* a = m_indices.m_appData.Add (-1);
        *a = { h + i, h + j, k + j, k + i };
        m_texCoords.m_appData += m_quadTexCoords;
        if (k == 0)
            break;
    }
}


// =================================================================================================
