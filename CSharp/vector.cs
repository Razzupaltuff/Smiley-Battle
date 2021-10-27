
using System;

// =================================================================================================
// Vector math for 3D rendering

public class Vector
{

    public float[] m_data;

    public Vector()
    {
        m_data = new float[4];
        X = 0.0f;
        Y = 0.0f;
        Z = 0.0f;
        W = 0.0f;
    }

    public Vector(float x, float y, float z, float w = 0.0f)
    {
        m_data = new float[4];
        X = x;
        Y = y;
        Z = z;
        W = w;
    }

    public static Vector Origin { get; } = new Vector(0, 0, 0, 0);
    public static Vector None { get; } = new Vector(float.NaN, float.NaN, float.NaN, float.NaN);

    public float[] Data { get => m_data; }

    public float X
    {
        get => m_data [0];
        set => m_data [0] = value;
    }

    public float Y
    {
        get => m_data [1];
        set => m_data [1] = value;
    }

    public float Z
    {
        get => m_data [2];
        set => m_data [2] = value;
    }

    public float W
    {
        get => m_data[3];
        set => m_data[3] = value;
    }

    public float R
    {
        get => m_data[0];
        set => m_data[0] = value;
    }

    public float G
    {
        get => m_data[1];
        set => m_data[1] = value;
    }

    public float B
    {
        get => m_data[2];
        set => m_data[2] = value;
    }

    public float A
    {
        get => m_data[3];
        set => m_data[3] = value;
    }

    public static Vector operator +(Vector a, Vector b) 
    {
        return new Vector (a.X + b.X, a.Y + b.Y, a.Z + b.Z, a.W + b.W);
    }


    public static Vector operator -(Vector a, Vector b) 
    {
        return new Vector(a.X - b.X, a.Y - b.Y, a.Z - b.Z, a.W - b.W);
    }


    static public Vector operator *(Vector v, float n) 
    {
        return new Vector(v.X * n, v.Y * n, v.Z * n);
    }


    static public Vector operator /(Vector v, float n)
    {
        return new Vector(v.X / n, v.Y / n, v.Z / n);
    }


    static public Vector operator -(Vector a) 
    {
        return new Vector(-a.X, -a.Y, -a.Z);
    }

    public Vector Inc (Vector other)
    {
        X += other.X;
        Y += other.Y;
        Z += other.Z;
        return this;
    }

    public Vector Dec(Vector other)
    {
        X -= other.X;
        Y -= other.Y;
        Z -= other.Z;
        return this;
    }
    //public static bool operator== (Vector a, Vector b) 
    //{
    //    return (a.X == b.X) &(a.Y == b.Y) &(a.Z == b.Z);
    //}


    //public static bool operator !=(Vector a, Vector b)
    //{
    //    return (a.X != b.X) || (a.Y != b.Y) || (a.Z != b.Z);
    //}


    //public override bool Equals (Object obj)
    //{
    //    if ((obj == null) || !this.GetType().Equals(obj.GetType()))
    //    {
    //        return false;
    //    }
    //    Vector v = (Vector)obj;
    //    return this == v;
    //}


    //public override uint GetHashcode(Vector v)
    //{
    //    v = v * 1664525 + 1013904223;
    //    v.X += v.Y * v.W; 
    //    v.Y += v.Z * v.X; 
    //    v.Z += v.X * v.Y; 
    //    v.W += v.Y * v.Z;
    //    v ˆ = v >> 16;
    //    v.X += v.Y * v.W; 
    //    v.Y += v.Z * v.X; 
    //    v.Z += v.X * v.Y; 
    //    v.W += v.Y * v.Z;
    //    return v;
    //}

    public float Dot(Vector b) 
    {
        return X * b.X + Y * b.Y + Z * b.Z;
    }


    public Vector Cross(Vector b) 
    {
        return new Vector(Y * b.Z - Z * b.Y, Z * b.X - X * b.Z, X * b.Y - Y * b.X);
    }


    public float Len() 
    {
        return (float) Math.Sqrt (Dot (this));
    }


    public Vector Normalize() 
    {
        float l = this.Len();
        if ((l != 0.0) & (l != 1.0))
        {
            X /= l;
            Y /= l;
            Z /= l;
        }
        return this;
    }


    public Vector Scale (float n)
    {
        X *= n;
        Y *= n;
        Z *= n;
        return this;
    }

    public float Min() 
    {
        return (X < Y) ? (X < Z) ? X : Z : (Y < Z) ? Y : Z;
    }


    public float Max() 
    {
            return (X > Y) ? (X > Z) ? X : Z : (Y > Z) ? Y : Z;
    }


    public Vector Minimize(Vector b) 
    {
        if (X > b.X) X = b.X;
        if (Y > b.Y) Y = b.Y;
        if (Z > b.Z) Z = b.Z;
        return this;
    }


    public Vector Maximize(Vector b) 
    {
        if (X < b.X) X = b.X;
        if (Y < b.Y) Y = b.Y;
        if (Z < b.Z) Z = b.Z;
        return this;
    }


    public bool IsValid() 
    {
        return (X == X) && (Y == Y) && (Z == Z);   // if any component is NaN, check for equality with itself should return false
    }

    public static Vector Perp(Vector v0, Vector v1, Vector v2) 
    {
        return (v1 - v0).Cross(v2 - v1);
    }

    public static Vector Normal(Vector v0, Vector v1, Vector v2) 
    {
        return Perp(v0, v1, v2).Normalize();
    }

    public static int Compare(Vector v0, Vector v1)
    {
        if (v0.X < v1.X)
            return -1;
        if (v0.X > v1.X)
            return 1;
        if (v0.Y < v1.Y)
            return -1;
        if (v0.Y > v1.Y)
            return 1;
        if (v0.Z < v1.Z)
            return -1;
        if (v0.Z > v1.Z)
            return 1;
        return 0;
    }

    public Vector Clone ()
    {
        Vector clone = new Vector();
        m_data.CopyTo (clone.m_data, 0);
        return clone;
    }

    public override string ToString()
    {
        return
            Convert.ToString(X, System.Globalization.CultureInfo.InvariantCulture) + "," +
            Convert.ToString(Y, System.Globalization.CultureInfo.InvariantCulture) + "," +
            Convert.ToString(Z, System.Globalization.CultureInfo.InvariantCulture);
    }

    public Vector FromString(string values)
    {
        string[] coords = values.Split(',');
        X = Convert.ToSingle(coords[0], System.Globalization.CultureInfo.InvariantCulture);
        Y = Convert.ToSingle(coords[1], System.Globalization.CultureInfo.InvariantCulture);
        Z = Convert.ToSingle(coords[2], System.Globalization.CultureInfo.InvariantCulture);
        return this;
    }

}

// =================================================================================================


