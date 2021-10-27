import pygame
import globals

from OpenGL.GL import *
from camera import *
from quad import *

# =================================================================================================
# basic renderer class. Initializes display and OpenGL and sets up projections and view matrix

class CRenderer:
    def __init__ (self, width, height, viewer = None):
        self.viewer = viewer
        displayInfo = pygame.display.Info ()
        self.maxWidth = displayInfo.current_w
        self.maxHeight = displayInfo.current_h
        width = globals.argHandler.IntVal ("display", 0, width)
        height = globals.argHandler.IntVal ("display", 1, height)
        if (width * height > 0):
            fullscreen = False
        else:
            width = self.maxWidth
            height = self.maxHeight
            fullscreen = True
        self.screen = self.SetupDisplay (width, height, fullscreen)
        pygame.font.init ()
        self.scoreFont = pygame.font.SysFont ('couriernew', int ((self.statusHeight // 3) * 0.85), True)
        self.SetupOpenGL ()
        self.ResetProjection ()
        

    def SetupDisplay (self, width, height, fullscreen):
        self.width = width
        self.height = height
        # The status bar contains a full size smiley for the local player at the left border and up to two rows
        # of eight half size smileys for the remote players. The score width equals the width W of a full size smiley.
        # So we need horizontal room of 2 x W + 8 x W/2 + 8 x W = 4 x W/2 + 8 x W/2 + 16 x W/2 = 28 * W/2 half size smileys
        # in the status bar.
        # This means that smiley width = 2 x window width / 28 = window width / 14. This is also the status bar height,
        # since smileys cover a square area.
        self.scoreWidth = self.statusHeight = min (160, self.width // 14)
        screenType = pygame.DOUBLEBUF | pygame.OPENGL
        if fullscreen or (globals.argHandler.IntVal ("fullscreen", 0, 0) == 1):
            screenType |= pygame.FULLSCREEN
            self.height -= self.statusHeight
        elif (self.height + self.statusHeight > self.maxHeight):
            self.height = self.maxHeight - self.statusHeight
        self.aspectRatio = self.width / self.height
        try:
            screen = pygame.display.set_mode ((self.width, self.height + self.statusHeight), screenType)
        except pygame.error:
            print ("Couldn't set screen mode ({0})".format (pygame.get_error ()))
            exit ()
        return screen


# set basic render parameters
    def SetupOpenGL (self):
        glColorMask (1, 1, 1, 1)
        glDepthMask (1)
        glEnable (GL_DEPTH_TEST)
        glDepthFunc (GL_LESS)
        glEnable (GL_BLEND)
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        glEnable (GL_ALPHA_TEST)
        glEnable (GL_CULL_FACE)
        glCullFace (GL_BACK)
        glEnable (GL_MULTISAMPLE)
        glFrontFace (GL_CW)
        glDisable (GL_POLYGON_OFFSET_FILL)
        glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST)
        clampFOV = lambda fov : min (max (fov, 90), 120)
        self.fov = clampFOV (globals.argHandler.FloatVal ("fov", 0, 90)) / 2
        self.zNear = 0.01
        self.zFar = 1000.0
        self.zoom = 1.0 #0.5625 # 0x9000
        self.ComputeProjection ()
        self.SetViewport ()


    # self.projection will receive the calculated perspective self.projection.
    # You would have to upload to your shader
    # or use glLoadMatrixf if you aren't using shaders.
    def ComputeProjection (self):
        yMax = self.zNear * np.tan (self.fov * np.pi / 360.0)
        xMax = yMax * self.aspectRatio
        self.ComputeFrustum (-xMax, xMax, -yMax, yMax)


    def ComputeFrustum (self, left, right,  bottom, top):
        nearPlane = 2.0 * self.zNear
        depth = self.zFar - self.zNear
        width = right - left
        height = top - bottom
        self.projection = CMatrix ()
        self.projection.data = np.array ([
            CVector (nearPlane / width, 0.0, (left + right) / width, 0.0),
            CVector (0.0, nearPlane / height, (top + bottom) / height, 0.0),
            CVector (0.0, 0.0, -(self.zFar + self.zNear) / depth, -1.0),
            CVector (0.0, 0.0, (-nearPlane * self.zFar) / depth, 0.0)
        ])


    # set up perspective projection (3D -> 2D)
    def SetupProjection (self):
        self.SetViewport ("game")
        glMatrixMode (GL_PROJECTION)
        # gluPerspective (self.fov * self.zoom, self.aspectRatio, self.zNear, self.zFar)
        glLoadMatrixf (self.projection.AsArray ())


    # reset projection to rectangular 2D projection
    def ResetProjection (self):
        glMatrixMode (GL_PROJECTION)
        glLoadIdentity ()
        glOrtho (0.0, 1.0, 0.0, 1.0, -1.0, 1.0)
        glMatrixMode (GL_MODELVIEW)
        glLoadIdentity ()


    def SetViewer (self, viewer):
        self.viewer = viewer


    # setup OpenGL for rendering
    def Start (self):
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        self.SetupProjection ()
        glMatrixMode (GL_MODELVIEW)
        glLoadIdentity ()
        glColor (1,1,1)
        self.viewer.camera.Enable ()


    # reset OpenGL projections
    def Stop (self):
        self.viewer.camera.Disable ()
        self.ResetProjection ()


    # Set proper view port depending on what needs to be rendered
    def SetViewport (self, area = None, position = 0):
        if (area == "game"):
            width = self.width
            height = self.height
            glViewport (0, 0, self.width, self.height)
        elif (area == "status"):
            if (position == 0):
                width = self.statusHeight
                height = self.statusHeight
                glViewport (0, self.height, self.statusHeight, self.statusHeight)
            else:
                # split status area right of viewer's status icon in two rows and eight columns
                # row = ((position - 1) // 8)
                l = self.statusHeight + self.scoreWidth  # horizontal space of the viewer's status icon
                h = self.statusHeight // 2          # half of the vertical space, for two rows
                w = (self.width - l) // 8           # total horizontal space for a non viewer's status (icon + score)
                position -= 1
                width = h
                height = h
                glViewport (l + (position % 8) * w, self.height + (1 - (position // 8)) * h, width, height)
        elif (area == "score"):
            if (position == 0):
                width = self.scoreWidth
                height = int (self.statusHeight / 3)
                glViewport (self.statusHeight, self.height + 2 * height, width, height)
            else:
                # split status area right of viewer's status icon in two rows and eight columns
                # row = ((position - 1) // 8)
                l = self.statusHeight + self.scoreWidth  # horizontal space of the viewer's status icon and score area
                h = self.statusHeight // 2
                height = self.statusHeight // 3     # a third of the total status height, like for viewer scores
                w = (self.width - l) // 8           # total horizontal space for a non viewer's status (icon + score)
                position -= 1
                width = self.scoreWidth
                glViewport (l + (position % 8) * w + h, self.height + (1 - (position // 8)) * h + self.statusHeight // 12, width, height)
        elif (area == "kills"):
            width = self.scoreWidth
            height = int (self.statusHeight / 3)
            glViewport (self.statusHeight, self.height + height, width, height)
        elif (area == "deaths"):
            width = self.scoreWidth
            height = int (self.statusHeight / 3)
            glViewport (self.statusHeight, self.height, width, height)
        elif (area == "scoreboard"):
            width = self.width
            height = self.statusHeight
            glViewport (0, self.height, self.width, self.statusHeight)
        else:
            width = self.width
            height = self.height + self.statusHeight
            glViewport (0, 0, self.width, height)
        self.ResetProjection ()
        return width, height


# =================================================================================================
