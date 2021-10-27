// =================================================================================================
// Texture coordinate representation

public class TexCoord
{
    public float m_u;
    public float m_v;

    public TexCoord(float u = 0.0f, float v = 0.0f)
    {
        m_u = u;
        m_v = v;
    }

    public float U
    {
        get => m_u;
        set => m_u = value;
    }

    public float V
    {
        get => m_v;
        set => m_v = value;
    }

    public static TexCoord operator +(TexCoord a, TexCoord b)
    {
        return new TexCoord(a.U + b.U, a.V + b.V);
    }

    public static TexCoord operator -(TexCoord a, TexCoord b)
    {
        return new TexCoord(a.U - b.U, a.V - b.V);
    }

    public static TexCoord operator -(TexCoord tc)
    {
        return new TexCoord(-tc.U, -tc.V);
    }

    public static TexCoord operator *(TexCoord tc, int n)
    {
        return new TexCoord(tc.U * n, tc.V * n);
    }

}

// =================================================================================================
