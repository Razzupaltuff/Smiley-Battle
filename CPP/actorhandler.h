#pragma once

#include <math.h>
#include "icosphere.h"
#include "actor.h"
#include "player.h"
#include "projectile.h"
#include "gameData.h"
#include "soundHandler.h"

// =================================================================================================

class CActorHandler {
    public:
        CRectangleIcoSphere     m_playerSphere;
        CRectangleIcoSphere     m_projectileSphere;
        CPlayerShadow           m_playerShadow;
        CPlayerHalo             m_playerHalo;
        CPlayerOutline          m_playerOutline;
        CViewer *               m_viewer;
        CList<CActor*>          m_actors;
        CList<CVector>          m_colorPool;
        size_t                  m_maxPlayers;
        int                     m_actorId;


        CActorHandler();

        void Destroy(void);

        inline int GetActorId(void) {
            return ++m_actorId; // zero is reserved for players
        }


        CPlayer* CreatePlayer(int colorIndex = -1, CVector position = CVector(NAN, NAN, NAN), CVector orientation = CVector(0, 0, 0), CString address = "127.0.0.1", uint16_t inPort = 0, uint16_t outPort = 0);

        CViewer* CreateViewer(void);

        CProjectile* CreateProjectile(CPlayer* parent, int id = -1);

        CActor* CreateActor(int id, int colorIndex, CVector& position, CVector& orientation);

        bool DeletePlayer(int colorIndex);

        bool DeleteActor(int id, int colorIndex);

        inline void SetViewer (CViewer* viewer) {
            m_viewer = viewer;
        }

        inline CPlayer* FindPlayer(int colorIndex) {
            return (CPlayer*)FindActor(0, colorIndex);
        }

        CActor* FindActor(int id, int colorIndex);

        void CleanupActors(void);

        void Cleanup (void);

        inline size_t PlayerCount(void) {
            return m_maxPlayers - gameData->m_availableColors.Length();
        }

};

extern CActorHandler* actorHandler;

// =================================================================================================

