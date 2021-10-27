#include "router.h"

// =================================================================================================

#ifdef _DEBUG
#   define LOG(msg, ...) fprintf(stderr, msg, ##__VA_ARGS__);
#else
#   define LOG(msg, ...)
#endif

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

void CDialHeap::Create(int maxNodes) {
    m_maxNodes = maxNodes;
    m_nodeLists.Create(65536);
    m_nodeLists.Fill(-1);
    m_nodeListLinks.Create(m_maxNodes);
    m_nodeListLinks.Fill(-1);
    m_predecessors.Create(m_maxNodes);
    m_predecessors.Fill(-1);
    m_pathCost.Create(m_maxNodes);
    m_pathCost.Fill(65535);
    m_finalCost.Create(m_maxNodes);
    m_finalCost.Fill(65535);
    m_edges.Create(m_maxNodes);
    m_edges.Clear(0);
    m_dirtyIndex.Create(65536);
    m_dirtyCost.Create(65536);
    m_dirtyFinalCost.Create(65536);
}


void CDialHeap::Destroy(void) {
    m_maxNodes = 0;
    m_nodeLists.Destroy();
    m_nodeListLinks.Destroy();
    m_predecessors.Destroy();
    m_pathCost.Destroy();
    m_finalCost.Destroy();
    m_edges.Destroy();
    m_dirtyIndex.Destroy();
    m_dirtyCost.Destroy();
    m_dirtyFinalCost.Destroy();
}


// reset all used list data
void CDialHeap::Reset(void) {
    while (m_dirtyIndex.ToS())
        m_nodeLists[m_dirtyIndex.Pop()] = -1;
    while (m_dirtyCost.ToS())
        m_pathCost[m_dirtyCost.Pop()] = 65535;
    while (m_dirtyFinalCost.ToS())
        m_finalCost[m_dirtyFinalCost.Pop()] = 65535;
    m_costIndex = 0;
}


// start path finding for node node
void CDialHeap::Setup(int nodeId) {
    Reset();
    Push(nodeId, -1, -1, 0);
}


// put the current node node with path cost newCost in the heap or update its cost
// if it is already in the heap and has higher path cost
bool CDialHeap::Push(int nodeId, int predNodeId, int edgeId, uint16_t newCost) {
    uint16_t oldCost = m_pathCost[nodeId];
    if (newCost >= oldCost) 
        return false;    // new path is longer than the currently stored one

    uint16_t costIndex = newCost;
    if (oldCost == 65535) 
        m_dirtyCost.Push (nodeId);
    else {
        // node already in heap with higher pathCost, so unlink node from node list at current path cost position
        // find the node in the node list attached to the nodeList at the current path cost and let its successor take its place in the list
        uint16_t listRoot = oldCost;
        int16_t currNodeId = m_nodeLists[listRoot];
        int16_t nextNodeId = -1;
        while (currNodeId >= 0) {
            if (currNodeId == nodeId) {
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
int CDialHeap::Scan(int nStart) {
    int l = int(m_nodeLists.Length());
    int i = nStart;
    int j = l;
    while (j-- > 0) {
        if (m_nodeLists[i] >= 0)
            return i;
        ++i %= l;
    }
    return -1;
}


// remove node from path finding tree
auto CDialHeap::Pop(void) {
    struct retVals {
        int nodeId, cost;
    };

    int i = Scan(m_costIndex);
    if (i < 0)
        return retVals{ -1, -1 };
    m_costIndex = i;
    uint16_t nodeId = m_nodeLists[m_costIndex];
    m_nodeLists[m_costIndex] = m_nodeListLinks[nodeId];
    uint16_t cost = m_pathCost[nodeId];
    m_finalCost[nodeId] = cost;
    m_dirtyFinalCost.Push(nodeId);
    return retVals{ nodeId, cost };
}


// calculate length in segments from a node to the start node
int CDialHeap::RouteNodeCount(int nodeId) {
    // i = 65535
    // while (i > 0) {
    //     nodeId = m_predecessors [nodeId]
    //     if (h < 0) {
    //         break
    // return 65536 - i

    int i = 0;
    for (;;) {
        i++;
        if (0 > (nodeId = m_predecessors[nodeId]))
            break;
    }
    return i;
}


CList<CRouteNode>& CDialHeap::BuildRoute(int nodeId) {
    m_route.Destroy();
    for (;;) {
        m_route.Insert(0, CRouteNode(nodeId, m_edges[nodeId]));
        nodeId = m_predecessors[nodeId];
        if (nodeId < 0)
            break;
    }
    return m_route;
}


// ================================================================================
// unidirectional Dijkstra address calculation sort

CList<CRouteNode>& CRouter::BuildPath(int segmentId) {
    m_route.Destroy();
    if (m_pathCost[segmentId] != 65535)
        BuildRoute(segmentId);
    return m_route;
}


// find a path from segment startSegId to segment destSegId in a game map
// if destSegId is -1, compute minimal path cost to each other segment in the map
// that is reachable from the start segment
int CRouter::FindPath(int startSegId, int destSegId, CSegmentMap& segmentMap) {
    Setup(startSegId);
    int expanded = 1;

    LOG ("\nfinding path from %d to %d\n", startSegId, destSegId)
    for (;;) {
        auto [segId, dist] = Pop();
        LOG ("%5d: %5d\n", segId, dist)
        if (segId < 0)
            return (destSegId < 0) ? expanded : -1;
        if (segId == destSegId)
            return int (BuildPath(segId).Length());

        CMapSegment& segment = segmentMap[segId];
        for (auto [i, edgeId] : segment.m_pathEdgeIds) {
            CSegmentPathEdge e = segmentMap.m_pathEdgeTable [edgeId];
            if (Push(e.m_segmentId, segId, edgeId, dist + e.m_distance))   
                expanded++;
        }
    }
}

CRouter router;

// ================================================================================
