#pragma once

#include <string.h>
#include "projectile.h"
#include "gameData.h"
#include "gameItems.h"
#include "networkHandler.h"

// =================================================================================================
// Handling of shots. Shots have straight movement at a fixed speed.
// If they hit something (other actor or wall), they disappear

CProjectile::CProjectile(int id) 
    : CActor(CString("projectile"), 1, false) 
{
    m_id = id;
    m_speed = 0.02f;
    m_offset = CVector(0, 0, 0);
    m_outline = nullptr;
    m_frozenTime = 0;
}


void CProjectile::Create(CActor* parent) {
    m_parent = parent;
    if (!m_parent->GetProjectileMesh ())
        m_mesh = parent->m_mesh;
    else {
        m_mesh = parent->GetProjectileMesh ();
        SetTexture(parent->GetTexture());
        m_speed = gameData->m_projectileSpeed;
        m_camera = parent->m_camera;
        m_camera.m_parent = &parent->m_camera;
        m_camera.m_name = m_parent->GetColor () + "projectile" + CString (size_t (m_id));
        m_camera.m_isViewer = false;
        m_offset = m_camera.m_orientation.Unrotate(CVector(0, 0, parent->m_camera.GetSize() / 2));
        m_camera.SetPosition(m_camera.GetPosition() - m_offset);
        m_offset *= m_speed / parent->m_camera.GetSize () * 2;
        m_camera.BumpPosition();
        m_camera.SetSize(gameData->m_projectileSize);
    }
}


void CProjectile::Update(float dt, CVector angles, CVector offset) {
    if (IsDead())
        m_delete = true;
    else if (IsLocalActor()) {  // in multiplayer games, only move the projectiles fired by the local player
        m_camera.SetPosition(m_camera.GetPosition() - m_offset * dt);
        m_camera.UpdateAngles(CVector(0, 30, 0));
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

void CProjectile::Delete(bool force) {
    if (force || IsLocalActor())
        networkHandler->BroadcastDestroy(this);
    SetHitPoints(0);
    m_delete = true;
}


void CProjectile::Render(bool bAutoCamera) {
    // print ("Projectile @ {:1.4f} {:1.4f}".format (m_GetPosition ().x, m_GetPosition ().z))
    m_mesh->PushColor(m_parent->GetColorValue());
    CActor::Render();
    if (m_outline)
        m_outline->Render(m_camera.GetSize());
    m_mesh->PopColor();
}


void CProjectile::UpdateFrozenTime(void) {
    if (GetPosition(0) != GetPosition(1))
        m_frozenTime = 0;
    else {
        if (m_frozenTime == 0)
            m_frozenTime = gameData->m_gameTime;
        else if (gameData->m_gameTime - m_frozenTime > gameData->m_frozenTimeout)
            Delete();
    }
}

// =================================================================================================
