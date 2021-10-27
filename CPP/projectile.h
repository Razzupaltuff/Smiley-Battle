#pragma once

#include <string.h>
#include "camera.h"
#include "actor.h"
#include "player.h"

// =================================================================================================
// Handling of shots. Shots have straight movement at a fixed speed.
// If they hit something (other actor or wall), they disappear

class CProjectile : public CActor {
    public:
        float           m_speed;
        CVector         m_offset;
        CActor*         m_parent;
        CPlayerOutline* m_outline;
        size_t          m_frozenTime;

        CProjectile(int id = -1);


        void Create(CActor* parent);


        virtual bool SetupTextures(CTexture* texture, CList<CString> textureNames = CList<CString>()) { // required for CActor not recursively calling its SetupTexture method when the child doesn't have one of its own
            return false;
        }


        inline void UpdateOffset(void) {
            m_camera.BumpPosition();
        }


        virtual void Update(float dt = 1.0f, CVector angles = CVector(0, 0, 0), CVector offset = CVector(0, 0, 0));

        inline void StartCollisionHandling(void) {
           CActor::StartCollisionHandling();
        }

        void Delete(bool force = false);

        virtual void Render(bool bAutoCamera = true);


        virtual float BorderScale(void) {
            return m_outline ? m_outline->Scale() : 1.0f;
        }


        virtual void UpdateLastMessageTime(void) {
            if (m_parent)
                m_parent->UpdateLastMessageTime ();
        }


        inline int GetColorIndex(void) {
            return m_parent->GetColorIndex ();
        }


        inline CString GetAddress(void) {
            return m_parent->GetAddress ();
        }


        inline size_t GetPort(int i = 0) {
            return m_parent->GetPort(i);
        }


        virtual void UpdateFrozenTime(void);

};

// =================================================================================================
