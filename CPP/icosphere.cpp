#include "icosphere.h"

// =================================================================================================
// Basic ico sphere class.
// Ico spheres are created from basic geometric structures with equidistant corners (vertices},
// e.g. cubes, octa- or icosahedrons.
// The faces of the basic structures are subdivided in equally sized child faces. The resulting new
// vertices are normalized. The more iterations this is run through, the finer the resulting mesh
// becomes and the smoother does the sphere look.

size_t CIcoSphere::VertexIndex(CAvlTree<tVertexKey, size_t>& indexLookup, size_t i1, size_t i2) { // find index pair i1,i2 in 
    tVertexKey key;
    if (i1 < i2)
        key = { i1, i2 };
    else
        key = { i2, i1 };
    size_t* keyPtr = indexLookup.Find(key);
    if (keyPtr)
        return *keyPtr;
    indexLookup.Insert(key, m_vertexCount);
    CVector* p = m_vertices.m_appData.Add (-1);
    *p = m_vertices [i1] + m_vertices [i2];
    p->Normalize();
    *p *= 0.5f;
    return m_vertexCount++;
}


CList<CVector> CIcoSphere::CreateFaceNormals(CVertexBuffer& vertices, CList<CArray<size_t>>& faces) {
    CList<CVector> faceNormals;
    for (auto [i, f] : faces)
        faceNormals.Append(CVector::Normal(vertices[f[0]], vertices[f[1]], vertices[f[2]]));
    return faceNormals;
}

// =================================================================================================
// Create an ico sphere based on a shape with triangular faces

void CTriangleIcoSphere::Create(int quality) {
    CreateBaseMesh(0);
    m_vertexCount = m_vertices.AppDataLength ();
    Refine(m_indices.m_appData, quality);
    m_faceCount = m_indices.AppDataLength();
    CreateVAO();
}


void CTriangleIcoSphere::CreateBaseMesh(int quality) {
    if (quality == 0)
        CreateOctahedron();
    else
        CreateIcosahedron();
    for (auto [i, v] : m_vertices.m_appData) {
        v.Normalize();
        v *= 0.5f;
    }
}



void CTriangleIcoSphere::CreateOctahedron(void) {
    float X = 0.5f;
    float Y = float(sqrt(0.5));
    float Z = 0.5;
    m_vertices.m_appData = { 
        CVector(-X,0,-Z), CVector(X,0,-Z), CVector(X,0,Z), CVector(-X,0,Z), CVector(0,-Y,0), CVector(0,Y,0) 
    };
    m_indices.m_appData = { 
        CArray<size_t>({0,1,5}),CArray<size_t>({1,2,5}),CArray<size_t>({2,3,5}),CArray<size_t>({3,0,5}),
        CArray<size_t>({0,1,6}),CArray<size_t>({1,2,6}),CArray<size_t>({2,3,6}),CArray<size_t>({3,0,6}) 
    };
}


void CTriangleIcoSphere::CreateIcosahedron(void) {
    float X = 0.525731112119133606f;
    float Z = 0.850650808352039932f;
    float N = 0.0f;
    m_vertices.m_appData.Destroy ();
    m_vertices.m_appData = { 
        CVector(-X,+N,+Z), CVector(+X,+N,+Z), CVector(-X,+N,-Z), CVector(+X,+N,-Z),
        CVector(+N,+Z,+X), CVector(+N,+Z,-X), CVector(+N,-Z,+X), CVector(+N,-Z,-X),
        CVector(+Z, +X, +N), CVector(-Z, +X, +N), CVector(+Z, -X, +N), CVector(-Z, -X, +N) 
    };
    m_indices.m_appData.Destroy ();
    m_indices.m_appData = {
        CArray<size_t>({0,4,1}), CArray<size_t>({0,9,4}), CArray<size_t>({9,5,4}), CArray<size_t>({4,5,8}), CArray<size_t>({4,8,1}),
        CArray<size_t>({8,10,1}), CArray<size_t>({8,3,10}), CArray<size_t>({5,3,8}), CArray<size_t>({5,2,3}), CArray<size_t>({2,7,3}),
        CArray<size_t>({7,10,3}), CArray<size_t>({7,6,10}), CArray<size_t>({7,11,6}), CArray<size_t>({11,0,6}), CArray<size_t>({0,1,6}),
        CArray<size_t>({6,1,10}), CArray<size_t>({9,0,11}), CArray<size_t>({9,11,2}), CArray<size_t>({9,2,5}), CArray<size_t>({7,2,11})
    };
}


