#pragma once

#include "cstring.h"
#include "carray.h"
#include "clist.h"
#include "vector.h"
#include "quad.h"
#include "mesh.h"
#include "mapsegments.h"

// =================================================================================================

class CMapData {
    public:
        CList<CWall>            m_walls;
        CQuad                   m_floor;
        CQuad                   m_ceiling;
        CMesh                   m_mesh;
        CVector                 m_color;
        CVector                 m_vMin;             // map boundaries
        CVector                 m_vMax;
        int                     m_vertexCount;      // total number of wall vertices
        float                   m_scale;            // scale of the map (base unit is 1.0)
        int                     m_distanceQuality;
        CSegmentMap             m_segmentMap;       // map segments
        CList<CString>          m_stringMap;        // map layout data
        CList<CTexture*>        m_textures;
        CArray<float>           m_spawnHeadings;
        CArray<CMapPosition>    m_neighbourOffsets;


        CMapData();

        void Init(void);

        // create all textures required for map rendering (walls, floor, ceiling)
        void SetupTextures(CList<CString>textureNames);

        void Destroy(void);

        inline CTexture* GetTexture(size_t i) {
            return (m_textures.Length() > i) ? m_textures[i] : nullptr;
        }

};

// =================================================================================================
