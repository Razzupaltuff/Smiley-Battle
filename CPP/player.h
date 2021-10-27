#pragma once

#include <stdint.h>

#include "cstring.h"
#include "clist.h"
#include "cavltree.h"

#include "cubemap.h"
#include "quad.h"
#include "torus.h"
#include "camera.h"
#include "actor.h"
#include "timer.h"

// =================================================================================================
// Shadow for players (smileys). Just a 2D texture rendered near the ground

class CPlayerShadow : public CQuad {
    public:
        float   m_groundClearance;

        CPlayerShadow ();

        CTexture* CreateTexture(void);

        void Render(float offset);
};

// =================================================================================================
// Shadow for players (smileys). Just a 2D texture rendered near the ground

class CPlayerHalo : public CTorus {
    public:
        float m_tolerance;

        CPlayerHalo ();

        CTexture* CreateTexture(void);

        void Render(float offset);
};

// =================================================================================================
// Outline for players (smileys). The outline is created by rendering the backwards facing faces of
// the sphere in black && a tad bigger than the player smileys.
// The outline is rendered after the smiley to improve performance, since the z culling will discard
// most of the render data (pixels) early

class CPlayerOutline {
    public:
        CMesh*              m_mesh;
        float               m_scale;
        CList<CString>      m_textureNames;
        CList<CTexture*>    m_textures;

        CPlayerOutline ();

        void Create (CMesh* mesh);

        inline float Scale(void) {
            return m_scale;
        }

        void Render(float size, int colorIndex = 0);
};

// =================================================================================================
// Player actor. Standard actor with a few extra properties: Shadow, outline, firing shots, changing
// color when hit, dieing animation

class CPlayer : public CActor {
    public:
        CPlayerShadow*      m_shadow;
        CPlayerOutline*     m_outline;
        CPlayerHalo*        m_halo;
        CMesh*              m_projectileMesh;
        CList<CTexture*>    m_textures;
        CList<CString>      m_moods;
        CString             m_color;
        CVector             m_colorValue;
        int                 m_colorIndex;
        bool                m_whiteForBlack;
        CString             m_address;
        uint16_t            m_ports[2];
        size_t              m_lastMessageTime;
        bool                m_isConnected;
        int                 m_score;
        int                 m_kills;
        int                 m_deaths;
        CTimer              m_wiggleTimer;
        size_t              m_wiggleAngle;

        CPlayer(CString type, int colorIndex = -1, CPlayerShadow* shadow = nullptr, CPlayerHalo* halo = nullptr, CPlayerOutline* outline = nullptr, bool isViewer = false);

        void Destroy(void);

        void Create(CString name, CMesh * mesh, int quality, CAvlTree<CString, CTexture*>& textures, CList<CString> textureNames, CVector position, CVector angles, float size, CCamera* parent = nullptr);


        void SetupTextures(CAvlTree<CString, CTexture*>& textures, CList<CString> textureNames = CList<CString>());


        inline void SetProjectileMesh(CMesh* mesh) {
           m_projectileMesh = mesh;
        }

        virtual CMesh * GetProjectileMesh(void) {
            return m_projectileMesh;
        }

        inline void SetShadow(CPlayerShadow* shadow) {
            m_shadow = shadow;
        }

        inline void SetOutline(CPlayerOutline* outline) {
            m_outline = outline;
        }

        inline void SetHalo(CPlayerHalo* halo) {
            m_halo = halo;
        }


        void SetColorIndex(int colorIndex, bool replace = false);

        virtual int GetColorIndex(void) {
            return m_colorIndex;
        }

        virtual CVector GetColorValue(void) {
            return m_colorValue;
        }

        virtual CString GetColor(void) {
            return m_color;
        }

        virtual void UpdateLastMessageTime (void);

        virtual CVector GetPlayerColorValue(void) {
            return m_colorValue;
        }


        inline void SetAddress(CString address, uint16_t ports[]) {
            m_address = address;
            memcpy(m_ports, ports, sizeof(m_ports));
        }


        virtual CString GetAddress(void) {
            return m_address;
        }


        virtual uint16_t GetPort(size_t i = 0) {
            return m_ports[i];
        }


        inline void SetPort(uint16_t port, size_t i = 0) {
            m_ports[i] = port;
        }


        virtual int Mood(int mood = -1);

        virtual CTexture* GetTexture(int mood = -1) {
            return m_textures[Mood(mood)];
        }

        virtual void Render (bool autoCamera = true);

        void Wiggle(void);


        virtual float BorderScale(void) {
            if (m_outline)
                return m_outline->Scale();
            return 1.0f;
        }


        virtual void AddScore(int points) {
            m_score += points;
        }


        virtual void SetScore(int score) {
            m_score = score;
        }


        virtual size_t GetScore(void) {
            return m_score;
        }

        virtual void AddKill(void) {
            m_kills++;
        }


        virtual void AddDeath(void) {
            m_deaths++;
        }

};

// =================================================================================================

class CViewer : public CPlayer {
    public:
        bool    m_fire;
        size_t  m_fireTime;

        CViewer() : CPlayer(CString("viewer")) {
            m_isViewer = true;
            m_fire = false;
            m_fireTime = 0;
        }

        bool ReadyToFire(void);

        void Fire(void);

        virtual void Render (bool autoCamera = true) {
        // empty
        }

        virtual void Update(float dt = 1.0f, CVector angles = CVector(0, 0, 0), CVector offset = CVector(0, 0, 0));
};

// =================================================================================================
