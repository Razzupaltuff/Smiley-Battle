#include "actor.h"
#include "gameData.h"
#include "gameItems.h"
#include "networkHandler.h"
#include "effectHandler.h"
#include "soundHandler.h"

// =================================================================================================
// Basic game object with physical properties. Can be mobile or stationary, but basically is everything 
// inside a map that is not a map and not an effect (which usually don't have physical properties relevant
// for game events, like collisions or hits)
// Contains basic handling of collisions with other actors or the map

CActor::CActor(CString type, int hitPoints, bool isViewer) {
    m_stationary = false;
    m_isViewer = isViewer;
    m_type = type;
    m_timeOfDeath = 0;
    m_hitter = nullptr;
    m_angles = CVector(0, 0, 0);
    m_offset = CVector(0, 0, 0);
    m_size = 1.0f;
    m_scale = 1.0f;
    m_maxHitPoints = hitPoints;
    m_hitPoints = hitPoints;
    m_hitTime = 0;
    m_healTime = 0;
    m_deathTime = 0;
    m_hiddenTime = 0;
    m_respawnTime = 0;
    m_immuneTime = 0;
    m_animationDuration = 750;    // duration [ms] of the shrinking and growing animations after death and when respawning
    m_hitSoundTime[0] =
    m_hitSoundTime[1] = 0;
    m_hitSoundDelay = 1000;       // minimal time [ms] between two wall hit sounds to avoid the sounds to stutter during frequent wall contact
    m_hitEffectTime = 750;       // duration [ms] of taking on the hitter's color after a hit
    m_soundId = -1;
    m_lifeState = lsAlive;
    m_lifeStateHandlers = { &CActor::Die, &CActor::Disappear, &CActor::Bury, &CActor::Hide, &CActor::Resurrect, &CActor::Reappear, &CActor::Immunize, &CActor::Protect };
    m_animation = 0;
    m_delete = false;
    m_needPosition = false;
}

// set a mesh (shape), texture, position and initial spatial orientation of the actor
void CActor::Create(CString name, CMesh * mesh, int quality, CTexture* texture, CList<CString> textureNames, CVector position, CVector angles, float size, CCamera *parent) {
    // global gameData
    m_size = size;
    SetupTextures(texture, textureNames);
    SetupMesh(mesh, quality, m_texture, textureNames);
    SetupCamera(name, size, position, angles, parent);
    m_respawnTime = gameData->m_gameTime;
    ForceRespawn();
}


void CActor::Destroy(void) {
    // m_mesh->Destroy();
}

bool CActor::SetupTextures (CTexture * texture, CList<CString> textureNames) {
    m_texture = texture;
    if (!textureNames.Empty())
        m_texture->CreateFromFile(textureNames);
    return true;
    }


void CActor::SetupMesh (CMesh * mesh, int quality, CTexture * texture, CList<CString>& textureNames) {
    m_mesh = mesh;
    if (quality > 0) 
        m_mesh->Create(quality, texture, textureNames);
    }


void CActor::SetupCamera(CString name, float size, CVector position, CVector angles, CCamera * parent) {
    m_camera = CCamera(name, m_isViewer);
    m_camera.UpdateAngles(angles);
    m_camera.SetPosition(position);
    m_camera.SetSize(size);
    m_camera.SetParent(parent);
}


void CActor::SetPosition (CVector position) {
    m_camera.SetPosition(position);
    if (!m_camera.HavePosition(1))
        m_camera.SetPosition(position, 1);
    m_needPosition = false;
    }


void CActor::Render(bool autoCamera) {
    if (m_isViewer)
        return;
    if (autoCamera)
        EnableCamera();
    glPushMatrix();
    float size = m_camera.m_size / BorderScale();
    glScalef(size, size, size);
    if (!m_hitter)
        m_mesh->PushTexture(m_texture);
    else {
        m_mesh->PushTexture(m_hitter->GetTexture(Mood()));
        m_mesh->PushColor(m_hitter->GetPlayerColorValue());
    }
    m_mesh->Render();
    m_mesh->PopTexture();
    if (m_hitter)
        m_mesh->PopColor();
    glPopMatrix();
    if (autoCamera)
        DisableCamera();
}


bool CActor::MayPlayHitSound (size_t type) {
    if (gameData->m_gameTime < m_hitSoundTime[type])
        return false;
    m_hitSoundTime[type] = gameData->m_gameTime + m_hitSoundDelay;
    return true;
    }


// called when this actor has been hit by shot
void CActor::RegisterHit(CActor* hitter) {
    if (m_hitPoints > 0) {
        SetHitPoints(m_hitPoints - 1, hitter);
        if (IsLocalActor() && hitter)
            networkHandler->BroadcastHit(this, hitter);
    }
}


void CActor::SetAnimation(int animation) {
    m_animation = animation;
    if (m_isViewer)
        networkHandler->BroadcastAnimation();
}


