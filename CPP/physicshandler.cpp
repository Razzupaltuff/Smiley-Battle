#include "physicshandler.h"
#include "gameData.h"
#include "gameItems.h"
#include "soundHandler.h"
#include "actorHandler.h"
#include "collisionhandler.h"
#include "controlsHandler.h"

// =================================================================================================
// physics handling (collisions, movement, animation) for Smiley Battle

    void CPhysicsHandler::Update (void) {
        UpdateMovement ();
        HandleCollisions ();
        AnimateActors ();
    }


    void CPhysicsHandler::AnimateActors (void) {
        if (controlsHandler->m_animate) {
            if (!m_animationTimer.HasPassed (m_frameTime, true))
                return;
            for (auto [i, a] : actorHandler->m_actors)
                if (a->GetName () == "dummy")  // make smileys rotate slowly for test purposes
                    a->m_camera.UpdateAngles (CVector (0, 1 * controlsHandler->m_turnSpeed.m_max * controlsHandler->m_speedScale, 0));
        }
    }


    // reset all per frame collision handling gameItems (e.g. the stationary flag of actors)
    void CPhysicsHandler::StartCollisionHandling (void) {
        for (auto [i, a] : actorHandler->m_actors)
            a->StartCollisionHandling ();
    }


    // handle all collisions of actors with other actors
    int CPhysicsHandler::HandleActorCollisions (void) {
        int collisions = 0;
        int l = int (actorHandler->m_actors.Length ());
        for (int i = 0; i < l - 1; i++) {
            CActor * thisActor = actorHandler->m_actors [i];
            // print ("   " + this.camera->name)
            for (int j = i + 1; j < l; j++) {
                CActor * otherActor = actorHandler->m_actors [j];
                CVector vHit;
                int collision;
                if (otherActor->IsProjectile ())
                    collision = HandleProjectileActorCollision ((CProjectile*) otherActor, thisActor, vHit);
                else
                    collision = HandleActorActorCollision (thisActor, otherActor, vHit);
                if (vHit.IsValid ()) {
                    collisions += collision; // projectile collision will return zero here because it doesn't require another collision resolution loop
                    if (thisActor->IsProjectile () || otherActor->IsProjectile ())
                        soundHandler->Play ("hit", vHit, 0.25f, 0, nullptr, 2);
                    else if (thisActor->MayPlayActorHitSound ())
                        soundHandler->Play ("collide", vHit, 0.25f, 0, nullptr, 2);
                }
            }
        }
        return collisions;
    }



    // handle all collisions of actors with static geometry (the map)
    int CPhysicsHandler::HandleMapCollisions (void) {
        int collisions = 0;
        for (auto [i, actor] : actorHandler->m_actors) {
            CVector vHit;
            int collision;
            if (actor->IsProjectile ())
                collision = HandleProjectileMapCollision ((CProjectile*) actor, vHit);
            else
                collision = HandleActorMapCollision (actor, vHit);
            if (vHit.IsValid ()) {
                collisions += collision;
                if (actor->IsProjectile ())
                    soundHandler->Play ("wallhit", vHit, 0.25f, 0, nullptr, 2);
                else if (actor->MayPlayWallHitSound ())
                    soundHandler->Play ("collide", vHit, 0.25f, 0, nullptr, 2);
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
    bool CPhysicsHandler::HandleCollisions (void) {
        StartCollisionHandling ();
        for (int i = 0; i < 10; i++) {
            // handle actor/actor collisions first
            int collisions = HandleActorCollisions ();
            // now handle map collisions. 
            collisions += HandleMapCollisions ();
            if (collisions == 0)
                break;
        }
        return true;
    }


    // Update viewer position and orientation
    void CPhysicsHandler::UpdateViewer (void) {
        float dt = float (m_updateTimer.m_lapTime) / float (m_frameTime);
        controlsHandler->ComputeSpeedScale (float (m_fps));
        gameItems->m_viewer->Update (dt, controlsHandler->m_angles, controlsHandler->m_offset);
        controlsHandler->Ramp ();

        if (controlsHandler->m_fire) {
            controlsHandler->m_fire = false;
            if (gameItems->m_viewer->ReadyToFire ()) {
                gameItems->m_viewer->Fire ();
                soundHandler->Play ("laser", gameItems->m_viewer->GetPosition (), 0.25f, 0, nullptr, 1);
            }
        }
    }


    // update actor states and effects (disappearance, reappearance, respawning)
    void CPhysicsHandler::UpdateActors (void) {
        float dt = float (m_updateTimer.m_lapTime) / float (m_frameTime);
        for (auto [i, a] : actorHandler->m_actors) {
            if (!a->IsViewer())
                a->Update (dt);
            if (a->HavePosition ()) {  // will happen during multiplayer games, when a new player has joined but hasn't got a position yet
                a->UpdateSound ();
                if (a->m_animation == 1) {
                    a->m_animation = 0;
                    soundHandler->Play ("disappear", a->GetPosition (), 0.25f, 0, a, 3);
                }
                else if (a->m_animation == 2) {
                    a->m_animation = 0;
                    soundHandler->Play ("reappear", a->GetPosition (), 0.25f, 0, a, 3);
                }
            }
        }
    }


    void CPhysicsHandler::UpdateMovement (void) {
        if (m_updateTimer.HasPassed (m_frameTime, true)) {
            UpdateViewer ();
            UpdateActors ();
        }
    }

CPhysicsHandler* physicsHandler = nullptr;

// =================================================================================================
