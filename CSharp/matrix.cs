using System;
using System.Runtime.CompilerServices;
// using Vector;
// using ITuple;

// =================================================================================================
// Matrix math.Just the usual 3D stuff.

public class Matrix
{
    public Vector[] m_data;
    public float[] m_array;

    public Matrix()
    {
        m_data = new Vector[4];
        R = new Vector(1.0f, 0.0f, 0.0f, 0.0f);
        U = new Vector(0.0f, 1.0f, 0.0f, 0.0f);
        F = new Vector(0.0f, 0.0f, 1.0f, 0.0f);
        T = new Vector(0.0f, 0.0f, 0.0f, 1.0f);
    }
    public Matrix(Vector r, Vector u, Vector f)
    {
        m_data = new Vector[4];
        R = r;
        U = u;
        F = f;
        T = new Vector (0.0f, 0.0f, 0.0f, 1.0f);

    }

    public Matrix(Vector r, Vector u, Vector f, Vector t)
    {
        m_data = new Vector[4];
        R = r;
        U = u;
        F = f;
        T = t;

    }

    public Vector R
    {
        get => m_data[0];
        set => m_data[0] = value;
    }

    public Vector U
    {
        get => m_data[1];
        set => m_data[1] = value;
    }

    public Vector F
    {
        get => m_data[2];
        set => m_data[2] = value;
    }

    public Vector T
    {
        get => m_data[3];
        set => m_data[3] = value;
    }

    public Matrix Compute(float sinX, float cosX, float sinY, float cosY, float sinZ, float cosZ)
    {
        R = new Vector (cosY * cosZ, -cosY * sinZ, sinY);
        U = new Vector(sinX * sinY * cosZ + cosX * sinZ, -sinX * sinY * sinZ + cosX * cosZ, -sinX * cosY);
        F = new Vector(-cosX * sinY * cosZ + sinX * sinZ, cosX * sinY * sinZ + sinX * cosZ, cosX * cosY);
        T = new Vector(0, 0, 0, 1);
        return this;
    }

    public Vector this[int i]
    {
        get => m_data[i];
        set => m_data[i] = value;
    }

    public Matrix Identity() 
    {
        return new Matrix (new Vector (1.0f, 0.0f, 0.0f, 0.0f), new Vector (0.0f, 1.0f, 0.0f, 0.0f), new Vector (0.0f, 0.0f, 1.0f, 0.0f), new Vector (0.0f, 0.0f, 0.0f, 1.0f));
    }

    static public Matrix operator *(Matrix a, Matrix b)
    {
        Matrix m = new Matrix();
        Vector v = new Vector (m.R.X, m.U.X, m.F.X);
        m.R.X = v.Dot(b.R);
        m.U.X = v.Dot(b.U);
        m.F.X = v.Dot(b.F);
        v = new Vector(m.R.Y, m.U.Y, m.F.Y);
        m.R.Y = v.Dot(b.R);
        m.U.Y = v.Dot(b.U);
        m.F.Y = v.Dot(b.F);
        v = new Vector(m.R.Z, m.U.Z, m.F.Z);
        m.R.Z = v.Dot(b.R);
        m.U.Z = v.Dot(b.U);
        m.F.Z = v.Dot(b.F);
        m.T = new Vector (0, 0, 0, 1);
        return m;

    }

    public float Det()
    {
        return R.X * (U.Y * F.Z - U.Z * F.Y) + R.Y * (U.Z * F.X - U.X * F.Z) + R.Z * (U.X * F.Y - U.Y * F.X);

    }

    public Matrix Inverse()
    {
        float det = Det ();
        return new Matrix 
        (
            new Vector (U.Y * F.Z - U.Z * F.Y, R.Z * F.Y - R.Y * F.Z, R.Y * U.Z - R.Z * U.Y) / det,
            new Vector (U.Z * F.X - U.X * F.Z, R.X * F.Z - R.Z * F.X, R.Z * U.X - R.X * U.Z) / det,
            new Vector (U.X * F.Y - U.Y * F.X, R.Y * F.X - R.X * F.Y, R.X * U.Y - R.Y * U.X) / det,
            new Vector (0, 0, 0, 1)
        );
    }

    public Matrix Transpose()
    {
        return new Matrix(new Vector (R.X, U.X, F.X), new Vector (R.Y, U.Y, F.Y), new Vector (R.Z, U.Z, F.Z), new Vector(0, 0, 0, 1));
    }

    public Vector Rotate(Vector v) 
    {
        Matrix t = Transpose();
        return new Vector(v.Dot(t[0]), v.Dot(t[1]), v.Dot(t[2]));
    }

    public Vector Unrotate(Vector v) 
    {
        return new Vector(v.Dot(R), v.Dot(U), v.Dot(F));
    }


    public float[] AsArray()
    {
        m_array = new float[16];
        int h = 0;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                m_array[h++] = m_data[i].m_data[j];
        return m_array;
    }


    public float[] A
    {
        get => m_array;
    }

    public Matrix Clone()
    {
        Matrix clone = new Matrix();
        m_data.CopyTo(clone.m_data, 0);
        return clone;
    }

}

// =================================================================================================

