#pragma once

#include "gameItems.h"
#include "actorHandler.h"

// =================================================================================================

CActorHandler::CActorHandler() {
    m_viewer = nullptr;
    // two sphere meshes which will be used where ever a sphere is needed. Sphere texturing && sizing is dynamic to allow for reuse.
    m_playerSphere.Create(4);
    m_projectileSphere.Create(3);
    // player shadow && outline
    m_playerShadow.Create();
    m_playerHalo.Create(5, 0.2f, 0.02f);
    m_playerOutline.Create(&m_playerSphere);
    m_maxPlayers = gameData->m_playerColors.Length();
    m_actorId = 0;
}


void CActorHandler::Destroy(void) {
    for (auto [i, a] : m_actors) {
        a->Destroy ();
        delete a;
    }
    m_playerShadow.Destroy();
    m_playerHalo.Destroy();
}


CPlayer* CActorHandler::CreatePlayer(int colorIndex, CVector position, CVector orientation, CString address, uint16_t inPort, uint16_t outPort) {
    if ((colorIndex < 0) || !gameData->ColorIsAvailable(colorIndex)) {
        colorIndex = gameData->GetColorIndex();
        if (colorIndex < 0)
            return nullptr;
    }
    else
        gameData->RemoveColorIndex(colorIndex);
    CPlayer* player = new CPlayer("player", colorIndex, &m_playerShadow, &m_playerHalo, &m_playerOutline);
    uint16_t ports[2] = { inPort, outPort };
    player->SetAddress(address, ports);
    player->Create(gameData->GetColor(colorIndex) + " player", &m_playerSphere, 0, gameData->m_textures, CList<CString>(), position, orientation, 1.0, &m_viewer->m_camera);
    player->SetColorIndex(colorIndex);
    player->SetProjectileMesh (&m_projectileSphere);
    m_actors.Append(player);
    return player;
}


CViewer* CActorHandler::CreateViewer(void) {
    CViewer* m_viewer = new CViewer ();
    m_viewer->SetColorIndex (gameData->GetColorIndex ());
    m_viewer->SetupTextures(gameData->m_textures);
    m_viewer->SetMesh(&m_playerSphere);
    m_viewer->SetProjectileMesh(&m_projectileSphere);
    m_actors.Append(m_viewer);
    return m_viewer;
}


CProjectile* CActorHandler::CreateProjectile(CPlayer* parent, int id) {
    if (id < 1)
        id = GetActorId();
    CProjectile * projectile = new CProjectile (id);
    projectile->Create(parent);
    m_actors.Append(projectile);
    return projectile;
}


CActor* CActorHandler::CreateActor (int id, int colorIndex, CVector& position, CVector& orientation) {
    if (id == 0)
        return (CActor*) CreatePlayer(colorIndex, position, orientation);
    CPlayer* parent = FindPlayer(colorIndex);
    return parent ? CreateProjectile(parent, id) : nullptr;
    }


bool CActorHandler::DeletePlayer(int colorIndex) {
    CPlayer* player = FindPlayer(colorIndex);
    if (!player)
        return false;
    if (player->IsViewer()) {
        fprintf (stderr, "Trying to delete local player\n");
        return false;
    }
    soundHandler->StopActorSounds(player);
    gameData->ReturnColorIndex(colorIndex);
    // delete all child objects (projectiles) of this player
    for (auto [i, a] : m_actors)
        if (a->GetColorIndex() == colorIndex)
            a->Delete();
    player->Delete();
    return true;
}


bool CActorHandler::DeleteActor(int id, int colorIndex) {
    if (id == 0)
        return DeletePlayer(colorIndex);
    CActor* actor = FindActor(id, colorIndex);
    if (!actor)
        return false;
    actor->Delete();
    return true;
}


CActor* CActorHandler::FindActor(int id, int colorIndex) {
    for (auto [i, a] : m_actors)
        if ((a->GetId() == id) && (a->GetColorIndex() == colorIndex))
            return a;
    return nullptr;
}


CProjectile* CActorHandler::FindProjectile (int colorIndex) {
    for (auto [i, a] : m_actors)
        if (a->IsProjectile () && (a->GetColorIndex () == colorIndex))
            return (CProjectile*) a;
    return nullptr;
}


void CActorHandler::CleanupActors (void) {     // required when the local player disconnected && needs to rejoin #include "a clean slate
    for (auto [i, a] : m_actors)
        if ((a->GetId() != 0) || (a->GetColorIndex() != m_viewer->GetColorIndex())) 
            a->Delete();
}


void CActorHandler::Cleanup(void) {
    for (size_t i = m_actors.Length(); i; --i ) {
        CActor* a = m_actors[i];
        if (a->m_delete) {
            if (a->IsViewer())
                fprintf (stderr, "Trying to delete local player\n");
            else {
                m_actors.Pop (int (i));
                delete a;
            }
        }
        else {
            CVector p = a->GetPosition();
            if (p.IsValid() && !gameItems->m_map->Contains(p.X(), p.Z()))
                a->SetHitPoints(0);  // will delete projectiles && respawn players at a valid position
        }
    }
}


CActorHandler* actorHandler = nullptr;

// =================================================================================================

