
#include "player.h"
#include "projectile.h"
#include "gameData.h"
#include "gameItems.h"
#include "actorHandler.h"
#include "networkHandler.h"
#include "textureHandler.h"
#include "argHandler.h"

// =================================================================================================
// Shadow for players (smileys). Just a 2D texture rendered near the ground

CPlayerShadow::CPlayerShadow () 
    : CQuad ({ CVector (-0.5, 0.0, 0.5), CVector (-0.5, 0.0, -0.5), CVector (0.5, 0.0, -0.5), CVector (0.5, 0.0, 0.5) }, CreateTexture ()) {
    m_groundClearance = 0.0002f;
}


CTexture* CPlayerShadow::CreateTexture(void) {
    CTexture* texture = textureHandler->GetTexture();
    CList<CString> fileName = { gameData->m_textureFolder + "shadow.png" };
    if (texture->CreateFromFile(fileName))
        return texture;
    textureHandler->Remove(texture);
    return nullptr;
}


void CPlayerShadow::Render(float offset) {
    glPushMatrix();
    glDisable(GL_CULL_FACE);
    glTranslatef(0.0f, offset + m_groundClearance, 0.0f);
    CQuad::Render();
    glEnable(GL_CULL_FACE);
    glPopMatrix();
}

// =================================================================================================
// Shadow for players (smileys). Just a 2D texture rendered near the ground

CPlayerHalo::CPlayerHalo () 
    : CTorus (nullptr, CList<CString> ({ CString ("white.png") }), gameData->GetColorValue ("gold")) {
    m_tolerance = 0.0001f;
}

CTexture* CPlayerHalo::CreateTexture (void) {
    CTexture* texture = textureHandler->GetTexture ();
    CList<CString> fileName = CList{ gameData->m_textureFolder + CString ("halo.png") };
    if (texture->CreateFromFile (fileName))
        return texture;
    textureHandler->Remove(texture);
    return nullptr;
}


void CPlayerHalo::Render(float offset) {
    glPushMatrix();
    glDisable(GL_CULL_FACE);
    glTranslatef(0.0f, offset + m_tolerance, 0.0f);
    glScalef(0.667f, 1.0f, 0.667f);
    CTorus::Render();
    glEnable(GL_CULL_FACE);
    glPopMatrix();
}

// =================================================================================================
// Outline for players (smileys). The outline is created by rendering the backwards facing faces of
// the sphere in black && a tad bigger than the player smileys.
// The outline is rendered after the smiley to improve performance, since the z culling will discard
// most of the render data (pixels) early

CPlayerOutline::CPlayerOutline () 
    : m_mesh (nullptr), m_scale (1.02f), m_textureNames (CList<CString> ({ CString ("black.png"), CString ("white.png") })) 
{}

void CPlayerOutline::Create (CMesh* mesh) {
    m_mesh = mesh;
    m_textures = textureHandler->CreateCubemaps (m_textureNames);
}


void CPlayerOutline::Render(float size, int colorIndex) {
    glPushMatrix();
    glScalef(size, size, size);
    glCullFace(GL_FRONT);
    m_mesh->PushTexture(m_textures[colorIndex]);
    m_mesh->VAO ().SetMinBrightness(0.9f);
    m_mesh->Render();
    m_mesh->VAO ().SetMinBrightness(0.0f);
    m_mesh->PopTexture();
    glCullFace(GL_BACK);
    glPopMatrix();
}

// =================================================================================================
// Player actor. Standard actor with a few extra properties: Shadow, outline, firing shots, changing
// color when hit, dieing animation

CPlayer::CPlayer(CString type, int colorIndex, CPlayerShadow* shadow, CPlayerHalo* halo, CPlayerOutline* outline, bool isViewer)
    : CActor::CActor(type, 3, isViewer)
{
    m_shadow = shadow;
    m_outline = outline;
    m_halo = halo;
    m_colorIndex = -1;
    m_whiteForBlack = false;
    SetColorIndex (colorIndex);
    m_moods = CList<CString>({ CString("-sad"), CString("-neutral"), CString("-happy") });
    m_colorValue = CVector(1, 1, 1);
    m_color = CString("white");
    m_address = CString("127.0.0.1");
    m_ports[0] =
    m_ports[1] = 0;
    m_lastMessageTime = 0;    // time when the last network message #include "this player was received
    m_isConnected = false;    // remote player is connected with local player
    m_score = 0;
    m_kills = 0;
    m_deaths = 0;
    m_wiggleAngle = size_t(rand() * 180);
}


