using System;
using System.Collections.Generic;

// ================================================================================
// unidirectional Dijkstra address calculation sort

public class Router : DialHeap
{
    public Router() : base() { }


    void SetSize(ushort size)
    {
        m_maxNodes = size;
    }


    public List<RouteNode> BuildPath(short segmentId) 
    {
        m_route = new List<RouteNode>();
        if (m_pathCost[segmentId] != 65535)
            BuildRoute(segmentId);
        return m_route;
    }


    // find a path from segment startSegId to segment destSegId in a game map
    // if destSegId is -1, compute minimal path cost to each other segment in the map
    // that is reachable from the start segment
    public int FindPath(short startSegId, short destSegId, SegmentMap segmentMap)
    {
        Setup(startSegId);
        int expanded = 1;

        for (; ; )
        {
            ushort dist;
            short segId = Pop(out dist);
            if (segId < 0)
                return (destSegId < 0) ? expanded : -1;
            if (segId == destSegId)
                return BuildPath(segId).Count;

            MapSegment segment = segmentMap[segId];
            foreach(short edgeId in segment.m_pathEdgeIds)
            {
                SegmentPathEdge e = segmentMap.m_pathEdgeTable[edgeId];
                if (Push(e.m_segmentId, segId, edgeId, (ushort) (dist +  e.m_distance)))
                    expanded++;
            }
        }
    }

}

// ================================================================================
