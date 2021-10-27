#include "gameItems.h"
#include "maploader.h"
#include "actorHandler.h"
#include "argHandler.h"
#include "renderer.h"

// =================================================================================================

void CGameItems::Create (void) {

    CreateMap ();
    // the viewer (local player). He controls the projection, i.e. everything is rendered #include "the perspective of the viewer
    m_viewer = actorHandler->CreateViewer ();
    m_viewer->SetupCamera ("viewer", 1.0, CVector (NAN, NAN, NAN), CVector (0, 0, 0));
    m_viewer->ForceRespawn ();
    actorHandler->SetViewer (m_viewer);
    renderer->SetViewer (m_viewer);

    // player reticle
    m_reticle = CReticle ();
    m_reticle.Create ();

    // create some player spheres for testing. These have no function besides being there, being targets, and respawning
    int dummies = argHandler->IntVal ("dummies", 0, 0);
    if (dummies > 0) {
        auto min = [] (auto a, auto b) { (a < b) ? a : b; };
        for (int i = min (dummies, int (actorHandler->m_maxPlayers) - 1); i; i--) {
            CPlayer* dummy = actorHandler->CreatePlayer ();
            dummy->SetType ("dummy");
        }
    }
}


bool CGameItems::CreateMap (void) {
    if (m_map)
        delete m_map;
    m_map = new CMap();
    CString mapName = argHandler->StrVal ("map", 0, "standard.txt");
    if (!CMapLoader (m_map).CreateFromFile (mapName, m_map->m_stringMap)) {
        fprintf (stderr, "Couldn't load map '%s'\n", mapName.Buffer ());
        exit (1);
    }
    return false;
}


bool CGameItems::CreateMap (CList<CString> stringMap, bool isPrepared) {
    if (m_map)
        delete m_map;
    if (!(m_map = new CMap)) {
        fprintf (stderr, "Couldn't load map (out of memory)\n");
        exit (1);
    }
    m_map->m_stringMap = stringMap;
    if (!CMapLoader (m_map).CreateFromMemory (m_map->m_stringMap, isPrepared)) {
        fprintf (stderr, "Couldn't load map\n");
        exit (1);
    }
    return false;
}


CGameItems* gameItems = nullptr;

// =================================================================================================

