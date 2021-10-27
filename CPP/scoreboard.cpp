#include "scoreboard.h"
#include "gamedata.h"
#include "texturehandler.h"
#include "renderer.h"
#include "actorhandler.h"
#include "arghandler.h"

// =================================================================================================

CScoreBoard::CScoreBoard () {
    m_textureNames = {
        "smileyface-mask-black-sad.png", "smileyface-mask-black-neutral.png", "smileyface-mask-black-happy.png", "smiley-mask-black.png",
        "smileyface-mask-white-sad.png", "smileyface-mask-white-neutral.png", "smileyface-mask-white-happy.png", "smiley-mask-white.png",
        "smiley-strikeout-red.png", "smiley-strikeout-yellow.png"
    };
    Create ();
    CreateDigitTextures ();
    CreateStatusBackground ();
    CreateStatusSmiley ();
    CreateDigitQuads ();
    m_coloredScore = argHandler->BoolVal ("coloredscore", 1, "0");
}


void CScoreBoard::Create (void) {
    m_textures = textureHandler->CreateTextures (m_textureNames);
}


void CScoreBoard::CreateDigitTextures (void) {
    const char* digits[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
    for (int i = 0; i < sizeof (digits) / sizeof (*digits); i++) {
        CTexture* texture = textureHandler->GetTexture ();
        if  (!texture->CreateFromSurface (TTF_RenderText_Solid (renderer->m_scoreFont, digits [i], SDL_Color (224, 224, 224, 255))))
            continue;
        texture->Create ();
        texture->Deploy ();
        m_digitTextures.Append (texture);
    }
}


void CScoreBoard::CreateStatusBackground (void) {
    float border = 0.1f;
    m_statusBackground.Init ({ CVector (border, border, 0.0f), CVector (border, 1.0f - border, 0.0f), CVector (1.0f - border, 1.0f - border, 0.0f), CVector (1.0f - border, border, 0.0f) });
    m_statusBackground.Create ();
}


void CScoreBoard::CreateStatusSmiley (void) {
    float border = 0.2f;
    m_statusSmiley.Init ({ CVector (border, border, 0.0f), CVector (border, 1.0f - border, 0.0f), CVector (1.0f - border, 1.0f - border, 0.0f), CVector (1.0f - border, border, 0.0f) });
    m_statusSmiley.Create ();
}


CQuad* CScoreBoard::CreateDigitQuad (float l, float w, float h, CQuad* q) {
    float border = 0.0f; //w * 0.1f
    *q = { CVector (l + border, h + border, 0.0f), CVector (l + border, 1.0f - h - border, 0.0f), CVector (l + w - border, 1.0f - h - border, 0.0f), CVector (l + w - border, h + border, 0.0f) };
    q->Create ();
    return q;
}


void CScoreBoard::CreateDigitQuads (void) {
    CRenderer::tViewport vp = renderer->SetViewport ("score", 1);
    vp.width /= 5;     // 4 characters + one space
    CTexture* t = m_digitTextures [0];
    float l = 0;
    m_digitQuads.Destroy ();
    for (int i = 0; i < 4; i++) {
        CQuad* q = m_digitQuads.Add (-1);
        float cw = float (t->GetWidth ());
        float ch = float (t->GetHeight ());
        float ar = cw / ch;
        float h, w = ar * float (vp.height);
        if (w <= float (vp.width)) {
            w = 0.225f * w / float (vp.width);
            h = 1.0f;
        }
        else {
            w = 0.225f;
            h = float (vp.width) / ar;
            if (h > float (vp.height))
                w *= float (vp.height) / h;
            h = 1.0f;
        }
        CreateDigitQuad (l, w, (1.0f - h) / 2, q);
        l += w;
    }
}


// The status icon is painted by first drawing a rectangle in the corresponding player's color,
// then masking the corners to create a colored circle and finally drawing a smiley face on top of it.
// If the player is currently dead, a strikeout will be painted over it
// Black smiley get a circular mask with a white border to make them visible against the black status area background
void CScoreBoard::RenderStatus (CPlayer * player, int position) {
    renderer->SetViewport ("status", position);
    CVector colorValue;
    CString color;
    bool whiteForBlack = gameData->GetPlayerColorValue (player, colorValue, color);
    glDepthFunc (GL_ALWAYS);
    m_statusBackground.Fill (colorValue);
    int textureOffset = (color == "black") ? 4 : 0;
    m_statusBackground.SetTexture (m_textures [textureOffset + 3]);
    m_statusBackground.Render ();
    m_statusSmiley.SetTexture (m_textures [textureOffset + player->Mood ()]);
    m_statusSmiley.Render ();
    if (player->IsHidden ()) {
        if ((color == "red") || (color == "darkred"))
            m_statusSmiley.SetTexture (m_textures [9]);
        else
            m_statusSmiley.SetTexture (m_textures [8]);
        m_statusSmiley.Render ();
    }
    glDepthFunc (GL_LESS);
}



int CScoreBoard::Pot10 (int i) {
    int b = 1;
    while (b <= i)
        b *= 10;
    return b / 10;
}


void CScoreBoard::RenderScore (int position, CPlayer * player, int score) {
    if (position > 0)
        renderer->SetViewport ("score", position);
    bool whiteForBlack = true;
    CVector colorValue;
    CString color;
    if (!m_coloredScore)
        colorValue = CVector (1, 1, 1);
    else
        whiteForBlack = gameData->GetPlayerColorValue (player, colorValue, color, whiteForBlack);
    glDepthFunc (GL_ALWAYS);
    int b = Pot10 (score);
    if (b < 1000)
        b = 1000;
    for (int i = 0; b > 0; b /= 10, i++) {
        int d = score / b;
        m_digitQuads [i].SetTexture (m_digitTextures [d]);
        m_digitQuads [i].SetColor (colorValue);
        m_digitQuads [i].Render ();
        score %= b;
    }
    glDepthFunc (GL_LESS);
}


void CScoreBoard::RenderViewerStatus (void) {
    RenderStatus (actorHandler->m_viewer, 0);
}


void CScoreBoard::RenderPlayerStatus (void) {
    int position = 0;
    for (auto [i, a] : actorHandler->m_actors)
        if (a->IsPlayer () && !a->IsViewer())
            RenderStatus ((CPlayer*) a, ++position);
}


void CScoreBoard::RenderViewerScores (void) {
    renderer->SetViewport ("score", 0);
    RenderScore (-1, actorHandler->m_viewer, actorHandler->m_viewer->m_score);
    renderer->SetViewport ("kills", 0);
    RenderScore (-1, actorHandler->m_viewer, actorHandler->m_viewer->m_kills);
    renderer->SetViewport ("deaths", 0);
    RenderScore (-1, actorHandler->m_viewer, actorHandler->m_viewer->m_deaths);
}


void CScoreBoard::RenderPlayerScores (void) {
    int position = 0;
    for (auto [i, a] : actorHandler->m_actors)
        if (a->IsPlayer () && !a->IsViewer())
            RenderScore (++position, (CPlayer*) a, ((CPlayer*) a)->m_score);
}


void CScoreBoard::Render (void) {
    RenderViewerStatus ();
    RenderPlayerStatus ();
    RenderViewerScores ();
    RenderPlayerScores ();
    renderer->SetViewport ();
}
 
CScoreBoard* scoreBoard = nullptr;

// =================================================================================================
