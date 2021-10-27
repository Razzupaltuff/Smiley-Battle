import pygame
import globals

from vector import *
from cubemap import *
from arghandler import *

# =================================================================================================

class CGameData:
    def __init__ (self):
    # create all "global" game data
        self.resourceFolder = "resources\\"
        self.textureFolder = self.resourceFolder + "textures\\"
        self.soundFolder = self.resourceFolder + "sounds\\"
        self.mapFolder = "maps\\"
        self.playerColors = ["white", "black", "yellow", "gold", "orange", "red", "darkred", "pink", "purple", "lightgreen", "darkgreen", "lightblue", "blue", "darkblue", "lightgray", "darkgray"] # "dead-black", "dead-white"]
        self.colorValues = {"white"      : CVector (255, 255, 255).Scale (1.0 / 255.0),
                            "black"      : CVector (  0,   0,   0), 
                            "yellow"     : CVector (255, 255,   0).Scale (1.0 / 255.0), 
                            "gold"       : CVector (255, 217,   0).Scale (1.0 / 255.0),
                            "orange"     : CVector (255, 140,   0).Scale (1.0 / 255.0),
                            "red"        : CVector (217,  26,   0).Scale (1.0 / 255.0),
                            "darkred"    : CVector (128,  26,   0).Scale (1.0 / 255.0),
                            "pink"       : CVector (255, 127, 255).Scale (1.0 / 255.0),
                            "purple"     : CVector (153,   0, 255).Scale (1.0 / 255.0),
                            "lightgreen" : CVector (102, 229,   0).Scale (1.0 / 255.0),
                            "darkgreen"  : CVector (  0, 153,  38).Scale (1.0 / 255.0),
                            "lightblue"  : CVector (  0, 204, 255).Scale (1.0 / 255.0),
                            "blue"       : CVector (  0, 128, 255).Scale (1.0 / 255.0),
                            "darkblue"   : CVector (  0,  26, 255).Scale (1.0 / 255.0),
                            "lightgray"  : CVector (179, 176, 176).Scale (1.0 / 255.0),
                            "darkgray"   : CVector (112, 112, 112).Scale (1.0 / 255.0)
                            } # "dead-black", "dead-white"]

        self.availableColors = []
        self.colorIndices = {}
        for i in range (len (self.playerColors)):
            self.availableColors.append (i)
            self.colorIndices [self.playerColors [i]] = i
        self.playerMoods = ["-sad", "-neutral", "-happy"]
        self.CreatePlayerTextures ()

        self.fireDelay = globals.argHandler.IntVal ("firedelay", 0, 250)                  # limit fire rate to one short per 500 ms (2 shots/s)
        self.healDelay = globals.argHandler.IntVal ("healdelay", 0, 5000)
        self.respawnDelay = globals.argHandler.IntVal ("respawndelay", 0, 5000)           # time [ms] between disappearing and reappearing after death
        self.immunityDuration = globals.argHandler.IntVal ("immunityduration", 0, 3000)   # duration [ms] of immunity after having respawned to allow for reorientation
        self.projectileSpeed = globals.argHandler.FloatVal ("projectilespeed", 0, 0.2)
        self.projectileSize = min (1.0, globals.argHandler.FloatVal ("projectilesize", 0, 0.3))
        self.wigglePlayers = globals.argHandler.BoolVal ("wiggleplayers", 0, False)
        self.wiggleViewer = globals.argHandler.BoolVal ("wiggleviewer", 0, False)
        self.pointsForKill = globals.argHandler.IntVal ("pointsforkill", 0, 1)
        self.frozenTimeout = 5000

        self.gameTime = pygame.time.get_ticks ()
        self.frameCap = 240 # fps
        self.minFrameTime = 1000 // self.frameCap
        self.isNetGame = False
        self.suspend = False
        self.run = True


    def Destroy (self):
        pass    # textures will be tracked and destroyed in textureHandler
        # for color in self.playerColors:
        #     for mood in self.playerMoods:
        #         self.textures [color + mood].Destroy ()
    

    def CreatePlayerTextures (self):
        self.textures = {}
        for i in range (2):
            color = self.playerColors [i]
        # for color in self.playerColors:
            for mood in self.playerMoods:
                skinName = self.textureFolder + "smiley-" + color + ".png"
                faceName = self.textureFolder + "smileyface-" + color + mood + ".png"
                texture = globals.textureHandler.GetCubemap ()
                texture.CreateFromFile ([skinName, "", "", "", "", faceName], False)
                # create test texturing with different colors on each side of a sphere
                # texture.CreateFromFile ([self.textureFolder + "smiley-white.png", 
                #                          self.textureFolder + "smiley-lightgreen.png", 
                #                          self.textureFolder + "smiley-red.png", 
                #                          self.textureFolder + "smiley-blue.png", 
                #                          self.textureFolder + "smiley-gold.png", 
                #                          self.textureFolder + "smileyface-black-happy.png"], 
                #                         False)
                self.textures [color + mood] = texture


    def GetColor (self, colorIndex):
        if (colorIndex >= 0):
            return self.playerColors [colorIndex]
        return None


    def GetColorValue (self, color):
        try:
            colorValue = globals.gameData.colorValues [color]
        except KeyError:
            return CVector (1,1,1)
        return colorValue


    def GetPlayerColorValue (self, player, whiteForBlack = False):
        color = globals.gameData.GetColor (player.colorIndex)
        if (whiteForBlack):
            if (color == "black"):
                color = "white"
            else:
                whiteForBlack = False
        try:
            colorValue = globals.gameData.colorValues [color]
        except KeyError:
            return None, None, whiteForBlack
        return colorValue, color, whiteForBlack


    # randomly select a color index from the available color indices
    def GetColorIndex (self):
        # return "white"
        if (len (self.availableColors) == 0):
             return None
        return self.availableColors.pop (np.random.randint (len (self.availableColors)))


    def RemoveColorIndex (self, colorIndex):
        if (colorIndex >= 0):
            for i in range (len (self.availableColors)):
                if (self.availableColors [i] == colorIndex):
                    self.availableColors.pop (i)
                    return


    def ReturnColorIndex (self, colorIndex):
        if (colorIndex >= 0):
            self.availableColors.append (colorIndex)


    def ColorIsAvailable (self, colorIndex):
        for i in self.availableColors:
            if (colorIndex == i):
                return True
        return False


    def ReplaceColorIndex (self, oldIndex, newIndex):
        if (newIndex != oldIndex):
            self.ReturnColorIndex (oldIndex)
            self.RemoveColorIndex (newIndex)
        return newIndex 

# =================================================================================================

