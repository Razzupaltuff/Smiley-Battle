#pragma once

#include "actor.h"
#include "map.h"
#include "timer.h"
#include "gameData.h"
#include "gameItems.h"
#include "soundHandler.h"
#include "actorHandler.h"
#include "collisionhandler.h"
#include "controlsHandler.h"

// =================================================================================================
// physics handling (collisions, movement, animation) for Smiley Battle

class CPhysicsHandler : public CCollisionHandler {
public:
    int         m_fps;
    int         m_frameTime;
    CTimer      m_updateTimer;
    CTimer      m_animationTimer;

    // physics are handled at a fixed framerate of 60 fps. If the system is too slow, the physics handler will compensate for it by scaling values accordingly
    CPhysicsHandler () : m_fps (60) {
        m_frameTime = 1000 / m_fps;
    }


    void Update (void);

    void AnimateActors (void);

    // reset all per frame collision handling gameItems (e.g. the stationary flag of actors)
    void StartCollisionHandling (void);

    // handle all collisions of actors with other actors
    int HandleActorCollisions (void);

    // handle all collisions of actors with static geometry (the map)
    int HandleMapCollisions (void);

    // handle all collisions. Loop until all collisions are resolved
    // Collisions may happen due to a player steering his smiley into a wall or due to one actor 
    // bumping another one into a wall. In the latter case, the actor being bumped into a wall will 
    // be bounced back and marked as stationary, causing the actor that bumped him in the wall to 
    // bounce back. This prevents players pushing other players through the walls and out of map
    // Looping until all collisions are resolved will iteratively move all actors into positions
    // where they do !intersect with other actors or the map. A deadlock might theoretically 
    // happen, but in practise should !occur, as an actor bumping another actor in a wall has
    // free space around it to be bounced back without being bumped into another wall.
    bool HandleCollisions (void);

    // Update viewer position and orientation
    void UpdateViewer (void);

    // update actor states and effects (disappearance, reappearance, respawning)
    void UpdateActors (void);

    void UpdateMovement (void);

};

extern CPhysicsHandler* physicsHandler;

// =================================================================================================
