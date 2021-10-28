using System;

// =================================================================================================

public class Viewer : Player
{
    public bool m_fire;
    public int m_fireTime;


    public Viewer() : base("viewer") { }


    public override void Render(bool autoCamera = true)
    {
        // empty
    }

    public bool ReadyToFire()
    {
        // global Globals.gameData
        if (m_hitPoints == 0)
            return false;
        if (m_fireTime == 0)
            return true;
        if (Globals.gameData.m_fireMode == 0)
            return (Globals.gameData.m_gameTime - m_fireTime > Globals.gameData.m_fireDelay);
        if (Globals.gameData.m_fireMode == 1)
            return (Globals.actorHandler.FindProjectile(GetColorIndex()) == null);
        return true;
    }


    public void Fire()
    {
        // global Globals.gameData, gameItems
        if (ReadyToFire())
        {
            Projectile projectile;
            if (Globals.gameData.m_fireMode == 2)
            {
                projectile = Globals.actorHandler.FindProjectile(GetColorIndex());
                if (projectile != null)
                    projectile.Delete();
            }
            m_fireTime = Globals.gameData.m_gameTime;
            projectile = Globals.actorHandler.CreateProjectile(this);
            if (projectile != null)
                Globals.networkHandler.BroadcastFire(projectile);
        }
    }


    // update viewer position && orientation
    // Scale with dt. Dt specifies the ratio of the actual frametime to the desired frametime
    // This compensates for high frametimes && assures that players on slow systems move as 
    // fast as players on fast systems
    public override void Update(float dt = 1.0f, Vector angles = null, Vector offset = null)
    {
        base.Update(dt);
        if (IsAlive() || IsImmune())
        {
            if ((angles != null) && (angles.Len() != 0))
                m_camera.UpdateAngles(angles * dt, true);   // compensate for higher frame times
            if ((offset != null) && (offset.Len() != 0))
                m_camera.UpdatePosition(m_camera.m_orientation.Unrotate(offset * dt));
        }
    }

}

// =================================================================================================
