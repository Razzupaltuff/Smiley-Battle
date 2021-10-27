#pragma once

#include <stdint.h>

#include "cstack.h"
#include "mapsegments.h"
#include "map.h"

// ================================================================================

class CRouteNode {
public:
    int m_nodeId;
    int m_edgeId;

    CRouteNode (int nodeId = -1, int edgeId = -1) : m_nodeId (nodeId), m_edgeId (edgeId) {}

};

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

class CDialHeap {
public:
    int     m_maxNodes;
    int     m_costIndex;
    int     m_maxCost;  // max. path cost of a single graph edge
    int     m_noCost;   // cost value for "no path cost calculated"

    CArray<int16_t>     m_nodeLists;
    CArray<int16_t>     m_nodeListLinks;
    CArray<int16_t>     m_predecessors;
    CArray<uint16_t>    m_pathCost;
    CArray<uint16_t>    m_finalCost;
    CArray<int16_t>     m_edges;
    CStack<uint16_t>    m_dirtyIndex;
    CStack<uint16_t>    m_dirtyCost;
    CStack<uint16_t>    m_dirtyFinalCost;
    CList<CRouteNode>   m_route;

    CDialHeap() : m_maxNodes(0), m_costIndex(0), m_maxCost (65534), m_noCost (65535) {}

    ~CDialHeap() {
        Destroy();
    }

    void Create(int maxNodes);

    void Destroy(void);

    // reset all used list data
    void Reset(void);

    // start path finding for node node
    void Setup(int nodeId);

    // put the current node node with path cost newCost in the heap or update its cost
    // if it is already in the heap and has higher path cost
    bool Push(int nodeId, int predNodeId, int edgeId, uint16_t newCost);

    // find node with lowest path cost from current path finding state by searching through the
    // node cost offset table from the current node's position there to the next position in the table
    // holding a node
    int Scan(int nStart);

    // remove node from path finding tree
    auto Pop(void);

    // calculate length in segments from a node to the start node
    int RouteNodeCount(int nodeId);

    CList<CRouteNode>& BuildRoute(int nodeId);

    inline int MaxCost (void) {
        return m_maxCost;
    }

    inline uint16_t FinalCost(int nodeId) {
        return m_finalCost[nodeId];
    }


    // required for bi-directional DACS (not implemented here)
    inline bool Pushed(int nodeId) {
        return m_pathCost[nodeId] < m_noCost;
    }


    inline bool Popped(int nodeId) {
        return !Pushed(nodeId) && (m_pathCost[nodeId] < 0);
    }


    inline CRouteNode& RouteNode(int i = 0) {
        return m_route[i];
    }

};

// ================================================================================
// unidirectional Dijkstra address calculation sort

class CRouter : public CDialHeap {
public:
    CRouter() : CDialHeap() {}


    void SetSize(uint16_t size) {
        m_maxNodes = size;
    }


    CList<CRouteNode>& BuildPath(int segmentId);

    // find a path from segment startSegId to segment destSegId in a game map
    // if destSegId is -1, compute minimal path cost to each other segment in the map
    // that is reachable from the start segment
    int FindPath(int startSegId, int destSegId, CSegmentMap& segmentMap);

};

extern CRouter router;

// ================================================================================
