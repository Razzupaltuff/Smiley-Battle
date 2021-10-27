using System;
using System.Collections.Generic;

// ================================================================================

public class RouteNode
{
    public int m_nodeId;
    public int m_edgeId;

    public RouteNode(short nodeId = -1, short edgeId = -1)
    {
        m_nodeId = nodeId;
        m_edgeId = edgeId;
    }

}

// ================================================================================
// Dial heap for DACS path finding in graphs
// costIndex contains the total path cost to each node stored in it
// Add nodes into costIndex at the position of their predecessor in costIndex plus their edge cost
// Each costIndex entry has a list of nodes with the same path cost
// Next node to expand the path from is always the next node in costIndex seen from the current costIndex position
// Path cost can and will wrap around costIndex; that's why costIndex needs to be larger than the highest
// edge cost.
// See the internets for a full explanation of DACS (Dijkstra Address Calculation Sort)
// nodeLists contains the root indices for lists of nodes with the same cost. These indices point into nodeListLinks.
// nodeLists is indexed with path costs, nodeListLinks is indexed with node ids. For each node id stored in it, it 
// holds the index of the next node id in the list at the index of the next node's predecessor.
//
//   0   1   2         // of nodes - 1
// +---+---+---+-   -+---+
// |   | 5 |   | ... |   |
// +---+---+---+-   -+---+
//       |
//       +----------------+
//           +--+ +-----+ |
//           |  | |     | |
//           v  | v     | v
// +---+---+---+---+---+---+---+---+---+
// |   |   |-1 | 2 |   | 3 |   |   |   | ...
// +---+---+---+---+---+---+---+---+---+
//   0   1   2   3   4   5   6   7   8
//
// This means that 1 there is a list of nodes with path cost 1. nodeList [1] contains 5, that's the 1st node in the list.
// nodeListLinks [5] contains 3, which is the 2nd node in the list. nodeListLinks [3] contains 2, and nodeListLinks [2]
// contains -1, so 2 is the last node in the list of nodes with cost 1.
// The resulting list is 5,3,2

public class DialHeap
{
    public ushort[] m_pathCost;
    public List<RouteNode> m_route;

    public int m_maxCost;  // max. path cost of a single graph edge
    public int m_noCost;   // cost value for "no path cost calculated"
    public int m_maxNodes;

    int m_costIndex;

    short[] m_nodeLists;
    short[] m_nodeListLinks;
    short[] m_predecessors;
    ushort[] m_finalCost;
    short[] m_edges;
    Stack<ushort> m_dirtyIndex;
    Stack<short> m_dirtyCost;
    Stack<short> m_dirtyFinalCost;

    public DialHeap()
    {
        m_maxCost = 65534;
        m_noCost = 65535;
    }

    ~DialHeap()
    {
        Destroy();
    }

    public void Destroy()
    {
        m_maxNodes = 0;
    }

    public int MaxCost()
    {
        return m_maxCost;
    }

    public ushort FinalCost(short nodeId)
    {
        return m_finalCost[nodeId];
    }


    // required for bi-directional DACS (not implemented here)
    public bool Pushed(short nodeId)
    {
        return m_pathCost[nodeId] < m_noCost;
    }


    public bool Popped(short nodeId)
    {
        return !Pushed(nodeId) && (m_pathCost[nodeId] < 0);
    }


    public RouteNode RouteNode(int i = 0)
    {
        return m_route[i];
    }

    // ================================================================================

    public void Create(int maxNodes)
    {
        m_maxNodes = maxNodes;
        m_nodeLists = new short[65536];
        for (int i = 0; i < m_nodeLists.Length; i++)
            m_nodeLists[i] = -1;
        m_nodeListLinks = new short[65536];
        for (int i = 0; i < m_nodeListLinks.Length; i++)
            m_nodeListLinks[i] = -1;
        m_predecessors = new short[65536];
        for (int i = 0; i < m_predecessors.Length; i++)
            m_predecessors[i] = -1;
        m_pathCost = new ushort[65536];
        for (int i = 0; i < m_pathCost.Length; i++)
            m_pathCost[i] = 65535;
        m_finalCost = new ushort[65536];
        for (int i = 0; i < m_finalCost.Length; i++)
            m_finalCost[i] = 65535;
        m_edges = new short[65536];
        for (int i = 0; i < m_edges.Length; i++)
            m_edges[i] = 0;
        m_dirtyIndex = new Stack<ushort>();
        m_dirtyCost = new Stack<short>();
        m_dirtyFinalCost = new Stack<short>();
    }