void CTriangleIcoSphere::SubDivide(CList<CArray<size_t>>& faces) {
    CList<CArray<size_t>> subFaces;
    CAvlTree<tVertexKey, size_t> indexLookup;
    indexLookup.SetComparator(CIcoSphere::KeyCmp);
    for (auto [i, f] : faces) {
        size_t i0 = VertexIndex(indexLookup, f[0], f[1]);
        size_t i1 = VertexIndex(indexLookup, f[1], f[2]);
        size_t i2 = VertexIndex(indexLookup, f[2], f[0]);
        CArray<size_t>* a = subFaces.Add (-1);
        *a = { f[0], i0, i2 };
        a = subFaces.Add (-1);
        *a = { f[1], i1, i2 };
        a = subFaces.Add (-1);
        *a = { f[2], i2, i1 };
        a = subFaces.Add (-1);
        *a = { f[0], i1, i2 };
    }
    faces.Destroy ();
    faces.Move (subFaces);
}


void CTriangleIcoSphere::Refine(CList<CArray<size_t>>& faces, int quality) {
    while (quality--)
        SubDivide(faces);
}


// =================================================================================================
// Create an ico sphere based on a shape with rectangular faces

void CRectangleIcoSphere::Create(int quality) {
    CreateBaseMesh(0);
    m_vertexCount = m_vertices.AppDataLength ();
    Refine(m_indices.m_appData, quality);
    m_faceCount = m_indices.AppDataLength();
    m_normals = m_vertices;
    CreateVAO();
}


void CRectangleIcoSphere::CreateBaseMesh (int quality = 1) {
    if (quality == 0) 
        CreateCube();
    else
        CreateIcositetragon();
    for (auto [i, v] : m_vertices.m_appData) {
        v.Normalize();
        v *= 0.5f;
    }
}


void CRectangleIcoSphere::CreateCube(void) {
    float X = 0.5f;
    float Y = 0.5f;
    float Z = 0.5f;
    m_vertices.m_appData = { 
        CVector(-X,-Y,-Z), CVector(+X,-Y,-Z), CVector(+X,+Y,-Z), CVector(-X,+Y,-Z),
        CVector(-X,-Y,+Z), CVector(+X,-Y,+Z), CVector(+X,+Y,+Z), CVector(-X,+Y,+Z) 
    };
    m_indices.m_appData = { 
        CArray<size_t>({0,1,2,3}), CArray<size_t>({0,4,5,1}), CArray<size_t>({0,3,7,4}),
        CArray<size_t>({6,2,1,5}), CArray<size_t>({6,7,3,2}), CArray<size_t>({6,5,4,7})
    };
}


