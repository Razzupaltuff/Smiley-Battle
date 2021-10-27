import numpy as np
from OpenGL import *
from OpenGL.GL import *
from OpenGL.GL import shaders

# =================================================================================================
# Some basic shader handling: Compiling, enabling, setting shader variables

class CShader:
    def __init__ (self):
        self.handle = 0


    def Create (self, vsCode, fsCode):
        vs = shaders.compileShader (vsCode, GL_VERTEX_SHADER)
        fs = shaders.compileShader (fsCode, GL_FRAGMENT_SHADER)
        self.handle = glCreateProgram ()
        glAttachShader (self.handle, vs)
        glAttachShader (self.handle, fs)
        glLinkProgram (self.handle)
        #self.handle = shaders.compileProgram (vs, fs)
        return self


    def Destroy (self):
        if (self.handle > 0):
            glDeleteProgram (self.handle)
            self.handle = 0


    def SetUniformMatrix (self, name, data):
        loc = glGetUniformLocation (self.handle, name)
        glUniformMatrix4fv (loc, 1, False, bytearray (data))


    def SetUniformVector (self, name, data):
        loc = glGetUniformLocation (self.handle, name)
        # glUniform4fv (loc, 1, data)
        glUniform4f (loc, data.x, data.y, data.z, data.w)


    def SetUniformFloat (self, name, data):
        loc = glGetUniformLocation (self.handle, name)
        # glUniform4fv (loc, 1, data)
        glUniform1f (loc, data)


    def GetFloatData (self, name, size):
        m  = np.empty (size, np.float32)
        glGetFloatv (name, m)
        return m


    def Enable (self):
        glUseProgram (self.handle)


    def Disable (self):
        glUseProgram (0)

# =================================================================================================

class CShaderHandler:
    def __init__ (self):
        self.shaders = []
        self.CreateShaders ()


    def SelectShader (self, useShader, texture):
        if (not useShader):
            return -1
        if (texture is None):
            shaderId = 2
        # select shader depending on texture type
        elif (texture.type == GL_TEXTURE_CUBE_MAP):
            shaderId = 0
        elif (texture.type == GL_TEXTURE_2D):
            shaderId = 1
        self.SetupShader (shaderId)
        return shaderId


    def SetupShader (self, shaderId):
        shader = self.shaders [shaderId]
        shader.Enable ()
        shader.SetUniformMatrix ("mModelView", shader.GetFloatData (GL_MODELVIEW_MATRIX, 16))
        shader.SetUniformMatrix ("mProjection", shader.GetFloatData (GL_PROJECTION_MATRIX, 16))
        return shader


    def CreateShaders (self):
        self.shaders.append (self.CreateCubemapShader ())
        self.shaders.append (self.CreateTextureShader ())
        self.shaders.append (self.CreateColorShader ())


    def CreateCubemapShader (self):
        shader = CShader ()
        return shader.Create (
            """
            #version 330
            layout(location = 0) in vec3 position;
            layout(location = 1) in vec3 normal;
            //layout(location = 2) in vec4 color;
            //out vec4 vertexColor;
            out vec3 texCoords;
            out vec3 viewNormal;
            uniform mat4 mModelView;
            uniform mat4 mProjection;
            uniform vec4 fillColor;
            out vec4 vertexColor;
            void main() {
                gl_Position = mProjection * mModelView * vec4 (position, 1.0);
                viewNormal = normalize (mat3 (mModelView) * normal);
                //vertexColor = vec4 (1.0, 1.0, 1.0, 1.0); //color;
                texCoords = position;
                vertexColor = fillColor;
            } 
            """,
            """ 
            #version 330
            in vec3 texCoords;
            in vec3 viewNormal;
            in vec4 vertexColor;
            out vec4 fragColor;
            uniform samplerCube cubeMap;
            uniform float minBrightness;
            void main() {
                float brightness = (dot (vec3 (0,0,1), viewNormal) + 1.0) * 0.5;
                brightness *= brightness;
                vec4 texColor = texture (cubeMap, texCoords); 
                fragColor = vec4 (texColor.rgb * vertexColor.rgb * max (brightness, minBrightness), 1.0);
                // fragColor = vec4 (vec3 (texColor.rgb * texColor.a + vertexColor.rgb * (1.0 - texColor.a)).rgb * brightness, 1.0);
            } 
            """
            )


    def CreateTextureShader (self):
        shader = CShader ()
        return shader.Create (
            """
            #version 330
            layout(location = 0) in vec3 position;
            layout(location = 1) in vec2 texCoord;
            out vec2 texCoords;
            uniform mat4 mModelView;
            uniform mat4 mProjection;
            uniform vec4 fillColor;
            out vec4 vertexColor;
            void main() {
                gl_Position = mProjection * mModelView * vec4 (position, 1.0);
                texCoords = texCoord;
                vertexColor = fillColor;
            } 
            """,
            """ 
            #version 330
            in vec2 texCoords;
            out vec4 fragColor;
            uniform sampler2D image;
            in vec4 vertexColor;
            void main() {
                vec4 texColor = texture (image, texCoords); 
                if (texColor.a == 0) discard;
                else fragColor = vec4 (texColor.rgb * vertexColor.rgb, texColor.a);
            } 
            """
            )


    def CreateColorShader (self):
        shader = CShader ()
        return shader.Create (
            """
            #version 330
            layout(location = 0) in vec3 position;
            uniform mat4 mModelView;
            uniform mat4 mProjection;
            uniform vec4 fillColor;
            out vec4 vertexColor;
            void main() {
                gl_Position = mProjection * mModelView * vec4 (position, 1.0);
                vertexColor = fillColor;
            } 
            """,
            """ 
            #version 330
            in vec4 vertexColor;
            out vec4 fragColor;
            void main() {
                fragColor = vec4 (vertexColor.rgb, 1.0);
            } 
            """
            )

# =================================================================================================
