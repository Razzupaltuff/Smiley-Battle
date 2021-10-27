#include "mapsegments.h"
#include "router.h"
#include "plane.h"

// =================================================================================================
// Segment class for map segments

CMapSegment::CMapSegment(int x, int y, int id) {
    m_connected[0] =
    m_connected[1] =
    m_connected[2] =
    m_connected[3] = false;
    m_connections = 0;
    m_position = { x, y };
    m_center = CVector(0, 0, 0);
    m_id = id;
    m_actorCount = 0;
}


CSegmentPathEdge* CMapSegment::FindPathEdge(CList<CSegmentPathEdge>& edgeList, int segmentId) {
    for (auto [i, edgeId] : m_pathEdgeIds) {
        CSegmentPathEdge& e = edgeList[edgeId];
        if (e.m_segmentId == segmentId)
            return &e;
    }
    return nullptr;
}


bool CMapSegment::AddPathEdge(CList<CSegmentPathEdge>& edgeList, int edgeId, int segmentId, CVector& startPos, CVector& endPos, int distance, CSegmentPathEdge& edge) {
    CSegmentPathEdge* e = FindPathEdge(edgeList, segmentId);
    if (!e) {
        m_pathEdgeIds.Append(edgeId);
        edge = CSegmentPathEdge(segmentId, startPos, endPos, distance);
        return true;
    }
    if (distance < e->m_distance) {
        e->m_startPos = startPos;
        e->m_endPos = endPos;
        e->m_distance = int(distance);
        return false;
    }
    return false;
}

// =================================================================================================
// rectangular (2D) map of all segments

void CSegmentMap::Create(int width, int height) {
    Destroy();
    m_width = width / 2;
    m_height = height / 2;
    m_size = m_width * m_height;
    int w = int (m_width * m_scale);
    int h = int (m_height * m_scale);
    m_segments.Create(height);
    for (auto row : m_segments)
        row->Create(width);
}


void CSegmentMap::Destroy(void) {
    m_pathEdgeList.Destroy();
    m_pathEdgeTable.Destroy();
    m_distanceTable.Destroy ();
    m_segments.Destroy();
    m_height = 
    m_width = 
    m_size = 0;
}


CMapSegment* CSegmentMap::AddSegment(int x, int y) {
    CMapSegment* s = &m_segments[y][x];
    *s = CMapSegment(x, y, y * m_width + x);
    return s;
}


CWall* CSegmentMap::FindWall(CList<CWall>& walls, CMapPosition position) {
    for (auto [i, wall] : walls)
        if (wall.m_position == position)
            return &wall;
    return nullptr;
}


// link current segment with neighbar at map position relative to the segment's position
// direction: Orthogonal direction #include "current to adjacent segment
// Add wall in direction if there is a wall, otherwise create link information in that 
// direction (grid position of the adjacent segment)
int CSegmentMap::LinkSegments(CList<CString>& stringMap, CMapSegment* segment, int x, int y, int dx, int dy, CList<CWall>& walls, int direction) {
    if (stringMap[y + dy][x + dx] != ' ') { // imap.m_emap.m_ there is a wall in that direction
        // so find wall by its position and append it to wall list
        segment->m_walls.Append(FindWall(walls, CMapPosition(x + dx, y + dy)));
        return 0;
    }
    else {   // no wall in direction direction
        // enter information about the adjacent segment's grid position
        // note: That segment will be or has been treated here as well, setting up its corresponding link information
        segment->m_connected[direction] = true;
        segment->m_connections++;
        return 1;
    }
}


