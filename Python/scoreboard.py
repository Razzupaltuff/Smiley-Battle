
import globals

from texturehandler import *
from arghandler import *
from quad import *

# =================================================================================================

class CScoreBoard:
    def __init__ (self):
        self.textures = None
        self.textureNames = ["smileyface-mask-black-sad.png", "smileyface-mask-black-neutral.png", "smileyface-mask-black-happy.png", "smiley-mask-black.png", 
                             "smileyface-mask-white-sad.png", "smileyface-mask-white-neutral.png", "smileyface-mask-white-happy.png", "smiley-mask-white.png", 
                             "smiley-strikeout-red.png", "smiley-strikeout-yellow.png"]
        self.textures = self.Create ()
        self.digitTextures = self.CreateDigitTextures ()
        self.statusBackground = self.CreateStatusBackground ()
        self.statusSmiley = self.CreateStatusSmiley ()
        self.digitQuads = self.CreateDigitQuads ()
        self.coloredScore = bool (int (globals.argHandler.StrVal ("coloredscore", 1, "0")))


    def Create (self):
        return globals.textureHandler.CreateTextures (self.textureNames)


    def Destroy (self):
        pass    # textures will be tracked and destroyed globally by the textureHandler
        # if (self.textures is not None):
        #     for t in self.textures:
        #         t.Destroy ()


    def CreateDigitTextures (self):
        digits = "0123456789"
        textures = []
        for i in range (10):
            texture = globals.textureHandler.GetTexture ()
            if not texture.CreateFromSurface (globals.renderer.scoreFont.render (digits [i], True, (224, 224, 224))):
                return None
            texture.Create ()
            texture.Deploy ()
            textures.append (texture)
        return textures


    def CreateStatusBackground (self):
        border = 0.1
        q = CQuad ([CVector (border, border, 0.0), CVector (border, 1.0 - border, 0.0), CVector (1.0 - border, 1.0 - border, 0.0), CVector (1.0 - border, border, 0.0)])
        q.Create ()
        return q


    def CreateStatusSmiley (self):
        border = 0.2
        q = CQuad ([CVector (border, border, 0.0), CVector (border, 1.0 - border, 0.0), CVector (1.0 - border, 1.0 - border, 0.0), CVector (1.0 - border, border, 0.0)])
        q.Create ()
        return q


    def CreateDigitQuad (self, l, w, h):
        border = 0 #w * 0.1
        q = CQuad ([CVector (l + border, h + border, 0.0), CVector (l + border, 1.0 - h - border, 0.0), CVector (l + w - border, 1.0 - h - border, 0.0), CVector (l + w - border, h + border, 0.0)])
        q.Create ()
        return q


    def CreateDigitQuads (self):
        width, height = globals.renderer.SetViewport ("score", 1)
        width //= 5     # 4 characters + one space
        t = self.digitTextures [0]
        l = 0
        digitQuads = []
        for i in range (4):
            cw = t.GetWidth ()
            ch = t.GetHeight ()
            ar = cw / ch
            w = ar * height
            if (w <= width):
                w = 0.225 * w / width
                h = 1.0
            else:
                w = 0.225
                h = width / ar
                if (h > height):
                    w *= height / h
                h = 1.0
            digitQuads.append (self.CreateDigitQuad (l, w, (1.0 - h) / 2))
            l += w
        return digitQuads


    # The status icon is painted by first drawing a rectangle in the corresponding player's color,
    # then masking the corners to create a colored circle and finally drawing a smiley face on top of it.
    # If the player is currently dead, a strikeout will be painted over it
    # Black smiley get a circular mask with a white border to make them visible against the black status area background
    def RenderStatus (self, player, position):
        globals.renderer.SetViewport ("status", position)
        colorValue, color, whiteForBlack = globals.gameData.GetPlayerColorValue (player)
        if (colorValue is None):
            return
        # print ("{} = {}, {}, {}".format (player.color, colorValue.x, colorValue.y, colorValue.z))
        glDepthFunc (GL_ALWAYS)
        self.statusBackground.Fill (colorValue)
        if (color == "black"):
            textureOffset = 4
        else:
            textureOffset = 0
        self.statusBackground.SetTexture (self.textures [textureOffset + 3])
        self.statusBackground.Render ()
        self.statusSmiley.SetTexture (self.textures [textureOffset + player.Mood ()])
        self.statusSmiley.Render ()
        if player.IsHidden ():
            if (color == "red") or (color == "darkred"):
                self.statusSmiley.SetTexture (self.textures [9])
            else:
                self.statusSmiley.SetTexture (self.textures [8])
            self.statusSmiley.Render ()
        glDepthFunc (GL_LESS)



    def Pot10 (self, i):
        b = 1
        while (b <= i):
            b *= 10
        return b // 10


    def RenderScore (self, position, player, score):
        if (position > 0):
            globals.renderer.SetViewport ("score", position)
        if not self.coloredScore:
            colorValue = CVector (1,1,1)
        else:
            colorValue, color, whiteForBlack = globals.gameData.GetPlayerColorValue (player, True)
            if (colorValue is None):
                colorValue = CVector (1,1,1)
        # print ("{} = {}, {}, {}".format (player.color, colorValue.x, colorValue.y, colorValue.z))
        glDepthFunc (GL_ALWAYS)
        b = self.Pot10 (score)
        if (b < 1000):
            b = 1000
        i = 0
        while (b > 0):
            d = score // b
            self.digitQuads [i].SetTexture (self.digitTextures [d])
            self.digitQuads [i].SetColor (colorValue)
            self.digitQuads [i].Render ()
            score %= b
            b //= 10
            i += 1
        glDepthFunc (GL_LESS)


    def RenderViewerStatus (self):
        self.RenderStatus (globals.actorHandler.viewer, 0)


    def RenderPlayerStatus (self):
        position = 0
        for a in globals.actorHandler.actors:
            if (a.id != 0) or a.isViewer:
                continue
            position += 1
            self.RenderStatus (a, position)


    def RenderViewerScores (self):
        globals.renderer.SetViewport ("score", 0)
        self.RenderScore (-1, globals.actorHandler.viewer, globals.actorHandler.viewer.score)
        globals.renderer.SetViewport ("kills", 0)
        self.RenderScore (-1, globals.actorHandler.viewer, globals.actorHandler.viewer.kills)
        globals.renderer.SetViewport ("deaths", 0)
        self.RenderScore (-1, globals.actorHandler.viewer, globals.actorHandler.viewer.deaths)


    def RenderPlayerScores (self):
        position = 0
        for a in globals.actorHandler.actors:
            if (a.id != 0) or a.isViewer:
                continue
            position += 1
            self.RenderScore (position, a, a.score)


    def Render (self):
        self.RenderViewerStatus ()
        self.RenderPlayerStatus ()
        self.RenderViewerScores ()
        self.RenderPlayerScores ()
        globals.renderer.SetViewport ()
 
# =================================================================================================
