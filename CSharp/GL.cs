using System;
using System.Runtime.InteropServices;
using SharpGL;
using SharpGL.Enumerations;
public static class GL
{
    public static OpenGL gl;
    static GL() { }
    public static void Translate(float x, float y, float z) { gl.Translate(x, y, z); }
    public static void ColorMask(byte red, byte green, byte blue, byte alpha) { gl.ColorMask(red, green, blue, alpha); }
    public static void DepthMask(byte depth) { gl.DepthMask(depth); }
    public static void Enable(uint cap) { gl.Enable(cap); }
    public static void Disable(uint cap) { gl.Disable(cap); }
    public static void DepthFunc(uint func) { gl.DepthFunc(func); }
    public static void BlendFunc(uint sFactor, uint dFactor) { gl.BlendFunc(sFactor, dFactor); }
    public static void CullFace(uint facing) { gl.CullFace(facing); }
    public static void FrontFace(uint rot) { gl.FrontFace(rot); }
    public static void MultMatrix(float[] mat) { gl.MultMatrix(mat); }
    public static void Scale(float xScale, float yScale, float zScale) { gl.Scale(xScale, yScale, zScale); }
    public static void MatrixMode(uint mode) { gl.MatrixMode(mode); }
    public static void LoadIdentity() { gl.LoadIdentity(); }
    public static void LoadMatrix(float[] mat) { gl.LoadMatrixf(mat); }
    public static void Ortho(float left, float right, float bottom, float top, float zNear, float zFar) { gl.Ortho(left, right, bottom, top, zNear, zFar); }
    public static void Viewport(int x, int y, int width, int height) { gl.Viewport(x, y, width, height); }
    public static void PushMatrix() { gl.PushMatrix(); }
    public static void PopMatrix() { gl.PopMatrix(); }
    public static void Clear(uint mask) { gl.Clear(mask); }
    public static void Color(float r, float g, float b) { gl.Color(r, g, b); }
    public static void Hint(uint target, uint mode) { gl.Hint(target, mode); }
    public static void TexParameter(uint target, uint pName, float param) { gl.TexParameter(target, pName, param); }
    public static void TexParameterI(uint target, uint pName, int[] parameters) { gl.TexParameterI(target, pName, parameters); }
    public static void BindTexture(uint target, uint handle) { gl.BindTexture(target, handle); }
    public static void ActiveTexture(uint target) { gl.ActiveTexture(target); }
    public static void TexImage2D(uint target, int level, uint internalFormat, int width, int height, int border, uint format, uint type, byte[] pixels)
    {
        gl.TexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
    }
    public static void GenTextures(int count, uint[] handles) { gl.GenTextures(count, handles); }
    public static void DeleteTextures(int count, uint[] handles) { gl.DeleteTextures(count, handles); }
    public static void TexEnv(uint target, uint pName, float param) { gl.TexEnv(target, pName, param); }
    public static void EnableVertexAttribArray (uint index) { gl.EnableVertexAttribArray(index); }
    public static void DisableVertexAttribArray(uint index) { gl.DisableVertexAttribArray(index); }
    public static void VertexAttribPointer(uint index, int size, uint type, bool normalized, int stride, IntPtr data)
    {
        gl.VertexAttribPointer(index, size, type, normalized, stride, data);
    }
    public static void BufferData (uint target, byte[] /*object*/ data, uint usage)
    {
        /*
        if (target == ARRAY_BUFFER)
            gl.BufferData(target, (float[]) data, usage);
        else
            gl.BufferData(target, (ushort[])data, usage);
        */
        /*
        GCHandle pinnedData = GCHandle.Alloc(data, GCHandleType.Pinned);
        gl.BufferData(target, data.Length, pinnedData.AddrOfPinnedObject(), usage);
        pinnedData.Free();
        */
        IntPtr p = Marshal.AllocHGlobal(data.Length);
        Marshal.Copy(data, 0, p, data.Length);
        gl.BufferData(target, data.Length, p, usage);
        Marshal.FreeHGlobal(p);
    }
    public static void BufferData(uint target, float[] data, uint usage)
    {
        gl.BufferData(target, data, usage);
    }
    public static void BufferData(uint target, ushort[] data, uint usage)
    {
        gl.BufferData(target, data, usage);
    }
    public static void GenBuffers (int count, uint[] handles) { gl.GenBuffers(count, handles); }
    public static void DeleteBuffers(int count, uint[] handles) { /*gl.DeleteBuffers(count, handles);*/ } // not supported?
    public static void BindBuffer(uint target, uint buffer) { gl.BindBuffer(target, buffer); }
    public static void GenVertexArrays (int count, uint[] handles) { gl.GenVertexArrays(count, handles); }
    public static void DeleteVertexArrays(int count, uint[] handles) { gl.DeleteVertexArrays(count, handles); }
    public static void BindVertexArray(uint handle) { gl.BindVertexArray(handle); }
    public static void DrawArrays (uint mode, int first, int count) { gl.DrawArrays(mode, first, count); }
    public static void DrawElements (uint mode, int count, uint type, IntPtr indices) { gl.DrawElements(mode, count, type, indices); }
    public static uint CreateShader (uint type) { return gl.CreateShader(type); }
    public static void DeleteShader(uint handle) { gl.DeleteShader(handle); }
    public static uint CreateProgram() { return gl.CreateProgram(); }
    public static void DeleteProgram(uint handle) { gl.DeleteProgram(handle); }
    public static void ShaderSource (uint handle, string source) { gl.ShaderSource(handle, source); }
    public static int GetShaderiv (uint handle, uint name)
    {
        int[] values = new int[1];
        gl.GetShader(handle, name, values);
        return values[0];
    }
    public static int GetProgramiv(uint handle, uint name)
    {
        int[] values = new int[1];
        gl.GetProgram(handle, name, values);
        return values[0];
    }
    public static string GetShaderInfoLog(uint handle)
    {
        int bufSize = GetProgramiv(handle, INFO_LOG_LENGTH);
        if (bufSize == 0)
            return "";
        System.Text.StringBuilder text = new System.Text.StringBuilder(bufSize);
        gl.GetShaderInfoLog(handle, bufSize, IntPtr.Zero, text);
        return text.ToString();
    }
    public static string GetProgramInfoLog(uint handle)
    {
        int bufSize = GetProgramiv(handle, INFO_LOG_LENGTH);
        if (bufSize == 0)
            return "";
        System.Text.StringBuilder text = new System.Text.StringBuilder (bufSize);
        gl.GetProgramInfoLog(handle, bufSize, IntPtr.Zero, text);
        return text.ToString();
    }

