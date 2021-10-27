import pygame
import globals

from datetime import datetime

from OpenGL.GL import *
from gamedata import *
from gameitems import *
from actorhandler import *
from controlshandler import *
from physicshandler import *
from soundhandler import *
from networkhandler import *
from shaders import *
from texturehandler import *
from arghandler import *
from effecthandler import *
from scoreboard import *

# =================================================================================================
# Smiley Battle is a remake of Midimaze, which was probably the first first person multiplayer shooter 
# with fluid movement. It ran on a ring network of up two 16 Atari STs which were connected via their 
# midi out and midi in ports. Players roam a maze like map with their smiley avatars and throw
# color bags at other players. Receiving three hits within a sufficiently short time puts a player in
# "limbo" for a short time before he respawns at a random position. Scoring a hit yields one point,
# sending a player to limbo yields another point. Players start with three hitpoints. When hit,
# hitpoints regenerate after a short time unless the player is hit again.
#
# I wrote Smiley Battle as a small Python exercise to explore the language and learn its strengths
# and weaknesses. It uses very limited OpenGL features (the most advanced are VAOs, VBOs and two 
# very simple shaders to render cubemaps on spheres.) It still probably won't run on integrated
# graphics.
#
# Smiley battle requires numpy, pygame, pyopengl and pyopengl-accelerate. To build pyopengl-accelerate,
# you will unfortunately have to install MS Visual Studio and its C++ application building components.
# That's however all you need to do, it just needs to be present on your computer to install
# pyopengl-accelerate (which will be built from C source during installation.)
#
# Smiley battle has a simple UDP based networking interface to allow multiplayer matches over the
# internet. The networking model is peer to peer (see networkhandler.py for a brief documentation).
#
# Other "fancy" stuff is a DACS based shortest path search to quickly compute distances between
# movement nodes the application's map handler can create. Right now, the only purpose is to approximate
# the distance between the viewer and sound sources for proper sound volumes. Unfortunately, this
# is slow with Python even on a fast computer.
# 
# The only shape used in this game (besides some sprites) is a sphere, which is generated from a cube. 
# Smiley Battle has proper collision handling, so you can bump into walls and push other players around.
#
# Smiley Battle supports gamepads and keyboard. Control movement with W-A-S-D, arrow keys or numpad 
# 4-5-6-8 keys. Fire with spacebar. Alternatively, control movement with the gamepad sticks and 
# shoot with its fire buttons. 


# =================================================================================================
# Main application class
# Contains all application data, initialization and main loop

# =================================================================================================

class CApplication:
    def __init__ (self) -> None:
        dt = datetime.now ()
        np.random.seed (int (dt.microsecond))
        self.InitSound ()   # must be called before InitDisplay () to set proper init parameters for pygame.mixer, which is initialized in pygame.init ()
        self.InitDisplay ()
        globals.argHandler = CArgHandler ()
        globals.argHandler.LoadArgs ()
        globals.renderer = CRenderer (1920, 1080)
        globals.textureHandler = CTextureHandler ()
        globals.gameData = CGameData ()
        globals.actorHandler = CActorHandler ()
        globals.gameItems = CGameItems ()
        globals.soundHandler = CSoundHandler ()
        globals.controlsHandler = CControlsHandler ()
        globals.physicsHandler = CPhysicsHandler ()
        globals.networkHandler = CNetworkHandler ()
        globals.shaderHandler = CShaderHandler ()
        globals.scoreBoard = CScoreBoard ()
        globals.effectHandler = CEffectHandler ()
        globals.gameItems.Create ()
        globals.networkHandler.Create ()


    def InitDisplay (self) -> None:
        pygame.init ()
        if pygame.display.get_init ():
            pygame.display.set_caption ("[Python] Smiley Battle 1.0.5 by Dietfrid Mali")
            return True
        print ("Couldn't initialize display")
        exit ()


    def InitSound (self) -> None:
        pygame.mixer.pre_init (channels = 2)


    def Destroy (self) -> None:
        # global gameData, gameItems
        globals.textureHandler.Destroy ()
        globals.gameData.Destroy ()
        globals.gameItems.Destroy ()
        globals.actorHandler.Destroy ()
        globals.scoreBoard.Destroy ()
        globals.networkHandler.Destroy ()


    def Quit (self) -> None:
        self.run = False
        self.Destroy ()
        pygame.quit ()
        sys.exit (0)


    def RenderScene (self) -> None:
        globals.renderer.Start ()
        if globals.actorHandler.viewer.IsHidden ():
            globals.renderer.Stop ()
        else:
            globals.actorHandler.viewer.Wiggle ()
            globals.gameItems.map.Render ()
            for a in globals.actorHandler.actors:
                a.Render ()
            globals.renderer.Stop ()
            globals.gameItems.reticle.Render ()
        globals.effectHandler.Fade ()
        globals.scoreBoard.Render ()


    def HandleEvents (self, mode : int) -> None:
        for event in pygame.event.get ():
            if (event.type == pygame.QUIT):
                return False
            if globals.controlsHandler.HandleEvent (event, mode):
                return True
            elif (event.type == pygame.KEYDOWN):
                if (event.key == pygame.K_ESCAPE):
                    return False
        return True


    def Run (self) -> None:
        # global gameData, gameItems, physicsHandler, soundHandler

        frames = 0
        t0 = pygame.time.get_ticks ()
        frameTime = CTimer (globals.gameData.minFrameTime)

        while self.HandleEvents (0):
            if (globals.gameData.suspend):
                globals.networkHandler.Update ()
            else:
                frames += 1
                frameTime.Start ()  # start frame time measurement
                globals.gameData.gameTime = frameTime.time
                globals.physicsHandler.Update ()
                self.RenderScene ()
                globals.soundHandler.Update ()
                globals.networkHandler.Update ()
                globals.actorHandler.Cleanup ()
                pygame.display.flip ()
                frameTime.Delay ()  # wait until the minimum frame time has been reached or exceeded. For max fps comment this statement

        t = pygame.time.get_ticks () - t0
        fps = frames * 1000 / t
        print ("On your system, Smiley Battle runs at {0:1.2f} fps.".format (fps))
        globals.networkHandler.BroadcastLeave ()

# =================================================================================================

def Run () -> None:
    app = CApplication ()
    app.Run ()
    app.Quit ()

Run ()

# =================================================================================================
