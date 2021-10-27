#include "shaders.h"
#include "renderer.h"

// =================================================================================================
// Some basic shader handling: Compiling, enabling, setting shader variables


CString CShader::GetInfoLog (GLuint handle, bool isProgram)
{
    int logLength = 0;
    int charsWritten = 0;

    if (isProgram)
        glGetProgramiv (handle, GL_INFO_LOG_LENGTH, &logLength);
    else
        glGetShaderiv (handle, GL_INFO_LOG_LENGTH, &logLength);

    if (!logLength)
        return CString ("no log found.\n");
    CString infoLog;
    infoLog.Create (logLength);
    if (isProgram)
        glGetProgramInfoLog (handle, logLength, &charsWritten, infoLog.Buffer ());
    else
        glGetShaderInfoLog (handle, logLength, &charsWritten, infoLog.Buffer ());
    fprintf (stderr, "Shader info: %s\n\n", infoLog.Buffer ());
    return infoLog;
    }


GLuint CShader::Compile(const char* code, GLuint type) {
    GLuint handle = glCreateShader(type);
    glShaderSource(handle, 1, (GLchar**)&code, nullptr);
    glCompileShader(handle);
    GLint isCompiled;
    glGetShaderiv (handle, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_TRUE)
        return handle;
    CString shaderLog = GetInfoLog (handle);
    char buffer [10000];
    GLsizei bufLen;
    glGetShaderSource (handle, sizeof (buffer), &bufLen, buffer);
    fprintf (stderr, "\nshader source:\n%s\n\n", buffer);
    glDeleteShader(handle);
    return 0;
}


GLuint CShader::Link(GLuint vsHandle, GLuint fsHandle) {
    if (!vsHandle || !fsHandle)
        return 0;
    GLuint handle = glCreateProgram();
    if (!handle)
        return 0;
    glAttachShader(handle, vsHandle);
    glAttachShader(handle, fsHandle);
    glLinkProgram(handle);
    GLint isLinked = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_TRUE) {
        glDetachShader(handle, vsHandle);
        glDetachShader(handle, fsHandle);
        return handle;
    }
    CString shaderLog = GetInfoLog (handle, true);
    char buffer [10000];
    GLsizei bufLen;
    glGetShaderSource (vsHandle, sizeof (buffer), &bufLen, buffer);
    fprintf (stderr, "\nVertex shader:\n%s\n\n", buffer);
    glGetShaderSource (fsHandle, sizeof (buffer), &bufLen, buffer);
    fprintf (stderr, "\nFragment shader:\n%s\n\n", buffer);
    glDeleteShader(vsHandle);
    glDeleteShader(fsHandle);
    glDeleteProgram(handle);
    return 0;
}


CShader& CShader::Create(const char * vsCode, const char * fsCode) {
    m_handle = Link(Compile(vsCode, GL_VERTEX_SHADER), Compile(fsCode, GL_FRAGMENT_SHADER));
    return *this;
}


void CShader::Destroy (void) {
    if (m_handle > 0) {
        glDeleteProgram(m_handle);
        m_handle = 0;
    }
}


void CShader::SetUniformMatrix(const char* name, CArray<GLfloat>& data) {
    GLint loc = glGetUniformLocation(m_handle, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, data.Buffer ());
}


void CShader::SetUniformVector(const char* name, CVector data) {
    GLint loc = glGetUniformLocation(m_handle, name);
    // glUniform4fv (loc, 1, data);
    glUniform4f(loc, data.X(), data.Y(), data.Z(), data.W());
}


void CShader::SetUniformFloat(const char* name, float data) {
    GLint loc = glGetUniformLocation(m_handle, name);
    // glUniform4fv (loc, 1, data);
    glUniform1f(loc, GLfloat(data));
}


CArray<float>& CShader::GetFloatData(GLenum id, size_t size) {
    if (m_glData.Length () < size)
        m_glData.Create(size);
    glGetFloatv(id, m_glData.Buffer ());
    return m_glData;
}

// =================================================================================================

int CShaderHandler::SelectShader (bool useShader, CTexture* texture) {
    if (not useShader)
        return -1;
    int shaderId = -1;
    if (!texture)
        shaderId = 2;
    // select shader depending on texture type
    else if (texture->Type() == GL_TEXTURE_CUBE_MAP)
        shaderId = 0;
    else if (texture->Type() == GL_TEXTURE_2D)
        shaderId = 1;
    else
        return -1;
    SetupShader(shaderId);
    return shaderId;
}


