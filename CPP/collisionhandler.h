#pragma once

#include "vector.h"
#include "actor.h"
#include "projectile.h"
#include "map.h"

// =================================================================================================

class CCollisionHandler {
public:
    float   m_tolerance;

    CCollisionHandler () : m_tolerance (0.0001f) {}


    // handle collision between two spherical objects. Return 1 if a collision was detected, 0 otherwise
    // bounce back in the direction of the otherActor object's center to the collision point
    // by half of || the full the intrusion depth depending on the otherActor object's state
    // objects that have bounced off a wall in a preceding collision handling step will
    // have the stationary flag set to avoid them being bumped into the wall again.
    // Such objects will !move, instead, the otherActor object will bounce by the full
    // intrusion vector length
    int HandleActorActorCollision (CActor * thisActor, CActor * otherActor, CVector& vHit);

    // bounce off a wall and set stationary flag it that happens. 
    // Return 1 if a collision was detected, 0 otherwise
    int HandleActorMapCollision (CActor * actor, CVector& vHit);

    int GetActorMapCollisions (CVector* positions, float radius, CVector& vHit, CVector& vBounce);

    int HandleProjectileMapCollision (CProjectile * projectile, CVector& vHit);

    // In multiplayer games, the local player will only process hits he receives
    // That way, every client decides whether && when he gets hit, && communicates hits to the otherActor players
    // The result is a smooth gameplay experience for each local player regarding collisions with projectiles
    // && movement of his own projectiles
    int HandleProjectileActorCollision (CProjectile * projectile, CActor * actor, CVector& vHit);

    // Since projectiles can be so fast && are so small that they may !intersect with walls, but pass through them (i.e. their start && end positions lie on 
    // opposite sides of the wall), GetProjectileCollisions checks whether a projectile's translation vector #include "the previous to the current position intersects with 
    // a wall's plane && whether the intersection point lies within in the wall rectangle.
    // For a projectile, the information whether it collided with a wall || !is sufficient, since a wall collision will simply destroy it, as projectiles [currently] 
    // // will !bounce
    bool GetProjectileMapCollision (CVector* positions, float radius, CVector& vHit);

};

// =================================================================================================
