import numpy as np

from collections import deque
from queue import LifoQueue
from mapsegments import *
from map import *

# ================================================================================

class CPathNode:
    def __init__ (self, node = -1, edge = -1):
        self.node = node
        self.edge = edge

# ================================================================================
# Dial heap for DACS path finding in graphs
# costIndex contains the total path cost to each node stored in it
# Add nodes into costIndex at the position of their predecessor in costIndex plus their edge cost
# Each costIndex entry has a list of nodes with the same path cost
# Next node to expand the path from is always the next node in costIndex seen from the current costIndex position
# Path cost can and will wrap around costIndex; that's why costIndex needs to be larger than the highest
# edge cost.
# See the internets for a full explanation of DACS (Dijkstra Address Calculation Sort)
# nodeLists contains the root indices for lists of nodes with the same cost. These indices point into nodeListLinks.
# nodeLists is indexed with path costs, nodeListLinks is indexed with node ids. For each node id stored in it, it 
# holds the index of the next node id in the list at the index of the next node's predecessor.
#
#   0   1   2         # of nodes - 1
# +---+---+---+-   -+---+
# |   | 5 |   | ... |   |
# +---+---+---+-   -+---+
#       |
#       +----------------+
#           +--+ +-----+ |
#           |  | |     | |
#           v  | v     | v
# +---+---+---+---+---+---+---+---+---+
# |   |   |-1 | 2 |   | 3 |   |   |   | ...
# +---+---+---+---+---+---+---+---+---+
#   0   1   2   3   4   5   6   7   8
#
# This means that 1 there is a list of nodes with path cost 1. nodeList [1] contains 5, that's the 1st node in the list.
# nodeListLinks [5] contains 3, which is the 2nd node in the list. nodeListLinks [3] contains 2, and nodeListLinks [2]
# contains -1, so 2 is the last node in the list of nodes with cost 1.
# The resulting list is 5,3,2