void CShaderHandler::SetupShader(int shaderId) {
    CShader& shader = m_shaders[shaderId];
    shader.Enable();
    shader.SetUniformMatrix("mModelView", shader.GetFloatData(GL_MODELVIEW_MATRIX, 16));
    shader.SetUniformMatrix("mProjection", shader.GetFloatData (GL_PROJECTION_MATRIX, 16));
}


void CShaderHandler::CreateShaders(void) {
    CShader* shader = m_shaders.Add (-1);
    CreateCubemapShader (shader);
    shader = m_shaders.Add (-1);
    CreateTextureShader(shader);
    shader = m_shaders.Add (-1);
    CreateColorShader(shader);
}


void CShaderHandler::CreateCubemapShader (CShader* shader) {
    const char* vs = 
        "#version 330\n"
        "layout(location = 0) in vec3 position;\n"
        "layout(location = 1) in vec3 normal;\n"
        "uniform mat4 mModelView;\n"
        "uniform mat4 mProjection;\n"
        "uniform vec4 fillColor;\n"
        "out vec3 viewNormal;\n"
        "out vec3 texCoords;\n"
        "out vec4 vertexColor;\n"
        "void main() {\n"
        "    gl_Position = mProjection * mModelView * vec4 (position, 1.0);\n"
        "    viewNormal = normalize (mat3 (mModelView) * normal);\n"
        "    texCoords = position;\n"
        "    vertexColor = fillColor;\n"
        "}\n";
    const char* fs =
        "#version 330\n"
        "uniform samplerCube cubeMap;\n"
        "uniform float minBrightness;\n"
        "in vec3 texCoords;\n"
        "in vec3 viewNormal;\n"
        "in vec4 vertexColor;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    float brightness = (dot(vec3(0, 0, 1), viewNormal) + 1.0) * 0.5;\n"
        "    brightness *= brightness;\n"
        "    vec4 texColor = texture(cubeMap, texCoords);\n"
        "    fragColor = vec4(texColor.rgb * vertexColor.rgb * max(brightness, minBrightness), 1.0);\n"
        "}\n";
    shader->Create (vs, fs);
}


void CShaderHandler::CreateTextureShader(CShader* shader) {
    const char* vs = 
        "#version 330\n"
        "layout(location = 0) in vec3 position;\n"
        "layout(location = 1) in vec2 texCoord;\n"
        "uniform mat4 mModelView;\n"
        "uniform mat4 mProjection;\n"
        "uniform vec4 fillColor;\n"
        "out vec2 texCoords;\n"
        "out vec4 vertexColor;\n"
        "void main() {\n"
        "    gl_Position = mProjection * mModelView * vec4 (position, 1.0);\n"
        "    texCoords = texCoord;\n"
        "    vertexColor = fillColor;\n"
        "}\n";
    const char* fs = 
        "#version 330\n"
        "uniform sampler2D image;\n"
        "in vec2 texCoords;\n"
        "in vec4 vertexColor;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    vec4 texColor = texture (image, texCoords);\n"
        "    if (texColor.a == 0) discard;\n"
        "    else fragColor = vec4 (texColor.rgb * vertexColor.rgb, texColor.a);\n"
        "}\n";
    shader->Create (vs, fs);
}


void CShaderHandler::CreateColorShader(CShader* shader) {
    const char* vs = 
        "#version 330\n"
        "layout(location = 0) in vec3 position;\n"
        "uniform mat4 mModelView;\n"
        "uniform mat4 mProjection;\n"
        "uniform vec4 fillColor;\n"
        "out vec4 vertexColor;\n"
        "void main() {\n"
        "    gl_Position = mProjection * mModelView * vec4 (position, 1.0);\n"
        "    vertexColor = fillColor;\n"
        "}\n";
    const char* fs =
        "#version 330\n"
        "in vec4 vertexColor;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    fragColor = vec4 (vertexColor.rgb, 1.0);\n"
        "}\n";
    shader->Create (vs, fs);
}

CShaderHandler * shaderHandler = nullptr;

// =================================================================================================
