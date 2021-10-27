from typing import List, Tuple

import globals

from actor import *
from map import *

# =================================================================================================

class CCollisionHandler:
    def __init__ (self):
        self.tolerance = 0.0001


    # handle collision between two spherical objects. Return 1 if a collision was detected, 0 otherwise
    # bounce back in the direction of the other object's center to the collision point
    # by half of or the full the intrusion depth depending on the other object's state
    # objects that have bounced off a wall in a preceding collision handling step will
    # have the stationary flag set to avoid them being bumped into the wall again.
    # Such objects will not move, instead, the other object will bounce by the full
    # intrusion vector length
    def HandleActorActorCollision (self, this : CActor, other : CActor) -> Tuple [int, CVector]:   # gameTime is required to keep the same function interface as in CProjectile.HandleActorCollision
        if this.IsHidden () or other.IsHidden ():
            return 0, None
        p = this.GetPosition ()
        if (p is None):
            return 0, None
        if not p.IsValid ():
            return 0, None
        v = other.GetPosition () - p
        l = v.Length ()
        r = this.Radius () + other.Radius ()
        if (l >= r):
            return 0, None
        vHit = p + v.Clone ().Scale (0.5)
        if (l < 0.000001):
            return 1, vHit
        l = r / l - 1.0
        if (l < self.tolerance):
            l = self.tolerance
        if (this.stationary or this.IsDead ()):
            other.Bounce (v.Scale (l))
        elif (other.stationary or other.IsDead ()):
            this.Bounce (v.Scale (-l))
        else:
            v.Scale (l / 2)
            this.Bounce (-v)
            other.Bounce (v)
        return 1, vHit


    # bounce off a wall and set stationary flag it that happens. 
    # Return 1 if a collision was detected, 0 otherwise
    def HandleActorMapCollision (self, actor : CActor) -> Tuple [int, CVector]:
        # global gameItems
        if actor.IsHidden ():
            return 0, None
        p = actor.GetPosition ()
        if (p is None):
            return 0, None
        if not p.IsValid ():
            return 0, None
        v, l, vHit = self.GetActorMapCollisions (actor.camera.positions, actor.Radius ())
        if (l < 0):
            return 0, None
        actor.Bounce (-v)
        actor.stationary = True
        return 1, vHit


    # Get a displacement vector from all collisions of a sphere at position position with radius radius
    # This vector simply is the sum of the displacement vectors of each wall collision
    # The displacement vector is the inverse of a parallel vector to the wall's normal originating at the collision point
    # with the length of the distance of the collision point to the perimeter of the sphere
    def GetSimpleActorMapCollisions (self, positions : List [CVector], radius : float) -> Tuple [CVector, float, CVector]:
        collisions = 0
        vBounce = CVector (0,0,0)
        #radius += 0.0125
        position = positions [0]
        walls = globals.gameItems.map.GetNearbyWalls (position)
        for w in walls:
        # for f in self.walls:
            hitPosition, l = w.Project (position)
            if (l < 0):
                l = -l
            vHit = hitPosition - position
            if (l <= radius):
                if w.Contains (hitPosition):
                    # compute the length of the displacement vector computed from the sphere radius and the distance between sphere center and collision point
                    l = radius / l - 1.0 
                    if (l < self.tolerance):
                        l = self.tolerance
                    vBounce += vHit.Scale (l)
                    collisions += 1

        if (collisions == 0):
            return vBounce, -1.0, None
        return vBounce, 1.0, hitPosition


    def GetActorMapCollisions (self, positions : List [CVector], radius : float) -> Tuple [CVector, float, CVector]:
        v = positions [1] - positions [0]   # movement offset
        l = v.Length ()
        if (l == 0):
            return None, -1, None
        n = v.Clone ().Normalize ()
        vBounce = CVector (0,0,0)
        walls = globals.gameItems.map.GetNearbyWalls (positions [0])
        collisions = 0
        for w in walls:
        # for w in self.walls:
            d = w.normal.Dot (n)
            if ((d > -self.tolerance) and (d < self.tolerance)):
                continue
            hitPosition, penetrated = w.LineIntersection (positions, radius)
            if (hitPosition is None):         # v doesn't cross plane of current face f
                continue
            if (not w.Contains (hitPosition)):    # face rectangle contains intersection point i -> collision
                continue
            if (penetrated):    # actor went through wall. Place on proper side of wall in the direction of the wall normal and at the distance of the actor's radius
                hitPosition, l = w.Project (positions [0])
                vBounce = (hitPosition - positions [0]).Scale (1.0 + radius)
            else:
                vHit = hitPosition - positions [0]
                l = radius / vHit.Length () - 1.0 
                if (l < self.tolerance):
                    l = self.tolerance
                vBounce += vHit.Scale (l)
            collisions += 1

        if (collisions > 0):
            return vBounce, 1.0, hitPosition
        return vBounce, -1.0, None


    def HandleProjectileMapCollision (self, projectile : CProjectile) -> Tuple [int, CVector]:   # gameTime required to provide the same function interface as CActor.HandleMapCollision
        # global gameItems
        if not projectile.IsLocalActor (): 
            return 0, None
        collision, vHit = self.GetProjectileMapCollision (projectile.camera.positions, projectile.Radius ())
        if (collision):
            projectile.Delete ()
        return 0, vHit
        

    # In multiplayer games, the local player will only process hits he receives
    # That way, every client decides whether and when he gets hit, and communicates hits to the other players
    # The result is a smooth gameplay experience for each local player regarding collisions with projectiles
    # and movement of his own projectiles
    def HandleProjectileActorCollision (self, projectile : CProjectile, actor : CActor) -> Tuple [int, CVector]:
        if (actor == projectile.parent):  # don't shoot yourself (can happen right after the shot has been fired)
            return 0, None
        if not (actor.IsLocalActor ()):  # only register hits on the viewer (local player)
            return 0, None
        p = projectile.GetPosition ()
        v = actor.GetPosition () - p
        l = v.Length ()
        r = projectile.Radius () + actor.Radius ()
        if (l >= r):
            return 0, None
        actor.RegisterHit (projectile.parent)
        projectile.Delete (True)
        return 0, p + v.Scale (0.5)


    # Since projectiles can be so fast and are so small that they may not intersect with walls, but pass through them (i.e. their start and end positions lie on 
    # opposite sides of the wall), GetProjectileCollisions checks whether a projectile's translation vector from the previous to the current position intersects with 
    # a wall's plane and whether the intersection point lies within in the wall rectangle.
    # For a projectile, the information whether it collided with a wall or not is sufficient, since a wall collision will simply destroy it, as projectiles [currently] 
    # # will not bounce
    def GetProjectileMapCollision (self, positions : list [CVector], radius : float) -> Tuple [bool, CVector]:
        v = positions [1] - positions [0]   # movement offset
        l = v.Length ()
        n = v.Clone ().Normalize ()
        d = l
        walls = globals.gameItems.map.GetNearbyWalls (positions [0])
        for w in walls:
        # for w in globals.gameItems.map.walls:
            if (l > 0):
                d = w.normal.Dot (n)
                if ((d > -self.tolerance) and (d < self.tolerance)):
                    continue
                hitPosition, penetrated = w.LineIntersection (positions, radius)
                if (hitPosition is None):         # v doesn't cross plane of current face f
                    continue
                if (w.Contains (hitPosition)):    # face rectangle contains intersection point i -> collision
                    return True, hitPosition
        return False, None

# =================================================================================================
