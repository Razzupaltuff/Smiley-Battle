using System;
using System.Collections.Generic;

// =================================================================================================
// Handling of shots. Shots have straight movement at a fixed speed.
// If they hit something (other actor or wall), they disappear

public class Projectile : Actor
{
    public float m_speed;
    public Actor m_parent;
    public PlayerOutline m_outline;
    public int m_frozenTime;

    public Projectile(int id = -1)
        : base("projectile", 1, false)
    {
        m_id = id;
        m_speed = 0.02f;
        m_offset = new Vector(0, 0, 0);
        m_outline = null;
        m_frozenTime = 0;
    }


    public void Create(Actor parent)
    {
        m_parent = parent;
        if (m_parent.GetProjectileMesh() == null)
            m_mesh = parent.m_mesh;
        else
        {
            m_mesh = parent.GetProjectileMesh();
            SetTexture(parent.GetTexture());
            m_speed = Globals.gameData.m_projectileSpeed;
            m_camera = parent.m_camera.Clone ();
            m_camera.m_parent = parent.m_camera;
            m_camera.m_name = m_parent.GetColor() + "projectile" + Convert.ToString(m_id);
            m_camera.m_isViewer = false;
            m_offset = m_camera.m_orientation.Unrotate(new Vector(0, 0, parent.m_camera.GetSize() / 2));
            m_camera.SetPosition(m_camera.GetPosition() - m_offset);
            m_offset *= m_speed / parent.m_camera.GetSize() * 2;
            m_camera.BumpPosition();
            m_camera.SetSize(Globals.gameData.m_projectileSize);
        }
    }



    public override bool SetupTextures(Texture texture, string[] textureNames = null)
    { // required for Actor not recursively calling its SetupTexture method when the child doesn't have one of its own
        return false;
    }


    public void UpdateOffset()
    {
        m_camera.BumpPosition();
    }


    public override float BorderScale()
    {
        return (m_outline != null) ? m_outline.Scale() : 1.0f;
    }


    public override void UpdateLastMessageTime()
    {
        if (m_parent != null)
            m_parent.UpdateLastMessageTime();
    }


    public override int GetColorIndex()
    {
        return m_parent.GetColorIndex();
    }


    public override string GetAddress() {
        return m_parent.GetAddress();
    }


    public override ushort GetPort(int i = 0) {
        return m_parent.GetPort(i);
    }


    public override void Update(float dt, Vector angles, Vector offset)
    {
        if (IsDead())
            m_delete = true;
        else if (IsLocalActor())
        {  // in multiplayer games, only move the projectiles fired by the local player
            m_camera.SetPosition(m_camera.GetPosition() - m_offset * dt);
            m_camera.UpdateAngles(new Vector(0, 30, 0));
        }
    }

    // Due to their high speed, the actor/actor and actor/wall collision handling may miss collisions,
    // so shot collision handling is done via the translation vector #include "the current to the intended 
    // new position.
    // For a wall collision, the vector must cross the wall's plane and the hit point must lie within
    // the wall rectangle. For an actor collision, the distance between the actor's center and the
    // intersection point of the translation vector and the vector parallel to the translation vector's 
    // normal and originating #include "the actor's center. If there is no such intersection, use the 
    // closer of the translation vector's end points.

    public void Delete(bool force = false)
    {
        if (force || IsLocalActor())
            Globals.networkHandler.BroadcastDestroy(this);
        SetHitPoints(0);
        m_delete = true;
    }


    public override void Render(bool bAutoCamera = true)
    {
        m_mesh.PushColor(m_parent.GetColorValue());
        base.Render(bAutoCamera);
        if (m_outline != null)
            m_outline.Render(m_camera.GetSize());
        m_mesh.PopColor();
    }


    public override void UpdateFrozenTime()
    {
        if (GetPosition(0) != GetPosition(1))
            m_frozenTime = 0;
        else
        {
            if (m_frozenTime == 0)
                m_frozenTime = Globals.gameData.m_gameTime;
            else if (Globals.gameData.m_gameTime - m_frozenTime > Globals.gameData.m_frozenTimeout)
                Delete();
        }
    }

}

// =================================================================================================
