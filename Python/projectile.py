import globals

from camera import *
from actor import *
from gamedata import *
from gameitems import *
from networkhandler import *

# =================================================================================================
# Handling of shots. Shots have straight movement at a fixed speed.
# If they hit something (other actor or wall), they disappear

class CProjectile (CActor):
    def __init__ (self, id):
        super ().__init__ ("projectile", 1, False, self)
        self.id = id
        self.speed = 0.2
        self.offset = CVector (0,0,0)
        self.parent = None
        self.outline = None
        self.frozenTime = 0
 

    def Create (self, parent):
        self.parent = parent
        if (parent.projectileMesh is None):
            self.mesh = parent.mesh
        else:
            self.mesh = parent.projectileMesh
        self.SetTexture (parent.GetTexture ())
        self.speed = globals.gameData.projectileSpeed
        self.camera = parent.camera.Clone ()
        self.camera.parent = parent.camera
        self.camera.name = "projectile " + str (np.random.randint (0, 1000))
        self.camera.isViewer = False
        self.offset = self.camera.orientation.Unrotate (CVector (0, 0, parent.camera.size / 2))
        self.camera.SetPosition (self.camera.GetPosition () - self.offset)
        self.offset *= self.speed / parent.camera.size * 2
        self.camera.BumpPosition ()
        self.camera.SetSize (globals.gameData.projectileSize)
    

    def SetupTextures (self, texture, textureNames = None): # required for CActor not recursively calling its SetupTexture method when the child doesn't have one of its own
        return False


    def UpdateOffset (self):
    #     p = self.camera.GetPosition ()
        self.camera.BumpPosition ()
    #     p -= self.offset


    def Update (self, angles = None, offset = None, dt = 1.0):
        if self.IsDead ():
            self.delete = True
        elif self.IsLocalActor ():  # in multiplayer games, only move the projectiles fired by the local player
            self.camera.SetPosition (self.camera.GetPosition () - self.offset * dt)
            self.camera.UpdateAngles (CVector (0, 30, 0))

    # Due to their high speed, the actor/actor and actor/wall collision handling may miss collisions,
    # so shot collision handling is done via the translation vector from the current to the intended 
    # new position.
    # For a wall collision, the vector must cross the wall's plane and the hit point must lie within
    # the wall rectangle. For an actor collision, the distance between the actor's center and the
    # intersection point of the translation vector and the vector parallel to the translation vector's 
    # normal and originating from the actor's center. If there is no such intersection, use the 
    # closer of the translation vector's end points.

    def StartCollisionHandling (self):
        super ().StartCollisionHandling ()


    def Delete (self, force = False):
        if (force or self.IsLocalActor ()):
            globals.networkHandler.BroadcastDestroy (self)
        self.SetHitPoints (0)
        self.delete = True


    def Render (self):
        # print ("Projectile @ {:1.4f} {:1.4f}".format (self.GetPosition ().x, self.GetPosition ().z))
        self.mesh.PushColor (self.parent.GetPlayerColorValue ())
        super ().Render ()
        if (self.outline is not None):
            self.outline.Render (self.camera.size)
        self.mesh.PopColor ()


    def BorderScale (self):
        if (self.outline is not None):
            return self.outline.Scale ()
        return 1.0


    def GetColorIndex (self):
        return self.parent.colorIndex


    def GetAddress (self):
        return self.parent.address


    def GetPort (self):
        return self.parent.port


    def UpdateFrozenTime (self):
        if (self.GetPosition (0) != self.GetPosition (1)):
            self.frozenTime = 0
        else:
            if (self.frozenTime == 0):
                self.frozenTime = globals.gameData.gameTime
            elif (globals.gameData.gameTime - self.frozenTime > gameData.frozenTimeout):
                self.Delete ()

# =================================================================================================