// Build the complete segment grid, determining walls and reachable neighbours around each segment
void CSegmentMap::Build(CList<CString>& stringMap, CList<CWall>& walls, float scale) {
    int rows = int (stringMap.Length());
    int cols = int (stringMap[0].Length()) - 1;
    Create(cols, rows);
    for (int y = 1; y < rows; y += 2) {
        for (int x = 1; x < cols; x += 2) {
            CMapSegment* segment = AddSegment(x / 2, y / 2);
            LinkSegments(stringMap, segment, x, y, -1,  0, walls, 0);
            LinkSegments(stringMap, segment, x, y,  0, -1, walls, 1);
            LinkSegments(stringMap, segment, x, y,  1,  0, walls, 2);
            LinkSegments(stringMap, segment, x, y,  0,  1, walls, 3);
        }
    }
    CreatePathNodes(scale);
    if (m_distanceQuality == 1) {
        m_distanceScale = 1000;
        // if path edge lengths are too great for the router, CreatePathEdges will adjust the distance scale 
        // and then needs to be run again. This should only happen once.
        while (!CreatePathEdges (walls))
            ResetPathData ();
        CreateDistanceTable();
    }
}



void CSegmentMap::ComputeSegmentCenters(float scale) {
    for (int y = 0; y < m_height; y++)
        for (int x = 0; y < m_width; x++)
            m_segments[y][x].m_center = SegmentCenter(x, y, scale);
}


// create four 2D coordinates for each segment: edge centers
// outer coordinates are slightly offset into the segment
// Purpose: Finding out to which other segments a given segment has a los (line of sight)
// This will be used in distance calculation for sound sources create (pseudo) positional sound
// los coordinates are only created if there is no wall at their edge
void CSegmentMap::CreatePathNodes(float scale) {
    CMapPosition offsets[] = { CMapPosition(-1,0), CMapPosition(0,-1), CMapPosition(1,0), CMapPosition(0,1) };    // [(-1,0), (0,-1), (1,0), (0,1), (0,0), (-1,-1), (1,-1), (1,1), (-1,1)]
    int nodeCount = 0;
    int y = 0;
    for (auto segmentRow : m_segments) {
        float radius = scale / 2;
        int x = 0;
        for (auto segment : *segmentRow) {
            segment->m_center = SegmentCenter(x, y, scale);
            for (int direction = 0; direction < sizeofa(offsets); direction++) {
                if (segment->m_connected[direction]) {
                    CVector nodeOffset = CVector(radius * offsets[direction].m_x, 0, radius * offsets[direction].m_y);
                    segment->AddPathNode(segment->m_center + nodeOffset, nodeOffset.Len());
                    nodeCount++;
                }
            }
            x++;
        }
        y++;
    }
}


void CSegmentMap::ResetPathData (void) {
    for (int i = 0; i < m_size; i++)
        (*this) [i].m_pathEdgeIds.Destroy ();
    m_pathEdgeList.Destroy ();
}


