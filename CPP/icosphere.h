#include "glew.h"
#include "vector.h"
#include "texture.h"
#include "vertexdatabuffers.h"
#include "mesh.h"
#include "cstring.h"
#include "cavltree.h"

// =================================================================================================
// Basic ico sphere class.
// Ico spheres are created from basic geometric structures with equidistant corners (vertices},
// e.g. cubes, octa- or icosahedrons.
// The faces of the basic structures are subdivided in equally sized child faces. The resulting new
// vertices are normalized. The more iterations this is run through, the finer the resulting mesh
// becomes and the smoother does the sphere look.

class CIcoSphere : public CMesh {
    public:
        size_t          m_vertexCount;
        size_t          m_faceCount;
        CList<CVector>  m_faceNormals;


        typedef struct {
            size_t  i1, i2;
        } tVertexKey;

        static int KeyCmp(tVertexKey const& ki, tVertexKey const& kj) {
            return (ki.i1 < kj.i1) ? -1 : (ki.i1 > kj.i1) ? 1 : (ki.i2 < kj.i2) ? -1 : (ki.i2 > kj.i2) ? 1 : 0;
        }

        CIcoSphere(GLenum shape = GL_TRIANGLES) : CMesh(), m_vertexCount (0), m_faceCount (0) {
            CMesh::Init (shape);
        }

        CIcoSphere(GLenum shape, CTexture* texture, CList<CString> textureNames, CVector color = CVector(1, 1, 1))
            : CMesh() {
            m_vertexCount = 0;
            m_faceCount = 0;
            CMesh::Init (shape, texture, textureNames, GL_TEXTURE_CUBE_MAP, color);
        }

    protected:
        size_t VertexIndex(CAvlTree<tVertexKey, size_t>& indexLookup, size_t i1, size_t i2);

        CList<CVector> CreateFaceNormals(CVertexBuffer& vertices, CList<CArray<size_t>>& faces);

};

// =================================================================================================
// Create an ico sphere based on a shape with triangular faces

class CTriangleIcoSphere : public CIcoSphere {
    public:
        CTriangleIcoSphere(CTexture* texture, CList<CString> textureNames, CVector color = CVector(1, 1, 1))
            : CIcoSphere(GL_TRIANGLES, texture, textureNames, color)
        {}

        CTriangleIcoSphere() : CIcoSphere(GL_TRIANGLES) {}

        void Create(int quality);

    protected:
        void CreateBaseMesh(int quality = 1);

        void CreateOctahedron(void);

        void CreateIcosahedron(void);

        void SubDivide(CList<CArray<size_t>>& faces);

        void Refine(CList<CArray<size_t>>& faces, int quality);

};

// =================================================================================================
// Create an ico sphere based on a shape with rectangular faces

class CRectangleIcoSphere : public CIcoSphere {
    public:
        CRectangleIcoSphere(CTexture * texture, CList<CString> textureNames, CVector color = CVector(1, 1, 1))
            : CIcoSphere(GL_TRIANGLES, texture, textureNames, color)
        {}

        CRectangleIcoSphere() : CIcoSphere(GL_QUADS) {}

        void Create(int quality);

    protected:
        void CreateBaseMesh(int quality);

        void CreateCube(void);

        void CreateIcositetragon(void);
                
        void SubDivide(CList<CArray<size_t>>& faces);

        void Refine(CList<CArray<size_t>>& faces, int quality);

};

// =================================================================================================
