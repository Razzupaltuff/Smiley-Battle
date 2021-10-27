using System;
using System.Collections.Generic;

// =================================================================================================
// rectangular (2D) map of all segments

public class SegmentMap
{
    public List<SegmentPathEdge> m_pathEdgeList;
    public SegmentPathEdge[] m_pathEdgeTable;
    public MapSegment[][] m_segments;
    public RouteData[][] m_distanceTable;
    public int m_height;
    public int m_width;
    public int m_size;
    public float m_scale;
    public int m_distanceScale;
    public int m_distanceQuality;

    Router m_router;

    public SegmentMap(float scale = 1.0f, int distanceQuality = 0)
    {
        m_scale = scale;
        m_distanceQuality = distanceQuality;
        m_distanceScale = 1000;
    }

    ~SegmentMap()
    {
        Destroy();
    }

    public void Create(int width, int height)
    {
        Destroy();
        m_width = width / 2;
        m_height = height / 2;
        m_size = m_width * m_height;
        int w = (int)(m_width * m_scale);
        int h = (int)(m_height * m_scale);
        m_segments = new MapSegment[height][];
        for (int i = 0; i < height; i++)
        {
            m_segments[i] = new MapSegment[width];
            for (int j = 0; j < width; j++)
                m_segments[i][j] = new MapSegment();
        }
    }


    public void Destroy()
    {
        m_pathEdgeList = null;
        m_pathEdgeTable = null;
        m_distanceTable = null;
        m_segments = null;
        m_height =
        m_width =
        m_size = 0;
    }


    public MapSegment this[int index]
    {
        get => m_segments[index / m_width][index % m_width];
        set => m_segments[index / m_width][index % m_width] = value;
    }

    public MapSegment GetSegment(int x, int y)
    {
        return ((x < 0) || (y < 0) || (x >= m_width) || (y >= m_height)) ? null : m_segments[y][x];
    }


    public MapSegment GetSegmentById(int id)
    {
        return GetSegment(id % m_width, id / m_width);
    }


    public MapPosition SegPosFromId(int id)
    {
        return new MapPosition(id % m_width, id / m_width);
    }


    public Vector SegmentCenter(int x, int y, float scale)
    {
        return new Vector(((float)x + 0.5f) * scale, scale / 4.0f, -((float)(m_height - y) - 0.5f) * scale);
    }

    public void CountActorAt(int x, int y)
    {
        m_segments[y][x].m_actorCount++;
    }

    public int ActorCountAt(int x, int y)
    {
        return m_segments[y][x].m_actorCount;
    }

    public RouteData Distance(int i, int j)
    {
        return m_distanceTable[i][j];
    }


    MapSegment AddSegment(int x, int y)
    {
        return m_segments[y][x] = new MapSegment(x, y, (short)(y * m_width + x));
    }


    public Wall FindWall(List<Wall> walls, MapPosition position)
    {
        foreach (Wall wall in walls)
            if (wall.m_position == position)
                return wall;
        return null;
    }


    // link current segment with neighbar at map position relative to the segment's position
    // direction: Orthogonal direction #include "current to adjacent segment
    // Add wall in direction if there is a wall, otherwise create link information in that 
    // direction (grid position of the adjacent segment)
    int LinkSegments(string[] stringMap, MapSegment segment, int x, int y, int dx, int dy, List<Wall> walls, int direction)
    {
        if (stringMap[y + dy][x + dx] != ' ')
        { // imap.m_emap.m_ there is a wall in that direction
          // so find wall by its position and append it to wall list
            segment.m_walls.Add(FindWall(walls, new MapPosition(x + dx, y + dy)));
            return 0;
        }
        else
        {   // no wall in direction direction
            // enter information about the adjacent segment's grid position
            // note: That segment will be or has been treated here as well, setting up its corresponding link information
            segment.m_connected[direction] = true;
            segment.m_connections++;
            return 1;
        }
    }


