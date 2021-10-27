
import globals

from sprite import *

# =================================================================================================
# support texture animations. Texture animations are played back from a set of texture and last
# a specified duration. The parent parameter allows to animate different types of surfaces (quads, sprites)

class CAnimatedTexture:
    def __init__ (self, duration, parent):
        super ().__init ()
        self.parent = parent
        self.textures = []
        self.duration = duration    # total duration of the animation in [ms]
        self.animationTime = 0      # duration of the current animation cycle
        self.frameTime = 0          # display duration of a single animation frame [ms]


    def Create (self, fileNames):
        self.textures = globals.textureHandler.CreateTextures (fileNames)
        if (self.textures is not None):
            self.frameTime = self.duration / len (self.textures)


    def Render (self):
        if (self.animationTime > self.duration):
            self.animationTime = 0
        frame = self.animationTime // self.frameTime
        self.parent.SetTexture (self.textures [frame])
        self.parent.Render ()

# =================================================================================================
