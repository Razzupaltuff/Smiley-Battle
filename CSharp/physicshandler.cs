using System;

// =================================================================================================
// physics handling (collisions, movement, animation) for Smiley Battle

public class PhysicsHandler : CollisionHandler
{
    int m_fps;
    int m_frameTime;
    Timer m_updateTimer;
    Timer m_animationTimer;

    // physics are handled at a fixed framerate of 60 fps. If the system is too slow, the physics handler will compensate for it by scaling values accordingly
    public PhysicsHandler()
    {
        m_fps = 60;
        m_frameTime = 1000 / m_fps;
        m_updateTimer = new Timer();
        m_animationTimer = new Timer();
    }

    public void Update()
    {
        UpdateMovement();
        HandleCollisions();
        AnimateActors();
    }


    public void AnimateActors()
    {
        if (Globals.controlsHandler.m_animate)
        {
            if (!m_animationTimer.HasPassed(m_frameTime, true))
                return;
            foreach (Actor a in Globals.actorHandler.m_actors)
                if (a.GetName() == "dummy")  // make smileys rotate slowly for test purposes
                    a.m_camera.UpdateAngles(new Vector(0, 1 * Globals.controlsHandler.m_turnSpeed.m_max * Globals.controlsHandler.m_speedScale, 0));
        }
    }


    // reset all per frame collision handling gameItems (e.g. the stationary flag of actors)
    public void StartCollisionHandling()
    {
        foreach (Actor a in Globals.actorHandler.m_actors)
            a.StartCollisionHandling();
    }


    // handle all collisions of actors with other actors
    public int HandleActorCollisions()
    {
        int collisions = 0;
        int l = Globals.actorHandler.m_actors.Count;
        for (int i = 0; i < l - 1; i++)
        {
            Actor thisActor = Globals.actorHandler.m_actors[i];
            // print ("   " + this.camera.name)
            for (int j = i + 1; j < l; j++)
            {
                Actor otherActor = Globals.actorHandler.m_actors[j];
                Vector vHit = new Vector(float.NaN, float.NaN, float.NaN);
                int collision;
                if (otherActor.IsProjectile())
                    collision = HandleProjectileActorCollision((Projectile)otherActor, thisActor, out vHit);
                else
                    collision = HandleActorActorCollision(thisActor, otherActor, out vHit);
                if (vHit.IsValid())
                {
                    collisions += collision; // projectile collision will return zero here because it doesn't require another collision resolution loop
                    if (thisActor.IsProjectile() || otherActor.IsProjectile())
                        Globals.soundHandler.Play("hit", vHit, 0.25f, 0, null, 2);
                    else if (thisActor.MayPlayActorHitSound())
                        Globals.soundHandler.Play("collide", vHit, 0.25f, 0, null, 2);
                }
            }
        }
        return collisions;
    }



    // handle all collisions of actors with static geometry (the map)
    public int HandleMapCollisions()
    {
        int collisions = 0;
        foreach (Actor actor in Globals.actorHandler.m_actors)
        {
            Vector vHit = new Vector(float.NaN, float.NaN, float.NaN);
            int collision;
            if (actor.IsProjectile())
                collision = HandleProjectileMapCollision((Projectile)actor, out vHit);
            else
                collision = HandleActorMapCollision(actor, out vHit);
            if (vHit.IsValid())
            {
                collisions += collision;
                if (actor.IsProjectile())
                    Globals.soundHandler.Play("wallhit", vHit, 0.25f, 0, null, 2);
                else if (actor.MayPlayWallHitSound())
                    Globals.soundHandler.Play("collide", vHit, 0.25f, 0, null, 2);
            }
        }
        return collisions;
    }


    // handle all collisions. Loop until all collisions are resolved
    // Collisions may happen due to a player steering his smiley into a wall or due to one actor 
    // bumping another one into a wall. In the latter case, the actor being bumped into a wall will 
    // be bounced back and marked as stationary, causing the actor that bumped him in the wall to 
    // bounce back. This prevents players pushing other players through the walls and out of map
    // Looping until all collisions are resolved will iteratively move all actors into positions
    // where they do !intersect with other actors or the map. A deadlock might theoretically 
    // happen, but in practise should !occur, as an actor bumping another actor in a wall has
    // free space around it to be bounced back without being bumped into another wall.
    public bool HandleCollisions()
    {
        StartCollisionHandling();
        for (int i = 0; i < 10; i++)
        {
            // handle actor/actor collisions first
            int collisions = HandleActorCollisions();
            // now handle map collisions. 
            collisions += HandleMapCollisions();
            if (collisions == 0)
                break;
        }
        return true;
    }


    // Update viewer position and orientation
    public void UpdateViewer()
    {
        float dt = (float)m_updateTimer.m_lapTime / (float)m_frameTime;
        Globals.controlsHandler.ComputeSpeedScale((float)m_fps);
        Globals.gameItems.m_viewer.Update(dt, Globals.controlsHandler.m_angles, Globals.controlsHandler.m_offset);
        Globals.controlsHandler.Ramp();

        if (Globals.controlsHandler.m_fire)
        {
            Globals.controlsHandler.m_fire = false;
            if (Globals.gameItems.m_viewer.ReadyToFire())
            {
                Globals.gameItems.m_viewer.Fire();
                Globals.soundHandler.Play("laser", Globals.gameItems.m_viewer.GetPosition(), 0.25f, 0, null, 1);
            }
        }
    }


    // update actor states and effects (disappearance, reappearance, respawning)
    public void UpdateActors()
    {
        float dt = (float)m_updateTimer.m_lapTime / (float)m_frameTime;
        foreach (Actor a in Globals.actorHandler.m_actors)
        {
            if (!a.IsViewer())
                a.Update(dt);
            if (a.HavePosition())
            {  // will happen during multiplayer games, when a new player has joined but hasn't got a position yet
                a.UpdateSound();
                if (a.m_animation == 1)
                {
                    a.m_animation = 0;
                    Globals.soundHandler.Play("disappear", a.GetPosition(), 0.25f, 0, a, 3);
                }
                else if (a.m_animation == 2)
                {
                    a.m_animation = 0;
                    Globals.soundHandler.Play("reappear", a.GetPosition(), 0.25f, 0, a, 3);
                }
            }
        }
    }


    public void UpdateMovement()
    {
        if (m_updateTimer.HasPassed(m_frameTime, true))
        {
            UpdateViewer();
            UpdateActors();
        }
    }

}

// =================================================================================================
