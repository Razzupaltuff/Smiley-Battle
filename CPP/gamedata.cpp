#include <stdlib.h>

#include "gameData.h"
#include "textureHandler.h"
#include "argHandler.h"

// =================================================================================================

CGameData::CGameData() {
    m_resourceFolder = "resources\\";
    m_textureFolder = m_resourceFolder + "textures\\";
    m_soundFolder = m_resourceFolder + "sounds\\";
    m_mapFolder = "maps\\";

    m_colorValues.SetComparator(CString::Compare);
    m_colorIndices.SetComparator(CString::Compare);
    m_textures.SetComparator(CString::Compare);

    m_playerColors = {
        CString("white"), CString("black"), CString("yellow"), CString("gold"),
        CString("orange"), CString("red"), CString("darkred"), CString("pink"),
        CString("purple"), CString("lightgreen"), CString("darkgreen"), CString("lightblue"),
        CString("blue"), CString("darkblue"), CString("lightgray"), CString("darkgray")
    }; // "dead-black", "dead-white"]
    m_colorValues = {
        std::pair<CString, CVector>{ CString("white")     , CVector(255, 255, 255) / 255.0f },
        std::pair<CString, CVector>{ CString("black")     , CVector(  0,   0,   0),         },
        std::pair<CString, CVector>{ CString("yellow")    , CVector(255, 255,   0) / 255.0f },
        std::pair<CString, CVector>{ CString("gold")      , CVector(255, 217,   0) / 255.0f },
        std::pair<CString, CVector>{ CString("orange")    , CVector(255, 140,   0) / 255.0f },
        std::pair<CString, CVector>{ CString("red")       , CVector(217,  26,   0) / 255.0f },
        std::pair<CString, CVector>{ CString("darkred")   , CVector(128,  26,   0) / 255.0f },
        std::pair<CString, CVector>{ CString("pink")      , CVector(255, 127, 255) / 255.0f },
        std::pair<CString, CVector>{ CString("purple")    , CVector(153,   0, 255) / 255.0f },
        std::pair<CString, CVector>{ CString("lightgreen"), CVector(102, 229,   0) / 255.0f },
        std::pair<CString, CVector>{ CString("darkgreen") , CVector(  0, 153,  38) / 255.0f },
        std::pair<CString, CVector>{ CString("lightblue") , CVector(  0, 204, 255) / 255.0f },
        std::pair<CString, CVector>{ CString("blue")      , CVector(  0, 128, 255) / 255.0f },
        std::pair<CString, CVector>{ CString("darkblue")  , CVector(  0,  26, 255) / 255.0f },
        std::pair<CString, CVector>{ CString("lightgray") , CVector(172, 176, 176) / 255.0f },
        std::pair<CString, CVector>{ CString("darkgray")  , CVector(112, 112, 112) / 255.0f }
    }; // "dead-black", "dead-white"]

    for (auto [i, s] : m_playerColors) {
        m_availableColors.Append(int (i));
        m_colorIndices.Insert(*m_playerColors[i], int (i));
    }
    m_playerMoods = { CString("-sad"),  CString("-neutral"),  CString("-happy") };
    CreatePlayerTextures();

    m_fireMode = argHandler->IntVal ("firemode", 0, 0);
    m_fireDelay = argHandler->IntVal("firedelay", 0, 250);                  // limit fire rate to one short per 500 ms (2 shots/s)
    m_healDelay = argHandler->IntVal("healdelay", 0, 5000);
    m_respawnDelay = argHandler->IntVal("respawndelay", 0, 5000);           // time [ms] between disappearing and reappearing after death
    m_immunityDuration = argHandler->IntVal("immunityduration", 0, 3000);   // duration [ms] of immunity after having respawned to allow for reorientation
    m_frozenTimeout = 5000;
    m_projectileSpeed = argHandler->FloatVal("projectilespeed", 0, 0.2f);
    auto min = [](auto a, auto b) { return (a < b) ? a : b; };
    m_projectileSize = min(1.0f, argHandler->FloatVal("projectilesize", 0, 0.3f));
    m_wigglePlayers = argHandler->BoolVal("wiggleplayers", 0, false);
    m_wiggleViewer = argHandler->BoolVal("wiggleviewer", 0, false);
    m_pointsForKill = argHandler->IntVal("pointsforkill", 0, 1);

    m_gameTime = SDL_GetTicks();
    m_frameCap = argHandler->IntVal ("framecap", 0, 240); // fps
    m_minFrameTime = m_frameCap ? 1000 / m_frameCap : 0;
    m_isNetGame = false;
    m_suspend = false;
    m_run = true;
}


void CGameData::CreatePlayerTextures(void) {
    for (auto [i, color] : m_playerColors) {
        for (auto [j, mood] : m_playerMoods) {
            CString skinName = m_textureFolder + "smiley-" + *color + ".png";
            CString faceName = m_textureFolder + "smileyface-" + *color + *mood + ".png";
            CString noFile = CString ("");
            CCubemap* texture = textureHandler->GetCubemap();
            CList<CString> fileNames = { skinName, noFile, noFile, noFile, noFile, faceName };
            texture->CreateFromFile(fileNames, false);
            // create test texturing with different colors on each side of a sphere
            // texture.CreateFromFile ([m_textureFolder + "smiley-white.png", 
            //                          m_textureFolder + "smiley-lightgreen.png", 
            //                          m_textureFolder + "smiley-red.png", 
            //                          m_textureFolder + "smiley-blue.png", 
            //                          m_textureFolder + "smiley-gold.png", 
            //                          m_textureFolder + "smileyface-black-happy.png"], 
            //                         false)
            m_textures.Insert(color + mood, texture);
        }
    }
}


bool CGameData::GetPlayerColorValue (CPlayer* player, CVector& colorValue, CString& color, bool whiteForBlack) {
    color = GetColor(player->GetColorIndex());
    if (whiteForBlack) {
        if (color == "black")
            color = "white";
        else
            whiteForBlack = false;
    }
    CVector * cv = m_colorValues.Find(color);
    colorValue = cv ? *cv : CVector (1,1,1);
    return whiteForBlack;
}


// randomly select a color index #include "the available color indices
int CGameData::GetColorIndex(void) {
    // return "white"
    if (m_availableColors.Empty())
        return -1;
    return m_availableColors.Pop(rand() % m_availableColors.Length());
}


int CGameData::ReplaceColorIndex(int oldIndex, int newIndex) {
    if (newIndex != oldIndex) {
        ReturnColorIndex(oldIndex);
        RemoveColorIndex(newIndex);
    }
    return newIndex;
}

CGameData* gameData = nullptr;

// =================================================================================================