void CRectangleIcoSphere::CreateIcositetragon(void) {
    float X = 0.5f;
    float Y = 0.5f;
    float Z = 0.5f;
    m_vertices.m_appData.Destroy ();
    m_vertices.m_appData = {
        // base cube corner vertices
        CVector(-X, -Y, -Z), CVector(+X, -Y, -Z), CVector(+X, +Y, -Z), CVector(-X, +Y, -Z),
        CVector(-X, +Y, +Z), CVector(-X, -Y, +Z), CVector(+X, +Y, +Z), CVector(+X, -Y, +Z),
        // base cube face center vertices
        CVector(0, 0, -Z), CVector(-X, 0, 0), CVector(+X, 0, 0), CVector(0, 0, +Z), CVector(0, +Y, 0), CVector(0, -Y, 0),
        // front face edge center vertices
        CVector(0, -Y, -Z), CVector(+X, 0, -Z), CVector(0, +Y, +Z), CVector(-X, 0, -Z),
        // left side face edge center vertices
        CVector(-X, +Y, 0), CVector(-X, 0, +Z), CVector(-X, -Y, 0),
        // right side face edge center vertices
        CVector(+X, -Y, 0), CVector(+X, 0, +Z), CVector(+X, +Y, 0),
        // front and bottom face rear edge center vertices / back face top and bottom edge center vertices
        CVector(0, +Y, +Z), CVector(0, -Y, +Z)
    };
    m_indices.m_appData.Destroy ();
    m_indices.m_appData = {
        CArray<size_t>({ 8, 17,  0, 14}), CArray<size_t>({ 8, 14,  1, 15}), CArray<size_t>({ 8, 15,  2, 16}), CArray<size_t>({ 8, 16,  3, 17}),
        CArray<size_t>({ 9, 17,  3, 18}), CArray<size_t>({ 9, 18,  4, 19}), CArray<size_t>({ 9, 19,  5, 20}), CArray<size_t>({ 9, 20,  0, 17}),
        CArray<size_t>({10, 15,  1, 21}), CArray<size_t>({10, 21,  7, 22}), CArray<size_t>({10, 22,  6, 23}), CArray<size_t>({10, 23,  2, 15}),
        CArray<size_t>({11, 22,  7, 25}), CArray<size_t>({11, 25,  5, 19}), CArray<size_t>({11, 19,  4, 24}), CArray<size_t>({11, 24,  6, 22}),
        CArray<size_t>({12, 16,  2, 23}), CArray<size_t>({12, 23,  6, 24}), CArray<size_t>({12, 24,  4, 18}), CArray<size_t>({12, 18,  3, 16}),
        CArray<size_t>({13, 14,  0, 18}), CArray<size_t>({13, 18,  5, 25}), CArray<size_t>({13, 25,  7, 21}), CArray<size_t>({13, 21,  1, 14})
    };
}


// Create 4 child quads per existing quad by evenly subdiving each quad.
// To subdivide, compute the center of each side of a quad and the center of the quad.
// Create child quads between the corners and the center of the parent quad.
// Newly created edge center vertices will be shared with child quads of adjacent parent quads,
// So store them in a lookup table that is indexed with the vertex indices of the parent edge.
void CRectangleIcoSphere::SubDivide(CList<CArray<size_t>>& faces) {
    CList<CArray<size_t>> subFaces;
    CAvlTree<tVertexKey, size_t> indexLookup;
    indexLookup.SetComparator(CIcoSphere::KeyCmp);
    for (auto [i, f] : faces) {
        size_t f0 = f [0];
        size_t f1 = f [1];
        size_t f2 = f [2];
        size_t f3 = f [3];
        size_t i0 = VertexIndex(indexLookup, f0, f1);
        size_t i1 = VertexIndex(indexLookup, f1, f2);
        size_t i2 = VertexIndex(indexLookup, f2, f3);
        size_t i3 = VertexIndex(indexLookup, f3, f0);
        size_t i4 = m_vertexCount++;
        CVector* p = m_vertices.m_appData.Add (-1);
        *p = m_vertices [i0] + m_vertices [i1] + m_vertices [i2] + m_vertices [i3];
        p->Normalize();
        *p *= 0.5f;
        CArray<size_t>* a = subFaces.Add (-1);
        *a = { f0, i0, i4, i3 };
        a = subFaces.Add (-1);
        *a = { f1, i1, i4, i0 };
        a = subFaces.Add (-1);
        *a = { f2, i2, i4, i1 };
        a = subFaces.Add (-1);
        *a = { f3, i3, i4, i2 };
    }
    faces.Destroy ();
    faces.Move (subFaces);
}


void CRectangleIcoSphere::Refine(CList<CArray<size_t>>& faces, int quality) {
    while (quality--)
        SubDivide(faces);
}

// =================================================================================================
