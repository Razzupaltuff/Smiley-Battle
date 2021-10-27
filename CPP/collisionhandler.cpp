#pragma once

#include "collisionhandler.h"
#include "gameitems.h"

// =================================================================================================

// handle collision between two spherical objects. Return 1 if a collision was detected, 0 otherwise
// bounce back in the direction of the otherActor object's center to the collision point
// by half of || the full the intrusion depth depending on the otherActor object's state
// objects that have bounced off a wall in a preceding collision handling step will
// have the stationary flag set to avoid them being bumped into the wall again.
// Such objects will !move, instead, the otherActor object will bounce by the full
// intrusion vector length
int CCollisionHandler::HandleActorActorCollision (CActor * thisActor, CActor * otherActor, CVector& vHit) {   // gameTime is required to keep the same function interface as in CProjectile.HandleActorCollision
    vHit = CVector (NAN, NAN, NAN);
    if (thisActor->IsHidden () || otherActor->IsHidden ())
        return 0;
    if (!thisActor->HavePosition ())
        return 0;
    CVector p = thisActor->GetPosition ();
    if (!p.IsValid ())
        return 0;
    CVector v = otherActor->GetPosition () - p;
    float l = v.Len ();
    float r = thisActor->Radius () + otherActor->Radius ();
    if (l >= r)
        return 0;
    vHit = p + v * 0.5f;
    if (l < 0.000001)
        return 1;
    l = r / l - 1.0f;
    if (l < m_tolerance)
        l = m_tolerance;
    if (thisActor->m_stationary || thisActor->IsDead ())
        otherActor->Bounce (v * l);
    else if (otherActor->m_stationary || otherActor->IsDead ())
        thisActor->Bounce (v * -l);
    else {
        v *= l / 2.0f;
        thisActor->Bounce (-v);
        otherActor->Bounce (v);
    }
    return 1;
}


// bounce off a wall and set stationary flag it that happens. 
// Return 1 if a collision was detected, 0 otherwise
int CCollisionHandler::HandleActorMapCollision (CActor * actor, CVector& vHit) {
    vHit = CVector (NAN, NAN, NAN);
    if (actor->IsHidden ())
        return 0;
    if (!actor->HavePosition ())
        return 0;
    CVector p = actor->GetPosition ();
    if (!p.IsValid ())
        return 0;
    CVector v;
    if (GetActorMapCollisions (actor->m_camera.m_positions, actor->Radius (), vHit, v) < 0)
        return 0;
    actor->Bounce (-v);
    actor->m_stationary = true;
    return 1;
}


int CCollisionHandler::GetActorMapCollisions (CVector* positions, float radius, CVector& vHit, CVector& vBounce) {
    vHit = CVector (NAN, NAN, NAN);
    vBounce = CVector (NAN, NAN, NAN);
    CVector v = positions [1] - positions [0];   // movement offset
    float l = v.Len ();
    if (l == 0.0f)
        return -1;
    CVector n = v;
    n.Normalize ();
    vBounce = CVector (0, 0, 0);
    CList<CWall*> walls;
    gameItems->m_map->GetNearbyWalls (positions [0], walls);
    int collisions = 0;
    CVector hitPosition (NAN, NAN, NAN);
    for (auto [i, w] : walls) {
        // for w in m_walls:
        float d = w->m_normal.Dot (n);
        if ((d > -m_tolerance) && (d < m_tolerance))
            continue;
        bool penetrated = w->LineIntersection (hitPosition, positions [0], positions [1], radius);
        if (!hitPosition.IsValid ())          // v doesn't cross plane of current face f
            continue;
        if (!w->Contains (hitPosition))    // face rectangle contains intersection point i -> collision
            continue;
        if (penetrated) {    // actor went through wall. Place on proper side of wall in the direction of the wall normal && at the distance of the actor's radius
            l = w->Project (positions [0], hitPosition);
            vBounce = (hitPosition - positions [0]) * (1.0f + radius);
        }
        else {
            vHit = hitPosition - positions [0];
            l = radius / vHit.Len () - 1.0f;
            if (l < m_tolerance)
                l = m_tolerance;
            vBounce += vHit * l;
        }
        collisions++;
    }
    return (collisions > 0) ? 1 : -1;
}


int CCollisionHandler::HandleProjectileMapCollision (CProjectile * projectile, CVector& vHit) {   // gameTime required to provide the same function interface as CActor->HandleMapCollision
    vHit = CVector (NAN, NAN, NAN);
    if (!projectile->IsLocalActor ())
        return 0;
    if (GetProjectileMapCollision (projectile->m_camera.m_positions, projectile->Radius (), vHit))
        projectile->Delete ();
    return 0;
}


// In multiplayer games, the local player will only process hits he receives
// That way, every client decides whether && when he gets hit, && communicates hits to the otherActor players
// The result is a smooth gameplay experience for each local player regarding collisions with projectiles
// && movement of his own projectiles
int CCollisionHandler::HandleProjectileActorCollision (CProjectile * projectile, CActor * actor, CVector& vHit) {
    vHit = CVector (NAN, NAN, NAN);
    if (actor == projectile->m_parent)  // don't shoot yourself (can happen right after the shot has been fired)
        return 0;
    if (!actor->IsLocalActor ())  // only register hits on the viewer (local player)
        return 0;
    CVector p = projectile->GetPosition ();
    CVector v = actor->GetPosition () - p;
    float l = v.Len ();
    float r = projectile->Radius () + actor->Radius ();
    if (l >= r)
        return 0;
    actor->RegisterHit (projectile->m_parent);
    projectile->Delete (true);
    vHit = p + v * 0.5f;
    return 0;
}


// Since projectiles can be so fast && are so small that they may !intersect with walls, but pass through them (i.e. their start && end positions lie on 
// opposite sides of the wall), GetProjectileCollisions checks whether a projectile's translation vector #include "the previous to the current position intersects with 
// a wall's plane && whether the intersection point lies within in the wall rectangle.
// For a projectile, the information whether it collided with a wall || !is sufficient, since a wall collision will simply destroy it, as projectiles [currently] 
// // will !bounce
bool CCollisionHandler::GetProjectileMapCollision (CVector* positions, float radius, CVector& vHit) {
    CVector v = positions [1] - positions [0];   // movement offset
    float l = v.Len ();
    CVector n = v;
    n.Normalize ();
    float d = l;
    CList<CWall*> walls;
    gameItems->m_map->GetNearbyWalls (positions [0], walls);
    for (auto [i, w] : walls) {
        if (l > 0) {
            d = w->m_normal.Dot (n);
            if ((d > -m_tolerance) && (d < m_tolerance))
                continue;
            CVector hitPosition (NAN, NAN, NAN);
            bool penetrated = w->LineIntersection (hitPosition, positions [0], positions [1], radius);
            if (!hitPosition.IsValid ())       // v doesn't cross plane of current face f
                continue;
            if (w->Contains (hitPosition)) {     // face rectangle contains intersection point i -> collision
			    vHit = hitPosition;
                return true;
			}
        }
    }
    return false;
}

// =================================================================================================
