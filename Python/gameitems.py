import globals
import numpy as np

from icosphere import *
from reticle import *
from map import *
from actor import *
from player import *
from actorhandler import *
from arghandler import *

# =================================================================================================

class CGameItems:
    def __init__ (self):
    # create game items (may be [partially] moved into separate classes)
        pass
    

    def Create (self):
        self.map = CMap (["wall.png", "floor3.png", "ceiling2.png"])
        self.map.CreateFromFile (globals.argHandler.StrVal ("map", 0, "standard.txt"))

        # the viewer (local player). He controls the projection, i.e. everything is rendered from the perspective of the viewer
        self.viewer = globals.actorHandler.CreateViewer ()
        self.viewer.SetupCamera ("viewer", 1.0, None, CVector (0,0,0)) # CVector (self.map.scale / 2, self.map.scale / 4, -self.map.scale / 2), CVector (0, 0, 0))
        self.viewer.ForceRespawn ()
        globals.renderer.SetViewer (self.viewer)

        # player reticle
        self.reticle = CReticle ()
        self.reticle.Create ()

        # create some player spheres for testing. These have no function besides being there, being targets, and respawning
        dummies = globals.argHandler.IntVal ("dummies", 0, 0)
        if (dummies > 0):
            for i in range (min (dummies, globals.actorHandler.maxPlayers - 1)):
                dummy = globals.actorHandler.CreatePlayer ()
                dummy.SetType ("dummy")

        # self.CreatePlayer ("gold", data.textures, (self.map.vMax + self.map.vMin).Scale (0.5), data.gameTime)
        # self.CreatePlayer ("darkblue", data.textures, self.map.vMax - CVector (self.map.scale / 2, self.map.scale / 4, -self.map.scale / 2), data.gameTime)
        # self.CreatePlayer ("orange", data.textures, CVector (0, self.map.vMax.y, self.map.vMax.z) - CVector (-self.map.scale / 2, self.map.scale / 4, -self.map.scale / 2), data.gameTime)


    def Destroy (self):
        self.map.Destroy ()
        self.reticle.Destroy ()


    def Cleanup (self):
        globals.actorHandler.Cleanup ()


    def ViewerDistance (self, p):
        return self.map.Distance (self.viewer.GetPosition (), p)

# =================================================================================================