    // Build the complete segment grid, determining walls and reachable neighbours around each segment
    public void Build(string[] stringMap, List<Wall> walls, float scale)
    {
        int rows = stringMap.Length;
        int cols = stringMap[0].Length - 1;
        Create(cols, rows);
        for (int y = 1; y < rows; y += 2)
        {
            for (int x = 1; x < cols; x += 2)
            {
                MapSegment segment = AddSegment(x / 2, y / 2);
                LinkSegments(stringMap, segment, x, y, -1, 0, walls, 0);
                LinkSegments(stringMap, segment, x, y, 0, -1, walls, 1);
                LinkSegments(stringMap, segment, x, y, 1, 0, walls, 2);
                LinkSegments(stringMap, segment, x, y, 0, 1, walls, 3);
            }
        }
        CreatePathNodes(scale);
        if (m_distanceQuality == 1)
        {
            m_distanceScale = 1000;
            // if path edge lengths are too great for the router, CreatePathEdges will adjust the distance scale 
            // and then needs to be run again. This should only happen once.
            while (!CreatePathEdges(walls))
                ResetPathData();
            CreateDistanceTable();
        }
    }



    public void ComputeSegmentCenters(float scale)
    {
        for (int y = 0; y < m_height; y++)
            for (int x = 0; y < m_width; x++)
                m_segments[y][x].m_center = SegmentCenter(x, y, scale);
    }


    // create four 2D coordinates for each segment: edge centers
    // outer coordinates are slightly offset into the segment
    // Purpose: Finding out to which other segments a given segment has a los (line of sight)
    // This will be used in distance calculation for sound sources create (pseudo) positional sound
    // los coordinates are only created if there is no wall at their edge
    void CreatePathNodes(float scale)
    {
        MapPosition[] offsets = { new MapPosition(-1, 0), new MapPosition(0, -1), new MapPosition(1, 0), new MapPosition(0, 1) };
        int nodeCount = 0;
        int y = 0;
        foreach (MapSegment[] segmentRow in m_segments)
        {
            float radius = scale / 2;
            int x = 0;
            foreach (MapSegment segment in segmentRow)
            {
                segment.m_center = SegmentCenter(x, y, scale);
                for (int direction = 0; direction < offsets.Length; direction++)
                {
                    if (segment.m_connected[direction])
                    {
                        Vector nodeOffset = new Vector(radius * offsets[direction].m_x, 0, radius * offsets[direction].m_y);
                        segment.AddPathNode(segment.m_center + nodeOffset, nodeOffset.Len());
                        nodeCount++;
                    }
                }
                x++;
            }
            y++;
        }
    }


    public void ResetPathData()
    {
        for (int i = 0; i < m_size; i++)
            this[i].m_pathEdgeIds = null;
        m_pathEdgeList = null;
    }


