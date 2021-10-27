#pragma once

#include "cstring.h"
#include "clist.h"
#include "vector.h"
#include "reticle.h"
#include "map.h"
#include "actor.h"
#include "player.h"
#include "actorHandler.h"

// =================================================================================================

class CGameItems {
public:
    CMap*       m_map;
    CViewer*    m_viewer;
    CReticle    m_reticle;

    CGameItems () : m_map (nullptr), m_viewer (nullptr) {}

    void Create (void);

    bool CreateMap (void);

    bool CreateMap (CList<CString> stringMap, bool isPrepared = false);

    inline void Destroy (void) {
        m_map->Destroy ();
        m_reticle.Destroy ();
    }

    inline void Cleanup (void) {
        actorHandler->Cleanup ();
    }

    inline float ViewerDistance (CVector p) {
        return m_map->Distance (m_viewer->GetPosition (), p);
    }

};

extern CGameItems* gameItems;

// =================================================================================================

