import globals

from OpenGL.GL import *

from quad import *
from cubemap import *
from torus import *
from camera import *
from actor import *
from projectile import *
from gamedata import *
from gameitems import *
from networkhandler import *
from arghandler import *

# =================================================================================================
# Shadow for players (smileys). Just a 2D texture rendered near the ground

class CPlayerShadow (CQuad):
    def __init__ (self):
        self.groundClearance = 0.0002
        super ().__init__ ([CVector (-0.5, 0.0, 0.5), CVector (-0.5, 0.0, -0.5), CVector (0.5, 0.0, -0.5), CVector (0.5, 0.0, 0.5)], self.CreateTexture ())


    def CreateTexture (self):
        texture = globals.textureHandler.GetTexture ()
        if texture.CreateFromFile ([globals.gameData.textureFolder + "shadow.png"]):
            return texture
        return None


    def Render (self, offset):
        glPushMatrix ()
        glDisable (GL_CULL_FACE)
        glTranslatef (0.0, offset + self.groundClearance, 0.0)
        super ().Render ()
        glEnable (GL_CULL_FACE)
        glPopMatrix ()

# =================================================================================================
# Shadow for players (smileys). Just a 2D texture rendered near the ground

class CPlayerHalo (CTorus):
    def __init__ (self):
        self.tolerance = 0.0001
        super ().__init__ (None, ["white.png"], globals.gameData.GetColorValue ("gold"))
        # super ().__init__ ([CVector (-0.5, 0.0, 0.5), CVector (-0.5, 0.0, -0.5), CVector (0.5, 0.0, -0.5), CVector (0.5, 0.0, 0.5)], self.CreateTexture ())


    def CreateTexture (self):
        texture = globals.textureHandler.GetTexture ()
        if texture.CreateFromFile ([globals.gameData.textureFolder + "halo.png"]):
            return texture
        return None


    def Render (self, offset):
        glPushMatrix ()
        glDisable (GL_CULL_FACE)
        glTranslatef (0.0, offset + self.tolerance, 0.0)
        glScalef (0.667, 1.0, 0.667)
        super ().Render ()
        # glScalef (2.0, 1.0, 2.0)
        glEnable (GL_CULL_FACE)
        glPopMatrix ()

# =================================================================================================
# Outline for players (smileys). The outline is created by rendering the backwards facing faces of
# the sphere in black and a tad bigger than the player smileys.
# The outline is rendered after the smiley to improve performance, since the z culling will discard
# most of the render data (pixels) early

class CPlayerOutline ():
    def __init__ (self, mesh = None):
        self.mesh = mesh
        self.scale = 1.02
        self.textureNames = ["black.png", "white.png"]
        self.textures = None


    def Create (self):
        self.textures = globals.textureHandler.CreateCubemaps (self.textureNames)


    def Destroy (self):
        pass    # textures will be globally tracked and destroyed by the textureHandler
        # if (self.textures is not None):
        #     for t in self.textures:
        #         t.Destroy ()


    def Scale (self):
       return self.scale


    def Render (self, size, colorIndex = 0):
        glPushMatrix ()
        glScalef (size, size, size)
        glCullFace (GL_FRONT)
        self.mesh.PushTexture (self.textures [colorIndex])
        self.mesh.vao.SetMinBrightness (0.9)
        self.mesh.Render ()
        self.mesh.vao.SetMinBrightness (0)
        self.mesh.PopTexture ()
        glCullFace (GL_BACK)
        glPopMatrix ()

# =================================================================================================
# Player actor. Standard actor with a few extra properties: Shadow, outline, firing shots, changing
# color when hit, dieing animation

