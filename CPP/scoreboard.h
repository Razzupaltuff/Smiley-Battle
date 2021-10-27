#pragma once

#include "cstring.h"
#include "clist.h"
#include "gamedata.h"
#include "texturehandler.h"
#include "renderer.h"
#include "actorhandler.h"
#include "arghandler.h"
#include "quad.h"

// =================================================================================================

class CScoreBoard {
    public:
        CList<CString>      m_textureNames;
        CList<CTexture*>    m_textures;
        CList<CTexture*>    m_digitTextures;
        CList<CQuad>        m_digitQuads;
        CQuad               m_statusBackground;
        CQuad               m_statusSmiley;
        bool                m_coloredScore;

        CScoreBoard ();

        void Create (void);

        void CreateDigitTextures (void);

        void CreateStatusBackground (void);

        void CreateStatusSmiley (void);

        CQuad* CreateDigitQuad (float l, float w, float h, CQuad* q);

        void CreateDigitQuads (void);

            // The status icon is painted by first drawing a rectangle in the corresponding player's color,
            // then masking the corners to create a colored circle and finally drawing a smiley face on top of it.
            // If the player is currently dead, a strikeout will be painted over it
            // Black smiley get a circular mask with a white border to make them visible against the black status area background
        void RenderStatus (CPlayer* player, int position);

        int Pot10 (int i);

        void RenderScore (int position, CPlayer* player, int score);

        void RenderViewerStatus (void);

        void RenderPlayerStatus (void);

        void RenderViewerScores (void);

        void RenderPlayerScores (void);

        void Render (void);

};
 
extern CScoreBoard * scoreBoard;

// =================================================================================================
