using System;
using System.Collections.Generic;

// =================================================================================================

public class CollisionHandler
{
    float m_tolerance;

    public CollisionHandler() 
    {
        m_tolerance = 0.0001f;
    }


    // =================================================================================================

    // handle collision between two spherical objects. Return 1 if a collision was detected, 0 otherwise
    // bounce back in the direction of the otherActor object's center to the collision point
    // by half of || the full the intrusion depth depending on the otherActor object's state
    // objects that have bounced off a wall in a preceding collision handling step will
    // have the stationary flag set to avoid them being bumped into the wall again.
    // Such objects will !move, instead, the otherActor object will bounce by the full
    // intrusion vector length
    public int HandleActorActorCollision(Actor thisActor, Actor otherActor, out Vector vHit)
    {   // gameTime is required to keep the same function interface as in Projectile.HandleActorCollision
        vHit = new Vector(float.NaN, float.NaN, float.NaN);
        if (thisActor.IsHidden() || otherActor.IsHidden())
            return 0;
        if (!thisActor.HavePosition())
            return 0;
        Vector p = thisActor.GetPosition();
        if (!p.IsValid())
            return 0;
        Vector v = otherActor.GetPosition() - p;
        float l = v.Len();
        float r = thisActor.Radius() + otherActor.Radius();
        if (l >= r)
            return 0;
        vHit = p + v * 0.5f;
        if (l < 0.000001)
            return 1;
        l = r / l - 1.0f;
        if (l < m_tolerance)
            l = m_tolerance;
        if (thisActor.m_isStationary || thisActor.IsDead())
            otherActor.Bounce(v * l);
        else if (otherActor.m_isStationary || otherActor.IsDead())
            thisActor.Bounce(v * -l);
        else
        {
            v *= l / 2.0f;
            thisActor.Bounce(-v);
            otherActor.Bounce(v);
        }
        return 1;
    }


    // bounce off a wall and set stationary flag it that happens. 
    // Return 1 if a collision was detected, 0 otherwise
    public int HandleActorMapCollision(Actor actor, out Vector vHit)
    {
        vHit = new Vector(float.NaN, float.NaN, float.NaN);
        if (actor.IsHidden())
            return 0;
        if (!actor.HavePosition())
            return 0;
        Vector p = actor.GetPosition();
        if (!p.IsValid())
            return 0;
        Vector v;
        if (GetActorMapCollisions(actor.m_camera.m_positions, actor.Radius(), out vHit, out v) < 0)
            return 0;
        actor.Bounce(-v);
        actor.m_isStationary = true;
        return 1;
    }


    public int GetActorMapCollisions(Vector[] positions, float radius, out Vector vHit, out Vector vBounce)
    {
        vHit = new Vector(float.NaN, float.NaN, float.NaN);
        vBounce = new Vector(0, 0, 0);
        Vector v = positions[1] - positions[0];   // movement offset
        float l = v.Len();
        if (l == 0.0f)
            return -1;
        Vector n = v;
        n.Normalize();
        List<Wall> walls = Globals.gameItems.m_map.GetNearbyWalls(positions[0]);
        int collisions = 0;
        Vector hitPosition;
        foreach (Wall w in walls)
        {
            // for w in m_walls:
            float d = w.m_normal.Dot(n);
            if ((d > -m_tolerance) && (d < m_tolerance))
                continue;
            bool penetrated = w.LineIntersection(out hitPosition, positions[0], positions[1], radius);
            if (!hitPosition.IsValid())          // v doesn't cross plane of current face f
                continue;
            if (!w.Contains(hitPosition))    // face rectangle contains intersection point i . collision
                continue;
            if (penetrated)
            {    // actor went through wall. Place on proper side of wall in the direction of the wall normal && at the distance of the actor's radius
                l = w.Project(positions[0], out hitPosition);
                vBounce = (hitPosition - positions[0]) * (1.0f + radius);
            }
            else
            {
                vHit = hitPosition - positions[0];
                l = radius / vHit.Len() - 1.0f;
                if (l < m_tolerance)
                    l = m_tolerance;
                vBounce += vHit * l;
            }
            collisions++;
        }
        return (collisions > 0) ? 1 : -1;
    }


    public int HandleProjectileMapCollision(Projectile projectile, out Vector vHit)
    {   // gameTime required to provide the same function interface as Actor.HandleMapCollision
        vHit = new Vector(float.NaN, float.NaN, float.NaN);
        if (!projectile.IsLocalActor())
            return 0;
        if (GetProjectileMapCollision(projectile.m_camera.m_positions, projectile.Radius(), out vHit))
            projectile.Delete();
        return 0;
    }


    // In multiplayer games, the local player will only process hits he receives
    // That way, every client decides whether && when he gets hit, && communicates hits to the otherActor players
    // The result is a smooth gameplay experience for each local player regarding collisions with projectiles
    // && movement of his own projectiles
    public int HandleProjectileActorCollision(Projectile projectile, Actor actor, out Vector vHit)
    {
        vHit = new Vector(float.NaN, float.NaN, float.NaN);
        if (actor == projectile.m_parent)  // don't shoot yourself (can happen right after the shot has been fired)
            return 0;
        if (!actor.IsLocalActor())  // only register hits on the viewer (local player)
            return 0;
        Vector p = projectile.GetPosition();
        Vector v = actor.GetPosition() - p;
        float l = v.Len();
        float r = projectile.Radius() + actor.Radius();
        if (l >= r)
            return 0;
        actor.RegisterHit(projectile.m_parent);
        projectile.Delete(true);
        vHit = p + v * 0.5f;
        return 0;
    }


    // Since projectiles can be so fast && are so small that they may !intersect with walls, but pass through them (i.e. their start && end positions lie on 
    // opposite sides of the wall), GetProjectileCollisions checks whether a projectile's translation vector #include "the previous to the current position intersects with 
    // a wall's plane && whether the intersection point lies within in the wall rectangle.
    // For a projectile, the information whether it collided with a wall || !is sufficient, since a wall collision will simply destroy it, as projectiles [currently] 
    // // will !bounce
    public bool GetProjectileMapCollision(Vector[] positions, float radius, out Vector vHit)
    {
        Vector v = positions[1] - positions[0];   // movement offset
        float l = v.Len();
        Vector n = v;
        n.Normalize();
        float d = l;
        List<Wall> walls = Globals.gameItems.m_map.GetNearbyWalls(positions[0]);
        foreach (Wall w in walls)
        {
            if (l > 0)
            {
                d = w.m_normal.Dot(n);
                if ((d > -m_tolerance) && (d < m_tolerance))
                    continue;
                Vector hitPosition = new Vector (float.NaN, float.NaN, float.NaN);
                bool penetrated = w.LineIntersection(out hitPosition, positions[0], positions[1], radius);
                if (!hitPosition.IsValid())       // v doesn't cross plane of current face f
                    continue;
                if (w.Contains(hitPosition))      // face rectangle contains intersection point i . collision
                {
                    vHit = hitPosition;
                    return true;
                }
            }
        }
        vHit = new Vector(float.NaN, float.NaN, float.NaN);
        return false;
    }


}

// =================================================================================================
