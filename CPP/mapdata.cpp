#include "mapdata.h"
#include "textureHandler.h"
#include "argHandler.h"

// =================================================================================================

CMapData::CMapData()
    : m_vertexCount(0), m_scale(3.0f), m_distanceQuality(1)
    {
        SetupTextures (CList<CString> ({ "wall.png", "floor3.png", "ceiling2.png" }));
        m_distanceQuality = argHandler->IntVal ("distancequality", 0, 1);
        m_spawnHeadings = { 90, 0, -90, 180 };
        m_neighbourOffsets = { CMapPosition(0, 0), CMapPosition(-1, -1), CMapPosition(1, -1), CMapPosition(1, 1), CMapPosition(-1, 1) };
    }


void CMapData::Init(void) {
    //Destroy();
    m_vMin = CVector (1e6, 1e6, 1e6);
    m_vMax = CVector (-1e6, -1e6, -1e6);
    m_vertexCount = 0;
    m_mesh.Init (GL_QUADS, m_textures[0], CList<CString>());
    m_segmentMap = CSegmentMap(m_scale, m_distanceQuality);
}


// create all textures required for map rendering (walls, floor, ceiling)
void CMapData::SetupTextures(CList<CString>textureNames) {
    m_textures.Destroy();
    m_textures = textureHandler->CreateTextures(textureNames);
}


void CMapData::Destroy(void) {
    m_walls.Destroy();
    m_mesh.Destroy();
    m_stringMap.Destroy();
    m_segmentMap.Destroy();
}

// =================================================================================================