class CDialHeap:
    def __init__ (self):
        self.nodeLists = None
        self.dirtyIndex = None      # list of used nodeLists table entries
        self.nodeListLinks = None       # list of nodes with same position in costIndex
        self.predecessors = None
        self.edges = None
        self.pathCost = None
        self.finalCost = None
        self.dirtyCost = None       # list of used pathCost table entries
        self.dirtyFinalCost = None
        self.route = []
        self.maxNodes = int (0)
        self.costIndex = int (0)
        self.noCost = 65535
        self.maxCost = self.noCost - 1


    def Create (self, maxNodes):
        self.maxNodes = int (maxNodes)
        self.nodeLists = np.full (65536, int (-1), np.int16)
        self.pathCost = np.full (self.maxNodes, int (65535), np.uint16)
        self.finalCost = np.full (self.maxNodes, int (65535), np.uint16)
        self.nodeListLinks = np.full (self.maxNodes, int (-1), np.int16)
        self.predecessors = np.full (self.maxNodes, int (-1), np.int16)
        self.edges = np.zeros (self.maxNodes, np.int16)
        self.dirtyIndex = LifoQueue (maxsize = 65536)
        self.dirtyCost = LifoQueue (maxsize = 65536)
        self.dirtyFinalCost = LifoQueue (maxsize = maxNodes)


    def Destroy (self):
        self.maxNodes = 0


    # reset all used list data
    def Reset (self):
        while not self.dirtyIndex.empty():
            self.nodeLists [self.dirtyIndex.get()] = -1
        while not self.dirtyCost.empty():
            self.pathCost [self.dirtyCost.get()] = 65535
        while not self.dirtyFinalCost.empty():
            self.finalCost [self.dirtyFinalCost.get()] = 65535
        self.costIndex = 0


    # start path finding for node node
    def Setup (self, node):
        self.Reset ()
        self.Push (node, -1, -1, 0)


    # put the current node node with path cost newCost in the heap or update its cost
    # if it is already in the heap and has higher path cost
    def Push (self, node, predNode, edge, newCost):
        oldCost = self.pathCost [node]
        if (newCost >= oldCost):
            return False    # new path is longer than the currently stored one

        costIndex = np.uint16 (newCost)
        if (oldCost == 65535): 
            self.dirtyCost.put (node)
        else:
            # node already in heap with higher pathCost, so unlink
            # unlink node from node list at current path cost position
            # find the node in the node list attached to the nodeList at the current path cost
            # and let its successor take its place in the list
            listRoot = np.uint16 (oldCost)
            currNode = self.nodeLists [listRoot]
            nextNode = -1
            while (currNode >= 0):
                if (currNode == node):
                    if (nextNode < 0):
                        self.nodeLists [listRoot] = self.nodeListLinks [currNode]
                    else:
                        self.nodeListLinks [nextNode] = self.nodeListLinks [currNode]
                    break
                nextNode = currNode
                currNode = self.nodeListLinks [nextNode]

        if (0 > self.nodeLists [costIndex]):
            self.dirtyIndex.put (costIndex)
        self.pathCost [node] = newCost
        self.predecessors [node] = predNode
        self.nodeListLinks [node] = self.nodeLists [costIndex]
        self.nodeLists [costIndex] = node
        self.edges [node] = edge
        return True


    # find node with lowest path cost from current path finding state by searching through the
    # node cost offset table from the current node's position there to the next position in the table
    # holding a node
    def Scan (self, nStart):
        l = len (self.nodeLists)
        i = nStart
        j = l
        while (j > 0):
            if self.nodeLists [i] >= 0:
                return i
            j -= 1
            i += 1
            i %= l
        return -1


    # remove node from path finding tree
    def Pop (self):
        i = self.Scan (self.costIndex)
        if (i < 0):
            return -1, -1
        self.costIndex = i
        node = self.nodeLists [self.costIndex]
        self.nodeLists [self.costIndex] = self.nodeListLinks [node]
        cost = self.pathCost [node]
        #self.pathCost [node] = -1
        self.finalCost [node] = cost
        self.dirtyFinalCost.put (node)
        return node, cost


    # calculate length in segments from a node to the start node
    def RouteNodeCount (self, node):
        h = node

        # i = 65535
        # while (i > 0):
        #     h = self.predecessors [h]
        #     if (h < 0):
        #         break
        # return 65536 - i

        i = 0
        while True:
            i += 1
            h = self.predecessors [h]
            if (h < 0):
                break
        return i


    def BuildRoute (self, node):
        self.route = []
        while True:
            self.route.insert (0, CPathNode (node, self.edges [node]))
            node = self.predecessors [node]
            if (node < 0):
                break
        return self.route
            

    def FinalCost (self, node):
        return self.finalCost [node]


    def Pushed (self, node):
        return self.pathCost [node] < 65535


    def Popped (self, node):
        return not self.Pushed (node) and (self.pathCost [node] < 0)


    def RouteNode (self, i = 0):
        return self.route [i]

# ================================================================================
# unidirectional Dijkstra address calculation sort

class CRouter (CDialHeap):
    def __init__ (self):
        super ().__init__ ()


    def SetSize (self, size):
        self.maxNodes = size


    def BuildPath (self, segmentId):
        self.route = []
        if (self.pathCost [segmentId] != 65535):
            self.BuildRoute (segmentId)
        return self.route


    # find a path from segment startSegId to segment destSegId in a game map
    # if destSegId is -1, compute minimal path cost to each other segment in the map
    # that is reachable from the start segment
    def FindPath (self, startSegId, destSegId, segments):
        dist = 0
        self.Setup (startSegId)
        expanded = 1

        # print ("\nfinding path from {0} to {1}".format (startSegId, destSegId))
        while True:
            segId, dist = self.Pop ()
            if (segId < 0):
                if (destSegId < 0):
                    # print ("expanded {0} nodes".format (expanded))
                    return expanded
                else:
                    return -1
            if (segId == destSegId):
                return self.BuildPath (segId)

            segment = segments [segId]
            for edgeId in segment.edgeIds:
                e = segments.edgeTable [edgeId]
                if self.Push (e.segmentId, segId, edgeId, dist + e.distance): # internal error - overflow - tables too small
                    expanded += 1

# ================================================================================