class CPlayer (CActor):
    def __init__ (self, type = "player", colorIndex = -1, shadow = None, halo = None, outline = None, isViewer = False):
        super ().__init__ (type, 3, isViewer, self)
        self.shadow = shadow
        self.outline = outline
        self.halo = halo
        self.projectileMesh = None
        self.whiteForBlack = False
        self.colorIndex = -1
        self.SetColorIndex (colorIndex)
        self.textures = None
        self.moods = ["-sad", "-neutral", "-happy"]
        self.colorValue = CVector (1,1,1)
        self.color = "white"
        self.address = "127.0.0.1"
        self.ports = [0, 0]
        self.lastMessageTime = 0    # time when the last network message from this player was received
        self.isConnected = False    # remote player is connected with local player
        self.score = 0
        self.kills = 0
        self.deaths = 0
        self.wiggleTimer = CTimer ()
        self.wiggleAngle = np.random.randint (36) * 5


    def Destroy (self):
        if (self.shadow is not None):
            self.shadow.Destroy ()
        if (self.outline is not None):
            self.outline.Destroy ()
        super ().Destroy ()


    def SetupTextures (self, textures, textureNames = None):
        self.textures = []
        color = globals.gameData.GetColor (self.colorIndex)
        if (color != "black"):
           color = "white"
        for mood in self.moods:
            self.textures.append (textures [color + mood])
        return True


    def SetProjectileMesh (self, mesh):
        self.projectileMesh = mesh


    def SetShadow (self, shadow):
        self.shadow = shadow


    def SetOutline (self, outline):
        self.outline = outline


    def SetHalo (self, halo):
        self.halo = halo


    def SetColorIndex (self, colorIndex, replace = False):
        if (replace and (self.colorIndex >= 0)):
            globals.gameData.ReturnColorIndex (self.colorIndex)
        if (replace and (colorIndex >= 0)):
            globals.gameData.RemoveColorIndex (colorIndex)
        self.colorIndex = colorIndex
        if (self.colorIndex >= 0):
            self.colorValue, self.color, self.whiteForBlack = globals.gameData.GetPlayerColorValue (self, True)
        else:
            self.color = "white"
            self.colorValue = CVector (1,1,1)
            self.whiteForBlack = False


    def GetColorIndex (self):
        return self.colorIndex


    def GetPlayerColorValue (self):
        return self.colorValue


    def SetAddress (self, address, ports):
        self.address = address
        self.ports = ports


    def GetAddress (self):
        return self.address


    def GetPort (self, i = 0):
        return self.ports [i]


    def SetPort (self, port, i = 0):
        self.ports [i] = port


    def Mood (self, mood = -1):
        if (mood > -1):
            return mood
        if (self.IsImmune ()):
            return 2
        if (self.hitPoints > 0):
            return self.hitPoints - 1
        return 0


    def GetTexture (self, mood = -1):
        return self.textures [self.Mood (mood)]


    def Render (self):
        if self.IsHidden ():
            return
        super ().EnableCamera ()
        if (self.scale < 1.0):
            glScalef (self.scale, 1.0, self.scale)
        if (self.shadow is not None):
            self.shadow.Render (-super ().GetPosition ().y)
        self.Wiggle ()
        if (self.halo is not None) and self.IsImmune ():
            self.halo.Render (self.size * 0.55)
        if (self.scale < 1.0):
            glScalef (1.0, self.scale, 1.0)
        super ().SetTexture (self.GetTexture ())
        self.mesh.PushColor (self.colorValue)
        super ().Render (False)
        self.mesh.PopColor ()
        if (self.outline is not None):
            self.outline.Render (self.camera.size, int (self.whiteForBlack))
        super ().DisableCamera ()


    def Wiggle (self):
        if (self.isViewer and not globals.gameData.wiggleViewer):
            return
        if ((self.id != 0) or not globals.gameData.wigglePlayers):
            return
        if self.wiggleTimer.HasPassed (10, True):
            self.wiggleAngle += 5
            self.wiggleAngle %= 360
        dy = np.sin (self.camera.Rad (self.wiggleAngle)) / 20
        glTranslatef (0, dy, 0)


    def BorderScale (self):
        if (self.outline is not None):
            return self.outline.Scale ()
        return 1.0


    def Delete (self):
        self.delete = True

    def AddScore (self, points):
        self.score += points


    def SetScore (self, score):
        self.score = score


    def AddKills (self):
        self.kills += 1


    def AddDeaths (self):
        self.deaths += 1

# =================================================================================================

class CViewer (CPlayer):
    def __init__ (self):
        super ().__init__ ("viewer", isViewer = True)
        self.outline = None
        self.fire = False
        self.fireTime = 0


    def ReadyToFire (self):
        # global gameData
        if (self.hitPoints == 0):
            return False
        if (self.fireTime == 0):
            return True
        return (globals.gameData.gameTime - self.fireTime > globals.gameData.fireDelay)


    def Fire (self):
        # global gameData, gameItems
        if self.ReadyToFire ():
            self.fireTime = globals.gameData.gameTime
            projectile = globals.actorHandler.CreateProjectile (self)
            if (projectile is not None):
                globals.networkHandler.BroadcastFire (projectile)
            return projectile


    def Render (self):
        pass


    # update viewer position and orientation
    # Scale with dt. Dt specifies the ratio of the actual frametime to the desired frametime
    # This compensates for high frametimes and assures that players on slow systems move as 
    # fast as players on fast systems
    def Update (self,  dt = -1, angles = None, offset = None):
        super ().Update (dt)
        if (self.IsAlive () or self.IsImmune ()):
            if (angles.Length () != 0):
                angles = angles.Clone ()
                angles.Scale (dt) # compensate for higher frame times
                self.camera.UpdateAngles (angles, True)

            if (offset.Length () != 0):
                offset = offset.Clone ()
                offset.Scale (dt)
                self.camera.UpdatePosition (self.camera.orientation.Unrotate (offset))

# =================================================================================================
22