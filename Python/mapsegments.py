import numpy as np
import math

from vector import *
from plane import *
from router import *

# =================================================================================================

class CSegmentPathNode:
    def __init__ (self, id = -1, nodePos = CVector (0,0,0), distToCenter = 0):
        self.id = id
        self.nodePos = nodePos
        self.distToCenter = distToCenter    # distance to segment center

    
class CSegmentPathEdge:
    def __init__ (self, segmentId = -1, startPos = CVector (0,0,0), endPos = CVector (0,0,0), distance = -1):
        self.segmentId = segmentId
        self.startPos = startPos
        self.endPos = endPos
        self.distance = distance


class CRouteData:
    def __init__ (self, distance, startPos, endPos):
        self.distance = distance
        self.startPos = startPos
        self.endPos = endPos

# =================================================================================================

class CWall (CPlane):
    def __init__ (self, vertices, position, isBoundary = False):
        super ().__init__ (vertices)
        self.position = position
        self.isBoundary = isBoundary


    def Translate (self, t):
        self.center += t

# =================================================================================================
# Segment class for map segments

class CMapSegment:
    def __init__ (self, x, y, id = -1):
        self.connected = [False, False, False, False]   # orthogonal neighbours
        self.connections = 0
        self.walls = []                                 # list of walls around the segment
        self.pathNodes = []                             # reference coordinates for los (line of sight) testing
        self.edgeIds = []                               # indices into list of other segments a los exists to
        self.position = (x, y)                          # position in the segment grid
        self.center = CVector (0,0,0)
        self.id = id
        self.actorCount = 0


    def SetId (self, id):
        self.id = id


    def GetId (self):
        return self.id


    def IsConnected (self, direction):
        return self.connected [direction] 


    def AddPathNode (self, node, distToCenter):
        self.pathNodes.append (CSegmentPathNode (self.GetId () * 10 + len (self.pathNodes) + 1, node, distToCenter))


    def FindPathEdge (self, edgeList, segmentId):
        for i in self.edgeIds:
            e = edgeList [i]
            if (e.segmentId == segmentId):
                return e
        return None


    def AddPathEdge (self, edgeList, edgeId, segmentId, startPos, endPos, distance):
        e = self.FindPathEdge (edgeList, segmentId)
        if (e is None):
            self.edgeIds.append (edgeId)
            return CSegmentPathEdge (segmentId, startPos, endPos, distance), True
        if (int (distance) < e.distance):
            e.startPos = startPos
            e.endPos = endPos
            e.distance = int (distance)
            return e, False
        return None, False

# =================================================================================================
# rectangular (2D) map of all segments