// CreatePathEdges casts a ray #include "each path nodes of each segment to each path node of each other
// segmentmap.m_ When such a ray does not intersect any interor walls of the map, there is a line of sight
// between the two segmentsmap.m_ Rays will not be cast to path nodes behind the current path node as seen
// #include "the path node's segment's centermap.m_ That direction will be handled by the segment's path node at
// the opposite segment edgemap.m_
bool CSegmentMap::CreatePathEdges(CList<CWall>& walls) {
    router.Create (m_size);
    int edgeCount = 0;
    int lMax = 0;
    for (int i = 0; i < m_size - 1; i++) {
        CMapSegment& si = (*this)[i];
        for (int j = i + 1; j < m_size; j++) {
            CMapSegment& sj = (*this)[j];
            for (auto ri : si.m_pathNodes) {
                CSegmentPathNode& ni = ri.second;
                CVector n = ni.m_nodePos - si.m_center;
                for (auto rj : sj.m_pathNodes) {
                    CSegmentPathNode& nj = rj.second;
                    CVector v = nj.m_nodePos - ni.m_nodePos;
                    float d = v.Len();
                    bool haveLoS = false;
                    if (d == 0.0f)
                        haveLoS = true;
                    else {
                        if (n.Dot(v) < 0.0f)
                            continue;
                        haveLoS = true;
                        for (auto rw : walls) {
                            CWall& w = rw.second;
                            if (w.m_isBoundary)   // don't consider walls surrounding the map as there is no segment behind them
                                continue;
                            CVector vi;
                            w.LineIntersection(vi, ni.m_nodePos, nj.m_nodePos);
                            if (!vi.IsValid())   // vector #include "path nodes ni to nj crosses a wall
                                continue;
                            if (w.Contains(vi)) {
                                haveLoS = false;
                                break;   // no need for further tests, we don't have LoS anymore already
                            }
                        }
                    }
                    if (haveLoS) {
                        d += ni.m_distToCenter + nj.m_distToCenter;
                        int l = int (ceil (d * m_distanceScale));
                        if (lMax < l)
                            lMax = l;
                        CSegmentPathEdge e;
                        bool isNewEdge = si.AddPathEdge(m_pathEdgeList, edgeCount, sj.m_id, ni.m_nodePos, nj.m_nodePos, l, e);
                        if (isNewEdge) {
                            m_pathEdgeList.Append(e);
                            edgeCount++;
                        }
                        isNewEdge = sj.AddPathEdge(m_pathEdgeList, edgeCount, si.m_id, nj.m_nodePos, ni.m_nodePos, l, e);
                        if (isNewEdge) {
                            m_pathEdgeList.Append(e);
                        edgeCount++;
                        }
                    }
                }
            }
        }
    }
    if (lMax > router.MaxCost ()) { // max. permissible edge length in router
        m_distanceScale = router.MaxCost () * m_distanceScale / lMax;
        return false;
    }
    m_pathEdgeTable.Create(edgeCount);
    for (auto [i, e] : m_pathEdgeList)
        m_pathEdgeTable[i] = e;
    m_pathEdgeList.Destroy();
    return true;
}
                        

// build a table with distances #include "each segment to each other reachable segment
// For start and end segment, subtract the distance between the segment centers and the segments' edge center nodes used in
// the shortest path between these segmentmap.m_ When determining the distance between two actors, the distances of the
// actors to the edge center nodes of their respective segmentmap.m_ This yields an acceptable approximation of the 
// path distance between the two actorsmap.m_
// 
// +---------+---------+---------+---------+
// |         |         |         |S4       | 
// |      _/-x---------x---------x-\__     | 
// |     /   |         |         |    \-B  | 
// +----x----+---------+---------+---------+
// |S2  |    |
// |    |    |
// |    |    |
// +----x----+
// |S1   \   |
// |      \  |
// |       A |
// +---------+
    
void CSegmentMap::CreateDistanceTable(void) {
    m_distanceTable.Create(m_size);
    for (auto row : m_distanceTable)
        row->Create(m_size);
    for (int i = 0; i < m_size - 1; i++) {
        CMapSegment& si = (*this)[i];
        router.FindPath(si.m_id, -1, *this);
        for (int j = i + 1; j < m_size; j++) {
            CList<CRouteNode> route = router.BuildRoute(j);
            if (route.Length() < 3) {
                m_distanceTable[i][j] =
                m_distanceTable[j][i] = CRouteData(-1, CVector(0, 0, 0), CVector(0, 0, 0));
            }
            else {
                CVector& p0 = m_pathEdgeTable[route[1].m_edgeId].m_startPos;
                CVector& p1 = m_pathEdgeTable[route[-2].m_edgeId].m_startPos;
                auto sp0 = SegPosFromId(i);
                CVector v0 = p0 - m_segments[sp0.y][sp0.x].m_center;
                auto sp1 = SegPosFromId(j); 
                CVector v1 = p1 - m_segments[sp1.y][sp1.x].m_center;
                float cost = float (router.FinalCost (j)) / float (m_distanceScale) - v0.Len () - v1.Len ();
                m_distanceTable[i][j] = CRouteData(cost, p0, p1);
                m_distanceTable[j][i] = CRouteData(cost, p1, p0);
            }
        }
    }
    router.Destroy();
}


// reset the actor count in each segment that had previously been computed
void CSegmentMap::ResetActorCounts(void) {
    for (auto row : m_segments)
        for (auto col : *row)
            col->m_actorCount = 0;
}

// =================================================================================================