void CPlayer::Destroy(void) {
    // m_shadow->Destroy();
    // m_halo->Destroy();
    CActor::Destroy();
}


void CPlayer::Create(CString name, CMesh * mesh, int quality, CAvlTree<CString, CTexture*>& textures, CList<CString> textureNames, CVector position, CVector angles, float size, CCamera* parent) {
    SetupTextures(textures, textureNames);
    CActor::Create(name, mesh, quality, nullptr, CList<CString>(), position, angles, size, parent);
}


void CPlayer::SetupTextures(CAvlTree<CString, CTexture*>& textures, CList<CString> textureNames) {
    CString color = gameData->GetColor(m_colorIndex);
    if (color != "black")
        color = "white";
    for (auto [i, mood] : m_moods)
        m_textures.Append(textures[color + *mood]);
}


void CPlayer::SetColorIndex(int colorIndex, bool replace) {
    if (replace && (m_colorIndex >= 0))
        gameData->ReturnColorIndex(m_colorIndex);
    if (replace && (colorIndex >= 0))
        gameData->RemoveColorIndex(colorIndex);
    m_colorIndex = colorIndex;
    if (m_colorIndex >= 0)
        m_whiteForBlack = gameData->GetPlayerColorValue(this, m_colorValue, m_color, true);
    else {
        m_color = "white";
        m_colorValue = CVector(1, 1, 1);
        m_whiteForBlack = false;
    }
}


int CPlayer::Mood(int mood) {
    if (mood > -1)
        return mood;
    if (IsImmune())
        return 2;
    if (m_hitPoints > 0)
        return m_hitPoints - 1;
    return 0;
}


void CPlayer::Render (bool autoCamera) {
    if (IsHidden())
        return;
    CActor::EnableCamera();
    if (m_scale < 1.0f)
        glScalef(m_scale, 1.0f, m_scale);
    if (m_shadow)
        m_shadow->Render(-CActor::GetPosition().Y());
        Wiggle();
    if (m_halo && IsImmune()) 
        m_halo->Render(m_size * 0.55f);
    if (m_scale < 1.0f)
        glScalef(1.0f, m_scale, 1.0f);
    CActor::SetTexture(GetTexture());
    m_mesh->PushColor(m_colorValue);
    CActor::Render(false);
    m_mesh->PopColor();
    if (m_outline)
        m_outline->Render(m_camera.GetSize(), int(m_whiteForBlack));
    CActor::DisableCamera();
}


void CPlayer::Wiggle(void) {
    if (m_isViewer && !gameData->m_wiggleViewer)
        return;
    if (!IsPlayer () || !gameData->m_wigglePlayers)
        return;
    if (m_wiggleTimer.HasPassed(10, true)) {
        m_wiggleAngle += 5;
        m_wiggleAngle %= 360;
    }
    glTranslatef(0, sin(m_camera.Rad(float (m_wiggleAngle))) / 20, 0);
}


void CPlayer::UpdateLastMessageTime (void) {
    m_lastMessageTime = gameData->m_gameTime;
}


// =================================================================================================

bool CViewer::ReadyToFire(void) {
    // global gameData
    if (m_hitPoints == 0)
        return false;
    if (m_fireTime == 0)
        return true;
    return (gameData->m_gameTime - m_fireTime > gameData->m_fireDelay);
}


void CViewer::Fire(void) {
    // global gameData, gameItems
    if (ReadyToFire()) {
        m_fireTime = gameData->m_gameTime;
        CProjectile* projectile = actorHandler->CreateProjectile(this);
        if (projectile)
            networkHandler->BroadcastFire(projectile);
    }
}


// update viewer position && orientation
// Scale with dt. Dt specifies the ratio of the actual frametime to the desired frametime
// This compensates for high frametimes && assures that players on slow systems move as 
// fast as players on fast systems
void CViewer::Update(float dt, CVector angles, CVector offset) {
    CPlayer::Update(dt);
    if (IsAlive() || IsImmune()) {
        if (angles.Len() != 0)
            m_camera.UpdateAngles(angles * dt, true);   // compensate for higher frame times
        if (offset.Len() != 0)
            m_camera.UpdatePosition(m_camera.m_orientation.Unrotate(offset * dt));
    }
}

// =================================================================================================