    // reset all used list data
    public void Reset()
    {
        bool sparseReset = true;
        if (sparseReset)
        {
            for (int i = m_dirtyIndex.Count; i > 0;  i--)
                m_nodeLists[m_dirtyIndex.Pop()] = -1;
            for (int i = m_dirtyCost.Count; i > 0;  i--)
                m_pathCost[m_dirtyCost.Pop()] = 65535;
            for (int i = m_dirtyFinalCost.Count; i > 0; i--)
                m_finalCost[m_dirtyFinalCost.Pop()] = 65535;
        }
        else
        {
            for (int i = 0; i < m_nodeLists.Length; i++)
                m_nodeLists[i] = -1;
            for (int i = 0; i < m_pathCost.Length; i++)
                m_pathCost[i] = 65535;
            for (int i = 0; i < m_finalCost.Length; i++)
                m_finalCost[i] = 65535;
        }
        m_costIndex = 0;
    }


    // start path finding for node node
    public void Setup(short nodeId)
    {
        Reset();
        Push(nodeId, -1, -1, 0);
    }


    // put the current node node with path cost newCost in the heap or update its cost
    // if it is already in the heap and has higher path cost
    public bool Push(short nodeId, short predNodeId, short edgeId, ushort newCost)
    {
        ushort oldCost = m_pathCost[nodeId];
        if (newCost >= oldCost)
            return false;    // new path is longer than the currently stored one

        ushort costIndex = newCost;
        if (oldCost == m_noCost)
            m_dirtyCost.Push(nodeId);
        else
        {
            // node already in heap with higher pathCost, so unlink node from node list at current path cost position
            // find the node in the node list attached to the nodeList at the current path cost and let its successor take its place in the list
            ushort listRoot = oldCost;
            short currNodeId = m_nodeLists[listRoot];
            short nextNodeId = -1;
            while (currNodeId >= 0)
            {
                if (currNodeId == nodeId)
                {
                    if (nextNodeId < 0)
                        m_nodeLists[listRoot] = m_nodeListLinks[currNodeId];
                    else
                        m_nodeListLinks[nextNodeId] = m_nodeListLinks[currNodeId];
                    break;
                }
                nextNodeId = currNodeId;
                currNodeId = m_nodeListLinks[currNodeId];
            }
        }

        if (0 > m_nodeLists[costIndex])
            m_dirtyIndex.Push(costIndex);
        m_pathCost[nodeId] = newCost;
        m_predecessors[nodeId] = predNodeId;
        m_nodeListLinks[nodeId] = m_nodeLists[costIndex];
        m_nodeLists[costIndex] = nodeId;
        m_edges[nodeId] = edgeId;
        return true;
    }


    // find node with lowest path cost from current path finding state by searching through the
    // node cost offset table from the current node's position there to the next position in the table
    // holding a node
    int Scan(int nStart)
    {
        int l = m_nodeLists.Length;
        int i = nStart;
        int j = l;
        while (j-- > 0)
        {
            if (m_nodeLists[i] >= 0)
                return i;
            i = (i + 1) % l;
        }
        return -1;
    }


    // remove node from path finding tree
    public short Pop(out ushort cost)
    {
        int i = Scan(m_costIndex);
        if (i < 0)
        {
            cost = 65535;
            return -1;
        }
        m_costIndex = i;
        short nodeId = m_nodeLists[m_costIndex];
        m_nodeLists[m_costIndex] = m_nodeListLinks[nodeId];
        cost = m_pathCost[nodeId];
        m_finalCost[nodeId] = cost;
        m_dirtyFinalCost.Push(nodeId);
        return nodeId;
    }


    // calculate length in segments from a node to the start node
    public int RouteNodeCount(short nodeId)
    {
        int i = 0;
        for (; ; )
        {
            i++;
            if (0 > (nodeId = m_predecessors[nodeId]))
                break;
        }
        return i;
    }


    public List<RouteNode> BuildRoute(short nodeId)
    {
        if (m_route == null)
            m_route = new List<RouteNode>();
        else
            m_route.Clear();
        for (; ; )
        {
            m_route.Insert(0, new RouteNode(nodeId, m_edges[nodeId]));
            nodeId = m_predecessors[nodeId];
            if (nodeId < 0)
                break;
        }
        return m_route;
    }

}

// ================================================================================
