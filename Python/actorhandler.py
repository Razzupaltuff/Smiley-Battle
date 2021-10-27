from typing import List, Tuple

import numpy as np
import globals

from icosphere import *
from actor import *
from player import *
from gamedata import *
from soundhandler import *

# =================================================================================================

class CActorHandler:
    def __init__ (self) -> None:
        self.actors = []
        self.colorPool = []
        self.maxPlayers = len (globals.gameData.playerColors)
        # two sphere meshes which will be used where ever a sphere is needed. Sphere texturing and sizing is dynamic to allow for reuse.
        self.playerSphere = CRectangleIcoSphere ()
        self.playerSphere.Create (4)
        self.projectileSphere = CRectangleIcoSphere ()
        self.projectileSphere.Create (3)
        # player shadow and outline
        self.playerShadow = CPlayerShadow ()
        self.playerShadow.Create ()
        self.playerHalo = CPlayerHalo ()
        self.playerHalo.Create (5, 0.2, 0.02)
        self.playerOutline = CPlayerOutline (self.playerSphere)
        self.playerOutline.Create ()
        self.actorId = 0
        self.viewer = None


    def Destroy (self):
        for a in self.actors:
            a.Destroy ()
        self.playerOutline.Destroy ()
        self.playerShadow.Destroy ()
        self.playerHalo.Destroy ()


    def GetActorId (self):
        self.actorId += 1       # zero is reserved for players
        return self.actorId


    def CreatePlayer (self, colorIndex : int = -1, position : CVector = None, orientation : CVector = None, address : str = "127.0.0.1", ports : List [int] = [0, 0]) -> None:
        if (colorIndex < 0) or not globals.gameData.ColorIsAvailable (colorIndex):
            colorIndex = globals.gameData.GetColorIndex ()
            if (colorIndex is None):
                return None
        else:
            globals.gameData.RemoveColorIndex (colorIndex)
        player = CPlayer ("player", colorIndex, self.playerShadow, self.playerHalo, self.playerOutline)
        player.SetAddress (address, ports)
        player.Create (globals.gameData.GetColor (colorIndex) + " player", self.playerSphere, 0, globals.gameData.textures, None, position, orientation, 1.0, self.viewer.camera)
        player.SetColorIndex (colorIndex)
        self.actors.append (player)
        return player


    def CreateViewer (self) -> CViewer:
        self.viewer = CViewer ()
        self.viewer.SetColorIndex (globals.gameData.GetColorIndex ())
        self.viewer.SetupTextures (globals.gameData.textures)
        self.viewer.SetMesh (self.playerSphere)
        self.viewer.SetProjectileMesh (self.projectileSphere)
        self.actors.append (self.viewer)
        return self.viewer


    def CreateProjectile (self, parent : CPlayer, id : int = -1) -> CProjectile:
        if (id < 1):
            id = self.GetActorId ()
        projectile = CProjectile (id)
        projectile.Create (parent)
        self.actors.append (projectile)
        return projectile


    def CreateActor (self, id : int , colorIndex : int, position : CVector, orientation : CVector) -> CActor:
        if (id == 0):
            return self.CreatePlayer (colorIndex, position, orientation)
        parent = self.FindPlayer (colorIndex)
        if (parent is not None):
            return self. CreateProjectile (parent, id)
        return None


    def DeletePlayer (self, colorIndex : int) -> bool:
        player = self.FindPlayer (colorIndex)
        if (player is None):
            return False
        globals.soundHandler.StopActorSounds (player)
        globals.gameData.ReturnColorIndex (colorIndex)
        # delete all child objects (projectiles) of this player
        for a in self.actors:
            if (a.GetColorIndex () == colorIndex):
                a.Delete ()
        player.Delete ()
        return True


    def DeleteActor (self, id : int , colorIndex : int) -> bool:
        if (id == 0):
            return self.DeletePlayer (colorIndex)
        actor = globals.actorHandler.FindActor (id, colorIndex)
        if (actor is None):
            return False
        actor.Delete ()
        return True


    def FindActor (self, id : int, colorIndex : int) -> CActor:
        for a in self.actors:
            if (a.GetId () == id) and (a.GetColorIndex () == colorIndex):
                return a
        return None


    def FindPlayer (self, colorIndex : int) -> CPlayer:
        return self.FindActor (0, colorIndex)


    def CleanupActors (self) -> None:     # required when the local player disconnected and needs to rejoin from a clean slate
        for a in self.actors:
            if (a.id != 0) or (a.GetColorIndex () != self.viewer.colorIndex):
                a.Delete ()


    def Cleanup (self) -> None:
        actors = []
        for a in self.actors:
            if a.delete:
                del a
            else:
                p = a.GetPosition ()
                if (p is not None) and not globals.gameItems.map.Contains (p.x, p.z):
                    a.SetHitPoints (0)  # will delete projectiles and respawn players at a valid position
                actors.append (a)
        self.actors = actors


    def PlayerCount (self) -> int:
        return self.maxPlayers - len (globals.gameData.availableColors)


# =================================================================================================

