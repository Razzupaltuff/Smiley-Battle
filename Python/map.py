import numpy as np
import globals

from vector import *
from plane import *
from texcoord import *
from vertexdatabuffers import *
from quad import *
from vao import *
from mesh import *
from mapsegments import *
from router import *
from actorhandler import *
from maploader import *
from arghandler import *

# =================================================================================================

class CMapData:
    def __init__ (self, textureNames = [], color = CVector (1,1,1), distanceQuality = 0):
        self.walls = None                                   # the walls
        self.floor = CQuad ()                               # floor structure
        self.ceiling = CQuad ()                             # ceiling structure
        self.textures = self.CreateTextures (textureNames)
        self.mesh = CMesh (GL_QUADS, self.textures [0])
        self.color = color
        self.vMin = CVector (1e6, 1e6, 1e6)                 # map boundaries
        self.vMax = CVector (-1e6, -1e6, -1e6)
        self.vertexCount = 0                                # total number of wall vertices
        self.scale = 3.0                                    # scale of the map (base unit is 1.0)
        self.distanceQuality = globals.argHandler.IntVal ("distancequality", 0, distanceQuality)
        self.segmentMap = CSegmentMap (self.scale, self.distanceQuality)    # segments
        self.stringMap = []                                 # map layout data
        self.spawnHeadings = [90, 0, -90, 180]


    def Create (self):
        self.walls = []
        self.mesh.vertices = CVertexBuffer ()                    # 3D wall coordinates
        self.mesh.texCoords = CTexCoordBuffer ()                 # texture coordinates for the walls


    # create all textures required for map rendering (walls, floor, ceiling)
    def CreateTextures (self, textureNames):
        return globals.textureHandler.CreateTextures (textureNames)


    def Destroy (self):
        pass    # textures will be globally tracked and destroyed by the textureHandler
        # for texture in self.textures:
        #     texture.Destroy ()


    def GetTexture (self, i):
        if (len (self.textures) > i):
            return self.textures [i]
        return None

# =================================================================================================
# load a map from a description / layout file in plain text format (see maps/mazehunt.txt).
# map layout is extremely simple. Everything is rectangular. A map is a rectangle formed by equally 
# sized cuboid segments. Segments may be separated by (flat / 2D) walls.
# A map class instance generates the geometry (wall vertices) and a segment structure from the map description.
# Segments have a list of adjacent segments they are connected to (i.e. which are not separated by a wall)
# A simple collision detection handles collisions of spherical objects with map walls.
# The segment structure is used to rapidly identify the walls that are close enough for a collision.
# Segments will also be used for distance calculation to create positional sound.