// set hit points. If hit by a projectile, the player who had fired that projectile is passed in hitter
// That player will receive a point for hitting and another point if killing the current actor (which in
// this game will always be another player)
// start death animation if player is killed
void CActor::SetHitPoints (int hitPoints, CActor* hitter) {
    // global gameData
    if (m_hitPoints == hitPoints)
        return;
    m_hitPoints = hitPoints;
    m_hitTime = gameData->m_gameTime;
    m_healTime = 0;       // restart healing process when hit
    if (hitter)
        hitter->AddScore(1); // one point for the hit
    m_hitter = hitter;
    if (m_hitPoints == 0) {
        if (m_id == 0) {
            AddDeath();
            if (hitter) {
                hitter->AddScore(gameData->m_pointsForKill);         // another point for the kill
                hitter->AddKill();
            }
        }
        m_lifeState = lsDie;
    }
}


// The following functions are part of the death and respawn handling state engine
// They set the timeouts for the various effects and delays and make sure the player
// gets a new spawn position before reappearing
// In multiplayer matches, each client only handles death and respawning for itself
// and transmits his current status to the other players with UPDATE messages
void CActor::Die(void) {
    SetAnimation(1);
    if (this == actorHandler->m_viewer) 
        effectHandler->StartFade(CVector(0, 0, 0), m_animationDuration, false);
    m_deathTime = m_hitTime;   // start death animation
    m_lifeState = lsDisappear;
}


void CActor::Disappear(void) {
    size_t dt = gameData->m_gameTime - m_deathTime;
    if (dt <= m_animationDuration)
        m_scale = 1.0f - float (dt) / float (m_animationDuration);
    else
        m_lifeState = lsBury;
}


void CActor::Bury(void) {
    m_hitter = nullptr;
    m_hiddenTime = gameData->m_gameTime + gameData->m_respawnDelay;
    m_scale = 0.0f;
    m_lifeState = lsHide;
}


void CActor::Hide(void) {
    if (gameData->m_gameTime >= m_hiddenTime) {
        SetAnimation(2);
        if (IsLocalActor())
            gameItems->m_map->FindSpawnPosition(this);
        else
            m_needPosition = true;
        m_lifeState = lsResurrect;
    }
}


// start respawn animation. Signal request for a spawn position to the app
// start immunity period
void CActor::Resurrect(void) {
    if (m_needPosition)     // don't respawn without a valid spawn position
        return;
    m_respawnTime = gameData->m_gameTime + m_animationDuration;
    if (this == actorHandler->m_viewer)
        effectHandler->StartFade(CVector(0, 0, 0), m_animationDuration, true);
    m_lifeState = lsReappear;
}


void CActor::Reappear(void) {
    int dt = int (m_respawnTime) - int (gameData->m_gameTime);
    if (dt > 0)
        m_scale = float (m_animationDuration - dt) / float (m_animationDuration);
    else {
        m_scale = 1.0f;
        m_lifeState = lsImmunize;
    }
}


void CActor::Immunize(void) {
    m_immuneTime = gameData->m_gameTime + gameData->m_immunityDuration;
    m_lifeState = lsProtect;
}


void CActor::Protect (void) {
    if (gameData->m_gameTime > m_immuneTime) {
        m_hitPoints = m_maxHitPoints;
        m_lifeState = lsAlive;
    }
}


// The state engine driver
void CActor::UpdateLifeState(void) {
    eLifeStates oldState = lsAlive;
    while ((m_lifeState >= 0) && (oldState != m_lifeState)) {
        oldState = m_lifeState;
        (this->*m_lifeStateHandlers[m_lifeState])();
    }
}


void CActor::ForceRespawn(void) {
    m_hitPoints = 0;
    m_hiddenTime = 0;
    m_lifeState = lsHide;
}


// check whether the current actor is dead
bool CActor::IsLocalActor(void) {
    if (networkHandler->m_localAddress == "127.0.0.1")
        return true;
    return (GetColorIndex() == actorHandler->m_viewer->GetColorIndex ()) ||
           (GetAddress() == networkHandler->m_localAddress) ||
           (GetAddress() == "127.0.0.1");
}


// heal the actor until it has full health or gets hit. Wait gameData->healDelay ms for healing by one hitpoint
// don't heal when dead
void CActor::Heal(void) {
    // global gameData
    if (m_hitPoints == 0)
        return;
    if (m_hitPoints == m_maxHitPoints) {
        m_healTime = 0;
        return;
    }
    if (m_healTime == 0) {
        m_healTime = gameData->m_gameTime;    // start over
        return;
    }
    else if (gameData->m_gameTime - m_healTime < gameData->m_healDelay)
        return;
    m_hitPoints++;
    m_healTime = 0;
}


void CActor::UpdateSound(void) {
    // global soundHandler
    if (m_id != 0)
        return;
    if (IsAlive() || IsImmune()) {
        if (!m_isViewer && (m_soundId < 0))
            m_soundId = soundHandler->Play("hum", GetPosition(), 0.1f, -1, this, 4);
    }
    else {
        if (m_soundId >= 0) {
            soundHandler->Stop(m_soundId);
            m_soundId = -1;
        }
    }
}


// update actor status (death animation, respawn animation, healing)
void CActor::Update(float dt, CVector angles, CVector offset) {
    // global gameData
    if (IsLocalActor()) {
        UpdateLifeState();
        Heal();
    }
    if (m_hitter == nullptr)
        return;
    if (gameData->m_gameTime - m_hitTime < m_hitEffectTime)
        return;
    m_hitter = nullptr;
    m_hitTime = 0;
}

// =================================================================================================
