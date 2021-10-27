using System;
using System.Collections.Generic;

// =================================================================================================

public class ShaderHandler
{
    public List<Shader> m_shaders;

    public ShaderHandler()
    {
        CreateShaders();
    }


    ~ShaderHandler ()
    {
        Destroy();
    }

    public Shader Shader(int i)
    {
        return m_shaders[i];
    }


    public void Destroy ()
    {
        foreach (Shader shader in m_shaders)
            shader.Destroy();
        m_shaders.Clear();
    }

    public int SelectShader(bool useShader, Texture texture)
    {
        if (!useShader)
            return -1;
        int shaderId = -1;
        if (Equals (texture, null))
            shaderId = 2;
        // select shader depending on texture type
        else if (texture.Type() == GL.TEXTURE_CUBE_MAP)
            shaderId = 0;
        else if (texture.Type() == GL.TEXTURE_2D)
            shaderId = 1;
        else
            return -1;
        SetupShader(shaderId);
        return shaderId;
    }


    public void SetupShader(int shaderId)
    {
        Shader shader = m_shaders[shaderId];
        shader.Enable();
        shader.SetUniformMatrix("mModelView", shader.GetFloatData(GL.ModelviewMatrix, 16));
        shader.SetUniformMatrix("mProjection", shader.GetFloatData(GL.ProjectionMatrix, 16));
    }


    public void CreateShaders()
    {
        m_shaders = new List<Shader>();
        m_shaders.Add(CreateCubemapShader());
        m_shaders.Add(CreateTextureShader());
        m_shaders.Add(CreateColorShader());
    }


    public Shader CreateShader (string name, string vs, string fs)
    {
        Shader shader = new Shader(name);
        shader.Create(vs, fs);
        return shader;
    }

    public Shader CreateCubemapShader()
    {
        string vs = 
            "#version 330\n" +
            "layout(location = 0) in vec3 position;\n" +
            "layout(location = 1) in vec3 normal;\n" +
            "uniform mat4 mModelView;\n" +
            "uniform mat4 mProjection;\n" +
            "uniform vec4 fillColor;\n" +
            "out vec3 viewNormal;\n" +
            "out vec3 texCoords;\n" +
            "out vec4 vertexColor;\n" +
            "void main() {\n" +
            "    gl_Position = mProjection * mModelView * vec4 (position, 1.0);\n" +
            "    viewNormal = normalize (mat3 (mModelView) * normal);\n" +
            "    texCoords = position;\n" +
            "    vertexColor = fillColor;\n" +
            "}\n";
        string fs = 
            "#version 330\n" +
            "uniform samplerCube cubeMap;\n" +
            "uniform float minBrightness;\n" +
            "in vec3 texCoords;\n" +
            "in vec3 viewNormal;\n" +
            "in vec4 vertexColor;\n" +
            "out vec4 fragColor;\n" +
            "void main() {\n" +
            "    float brightness = (dot(vec3(0, 0, 1), viewNormal) + 1.0) * 0.5;\n" +
            "    brightness *= brightness;\n" +
            "    vec4 texColor = texture(cubeMap, texCoords);\n" +
            "    fragColor = vec4(texColor.rgb * vertexColor.rgb * max(brightness, minBrightness), 1.0);\n" +
            "}\n";
        return CreateShader ("cube map shader", vs, fs);
    }


    public Shader CreateTextureShader()
    {
        string vs = 
            "#version 330\n" +
            "layout(location = 0) in vec3 position;\n" +
            "layout(location = 1) in vec2 texCoord;\n" +
            "uniform mat4 mModelView;\n" +
            "uniform mat4 mProjection;\n" +
            "uniform vec4 fillColor;\n" +
            "out vec2 texCoords;\n" +
            "out vec4 vertexColor;\n" +
            "void main() {\n" +
            "    gl_Position = mProjection * mModelView * vec4 (position, 1.0);\n" +
            "    texCoords = texCoord;\n" +
            "    vertexColor = fillColor;\n" +
            "}\n";
        string fs = 
            "#version 330\n" +
            "uniform sampler2D image;\n" +
            "in vec2 texCoords;\n" +
            "in vec4 vertexColor;\n" +
            "out vec4 fragColor;\n" +
            "void main() {\n" +
            "    vec4 texColor = texture (image, texCoords);\n" +
            "    if (texColor.a == 0) discard;\n" +
            "    else fragColor = vec4 (texColor.rgb * vertexColor.rgb, texColor.a);\n" +
            "    //fragColor = vec4 (0.0, 0.5, 1.0, 1.0);\n" +
            "}\n";
        return CreateShader("texture shader", vs, fs);
    }


    public Shader CreateColorShader()
    {
        string vs = 
            "#version 330\n" +
            "layout(location = 0) in vec3 position;\n" +
            "uniform mat4 mModelView;\n" +
            "uniform mat4 mProjection;\n" +
            "uniform vec4 fillColor;\n" +
            "out vec4 vertexColor;\n" +
            "void main() {\n" +
            "    gl_Position = mProjection * mModelView * vec4 (position, 1.0);\n" +
            "    vertexColor = fillColor;\n" +
            "}\n";
        string fs = 
            "#version 330\n" +
            "in vec4 vertexColor;\n" +
            "out vec4 fragColor;\n" +
            "void main() {\n" +
            "    fragColor = vec4 (vertexColor.rgb, 1.0);\n" +
            "}\n";
        return CreateShader("color shader", vs, fs);
    }

}

// =================================================================================================
