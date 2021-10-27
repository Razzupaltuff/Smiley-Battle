import numpy as np

from OpenGL.GL import *
from OpenGL.GLU import *
from vector import *
from matrix import *

import copy

# =================================================================================================
# Position and orientation (scaling, translation and rotation) handling
# Can have parent cameras. Compute world position depending on parent positions if a parent other
# than the viewer is present. This allows for child actors that move with their parent actors.

class CCamera:
    def __init__ (self, name = "", isViewer = False):
        self.orientation = CMatrix ()
        self.positions = [None, None]
        self.angles = CVector (0,0,0)
        self.size = 1.0
        self.name = name
        self.parent = None
        self.isViewer = isViewer


    def Clone (self):
        return copy.deepcopy (self)


    def Move (self, v):
        glTranslatef (v.x, v.y, v.z)


    def Rotate (self, m):
        glMultMatrixf (m.AsArray ())


    def Scale (self):
        glScalef (self.size, self.size, self.size)


    def Setup (self, position, orientation):
        self.positions [0] = position
        self.orientation = orientation


    def SetParent (self, parent):
        self.parent = parent


    def ParentPosition (self, camera):
        if (camera.parent is None):
            return CVector (0,0,0)
        return camera.GetPosition () + self.ParentPosition (camera.parent)


    def WorldPosition (self):
        if (self.parent is None):
            return self.GetPosition ()
        if (self.parent.isViewer):
            return self.GetPosition ()
        return self.GetPosition () + self.ParentPosition (self.parent)


    def RelativePosition (self):
        return self.GetPosition () + self.ParentPosition (self.parent)


    def Enable (self):
        glMatrixMode (GL_MODELVIEW)
        glPushMatrix ()
        if (self.parent is None):
            self.Rotate (self.orientation)
            self.Move (-self.GetPosition ())
        else:   
            self.Move (self.GetPosition ()) # - self.parent.position)
            self.Rotate (self.orientation)
        self.Scale ()        


    def Disable (self):
        glMatrixMode (GL_MODELVIEW)
        glPopMatrix ()


    def GetOrientation (self):
        return self.angles


    def SetOrientation (self, angles):
        self.angles = angles
        self.ClampAngles ()
        self.UpdateOrientation ()


    def GetPosition (self, i = 0):
        return self.positions [i]


    def SetPosition (self, position, i = 0):
        if (i == 0):
            self.BumpPosition ()
        self.positions [i] = position


    def UpdatePosition (self, offset):
        self.BumpPosition ()
        self.positions [0] += offset


    def BumpPosition (self):
        if (self.positions [0] is not None):
            self.positions [1] = self.positions [0].Clone ()


    def R (self):
        return self.orientation.R ()


    def U (self):
        return self.orientation.U ()


    def F (self):
        return self.orientation.F ()


    def Rad (self, a):
        return a / 180.0 * np.pi


    def ClampAngles (self):
        if (self.angles.x < -180):
            self.angles.x += 360
        elif (self.angles.x > 180):
            self.angles.x -= 360
        if (self.angles.y < -180):
            self.angles.y += 360
        elif (self.angles.y > 180):
            self.angles.y -= 360
        if (self.angles.z < -180):
            self.angles.z += 360
        elif (self.angles.z > 180):
            self.angles.z -= 360


    # create an orientation (rotation) matrix from heading angles
    def UpdateAngles (self, angles, reverse = False):
        if (angles is not None):
            # m = CMatrix ()
            self.angles += angles
            self.ClampAngles ()
            self.UpdateOrientation ()


    def UpdateOrientation (self):
        radX = self.Rad (self.angles.x)
        radY = self.Rad (self.angles.y)
        radZ = self.Rad (self.angles.z)
        self.orientation.CreateFromAngles (np.sin (radX), np.cos (radX), np.sin (radY), np.cos (radY), np.sin (radZ), np.cos (radZ))
        # if (reverse):
        #     self.orientation = m * self.orientation
        # else:
        #     self.orientation *= m 


    def SetSize (self, size):
        self.size = size

# =================================================================================================