class CMap (CMapData):
    def __init__ (self, textureNames = [], color = CVector (1,1,1), distanceQuality = 0):
        super ().__init__ (textureNames, color, distanceQuality)
        self.loader = CMapLoader (self)
        self.router = None
        self.stringMap = None


    def __getitem__ (self, index):
        return self.segmentMap [index]


    def __setitem__ (self, index, value):
        self.segmentMap [index] = value


    def CreateFromFile (self, fileName):
        super ().Create ()
        if not self.loader.CreateFromFile (globals.gameData.mapFolder + fileName):
            return False
        self.CreateVAO ()                   # create the rendering data on OpenGL level
        return True


    def CreateFromMemory (self, stringMap, isPrepared = False):
        super ().Create ()
        if not self.loader.CreateFromMemory (stringMap, isPrepared):
            return False
        self.CreateVAO ()                   # create the rendering data on OpenGL level
        return True


    def Destroy (self):
        super ().Destroy ()
        self.mesh.Destroy ()


    def CreateVAO (self):
        self.mesh.CreateVAO ()


    def EnableTexture (self):
        if (self.textures [0] is None):
            return False
        self.textures [0].Enable ()
        return True


    def DisableTexture (self):
        if (self.textures [0] is not None):
            self.textures [0].Disable ()


    def RenderFloor (self):
        glDisable (GL_CULL_FACE)
        self.floor.Render ()
        glEnable (GL_CULL_FACE)


    def RenderCeiling (self):
        glDisable (GL_CULL_FACE)
        self.ceiling.Render ()
        glEnable (GL_CULL_FACE)


    def RenderWalls (self):
        glDisable (GL_CULL_FACE)
        self.mesh.Render ()
        glEnable (GL_CULL_FACE)


    def Render (self):
        glDisable (GL_CULL_FACE)
        self.mesh.Render ()
        self.floor.Render ()
        self.ceiling.Render ()
        glEnable (GL_CULL_FACE)


    def SegmentAt (self, position):
        clamp = lambda val, minVal, maxVal : min (max (val, minVal), maxVal)
        return clamp (int (position.x) // int (self.scale), 0, self.Width() - 1), \
               clamp (int (self.segmentMap.height + int (position.z / self.scale) - 1), 0, self.Height () - 1) # segments are added in reversed z order


    def SegmentCenter (self, x, y):
        return self.segmentMap.SegmentCenter (x, y, self.scale)


    # gather all walls from the segment a potentially colliding object sits in plus all walls
    # from the diagonally adjacent segments (this will yield all relevant walls without duplicates)
    def GetNearbyWalls (self, position):
        offsets = [(0, 0), (-1, -1), (1, -1), (1, 1), (-1, 1)]
        walls = []
        x, y = self.SegmentAt (position)
        for o in offsets:
            s = self.segmentMap.GetSegment (x + o [0], y + o [1])
            if (s is not None):
                for f in s.walls:
                    walls.append (f)
        return walls


    
    # determine the amount of actors in each map segment
    def CountActors (self):
        self.segmentMap.ResetActorCounts ()
        for a in globals.actorHandler.actors:
            p = a.GetPosition ()
            if (p is None):
                continue
            if (not self.Contains (p.x, p.z)):
                print ("actor '{0}' is out of map".format (a.camera.name))
            else:
                x, y = self.SegmentAt (p)
                self.segmentMap.CountActorAt (x, y)


    def ActorCountAt (self, x, y):
        return self.segmentMap.ActorCountAt (x, y)


    def RandomSegment (self):
        return np.random.randint (0, self.segmentMap.width), np.random.randint (0, self.segmentMap.height)


    def FindSpawnAngle (self, s):
        h = np.random.randint (s.connections) + 1
        for i in range (4):
            if (s.connected [i]):
                h -= 1
                if (h == 0):
                    return self.spawnHeadings [i]
        return 0


    # find a random spawn position by looking for a segment that is not inaccessible and has no actors inside it
    def FindSpawnPosition (self, actor):
        # actor.needSpawnPosition = False
        # return
        while True:
            x, y = self.RandomSegment ()
            s = self.segmentMap.segments [y,x]
            if (s.connections > 0) and (s.actorCount == 0):
                self.segmentMap.CountActorAt (x, y)
                # if not s.center.IsValid ():
                #     continue
                actor.SetPosition (s.center)
                actor.camera.BumpPosition ()
                actor.camera.SetOrientation (CVector (0, self.FindSpawnAngle (s), 0))
                actor.needPosition = False
                return


    # compute a segment id by linearizing its 2D coordinate in the segment map
    def SegPosFromId (self, id):
        self.segmentMap.SegPosFromId (id)


    # compute a segment's 2D coordinate in the segment map from its linearized coordinate
    def SegPosToId (self, x, y):
        return y * self.segmentMap.height + x


    def GetSegmentById (self, id):
        return self.segmentMap [id]


    def SetSegment (self, id, segment):
        x, y = self.SegPosFromId (id)
        self.segmentMap.segments [y,x] = segment


    # return the width of the segment map
    def Width (self):
        return self.segmentMap.width


    # return the height of the segment map
    def Height (self):
        return self.segmentMap.height


    # return the size of the segment map
    def Size (self):
        return self.Width () * self.Height ()


    def Contains (self, x, z):
        if (x < self.vMin.x):
            return False
        if (x > self.vMax.x):
            return False
        if (z > self.vMin.z):
            return False
        if (z < self.vMax.z):
            return False
        return True


    # compute distance between two actors
    # by adding the distances of each actor to the edge center node of its segment that lies
    # in the path between the actors to the distance from the segment distance table
    def Distance (self, p0, p1):
        if (self.distanceQuality == 0):
            return (p0 - p1).Length () * 1.3
        x, y = self.SegmentAt (p0)
        s0 = self.SegPosToId (x, y)
        x, y = self.SegmentAt (p1)
        s1 = self.SegPosToId (x, y)
        if (s0 == s1):
            return 0
        rd = self.segmentMap.Distance (s0, s1)
        return (p1 - p0).Length () if (rd.distance < 0) else rd.distance + (p0 - rd.startPos).Length () + (p1 - rd.endPos).Length ()

# =================================================================================================
