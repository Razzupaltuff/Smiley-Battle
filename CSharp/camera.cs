using System;

// =================================================================================================
// Position and orientation (scaling, translation and rotation) handling
// Can have parent cameras. Compute world position depending on parent positions if a parent other
// than the viewer is present. This allows for child actors that move with their parent actors.

public class Camera
{
    public string m_name;
    public Matrix m_orientation;
    public bool[] m_havePositions;
    public Vector[] m_positions;
    public Vector m_angles;
    public float m_size;
    public bool m_isViewer;
    public Camera m_parent = null;

    public Camera(string name = "", bool isViewer = false)
    {
        m_size = 1.0f;
        m_name = name;
        m_parent = null;
        m_isViewer = isViewer;
        m_havePositions = new bool[2] { false, false };
        m_positions = new Vector[2] { new Vector(), new Vector() };
        m_angles = new Vector();
        m_orientation = new Matrix();
    }


    public Camera Clone ()
    {
        Camera clone = new Camera();
        clone.m_orientation = m_orientation.Clone();
        clone.m_positions[0] = m_positions[0].Clone();
        clone.m_positions[1] = m_positions[1].Clone();
        m_havePositions.CopyTo(clone.m_havePositions, 0);
        clone.m_angles = m_angles.Clone();
        clone.m_size = m_size;
        return clone;
    }

    public void Move(Vector v)
    {
        GL.Translate(v.X, v.Y, v.Z);
    }


    public void Rotate(Matrix m)
    {
        GL.MultMatrix(m.AsArray());
    }


    public void Scale()
    {
        GL.Scale(m_size, m_size, m_size);
    }


    public void Setup(Vector position, Matrix orientation)
    {
        m_positions[0] = position;
        m_orientation = orientation;
    }


    public void SetParent(Camera parent)
    {
        m_parent = parent;
    }


    public float GetSize()
    {
        return m_size;
    }

    public void SetSize(float size)
    {
        m_size = size;
    }

    public void Enable()
    {
        GL.MatrixMode(GL.MODELVIEW);
        GL.PushMatrix();
        if (m_parent == null)
        {
            Rotate(m_orientation);
            Move(-m_positions[0]);
        }
        else
        {
            Move(m_positions[0]);
            Rotate(m_orientation);
        }
        Scale();

    }

    public void Disable()
    {
        GL.MatrixMode(GL.MODELVIEW);
        GL.PopMatrix();
    }

    public Vector GetOrientation()
    {
        return m_angles;
    }

    public void SetOrientation(Vector angles)
    {
        m_angles = angles;
        ClampAngles();
        UpdateOrientation();
    }


    public bool HavePosition(int i = 0)
    {
        return m_havePositions[i];
    }


    public Vector GetPosition(int i = 0)
    {
        return m_positions[i];
    }


    public void SetPosition(Vector position, int i = 0)
    {
        if (position != null)
        {
            if (i == 0)
                BumpPosition();
            m_positions[i] = position;
            m_havePositions[i] = position.IsValid();
        }
    }


    public void UpdatePosition(Vector offset)
    {
        if (offset != null)
        {
            BumpPosition();
            m_positions[0] += offset;
        }
    }


    public void BumpPosition()
    {
        m_positions[1] = m_positions[0];
        m_havePositions[1] = m_havePositions[0];
    }


    public Vector R
    {
        get => m_orientation.R;
    }


    public Vector U
    {
        get => m_orientation.U;
    }


    public Vector F
    {
        get => m_orientation.F;
    }


    public float Rad(float a)
    {
        return (float)(a / 180.0 * Math.PI);
    }


    private void ClampAngles()
    {
        Func<float, float> clamp = v => { return (v < -180.0f) ? 360.0f : (v > 180.0f) ? -360.0f : 0.0f; };
        m_angles.X += clamp(m_angles.X);
        m_angles.Y += clamp(m_angles.Y);
        m_angles.Z += clamp(m_angles.Z);
    }

    // create an orientation (rotation) matrix from heading angles
    public void UpdateAngles(Vector angles, bool reverse = false)
    {
        if (angles != null)
        {
            m_angles += angles;
            ClampAngles();
            UpdateOrientation();
        }
    }

    public void UpdateOrientation()
    {
        float radX = Rad(m_angles.X);
        float radY = Rad(m_angles.Y);
        float radZ = Rad(m_angles.Z);
        m_orientation.Compute((float)Math.Sin(radX), (float)Math.Cos(radX), (float)Math.Sin(radY), (float)Math.Cos(radY), (float)Math.Sin(radZ), (float)Math.Cos(radZ));

    }

}

// =================================================================================================
