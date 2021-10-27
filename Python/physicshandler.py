import globals

from actor import *
from controlshandler import *
from map import *
from timer import *
from gamedata import *
from gameitems import *
from soundhandler import *
from collisionhandler import *
from controlshandler import *

# =================================================================================================
# physics handling (collisions, movement, animation) for Smiley Battle

class CPhysicsHandler (CCollisionHandler):
    def __init__ (self):
        super ().__init__ ()
        # physics are handled at a fixed framerate of 60 fps. If the system is too slow, the physics handler will compensate for it by scaling values accordingly
        self.fps = 60           
        self.frameTime = 1000 // self.fps
        self.updateTimer = CTimer ()
        self.animationTimer = CTimer ()


    def Update (self):
        self.UpdateMovement ()
        self.HandleCollisions ()
        self.AnimateActors ()


    def AnimateActors (self):
        return
        if (globals.controlsHandler.animate):
            if not self.animationTimer.HasPassed (self.frameTime, True):
                return
            for a in globals.actorHandler.actors:
                if (a.GetName () == "player"):  # make smileys rotate slowly for test purposes
                    a.camera.UpdateAngles (CVector (0, 1 * globals.controlsHandler.turnSpeed.max * globals.controlsHandler.speedScale, 0))


    # reset all per frame collision handling gameItems (e.g. the stationary flag of actors)
    def StartCollisionHandling (self):
        for a in globals.actorHandler.actors:
            a.StartCollisionHandling ()
            

    # handle all collisions of actors with other actors
    def HandleActorCollisions (self):
        collisions = 0
        l = len (globals.actorHandler.actors)
        for i in range (0, l - 1):
            this = globals.actorHandler.actors [i]
            # print ("   " + this.camera.name)
            for j in range (i + 1, l):
                other = globals.actorHandler.actors [j]
                # print ("      " + other.camera.name)
                if (other.IsProjectile ()):
                    collision, vHit = self.HandleProjectileActorCollision (other, this)
                else:
                    collision, vHit = self.HandleActorActorCollision (this, other)
                if (vHit is not None):
                    collisions += collision # projectile collision will return zero here because it doesn't require another collision resolution loop
                    if (this.IsProjectile ()) or (other.IsProjectile ()):
                        globals.soundHandler.Play ("hit", vHit, volume = 0.25, level = 2)
                    elif this.MayPlayActorHitSound ():
                        globals.soundHandler.Play ("collide", vHit, volume = 0.25, level = 2)

        return collisions


    # handle all collisions of actors with static geometry (the map)
    def HandleMapCollisions (self):
        collisions = 0
        for actor in globals.actorHandler.actors:
            if (actor.IsProjectile ()):
                collision, vHit = self.HandleProjectileMapCollision (actor)
            else:
                collision, vHit = self.HandleActorMapCollision (actor)
            if (vHit is not None):
                collisions += collision
                if (actor.IsProjectile ()):
                    globals.soundHandler.Play ("wallhit", vHit, volume = 0.25, level = 2)
                elif actor.MayPlayWallHitSound ():
                    globals.soundHandler.Play ("collide", vHit, volume = 0.25, level = 2)
        return collisions


    # handle all collisions. Loop until all collisions are resolved
    # Collisions may happen due to a player steering his smiley into a wall or due to one actor 
    # bumping another one into a wall. In the latter case, the actor being bumped into a wall will 
    # be bounced back and marked as stationary, causing the actor that bumped him in the wall to 
    # bounce back. This prevents players pushing other players through the walls and out of map
    # Looping until all collisions are resolved will iteratively move all actors into positions
    # where they do not intersect with other actors or the map. A deadlock might theoretically 
    # happen, but in practise should not occur, as an actor bumping another actor in a wall has
    # free space around it to be bounced back without being bumped into another wall.
    def HandleCollisions (self):
        self.StartCollisionHandling ()
        for i in range (10):
            # handle actor/actor collisions first
            collisions = self.HandleActorCollisions ()
            # now handle map collisions. 
            collisions += self.HandleMapCollisions ()
            if (collisions == 0):
                break
        return True


    # Update viewer position and orientation
    def UpdateViewer (self):
        dt = self.updateTimer.lapTime / self.frameTime
        globals.controlsHandler.ComputeSpeedScale (self.fps)
        globals.gameItems.viewer.Update (dt, globals.controlsHandler.angles, globals.controlsHandler.offset)
        globals.controlsHandler.Ramp ()

        if (globals.controlsHandler.fire):
            globals.controlsHandler.fire = False
            if (globals.gameItems.viewer.ReadyToFire ()):
                globals.gameItems.viewer.Fire ()
                globals.soundHandler.Play ("laser", globals.gameItems.viewer.GetPosition (), volume = 0.25, level = 1)


    # update actor states and effects (disappearance, reappearance, respawning)
    def UpdateActors (self):
        dt = self.updateTimer.lapTime / self.frameTime
        for a in globals.actorHandler.actors:
            if (not a.isViewer):
                a.Update (dt)
            if (a.GetPosition () is not None):  # will happen during multiplayer games, when a new player has joined but hasn't got a position yet
                a.UpdateSound ()
                if (a.animation == 1):
                    a.animation = 0
                    globals.soundHandler.Play ("disappear", a.GetPosition (), volume = 0.25, owner = a, level = 3)
                elif (a.animation == 2):
                    a.animation = 0
                    globals.soundHandler.Play ("reappear", a.GetPosition (), volume = 0.25, owner = a, level = 3)
                

    def UpdateMovement (self):
        if self.updateTimer.HasPassed (self.frameTime, True):
            self.UpdateViewer ()
            self.UpdateActors ()

# =================================================================================================
