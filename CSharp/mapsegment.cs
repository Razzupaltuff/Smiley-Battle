using System;
using System.Collections.Generic;

// =================================================================================================

public class SegmentPathNode
{

    public short m_id;
    public Vector m_nodePos;
    public float m_distToCenter;

    public SegmentPathNode(short id = -1, Vector nodePos = null, float distToCenter = 0.0f)
    {
        m_id = id;
        m_nodePos = nodePos;
        m_distToCenter = distToCenter;
    }

}


public class SegmentPathEdge
{
    public short m_segmentId;
    public Vector m_startPos;
    public Vector m_endPos;
    public int m_distance;

    public SegmentPathEdge(short segmentId = -1, Vector startPos = null, Vector endPos = null, int distance = -1)
    {
        m_segmentId = segmentId;
        m_startPos = startPos;
        m_endPos = endPos;
        m_distance = distance;
    }

}


public class RouteData
{
    public float m_distance;
    public Vector m_startPos;
    public Vector m_endPos;

    public RouteData(float distance = 0.0f, Vector startPos = null, Vector endPos = null)
    {
        m_distance = distance;
        m_startPos = startPos;
        m_endPos = endPos;
    }

}

// =================================================================================================

public class MapPosition
{
    public int m_x, m_y;

    public MapPosition(int x = 0, int y = 0)
    {
        m_x = x;
        m_y = y;
    }

    public MapPosition(MapPosition other)
    {
        m_x = other.m_x;
        m_y = other.m_y;
    }

    static public bool operator ==(MapPosition a, MapPosition b)
    {
        return (a.m_x == b.m_x) && (a.m_y == b.m_y);
    }

    static public bool operator !=(MapPosition a, MapPosition b)
    {
        return (a.m_x != b.m_x) && (a.m_y != b.m_y);
    }

    public override int GetHashCode()
    {
        return (m_y << 32) | m_x;
    }

    public override bool Equals(Object obj)
    {
        //Check for null and compare run-time types.
        if ((obj == null) || !this.GetType().Equals(obj.GetType()))
        {
            return false;
        }
        else
        {
            MapPosition p = (MapPosition)obj;
            return (m_x == p.m_x) && (m_y == p.m_y);
        }
    }

}

// =================================================================================================

public class Wall : Plane
{
    public MapPosition m_position;
    public bool m_isBoundary;

    public Wall() : base() { }

    public Wall(Vector[] vertices, MapPosition position, bool isBoundary = false)
            : base(vertices)
    {
        m_position = position;
        m_isBoundary = isBoundary;
    }

    public void Init(Vector[] vertices, MapPosition position, bool isBoundary = false)
    {
        Init(vertices);
        m_position = position;
        m_isBoundary = isBoundary;
    }

}

// =================================================================================================
// Segment class for map segments

public class MapSegment
{
    public short m_id;
    public Vector m_center;
    public bool[] m_connected;         // orthogonal neighbours 
    public int m_connections;
    public int m_actorCount;
    public MapPosition m_position;             // position in the segment grid
    public List<Wall> m_walls;
    public List<short> m_pathEdgeIds;          // indices into list of other segments a los exists to
    public List<SegmentPathNode> m_pathNodes;    // reference coordinates for los (line of sight) testing

    public MapSegment(int x = -1, int y = -1, short id = -1)
    {
        m_connected = new bool[] { false, false, false, false };
        m_connections = 0;
        m_position = new MapPosition(x, y);
        m_center = new Vector(0, 0, 0);
        m_id = id;
        m_actorCount = 0;
        m_walls = new List<Wall>();
        m_pathEdgeIds = new List<short>();
        m_pathNodes = new List<SegmentPathNode>();
    }


    ~MapSegment()
    {
        Destroy();
    }

    public void Destroy()
    {
        m_walls = null;
        m_pathEdgeIds = null;
        m_pathNodes = null;
    }

    public void SetId(short id)
    {
        m_id = id;
    }

    public short GetId()
    {
        return m_id;
    }

    public bool IsConnected(int direction)
    {
        return m_connected[direction];
    }

    public void AddPathNode(Vector node, float distToCenter)
    {
        m_pathNodes.Add(new SegmentPathNode((short)(GetId() * 10 + m_pathNodes.Count + 1), node, distToCenter));
    }


    SegmentPathEdge FindPathEdge(List<SegmentPathEdge> edgeList, int segmentId)
    {
        foreach (short edgeId in m_pathEdgeIds)
        {
            SegmentPathEdge e = edgeList[edgeId];
            if (e.m_segmentId == segmentId)
                return e;
        }
        return null;
    }


    public bool AddPathEdge(List<SegmentPathEdge> edgeList, short edgeId, short segmentId, Vector startPos, Vector endPos, int distance, out SegmentPathEdge edge)
    {
        SegmentPathEdge e = FindPathEdge(edgeList, segmentId);
        if (e == null)
        {
            m_pathEdgeIds.Add(edgeId);
            edge = new SegmentPathEdge(segmentId, startPos, endPos, distance);
            return true;
        }
        if (distance < e.m_distance)
        {
            e.m_startPos = startPos;
            e.m_endPos = endPos;
            e.m_distance = distance;
        }
        edge = e;
        return false;
    }

}

// =================================================================================================
