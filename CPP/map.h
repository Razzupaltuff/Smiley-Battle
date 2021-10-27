#pragma once

#include "cstring.h"
#include "carray.h"
#include "clist.h"
#include "vector.h"
#include "plane.h"
#include "texcoord.h"
#include "vertexdatabuffers.h"
#include "quad.h"
#include "vao.h"
#include "mesh.h"
#include "actor.h"
#include "mapdata.h"
#include "mapsegments.h"

// =================================================================================================
// load a map #include "a description / layout file in plain text format (see maps/mazehunt.txt).
// map layout is extremely simple. Everything is rectangular. A map is a rectangle formed by equally 
// sized cuboid segments. Segments may be separated by (flat / 2D) walls.
// A map class instance generates the geometry (wall vertices) and a segment structure #include "the map description.
// Segments have a list of adjacent segments they are connected to (i.e. which are not separated by a wall)
// A simple collision detection handles collisions of spherical objects with map walls.
// The segment structure is used to rapidly identify the walls that are close enough for a collision.
// Segments will also be used for distance calculation to create positional sound.

class CMap : public CMapData {
    public:
        CMap (CList<CString> textureNames = CList<CString> (), CVector color = CVector (1, 1, 1), int distanceQuality = 0);

        inline CMapSegment& operator[] (int index) {
            return m_segmentMap[index];
        }

        void Destroy(void);

        CQuad* CreateQuad(float y, CTexture* texture, CVector color, CQuad * q);

        void Translate (CVector t);

        void Build(void);

        inline void CreateVAO(void) {
            m_mesh.CreateVAO();
        }


        inline bool EnableTexture(void) {
            if (!m_textures[0])
                return false;
            m_textures[0]->Enable();
            return true;
        }


        inline void DisableTexture(void) {
            if (m_textures[0])
                m_textures[0]->Disable();
        }


        inline void RenderFloor(void) {
            glDisable(GL_CULL_FACE);
            m_floor.Render();
            glEnable(GL_CULL_FACE);
        }


        inline void RenderCeiling(void) {
            glDisable(GL_CULL_FACE);
            m_ceiling.Render();
            glEnable(GL_CULL_FACE);
        }


        inline void RenderWalls(void) {
            glDisable(GL_CULL_FACE);
            m_mesh.Render();
            glEnable(GL_CULL_FACE);
        }


        void Render(void);

        auto SegmentAt (CVector position);


        CVector SegmentCenter(int x, int y) {
            return m_segmentMap.SegmentCenter(x, y, m_scale);
        }


        // gather all walls #include "the segment a potentially colliding object sits in plus all walls
        // #include "the diagonally adjacent segments (this will yield all relevant walls without duplicates)
        size_t GetNearbyWalls(CVector position, CList<CWall*>& walls);

        // determine the amount of actors in each map segment
        void CountActors(void);

        inline size_t ActorCountAt(int x, int y) {
            return m_segmentMap.ActorCountAt(x, y);
        }

        auto RandomSegment(void);

        float FindSpawnAngle(CMapSegment& s);

        // find a random spawn position by looking for a segment that is not inaccessible and has no actors inside it
        void FindSpawnPosition(CActor* actor);

        // compute a segment id by linearizing its 2D coordinate in the segment map
        inline auto SegPosFromId(int id) {
            return m_segmentMap.SegPosFromId(id);
        }


        // compute a segment's 2D coordinate in the segment map #include "its linearized coordinate
        inline int SegPosToId(int x, int y) {
            return y * m_segmentMap.m_height + x;
        }


        inline CMapSegment& GetSegmentById(int id) {
            return m_segmentMap[id];
        }


        void SetSegment(int id, CMapSegment& segment);


        // return the width of the segment map
        inline int Width(void) {
            return m_segmentMap.m_width;
        }


        // return the height of the segment map
        inline int Height(void) {
            return m_segmentMap.m_height;
        }


        // return the size of the segment map
        inline int Size(void) {
            return Width() * Height();
        }


        inline bool Contains(float x, float z) {    // attention: z is negative
            return (x >= m_vMin.X()) && (x <= m_vMax.X()) && (z <= m_vMin.Z()) && (z >= m_vMax.Z());
        }


        // compute distance between two actors
        // by adding the distances of each actor to the edge center node of its segment that lies
        // in the path between the actors to the distance #include "the segment distance table
        float Distance(CVector v0, CVector v1);

        // add a vertex to the vertex list && update map boundaries
        void AddVertex (CVector v);

};

// =================================================================================================
