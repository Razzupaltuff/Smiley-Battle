import numpy as np
import globals

from vector import *
from plane import *
from texcoord import *
from vertexdatabuffers import *
from quad import *
from vao import *
from mapsegments import *
from router import *
from actorhandler import *

# =================================================================================================

class CMapLoader:
    def __init__ (self, map):
        self.map = map
        self.quadTexCoords = [CTexCoord (0,0), CTexCoord (0,1), CTexCoord (1,1), CTexCoord (1,0)]



    def CreateFromFile (self, fileName):
        stringMap = self.Load (fileName)
        if (stringMap is None):  
            return False
        return self.CreateFromMemory (stringMap, False)


    def CreateFromMemory (self, stringMap, isPrepared = False):
        if not self.Parse (stringMap, isPrepared):   # load and parse the layout data
            return False
        return self.Build ()


    def Build (self):
        self.map.vMin.z = -self.map.vMin.z
        self.map.vMax.z = -self.map.vMax.z
        self.Translate (CVector (0, 0, self.map.vMax.z))        # translate the map into the view space
        self.map.floor = self.CreateQuad (self.map.vMin.y, self.map.GetTexture (1), CVector (1,1,1))
        self.map.ceiling = self.CreateQuad (self.map.vMax.y, self.map.GetTexture (2), CVector (1,1,1))
        self.map.segmentMap.Build (self.map)  # create the segment structure
        return True


    # create a floor or ceiling quad using the specified texture
    def CreateQuad (self, y, texture, color):
        q = CQuad ([CVector (self.map.vMin.x, y, self.map.vMin.z), 
                    CVector (self.map.vMin.x, y, self.map.vMax.z), 
                    CVector (self.map.vMax.x, y, self.map.vMax.z), 
                    CVector (self.map.vMax.x, y, self.map.vMin.z)], texture, color)
        q.Create ()
        return q


    # add a vertex to the vertex list and update map boundaries
    def AddVertex (self, v):
        self.map.mesh.vertices.Append (v)
        self.map.vMin.Minimize (v)
        self.map.vMax.Maximize (v)
        self.map.vertexCount += 1


    def AddTexCoords (self):
        self.map.mesh.texCoords.data += self.quadTexCoords


    # add wall coordinates and corresponding plane data (normal and additional data for fast collision computation)
    # the wall position consists of the row and column values of the map cell the wall sits in
    # it is needed to find the walls around each segment
    def AddWall (self, x1, z1, x2, z2, position):
        x1 *= self.map.scale
        x2 *= self.map.scale
        z1 *= self.map.scale
        z2 *= self.map.scale
        w = CWall ([CVector (x1, 0, z1), CVector (x1, self.map.scale / 2, z1), CVector (x2, self.map.scale / 2, z2), CVector (x2, 0, z2)], position)
        self.map.walls.append (w)
        for v in w.vertices:
            self.AddVertex (v)
        self.AddTexCoords ()
        return w


    def ParseError (self, rowString, row, col, expected, found):
        print (rowString)
        print ('^'.rjust (col, ' '))
        print ("invalid map data in line {0} (expected {1}, found '{2}')".format (row, expected, found))


    # parse horizontal walls from the map layout. A wall is basically a minus character between two plus characters ('+-+')
    # a blank between two pluses means there is no wall
    # the map has to be surrounded by a solid line of walls
    def ParseHorizontalWalls (self, rowString, row, rowCount):
        p = 0
        w = None
        for col, c in enumerate (rowString):
            if (col % 2 == 0):
                if (c != '+') and (c != ' '):
                    self.ParseError (rowString, row * 2, col + 1, "' ' or '+'", c)
                    return False
                if (col > 0):
                    if (p == '-'):
                        w = self.AddWall (col // 2 - 1, row, col // 2, row, (col - 1, 2 * row))
                        if (row == 0) or (row == rowCount):
                            w.isBoundary = True
                    elif (p != ' '):
                        self.ParseError (rowString, row * 2, col, "' '", p)
                        return False
            else:
                if (c == '\n'):
                    return True
                if (c != ' ') and (c != '-'):
                    self.ParseError (rowString, row * 2, col + 1, "' ' or '-'", c)
                    return False
            p = c
        return True


    # parse vertical walls from the layout. A vertical wall is a '|' sign. Inside the map, vertical walls are separated
    # by blanks; These denote a segment a player can move in or through
    def ParseVerticalWalls (self, rowString, row, rowCount):
        colCount = len (rowString) // 2
        w = None
        for col, c in enumerate (rowString):
            if (col % 2 == 0):
                if (c == '|'):
                    w = self.AddWall (col // 2, row, col // 2, row + 1, (col, 2 * row + 1))
                    w.isBoundary = (col == 0)
                elif (c != ' '):
                    self.ParseError (rowString, row * 2 + 1, col + 1, "' '", c)
                    return False
            else:
                if (c == '\n'):
                    w.isBoundary = True
                    return True
                if (c != ' ') and (c != 'O') and (c != 'o'):
                    self.ParseError (rowString, row * 2 + 1, col + 1, "' ' or 'o'", c)
                    return False
        w.isBoundary = True
        return True

    # compress the map by removing duplicate columns. In the map layout file, horizontal walls are denoted by '+--+' and 
    # segments consist of two successive blank characters. This makes the layout file far better readable. However, this
    # makes parsing harder. Btw, lower or upper case 'O's denote a player start position.
    def PrepareForParsing (self, stringMap):
        for i, rowString in enumerate (stringMap):
            if (i % 2 == 0):
                stringMap [i] = stringMap [i].replace ('--', '-')
                stringMap [i] = stringMap [i].replace ('  ', ' ')
            else:
                stringMap [i] = stringMap [i].replace ('  |', ' |')
                stringMap [i] = stringMap [i].replace (' O', 'O')
                stringMap [i] = stringMap [i].replace ('O ', 'O')
                stringMap [i] = stringMap [i].replace (' o', 'o')
                stringMap [i] = stringMap [i].replace ('o ', 'o')
                stringMap [i] = stringMap [i].replace ('   ', '  ')
        return stringMap


    # Parse a map layout file and produce wall data and initial segment data from it
    def Parse (self, stringMap, isPrepared = False):
        if (stringMap is None) or (len (stringMap) == 0):
            return False
        if (not isPrepared):
            stringMap = self.PrepareForParsing (stringMap)
        parseFuncs = [self.ParseHorizontalWalls, self.ParseVerticalWalls]
        rowCount = len (stringMap) // 2
        for row, rowString in enumerate (stringMap):
            if not parseFuncs [row % 2] (rowString, row // 2, rowCount):
                return False
        self.map.stringMap = stringMap
        return True
        

    # Load a map layout file
    def Load (self, fileName):
        try:
            with open (fileName, 'r') as f:
                stringMap = f.readlines ()
                f.close ()
        except:
            return None
        rows = len (stringMap)
        if (rows < 3):
            print ("Empty or malformed map file")
            return None
        cols = len (stringMap [0])
        for l in stringMap:
            if (len (l) != cols):
                print ("All lines in the map file must have the same length")
                return None
        return stringMap


    def Translate (self, t):
        for v in self.map.mesh.vertices:
            v += t
        for w in self.map.walls:
            w.Translate (t)

# =================================================================================================
