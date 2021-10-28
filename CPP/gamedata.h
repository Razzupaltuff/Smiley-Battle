#pragma once

#include <stdlib.h>

#include "cstring.h"
#include "clist.h"
#include "cavltree.h"

#include "vector.h"
#include "cubemap.h"
#include "player.h"

// =================================================================================================

class CGameData {
    public:
        CString         m_resourceFolder;
        CString         m_textureFolder;
        CString         m_soundFolder;
        CString         m_mapFolder;
        CList<CString>  m_playerColors;
        CList<int>      m_availableColors;
        CList<CString>  m_playerMoods;

        CAvlTree<CString, int>       m_colorIndices;
        CAvlTree<CString, CVector>   m_colorValues;
        CAvlTree<CString, CTexture*> m_textures;

        int             m_fireMode;
        int             m_fireDelay;
        int             m_healDelay;
        int             m_respawnDelay;
        int             m_immunityDuration;
        int             m_frozenTimeout;
        float           m_projectileSpeed;
        float           m_projectileSize;
        bool            m_wigglePlayers;
        bool            m_wiggleViewer;
        int             m_pointsForKill;
        int             m_gameTime;
        int             m_frameCap;
        int             m_minFrameTime;
        bool            m_isNetGame;
        bool            m_suspend;
        bool            m_run;


        CGameData();

        void CreatePlayerTextures(void);

        inline CString GetColor(int colorIndex) {
            return (colorIndex >= 0) ? *m_playerColors[colorIndex] : CString("");
        }

        inline CVector GetColorValue(CString color) {
            CVector* colorValue = m_colorValues.Find(color);
            return colorValue ? *colorValue : CVector(1, 1, 1);
        }

        bool GetPlayerColorValue(CPlayer* player, CVector& colorValue, CString& color, bool whiteForBlack = false);

        // randomly select a color index #include "the available color indices
        int GetColorIndex(void);

        inline void RemoveColorIndex(int colorIndex) {
            if (colorIndex >= 0)
                m_availableColors.Pop(colorIndex);
        }

        inline void ReturnColorIndex(int colorIndex) {
            if (colorIndex >= 0) 
                m_availableColors.Append(colorIndex);
        }

        inline bool ColorIsAvailable(int colorIndex) {
            return (m_availableColors.Find(colorIndex) >= 0);
        }

        int ReplaceColorIndex(int oldIndex, int newIndex);

};

extern CGameData* gameData;

// =================================================================================================