    public static string GetShaderSource (uint handle)
    {
        int bufSize = GetProgramiv(handle, SHADER_SOURCE_LENGTH);
        if (bufSize == 0)
            return "";
        System.Text.StringBuilder text = new System.Text.StringBuilder(bufSize);
        gl.GetShaderSource(handle, bufSize, IntPtr.Zero, text);
        return text.ToString();
    }
    public static void AttachShader (uint progHandle, uint shaderHandle) { gl.AttachShader(progHandle, shaderHandle); }
    public static void DetachShader(uint progHandle, uint shaderHandle) { gl.DetachShader(progHandle, shaderHandle); }
    public static void CompileShader (uint handle) { gl.CompileShader(handle); }
    public static void LinkProgram (uint handle) { gl.LinkProgram(handle); }
    public static void UseProgram(uint handle) { gl.UseProgram(handle); }
    static public int GetUniformLocation(uint handle, string name) { return gl.GetUniformLocation(handle, name); }
    public static void UniformMatrix (int loc, int count, bool transpose, float[] data) { gl.UniformMatrix4(loc, count, transpose, data); }
    public static void Uniform1(int loc, float data) { gl.Uniform1(loc, data); }
    public static void Uniform4(int loc, int count, float [] data) { gl.Uniform4(loc, count, data); }
    public static void GetFloat (GetTarget name, float[] data) { gl.GetFloat(name, data); }
    public static void Begin (BeginMode mode) { gl.Begin(mode); }
    public static void End() { gl.End(); }
    public static void Vertex2f (float x, float y) { gl.Vertex(x, y); }
    public static void Vertex3f (float x, float y, float z) { gl.Vertex(x, y, z); }
    public static void TexCoord2f(float u, float v) { gl.TexCoord(u, v); }
    public static void Color4f (float r, float g, float b, float a) { gl.Color(r, b, g, a);  }
    public static void Color4fv(Vector v) { gl.Color(v.m_data); }
    public static uint GetError()  { return gl.GetError(); }
    public static uint LINES { get => OpenGL.GL_LINES; }
    public static uint TRIANGLES { get => OpenGL.GL_TRIANGLES; }
    public static uint QUADS { get => OpenGL.GL_QUADS; }
    public static uint FALSE { get => OpenGL.GL_FALSE; }
    public static uint TRUE { get => OpenGL.GL_TRUE; }
    public static uint UNSIGNED_BYTE { get => OpenGL.GL_UNSIGNED_BYTE; }
    public static uint UNSIGNED_SHORT { get => OpenGL.GL_UNSIGNED_SHORT; }
    public static uint UNSIGNED_INT { get => OpenGL.GL_UNSIGNED_INT; }
    public static uint FLOAT { get => OpenGL.GL_FLOAT; }
    public static uint REPLACE { get => OpenGL.GL_REPLACE; }
    public static uint GENERATE_MIPMAP { get => 0x8191; } // not available in SharpGL ... ???
    public static uint TEXTURE_2D { get => OpenGL.GL_TEXTURE_2D; }
    public static uint CLAMP_TO_EDGE { get => OpenGL.GL_CLAMP_TO_EDGE; }
    public static uint TEXTURE_CUBE_MAP { get => OpenGL.GL_TEXTURE_CUBE_MAP; }
    public static uint TEXTURE_MIN_FILTER { get => OpenGL.GL_TEXTURE_MIN_FILTER; }
    public static uint TEXTURE_MAG_FILTER { get => OpenGL.GL_TEXTURE_MAG_FILTER; }
    public static uint TEXTURE_ENV_MODE { get => OpenGL.GL_TEXTURE_ENV_MODE; }
    public static uint TEXTURE_WRAP_S { get => OpenGL.GL_TEXTURE_WRAP_S; }
    public static uint TEXTURE_WRAP_T { get => OpenGL.GL_TEXTURE_WRAP_T; }
    public static uint TEXTURE_WRAP_R { get => OpenGL.GL_TEXTURE_WRAP_R; }
    public static uint REPEAT { get => OpenGL.GL_REPEAT; }
    public static uint TEXTURE_CUBE_MAP_POSITIVE_X { get => OpenGL.GL_TEXTURE_CUBE_MAP_POSITIVE_X; }
    public static uint TEXTURE_ENV { get => OpenGL.GL_TEXTURE_ENV; }
    public static uint TEXTURE0 { get => OpenGL.GL_TEXTURE0; }
    public static uint LINEAR { get => OpenGL.GL_LINEAR; }
    public static uint LINEAR_MIPMAP_LINEAR { get => OpenGL.GL_LINEAR_MIPMAP_LINEAR; }
    public static uint RGB { get => OpenGL.GL_RGB; }
    public static uint RGBA { get => OpenGL.GL_RGBA; }
    public static uint MODELVIEW { get => OpenGL.GL_MODELVIEW; }
    public static uint PROJECTION { get => OpenGL.GL_PROJECTION; }
    public static GetTarget MODELVIEW_MATRIX { get => (GetTarget) OpenGL.GL_MODELVIEW_MATRIX; }
    public static GetTarget PROJECTION_MATRIX { get => (GetTarget)OpenGL.GL_PROJECTION_MATRIX; }
    public static uint COLOR_BUFFER_BIT { get => OpenGL.GL_COLOR_BUFFER_BIT; }
    public static uint DEPTH_BUFFER_BIT { get => OpenGL.GL_DEPTH_BUFFER_BIT; }
    public static uint DEPTH_TEST { get => OpenGL.GL_DEPTH_TEST; }
    public static uint ALWAYS { get => OpenGL.GL_ALWAYS; }
    public static uint LESS { get => OpenGL.GL_LESS;  }
    public static uint BLEND { get => OpenGL.GL_BLEND; }
    public static uint SRC_ALPHA { get => OpenGL.GL_SRC_ALPHA; }
    public static uint ONE_MINUS_SRC_ALPHA { get => OpenGL.GL_ONE_MINUS_SRC_ALPHA; }
    public static uint ALPHA_TEST { get => OpenGL.GL_ALPHA_TEST; }
    public static uint CULL_FACE { get => OpenGL.GL_CULL_FACE; }
    public static uint FRONT{ get => OpenGL.GL_FRONT; }
    public static uint BACK { get => OpenGL.GL_BACK; }
    public static uint MULTISAMPLE { get => OpenGL.GL_MULTISAMPLE; }
    public static uint CW { get => OpenGL.GL_CW; }
    public static uint POLYGON_OFFSET_FILL { get => OpenGL.GL_POLYGON_OFFSET_FILL; }
    public static uint PERSPECTIVE_CORRECTION_HINT { get => OpenGL.GL_PERSPECTIVE_CORRECTION_HINT; }
    public static uint NICEST { get => OpenGL.GL_NICEST; }
    public static uint ARRAY_BUFFER { get => OpenGL.GL_ARRAY_BUFFER; }
    public static uint ELEMENT_ARRAY_BUFFER { get => OpenGL.GL_ELEMENT_ARRAY_BUFFER; }
    public static uint STATIC_DRAW { get => OpenGL.GL_STATIC_DRAW; }
    public static uint VERTEX_SHADER { get => OpenGL.GL_VERTEX_SHADER; }
    public static uint FRAGMENT_SHADER { get => OpenGL.GL_FRAGMENT_SHADER; }
    public static uint COMPILE_STATUS { get => OpenGL.GL_COMPILE_STATUS; }
    public static uint LINK_STATUS { get => OpenGL.GL_LINK_STATUS; }
    public static uint INFO_LOG_LENGTH { get => OpenGL.GL_INFO_LOG_LENGTH; }
    public static uint SHADER_SOURCE_LENGTH { get => OpenGL.GL_SHADER_SOURCE_LENGTH; }
    public static GetTarget ModelviewMatrix { get => GetTarget.ModelviewMatix; } // watch out for the typo
    public static GetTarget ProjectionMatrix { get => GetTarget.ProjectionMatrix; }

    public static bool Create ()
    {
        gl = new OpenGL();
        if (gl == null)
        {
            Console.Error.WriteLine("Couldn't initialize OpenGL");
            System.Environment.Exit(1);
        }
        return (gl != null);
    }
}