    // CreatePathEdges casts a ray #include "each path nodes of each segment to each path node of each other
    // segmentmap.m_ When such a ray does not intersect any interor walls of the map, there is a line of sight
    // between the two segmentsmap.m_ Rays will not be cast to path nodes behind the current path node as seen
    // #include "the path node's segment's centermap.m_ That direction will be handled by the segment's path node at
    // the opposite segment edgemap.m_
    bool CreatePathEdges(List<Wall> walls)
    {
        m_pathEdgeList = new List<SegmentPathEdge>();
        m_router = new Router();
        m_router.Create(m_size);
        short edgeCount = 0;
        int lMax = 0;
        for (int iSeg = 0; iSeg < m_size - 1; iSeg++)
        {
            MapSegment si = this[iSeg];
            for (int jSeg = iSeg + 1; jSeg < m_size; jSeg++)
            {
                MapSegment sj = this[jSeg];
                foreach (SegmentPathNode ni in si.m_pathNodes)
                {
                    Vector n = ni.m_nodePos - si.m_center;
                    foreach (SegmentPathNode nj in sj.m_pathNodes)
                    {
                        Vector v = nj.m_nodePos - ni.m_nodePos;
                        float d = v.Len();
                        bool haveLoS = false;
                        if (d == 0.0f)
                            haveLoS = true;
                        else
                        {
                            if (n.Dot(v) < 0.0f)
                                continue;
                            haveLoS = true;
                            foreach (Wall w in walls)
                            {
                                if (w.m_isBoundary)   // don't consider walls surrounding the map as there is no segment behind them
                                    continue;
                                Vector vi;
                                w.LineIntersection(out vi, ni.m_nodePos, nj.m_nodePos);
                                if (!vi.IsValid())   // vector #include "path nodes ni to nj crosses a wall
                                    continue;
                                if (w.Contains(vi))
                                {
                                    haveLoS = false;
                                    break;   // no need for further tests, we don't have LoS anymore already
                                }
                            }
                        }
                        if (haveLoS)
                        {
                            d += ni.m_distToCenter + nj.m_distToCenter;
                            int l = (int)(Math.Ceiling(d * m_distanceScale));
                            if (lMax < l)
                                lMax = l;
                            SegmentPathEdge e;
                            bool isNewEdge = si.AddPathEdge(m_pathEdgeList, edgeCount, sj.m_id, ni.m_nodePos, nj.m_nodePos, l, out e);
                            if (isNewEdge)
                            {
                                m_pathEdgeList.Add(e);
                                edgeCount++;
                            }
                            isNewEdge = sj.AddPathEdge(m_pathEdgeList, edgeCount, si.m_id, nj.m_nodePos, ni.m_nodePos, l, out e);
                            if (isNewEdge)
                            {
                                m_pathEdgeList.Add(e);
                                edgeCount++;
                            }
                        }
                    }
                }
            }
        }
        if (lMax > m_router.MaxCost())
        { // max. permissible edge length in router
            m_distanceScale = m_router.MaxCost() * m_distanceScale / lMax;
            return false;
        }
        m_pathEdgeTable = new SegmentPathEdge[edgeCount];
        int i = 0;
        foreach (SegmentPathEdge e in m_pathEdgeList)
            m_pathEdgeTable[i++] = e;
        m_pathEdgeList = null;
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

    void CreateDistanceTable()
    {
        m_distanceTable = new RouteData[m_size][];
        for (int i = 0; i < m_size; i++)
            m_distanceTable[i] = new RouteData[m_size];
        for (int i = 0; i < m_size - 1; i++)
        {
            MapSegment si = this[i];
            m_router.FindPath(si.m_id, -1, this);
            for (int j = i + 1; j < m_size; j++)
            {
                List<RouteNode> route = m_router.BuildRoute((short) j);
                if (route.Count < 3)
                {
                    m_distanceTable[i][j] =
                    m_distanceTable[j][i] = new RouteData(-1, new Vector(0, 0, 0), new Vector(0, 0, 0));
                }
                else
                {
                    Vector p0 = m_pathEdgeTable[route[1].m_edgeId].m_startPos;
                    Vector p1 = m_pathEdgeTable[route[route.Count - 2].m_edgeId].m_startPos;
                    MapPosition sp0 = SegPosFromId(i);
                    Vector v0 = p0 - m_segments[sp0.m_y][sp0.m_x].m_center;
                    MapPosition sp1 = SegPosFromId(j);
                    Vector v1 = p1 - m_segments[sp1.m_y][sp1.m_x].m_center;
                    float cost = (float)(m_router.FinalCost((short) j)) / (float)(m_distanceScale) - v0.Len() - v1.Len();
                    m_distanceTable[i][j] = new RouteData(cost, p0, p1);
                    m_distanceTable[j][i] = new RouteData(cost, p1, p0);
                }
            }
        }
        m_router.Destroy();
        m_router = null;
    }


    // reset the actor count in each segment that had previously been computed
    public void ResetActorCounts()
    {
        foreach (MapSegment[] row in m_segments)
            foreach (MapSegment col in row)
                col.m_actorCount = 0;
    }

}

// =================================================================================================