class CSegmentMap:
    def __init__ (self, scale, distanceQuality = 0):
        self.segments = None
        self.edgeList = []
        self.height = 0
        self.width = 0
        self.size = 0
        self.scale = scale
        self.distanceScale = 1000
        self.distanceQuality = distanceQuality
        self.router = None


    def __getitem__ (self, index):
        return self.segments [index // self.width, index % self.width]


    def __setitem__ (self, index, value):
        self.segments [index // self.width, index % self.width] = value


    def Create (self, width, height):
        self.width = width // 2
        self.height = height // 2
        self.size = self.width * self.height
        self.router = CRouter ()
        w = self.width * self.scale
        h = self.height * self.scale
        self.segments = np.full ([self.height, self.width], 0, CMapSegment)


    def AddSegment (self, x, y):
        s = CMapSegment (x, y, y * self.width + x)
        self.segments [y,x] = s
        return s


    def GetSegment (self, x, y):
        if (x < 0) or (y < 0) or (x >= self.width) or (y >= self.height):
            return None
        return self.segments [y,x]


    def GetSegmentById (self, id):
        return self.GetSegment (id % self.width, id // self.width)


    def SegPosFromId (self, id):
        return id % self.width, id // self.width


    def FindWall (self, walls, position):
        for wall in walls:
            if (wall.position == position):
                return wall
        return None


    # link current segment with neighbar at map position relative to the segment's position
    # direction: Orthogonal direction from current to adjacent segment
    # Add wall in direction if there is a wall, otherwise create link information in that 
    # direction (grid position of the adjacent segment)
    def LinkSegments (self, stringMap, segment, x, y, dx, dy, walls, direction):
        if (stringMap [y + dy][x + dx] != ' '): # i.e. there is a wall in that direction
            # so find wall by its position and append it to wall list
            segment.walls.append (self.FindWall (walls, (x + dx, y + dy)))  
            return 0
        else:   # no wall in direction direction
            # enter information about the adjacent segment's grid position
            # note: That segment will be or has been treated here as well, setting up its corresponding link information
            segment.connected [direction] = True
            segment.connections += 1
            return 1


    # Build the complete segment grid, determining walls and reachable neighbours around each segment
    def Build (self, map):
        rows = len (map.stringMap)
        cols = len (map.stringMap [0]) - 1
        self.Create (cols, rows)
        for y in range (1, rows, 2):
            for x in range (1, cols, 2):
                segment = self.AddSegment (x // 2, y // 2)
                self.LinkSegments (map.stringMap, segment, x, y, -1,  0, map.walls, 0)
                self.LinkSegments (map.stringMap, segment, x, y,  0, -1, map.walls, 1)
                self.LinkSegments (map.stringMap, segment, x, y,  1,  0, map.walls, 2)
                self.LinkSegments (map.stringMap, segment, x, y,  0,  1, map.walls, 3)
        self.CreatePathNodes (map)
        if (self.distanceQuality == 1):
            self.distanceScale = 1000
            while (not self.CreatePathEdges (map)):
                self.ResetPathData ()
            self.CreateDistanceTable ()


    def SegmentCenter (self, x, y, scale):
        return CVector ((x + 0.5) * scale, scale / 4, -(self.height - y - 0.5) * scale)


    def ComputeSegmentCenters (self, map):
        for y, segmentRow in enumerate (self.segments):
            for x, segment in enumerate (segmentRow):
                segment.center = self.SegmentCenter (x, y, map.scale)


    # create four 2D coordinates for each segment: edge centers
    # outer coordinates are slightly offset into the segment
    # Purpose: Finding out to which other segments a given segment has a los (line of sight)
    # This will be used in distance calculation for sound sources create (pseudo) positional sound
    # los coordinates are only created if there is no wall at their edge
    def CreatePathNodes (self, map):
        offsets = [(-1,0), (0,-1), (1,0), (0,1)]    # [(-1,0), (0,-1), (1,0), (0,1), (0,0), (-1,-1), (1,-1), (1,1), (-1,1)]
        nodeCount = 0
        for y, segmentRow in enumerate (self.segments):
            radius = map.scale / 2
            for x, segment in enumerate (segmentRow):
                segment.center = self.SegmentCenter (x, y, map.scale)
                for direction, o in enumerate (offsets):
                    if (segment.connected [direction]):
                        nodeOffset = CVector (radius * o [0], 0, radius * o [1])
                        segment.AddPathNode (segment.center + nodeOffset, nodeOffset.Length ())
                        nodeCount += 1
        print ("created " + str (nodeCount) + " path nodes")


    def ResetPathData (self):
        emptyList = []
        for i in range (self.size - 1):
            self [i].edgeIds = emptyList [:]
        self.edgeList = emptyList [:]

    # CreatePathEdges casts a ray from each path nodes of each segment to each path node of each other
    # segment. When such a ray does not intersect any interor walls of the map, there is a line of sight
    # between the two segments. Rays will not be cast to path nodes behind the current path node as seen
    # from the path node's segment's center. That direction will be handled by the segment's path node at
    # the opposite segment edge.
    def CreatePathEdges (self, map):    
        losTests = 0
        edgeCount = 0
        lMax = 0
        for i in range (self.size - 1):
            si = self [i]
            for j in range (i + 1, self.size):
                sj = self [j]
                for ni in si.pathNodes:
                    n = ni.nodePos - si.center
                    for nj in sj.pathNodes:
                        v = nj.nodePos - ni.nodePos
                        d = v.Length ()
                        if (d == 0):
                            haveLoS = True
                        else:
                            if (n.Dot (v) < 0.0):
                                continue
                            haveLoS = True
                            for w in map.walls:
                                if w.isBoundary:   # don't consider walls surrounding the map as there is no segment behind them
                                    continue
                                losTests += 1
                                vi, penetrated = w.LineIntersection ([ni.nodePos, nj.nodePos])
                                if (vi is None):   # vector from path nodes ni to nj crosses a wall
                                    continue
                                if w.Contains (vi):
                                    haveLoS = False
                                    break   # no need for further tests, we don't have LoS anymore already
                        if (haveLoS):
                            d += ni.distToCenter + nj.distToCenter
                            l = int (math.ceil (d * self.distanceScale))
                            e, isNewEdge = si.AddPathEdge (self.edgeList, edgeCount, sj.id, ni.nodePos, nj.nodePos, d)
                            if isNewEdge:
                                self.edgeList.append (e)
                                edgeCount += 1
                            e, isNewEdge = sj.AddPathEdge (self.edgeList, edgeCount, si.id, nj.nodePos, ni.nodePos, d)
                            if isNewEdge:
                                self.edgeList.append (e)
                                edgeCount += 1

            if (lMax > self.router.maxCost):
                m_distanceScale = int (self.router.maxCost * self.distanceScale) // lMax
                return False

            self.edgeTable = np.empty ([edgeCount], CSegmentPathEdge)
            for i, e in enumerate (self.edgeList):
                self.edgeTable [i] = e
            return True
        # diagnostic message for test purposes
        # wallCount = [0, 0]
        # for w in map.walls:
        #     wallCount [w.isBoundary] += 1
        # print ("performed {0} LoS tests for {1} walls ({2} boundary walls were ignored). Created {3} edges.".format (losTests, wallCount [0], wallCount [1], edgeCount))


    # build a table with distances from each segment to each other reachable segment
    # For start and end segment, subtract the distance between the segment centers and the segments' edge center nodes used in
    # the shortest path between these segment. When determining the distance between two actors, the distances of the
    # actors to the edge center nodes of their respective segment. This yields an acceptable approximation of the 
    # path distance between the two actors.
    # 
    # +---------+---------+---------+---------+
    # |         |         |         |S4       | 
    # |      /--x---------x---------x-\__     | 
    # |     /   |         |         |    \-B  | 
    # +----x----+---------+---------+---------+
    # |S2  |    |
    # |    |    |
    # |    |    |
    # +----x----+
    # |S1   \   |
    # |      \  |
    # |       A |
    # +---------+
    
    def CreateDistanceTable (self):
        self.router.Create (self.size)
        self.distanceTable = np.full ([self.size, self.size], 0, CRouteData)
        for i in range (self.size - 1):
            si = self [i]
            self.router.FindPath (si.id, -1, self)
            for j in range (i + 1, self.size):
                route = self.router.BuildRoute (j)
                if (len (route) < 3):
                    self.distanceTable [i,j] = \
                    self.distanceTable [j,i] = CRouteData (-1, CVector (0,0,0), CVector (0,0,0))
                else:
                    p0 = self.edgeTable [route [1].edge].startPos
                    p1 = self.edgeTable [route [-2].edge].startPos
                    x, y = self.SegPosFromId (i)
                    v0 = p0 - self.segments [y,x].center
                    x, y = self.SegPosFromId (j)
                    v1 = p1 - self.segments [y,x].center
                    cost = self.router.FinalCost (j) / self.distanceScale - v0.Length () - v1.Length ()
                    self.distanceTable [i,j] = CRouteData (cost, p0, p1)
                    self.distanceTable [j,i] = CRouteData (cost, p1, p0)
        self.router.Destroy ()


    # reset the actor count in each segment that had previously been computed
    def ResetActorCounts (self):
        for row in self.segments:
            for col in row:
                col.actorCount = 0

    # increase the actor count in the segment at map position x,y
    def CountActorAt (self, x, y):
        self.segments [y,x].actorCount += 1


    def ActorCountAt (self, x, y):
        return self.segments [y,x].actorCount


    def Distance (self, i, j):
        return self.distanceTable [i,j]

# =================================================================================================
