#include "map.h"
#include "gameData.h"
#include "actorHandler.h"
#include "argHandler.h"

// =================================================================================================

CMap::CMap (CList<CString> textureNames, CVector color, int distanceQuality) 
    : CMapData ()
{
}


// create a floor or ceiling quad using the specified texture
CQuad* CMap::CreateQuad(float y, CTexture* texture, CVector color, CQuad* q) {
    *q = CQuad ({ CVector(m_vMin.X(), y, m_vMin.Z()), CVector(m_vMin.X(), y, m_vMax.Z()), CVector(m_vMax.X(), y, m_vMax.Z()), CVector(m_vMax.X(), y, m_vMin.Z()) }, texture, color);
    q->Create();
    return q;
}


void CMap::Translate (CVector t) {
    for (auto [i, v] : m_mesh.m_vertices.m_appData)
        v += t;
    for (auto [i, w] : m_walls)
        w.Translate (t);
}


void CMap::Build(void) {
    //CMapData::Init ();
    m_vMin.Z() = -m_vMin.Z();
    m_vMax.Z() = -m_vMax.Z();
    Translate(CVector(0, 0, m_vMax.Z()));        // translate the map into the view space
    CreateQuad (m_vMin.Y (), GetTexture (1), CVector (1, 1, 1), &m_floor);
    CreateQuad (m_vMax.Y (), GetTexture (2), CVector (1, 1, 1), &m_ceiling);
    m_segmentMap.Build(m_stringMap, m_walls, m_scale);  // create the segment structure
    CreateVAO();
}


void CMap::Destroy(void) {
    m_floor.Destroy ();
    m_ceiling.Destroy ();
    CMapData::Destroy();
    m_mesh.Destroy();
}


void CMap::Render(void) {
    glDisable(GL_CULL_FACE);
#if 1
    m_mesh.Render();
#else
    glUseProgram (0);
    m_mesh.GetTexture ()->Enable ();
    float* pv = (float*) m_mesh.m_vertices.GLData ();
    float* pt = (float*) m_mesh.m_texCoords.GLData ();
    for (int i = m_mesh.m_vertices.GLDataLength () / 12; i; i--) {
        glBegin (GL_QUADS);
        for (int h = 4; h; h--, pv += 3, pt += 2) {
            glVertex3fv (pv);
            glTexCoord2fv (pt);
        }
        glEnd ();
    }
    m_mesh.GetTexture ()->Disable ();
#endif
    m_floor.Render();
    m_ceiling.Render();
    glEnable(GL_CULL_FACE);
}


auto CMap::SegmentAt(CVector position) {
    struct retVals {
        int x, y;
    };
    return retVals{ int(position.X()) / int(m_scale), int(m_segmentMap.m_height + int(position.Z() / m_scale) - 1) }; // segments are added in reversed z order
}


// gather all walls #include "the segment a potentially colliding object sits in plus all walls
// #include "the diagonally adjacent segments (this will yield all relevant walls without duplicates)
size_t CMap::GetNearbyWalls(CVector position, CList<CWall*>& walls) {
    auto [x, y] = SegmentAt(position);
    for (auto o : m_neighbourOffsets) {
        CMapSegment* s = m_segmentMap.GetSegment(x + o->m_x, y + o->m_y);
        if (s) {
            for (auto [i, w] : s->m_walls)
                walls.Append(w);
        }
    }
    return walls.Length();
}



// determine the amount of actors in each map segment
void CMap::CountActors(void) {
    m_segmentMap.ResetActorCounts();
    for (auto [i, a] : actorHandler->m_actors) {
        if (!a->HavePosition())
            continue;
        CVector p = a->GetPosition();
        if (!Contains(p.X(), p.Z()))
            fprintf(stderr, "actor '%s' is out of map", a->m_camera.m_name.Buffer());
        else {
            auto [x, y] = SegmentAt(p);
            m_segmentMap.CountActorAt(x, y);
        }
    }
}


auto CMap::RandomSegment(void) {
    struct retVal {
        int x, y;
    };
    return retVal { rand () % m_segmentMap.m_width, rand () % m_segmentMap.m_height };
}


float CMap::FindSpawnAngle(CMapSegment& s) {
    int h = rand() % s.m_connections + 1;
    for (int i = 0; i < 4; i++)
        if (s.m_connected[i] && !--h)
            return m_spawnHeadings[i];
    return 0;
}


// find a random spawn position by looking for a segment that is not inaccessible and has no actors inside it
void CMap::FindSpawnPosition(CActor* actor) {
    // actor.needSpawnPosition = false
    // return
    for (;;) {
        auto [x, y] = RandomSegment();
        CMapSegment& s = m_segmentMap.m_segments[y][x];
        if (s.m_connections && !s.m_actorCount) {
            m_segmentMap.CountActorAt(x, y);
            actor->SetPosition(s.m_center);
            actor->m_camera.BumpPosition();
            actor->m_camera.SetOrientation (CVector (0, FindSpawnAngle (s), 0));
            actor->m_needPosition = false;
            return;
        }
    }
}


void CMap::SetSegment(int id, CMapSegment& segment) {
    auto [x, y] = SegPosFromId(id);
    m_segmentMap.m_segments[y][x] = segment;
}


// compute distance between two actors
// by adding the distances of each actor to the edge center node of its segment that lies
// in the path between the actors to the distance #include "the segment distance table
float CMap::Distance(CVector v0, CVector v1) {
    if (m_distanceQuality == 0)
        return (v0 - v1).Len() * 1.3f;
    auto p0 = SegmentAt(v0);
    int s0 = SegPosToId(p0.x, p0.y);
    auto p1 = SegmentAt(v1);
    int s1 = SegPosToId(p1.x, p1.y);
    if (s0 == s1)
        return 0;
    CRouteData& rd = m_segmentMap.Distance(s0, s1);
    return rd.m_distance + (v0 - rd.m_startPos).Len() + (v1 - rd.m_endPos).Len();
}


// add a vertex to the vertex list && update map boundaries
void CMap::AddVertex (CVector v) {
    m_mesh.m_vertices.Append (v);
    m_vMin.Minimize (v);
    m_vMax.Maximize (v);
    m_vertexCount++;
}

// =================================================================================================
