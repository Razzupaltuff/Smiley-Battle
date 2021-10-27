#pragma once

#include "cstring.h"
#include "carray.h"
#include "clist.h"
#include "vector.h"
#include "plane.h"

// =================================================================================================

class CSegmentPathNode {
    public:
        int     m_id;
        CVector m_nodePos;
        float   m_distToCenter;

        CSegmentPathNode(int id = -1, CVector nodePos = CVector(NAN, NAN, NAN), float distToCenter = 0.0f)
            : m_id(id), m_nodePos(nodePos), m_distToCenter(distToCenter)
        {}

};

    
class CSegmentPathEdge {
    public:
        int     m_segmentId;
        CVector m_startPos;
        CVector m_endPos;
        int     m_distance;

        CSegmentPathEdge(int segmentId = -1, CVector startPos = CVector(NAN, NAN, NAN), CVector endPos = CVector(NAN, NAN, NAN), int distance = -1)
            : m_segmentId(segmentId), m_startPos(startPos), m_endPos(endPos), m_distance(distance)
        {
        }

};


class CRouteData {
    public:
        float   m_distance;
        CVector m_startPos;
        CVector m_endPos;

        CRouteData(float distance = 0.0f, CVector startPos = CVector(NAN, NAN, NAN), CVector endPos = CVector(NAN, NAN, NAN))
            : m_distance(distance), m_startPos(startPos), m_endPos(endPos)
        {}

};

// =================================================================================================

class CMapPosition {
    public:
        int m_x, m_y;

        CMapPosition(int x = 0, int y = 0) : m_x(x), m_y(y) {}

        CMapPosition(CMapPosition const& other) : m_x(other.m_x), m_y(other.m_y) {}

        CMapPosition& operator=(CMapPosition other) {
            m_x = other.m_x;
            m_y = other.m_y;
            return *this;
        }

        bool operator== (CMapPosition& other) {
            return (m_x == other.m_x) && (m_y == other.m_y);
        }

};

// =================================================================================================

class CWall : public CPlane {
    public:
        CMapPosition    m_position;
        bool            m_isBoundary;

        CWall() : CPlane(), m_isBoundary (false) {}

        CWall(std::initializer_list<CVector> vertices, CMapPosition position, bool isBoundary = false)
            : CPlane(vertices), m_position (position), m_isBoundary(isBoundary)
        {}

        void Init (std::initializer_list<CVector> vertices, CMapPosition position, bool isBoundary = false) {
            CPlane::Init (vertices);
            m_position = position;
            m_isBoundary = isBoundary;
        }

};

// =================================================================================================
// Segment class for map segments

class CMapSegment {
    public:
        int             m_id;
        CVector         m_center;
        bool            m_connected[4];         // orthogonal neighbours 
        int             m_connections;
        int             m_actorCount;
        CMapPosition    m_position;             // position in the segment grid
        CList<CWall*>   m_walls;
        CList<int>      m_pathEdgeIds;          // indices into list of other segments a los exists to
        CList<CSegmentPathNode> m_pathNodes;    // reference coordinates for los (line of sight) testing

        CMapSegment(int x = -1, int y = -1, int id = -1);

        ~CMapSegment () {
            Destroy ();
        }

        void Destroy (void) {
            m_walls.Destroy ();
            m_pathEdgeIds.Destroy ();
            m_pathNodes.Destroy();
        }

        inline void SetId(int id) {
            m_id = id;
        }

        inline int GetId(void) {
            return m_id;
        }

        inline bool IsConnected(int direction) {
            return m_connected[direction];
        }

        inline void AddPathNode(CVector node, float distToCenter) {
            m_pathNodes.Append(CSegmentPathNode(int(GetId() * 10 + m_pathNodes.Length() + 1), node, distToCenter));
        }

        CSegmentPathEdge* FindPathEdge(CList<CSegmentPathEdge>& edgeList, int segmentId);

        bool AddPathEdge(CList<CSegmentPathEdge>& edgeList, int edgeId, int segmentId, CVector& startPos, CVector& endPos, int distance, CSegmentPathEdge& edge);

};

// =================================================================================================
// rectangular (2D) map of all segments

class CSegmentMap {
    public:
        CList<CSegmentPathEdge>         m_pathEdgeList;
        CArray<CSegmentPathEdge>        m_pathEdgeTable;
        CArray<CArray<CMapSegment>>     m_segments;
        CArray<CArray<CRouteData>>      m_distanceTable;
        int                             m_height;
        int                             m_width;
        int                             m_size;
        float                           m_scale;
        int                             m_distanceScale;
        int                             m_distanceQuality;

        CSegmentMap(float scale = 1.0f, int distanceQuality = 0)
            : m_scale(scale), m_distanceQuality(distanceQuality), m_height(0), m_width(0), m_size(0), m_distanceScale (1000)
        {}

        ~CSegmentMap() {
            Destroy();
        }

        void Destroy(void);

        inline CMapSegment& operator[] (int index) {
            return m_segments[index / m_width][index % m_width];
        }

        void Create(int width, int height);


        CMapSegment* AddSegment(int x, int y);


        inline CMapSegment* GetSegment(int x, int y) {
            return ((x < 0) || (y < 0) || (x >= m_width) || (y >= m_height)) ? nullptr : &m_segments[y][x];
        }


        inline CMapSegment* GetSegmentById(int id) {
            return GetSegment(id % m_width, id / m_width);
        }


        inline auto SegPosFromId(int id) {
            struct retVals {
                int x, y;
            };
            return retVals{ id % m_width, id / m_width };
        }

        CWall* FindWall(CList<CWall>& walls, CMapPosition position);

        int LinkSegments(CList<CString>& stringMap, CMapSegment* segment, int x, int y, int dx, int dy, CList<CWall>& walls, int direction);

        // Build the complete segment grid, determining walls and reachable neighbours around each segment
        void Build(CList<CString>& stringMap, CList<CWall>& walls, float scale);

        inline CVector SegmentCenter(int x, int y, float scale) {
            return CVector((float(x) + 0.5f) * scale, scale / 4.0f, -(float(m_height - y) - 0.5f) * scale);
        }

        void ComputeSegmentCenters(float scale);

        void CreatePathNodes(float scale);

        bool CreatePathEdges(CList<CWall>& walls);

        void CreateDistanceTable(void);

        void ResetPathData (void);

        void ResetActorCounts(void);

        inline void CountActorAt(int x, int y) {
            m_segments[y][x].m_actorCount++;
        }

        inline int ActorCountAt(int x, int y) {
            return m_segments[y][x].m_actorCount;
        }

        inline CRouteData& Distance(int i, int j) {
            return m_distanceTable[i][j];
        }

};

// =================================================================================================
