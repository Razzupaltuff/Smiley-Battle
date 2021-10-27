using System;
using System.Collections.Generic;
using SharpGL.Enumerations;

// =================================================================================================
// Some basic shader handling: Compiling, enabling, setting shader variables

public class Shader
{
    public uint m_handle;
    public float[] m_glData;
    public string m_name;

    public Shader(string name = "") 
    {
        m_name = name;
    }

    ~Shader() {
        Destroy();
    }

    public uint Handle { get => m_handle; }

    public string GetInfoLog(uint handle, bool isProgram)
    {
        string infoLog = isProgram ? GL.GetProgramInfoLog(handle) : GL.GetShaderInfoLog(handle);
        if (infoLog.Length > 0)
            Console.Error.WriteLine ("Shader info: {0}\n", infoLog);
        return infoLog;
    }


    public uint Compile(string code, uint type)
    {
        uint handle = GL.CreateShader(type);
        GL.ShaderSource(handle, code);
        GL.CompileShader(handle);
        if (GL.GetShaderiv(handle, GL.COMPILE_STATUS) != 0)
            return handle;
        GetInfoLog(handle, false);
        Console.Error.WriteLine("\nshader source:\n\n{0}\n", GL.GetShaderSource(handle));
        return 0;
    }


    public uint Link(uint vsHandle, uint fsHandle)
    {
        if ((vsHandle == 0) || (fsHandle == 0))
            return 0;
        uint handle = GL.CreateProgram();
        if (handle == 0)
            return 0;
        GL.AttachShader(handle, vsHandle);
        GL.AttachShader(handle, fsHandle);
        GL.LinkProgram(handle);
        if (GL.GetProgramiv(handle, GL.LINK_STATUS) != 0)
        {
            GL.DetachShader(handle, vsHandle);
            GL.DetachShader(handle, fsHandle);
            return handle;
        }
        GetInfoLog(handle, true);
        Console.Error.WriteLine("\nVertex shader source:\n\n{0}\n", GL.GetShaderSource(vsHandle));
        Console.Error.WriteLine("\nFragment shader source:\n\n{0}\n", GL.GetShaderSource(fsHandle));
        GL.DeleteShader(vsHandle);
        GL.DeleteShader(fsHandle);
        GL.DeleteProgram(handle);
        return 0;
    }


    public Shader Create(string vsCode, string fsCode) {
        m_handle = Link(Compile(vsCode, GL.VERTEX_SHADER), Compile(fsCode, GL.FRAGMENT_SHADER));
        return this;
    }


    public void Destroy()
    {
        if (Handle != 0)
        {
            GL.DeleteProgram(m_handle);
            m_handle = 0;
        }
    }


    public void SetUniformMatrix(string name, float[] data)
    {
        int loc = GL.GetUniformLocation(m_handle, name);
        GL.UniformMatrix(loc, 1, false, data);
    }


    public void SetUniformVector(string name, Vector data)
    {
        int loc = GL.GetUniformLocation(m_handle, name);
        // glUniform4fv (loc, 1, data);
        GL.Uniform4(loc, 1, data.Data);
    }


    public void SetUniformFloat(string name, float data)
    {
        int loc = GL.GetUniformLocation(m_handle, name);
        // glUniform4fv (loc, 1, data);
        GL.Uniform1(loc, data);
    }


    public float[] GetFloatData(GetTarget id, int size) {
        if ((m_glData == null) || (m_glData.Length < size))
            m_glData = new float[size];
        GL.GetFloat(id, m_glData);
        return m_glData;
    }

    public void Enable()
    {
        GL.UseProgram(m_handle);
    }

    public void Disable()
    {
        GL.UseProgram(0);
    }

}

// =================================================================================================
