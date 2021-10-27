
from quad import *

# =================================================================================================
# compute and render a quad that is always perpendicular to the viewer

class CSprite (CQuad):
    def __init__ (self, size = 1.0, texture = None):
        self.size = size
        self.distance = 0
        super ().__init__ ([], texture)


    # compute sprite at position position where viewer gives the viewer position and orientation
    def Update (self, position, viewer):
        self.position = position
        f = position - viewer.position
        self.distance = f.Length ()
        f.Normalize ()
        u = viewer.orientation.U ().Clone ()
        r = f.Perpendicular (CVector (0,0,0), f, u)
        u.Scale (0.5)
        r.Scale (0.5)
        # self.vertices = [position - r - u, position - r + u, position + r + u, position + r - u]
        self.vertices = [-r - u, -r + u, r + u, r - u]


    def Render (self):
        glPushMatrix ()
        self.texture.Enable ()
        glTranslatef (self.position.x, self.position.y, self.position.z)
        glScalef (self.size, self.size, self.size)
        glDisable (GL_CULL_FACE)
        # no vao here as vertices get recomputed frequently. just render by passing the vertices 
        # and texture coordinates to OpenGL one by one
        glBegin (GL_QUADS)
        for i in range (0,4):
            glTexCoord2f (self.texCoords [i].u, self.texCoords [i].v)
            glVertex3f (self.vertices [i].x, self.vertices [i].y, self.vertices [i].z)
        glEnd ()
        glEnable (GL_CULL_FACE)
        self.texture.Disable ()
        glPopMatrix ()

# =================================================================================================
