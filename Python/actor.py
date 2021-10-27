
from typing import List, Callable, NoReturn

import globals

from texture import *
from cubemap import *
from mesh import *
from camera import *
from gamedata import *
from gameitems import *
from networkhandler import *
from effecthandler import *
from soundhandler import *
from baseclass import *

# =================================================================================================
# Basic game object with physical properties. Can be mobile or stationary, but basically is everything 
# inside a map that is not a map and not an effect (which usually don't have physical properties relevant
# for game events, like collisions or hits)
# Contains basic handling of collisions with other actors or the map

class CActor (CBaseClass):
    def __init__ (self, type : str = "", hitPoints : int = 1, isViewer : bool = False, child = None) -> None:
        self.id : int = 0
        self.texture : CTexture = None
        self.mesh : CMesh = None
        self.camera : CCamera = None
        self.textures : List [CTexture] = None
        self.stationary : bool = False
        self.isViewer : bool = isViewer
        self.child = child
        self.type : str = type
        self.timeOfDeath : int = 0
        self.hitter = None
        self.angles : CVector = CVector (0, 0, 0)
        self.offset : CVector = CVector (0, 0, 0)
        self.scale : float = 1.0
        self.maxHitPoints : int = hitPoints
        self.hitPoints : int = self.maxHitPoints
        self.hitTime : int = 0
        self.healTime : int = 0
        self.deathTime : int = 0
        self.hiddenTime : int = 0
        self.respawnTime : int = 0
        self.immuneTime : int = 0
        self.animationDuration : int = 750    # duration [ms] of the shrinking and growing animations after death and when respawning
        self.hitSoundTime : List [int] = [0, 0]
        self.hitSoundDelay : int = 1000       # minimal time [ms] between two wall hit sounds to avoid the sounds to stutter during frequent wall contact
        self.hitEffectTime : int = 750        # duration [ms] of taking on the hitter's color after a hit
        self.soundId : int = -1
        self.lifeState : int = -1
        self.lifeStateHandlers : list [Callable [[None], NoReturn]] = [self.Die, self.Disappear, self.Bury, self.Hide, self.Resurrect, self.Reappear, self.Immunize, self.Protect]
        self.animation : int = 0
        self.delete : bool = False
        self.needPosition : bool = False


    # set a mesh (shape), texture, position and initial spatial orientation of the actor
    def Create (self, name : str, mesh : CMesh, quality : int, texture : CTexture, textureNames : List [str], position : CVector, angles : CVector, size : float, parent = None) -> None:
        # global gameData
        self.size = size
        self.SetupTextures (texture, textureNames)
        self.SetupMesh (mesh, quality, self.texture, textureNames)
        self.SetupCamera (name, size, position, angles, parent)
        self.respawnTime = globals.gameData.gameTime
        self.ForceRespawn ()


    def Destroy (self) -> None:
        self.DestroyMesh ()


    def DestroyMesh (self) -> None:
        if (self.mesh is not None):
            self.mesh.Destroy ()
            self.mesh = None


    def SetupTextures (self, texture : CTexture, textureNames : List [str] = None) -> bool:
        if (self.child is None) or not self.child.SetupTextures (texture, textureNames):     # child instance is not None and has a SetupTexture method
            self.texture = texture
            if (textureNames is not None):
                self.texture.CreateFromFile (textureNames)
        return True


    def SetTexture (self, texture : CTexture) -> None:
        self.texture = texture


    def SetupMesh (self, mesh : CMesh, quality : int, texture : CTexture, textureNames : List [CTexture]) -> None:
        self.mesh = mesh
        if (quality > 0):
            self.mesh.Create (quality, texture, textureNames)


    def SetupCamera (self, name : str, size : float, position : CVector, angles : CVector, parent : CCamera = None) -> None:
        self.camera = CCamera (name, self.isViewer)
        self.camera.UpdateAngles (angles)
        self.camera.SetPosition (position)
        self.camera.SetSize (size) # / self.BorderScale ())
        self.camera.SetParent (parent)


    def EnableCamera (self) -> None:
        if (self.camera is not None):
            self.camera.Enable ()


    def DisableCamera (self) -> None:
        if (self.camera is not None):
            self.camera.Disable ()


    def SetTexture (self, texture : CTexture) -> None:
        self.texture = texture


    def SetMesh (self, mesh : CMesh) -> None:
        self.mesh = mesh


    def SetType (self, type : str) -> None:
        self.type = type


    def IsType (self, type) -> bool:
        return self.type == type


    def GetId (self) -> int:
        return self.id


    def GetType (self) -> str:
        return self.type


    def IsPlayer (self) -> bool:
        return self.id == 0


    def IsProjectile (self) -> bool:
        return self.id != 0


    def GetPosition (self, i : int = 0) -> CVector:
        return self.camera.GetPosition (i) if (self.camera is not None) else CVector (0,0,0)


    def GetOrientation (self) -> CVector:
            return self.camera.GetOrientation () if (self.camera is not None) else CVector (0,0,0)


    def SetPosition (self, position : CVector) -> None:
        if (self.camera is not None):
            self.camera.SetPosition (position)
            if (self.camera.GetPosition (1) is None):
                self.camera.SetPosition (position, 1)
            self.needPosition = False


    def SetOrientation (self, angles : CVector) -> None:
        if (self.camera is not None):
            self.camera.SetOrientation (angles)
        

    def GetName (self) -> str:
        return self.camera.name if (self.camera is not None) else ""


    def GetSize (self) -> float:
            return self.camera.size if (self.camera is not None) else 0


    def SetSize (self, size : float) -> None:
        if (self.camera is not None):
            self.camera.size = size # * self.BorderScale ()


    def SetScale (self, scale : float) -> None:
        self.scale = scale


    def SetLifeState (self, lifeState : int ) -> None:
        self.lifeState = lifeState


    def SetAnimation (self, animation : int) -> None:
        self.animation = animation
        if self.isViewer:
            globals.networkHandler.BroadcastAnimation ()


    def GetAnimation (self) -> int:
        return self.animation


    def Radius (self) -> float:
        return self.GetSize () * 0.5 # * self.BorderScale ()


    def BorderScale (self) -> float:
        return self.child.BorderScale () if (self.child is not None) else 1.0


    def Render (self, autoCamera : bool = True) -> None:
        if self.isViewer:
            return
        if (autoCamera):
            self.EnableCamera ()
        glPushMatrix ()
        size = self.camera.size / self.BorderScale ()
        glScalef (size, size, size)
        if (self.hitter is None):
            self.mesh.PushTexture (self.texture)
        else:
            self.mesh.PushTexture (self.hitter.GetTexture (self.child.Mood ()))
            self.mesh.PushColor (self.hitter.GetPlayerColorValue ())
        self.mesh.Render ()
        self.mesh.PopTexture ()
        if (self.hitter is not None):
            self.mesh.PopColor ()
        glPopMatrix ()
        if (autoCamera):
            self.DisableCamera ()


    def StartCollisionHandling (self) -> None:
        self.stationary = False


    def MayPlayHitSound (self, type : int) -> bool:
        if (globals.gameData.gameTime < self.hitSoundTime [type]):
            return False
        self.hitSoundTime [type] = globals.gameData.gameTime + self.hitSoundDelay
        return True


    def MayPlayActorHitSound (self) -> bool:
        return self.MayPlayHitSound (0)


    def MayPlayWallHitSound (self) -> bool:
        return self.MayPlayHitSound (1)


    # move the object by a displacement vector computed in the map or actor collision handling routines
    def Bounce (self, v : CVector) -> None:
        self.camera.positions [0] += v


    # called when this actor has been hit by shot
    def RegisterHit (self, hitter = None) -> None:
        if (self.hitPoints > 0):
            self.SetHitPoints (self.hitPoints - 1, hitter)
            if self.IsLocalActor () and (hitter is not None):
                globals.networkHandler.BroadcastHit (self, hitter)


    # set hit points. If hit by a projectile, the player who had fired that projectile is passed in hitter
    # That player will receive a point for hitting and another point if killing the current actor (which in
    # this game will always be another player)
    # start death animation if player is killed
    def SetHitPoints (self, hitPoints : int , hitter = None) -> None:
        # global gameData
        if (self.hitPoints == hitPoints):
            return
        self.hitPoints = hitPoints
        self.hitTime = globals.gameData.gameTime
        self.healTime = 0       # restart healing process when hit
        if (hitter is not None):
            hitter.AddScore (1) # one point for the hit
            self.hitter = hitter
        if (self.hitPoints == 0):
            if (self.id == 0):
                self.AddDeaths ()
            # addDeaths = self.GetMethod (self, "AddDeaths")
            # if (addDeaths):
            #     addDeaths ()
            if (hitter is not None):
                hitter.AddScore (globals.gameData.pointsForKill)         # another point for the kill
                hitter.AddKills ()
            self.lifeState = 0


    # The following functions are part of the death and respawn handling state engine
    # They set the timeouts for the various effects and delays and make sure the player
    # gets a new spawn position before reappearing
    # In multiplayer matches, each client only handles death and respawning for itself
    # and transmits his current status to the other players with UPDATE messages
    def Die (self) -> None:
        self.SetAnimation (1)
        if (self == globals.actorHandler.viewer):
            globals.effectHandler.StartFade (CVector (0,0,0), self.animationDuration, False)
        self.deathTime = self.hitTime   # start death animation
        self.lifeState = 1


    def Disappear (self) -> None:
        dt = globals.gameData.gameTime - self.deathTime 
        if (dt <= self.animationDuration):
            self.scale = 1.0 - dt / self.animationDuration
        else:
            self.lifeState = 2


    def Bury (self) -> None:
        self.hitter = None
        self.hiddenTime = globals.gameData.gameTime + globals.gameData.respawnDelay
        self.scale = 0.0
        self.lifeState = 3


    def Hide (self) -> None:
        if (globals.gameData.gameTime >= self.hiddenTime):
            self.SetAnimation (2)
            if self.IsLocalActor ():
                globals.gameItems.map.FindSpawnPosition (self)
            else:
                self.needPosition = True
            self.lifeState = 4


    # start respawn animation. Signal request for a spawn position to the app
    # start immunity period
    def Resurrect (self) -> None:
        if (self.needPosition):     # don't respawn without a valid spawn position
            return
        self.respawnTime = globals.gameData.gameTime + self.animationDuration
        if (self == globals.actorHandler.viewer):
            globals.effectHandler.StartFade (CVector (0,0,0), self.animationDuration, True)
        self.lifeState = 5


    def Reappear (self) -> None:
        dt = self.respawnTime - globals.gameData.gameTime
        if (dt > 0):
            self.scale = (self.animationDuration - dt) / self.animationDuration
        else:
            self.scale = 1.0
            self.lifeState = 6


    def Immunize (self) -> None:
        self.immuneTime = globals.gameData.gameTime + globals.gameData.immunityDuration
        self.lifeState = 7


    def Protect (self) -> None:
        if (globals.gameData.gameTime > self.immuneTime):
            self.hitPoints = self.maxHitPoints
            self.lifeState = -1


    # The state engine driver
    def UpdateLifeState (self) -> None:
        oldState = -1
        while (self.lifeState >= 0) and (oldState != self.lifeState):
            oldState = self.lifeState
            self.lifeStateHandlers [self.lifeState] ()


    def ForceRespawn (self) -> None:
            self.hitPoints = 0
            self.hiddenTime = 0
            self.lifeState = 3
            self.needPosition = True


    # check whether the current actor is dead
    def IsDead (self) -> bool:
        return self.hitPoints == 0


    def IsAlive (self) -> bool:
        return self.hitPoints > 0


    def IsDieing (self) -> bool:
        return (self.lifeState >= 0) and (self.lifeState <= 2)


    def IsHidden (self) -> bool:
        return (self.lifeState == 3) or (self.lifeState == 4)


    # check whether the actor is currently respawning
    def IsRespawning (self) -> bool:
        return (self.lifeState == 5)


    # check whether the actor can be hit and can shoot 
    def IsImmune (self) -> bool:
        # global gameData
        return (self.lifeState == 6) or (self.lifeState == 7)


    def IsLocalActor (self) -> bool:
        if (globals.networkHandler.localAddress == "127.0.0.1"):
            return True
        if (self.child is None):
            return False
        return (self.child.GetColorIndex () == globals.actorHandler.viewer.colorIndex) or \
               (self.child.GetAddress () == globals.networkHandler.localAddress) or \
               (self.child.GetAddress () == "127.0.0.1") 


    # heal the actor until it has full health or gets hit. Wait gameData.healDelay ms for healing by one hitpoint
    # don't heal when dead
    def Heal (self) -> None:
        # global gameData
        if (self.hitPoints == 0): 
            return
        if (self.hitPoints == self.maxHitPoints):
            self.healTime = 0
            return
        if (self.healTime == 0):
            self.healTime = globals.gameData.gameTime    # start over
            return
        elif (globals.gameData.gameTime - self.healTime < globals.gameData.healDelay):
            return
        self.hitPoints += 1
        print ("healed to " + str (self.hitPoints) + " HP")
        self.healTime = 0


    def UpdateSound (self) -> None:
        # global soundHandler
        if (self.id != 0):
            return
        if self.IsAlive () or self.IsImmune ():
            if not self.isViewer and (self.soundId < 0):
                self.soundId = globals.soundHandler.Play ("hum", self.GetPosition (), volume = 0.1, loops = -1, owner = self, level = 4)
        else:
            if (self.soundId >= 0):
                globals.soundHandler.Stop (self.soundId)
                self.soundId = -1


    # update actor status (death animation, respawn animation, healing)
    def Update (self, dt : float = 1.0, angles : CVector = None, offset : CVector = None) -> None:
        # global gameData
        if self.IsLocalActor ():
            self.UpdateLifeState ()
            self.Heal ()
        if (self.hitter is None):
            return
        if (globals.gameData.gameTime - self.hitTime < self.hitEffectTime):
            return
        self.hitter = None
        self.hitTime = 0
        return

# =================================================================================================
