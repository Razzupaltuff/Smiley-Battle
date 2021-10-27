#pragma once

#include "texture.h"
#include "cubemap.h"
#include "mesh.h"
#include "camera.h"

// =================================================================================================
// Basic game object with physical properties. Can be mobile or stationary, but basically is everything 
// inside a map that is not a map and not an effect (which usually don't have physical properties relevant
// for game events, like collisions or hits)

class CActor {
    public:

        typedef void (CActor::*tLifeStateHandler) (void);

        typedef enum {
            lsAlive = -1,
            lsDie = 0, 
            lsDisappear = 1, 
            lsBury = 2, 
            lsHide = 3, 
            lsResurrect = 4, 
            lsReappear = 5, 
            lsImmunize = 6, 
            lsProtect = 7
        } eLifeStates;

        int                 m_id;
        CTexture *          m_texture;
        CMesh *             m_mesh;
        CCamera             m_camera;
        CList<CTexture*>    m_textures;
        bool                m_stationary;
        bool                m_isViewer;
        CString             m_type;
        size_t              m_timeOfDeath;
        CActor*             m_hitter;
        CVector             m_angles;
        CVector             m_offset;
        float               m_size;
        float               m_scale;
        int                 m_maxHitPoints;
        int                 m_hitPoints;
        int                 m_hitTime;
        int                 m_healTime;
        int                 m_deathTime;
        int                 m_hiddenTime;
        int                 m_respawnTime;
        int                 m_immuneTime;
        int                 m_animationDuration;
        int                 m_hitSoundTime[2];
        int                 m_hitSoundDelay;
        int                 m_hitEffectTime;
        int                 m_soundId;
        int                 m_animation;
        bool                m_delete;
        bool                m_needPosition;
        eLifeStates         m_lifeState;
        CArray<tLifeStateHandler>   m_lifeStateHandlers;

        CActor(CString type = CString(""), int hitPoints = 1, bool isViewer = false);

    // set a mesh (shape), texture, position and initial spatial orientation of the actor
        void Create(CString name, CMesh * mesh, int quality, CTexture* texture, CList<CString> textureNames, CVector position, CVector angles, float size, CCamera* parent = nullptr);

        void Destroy(void);

        virtual bool SetupTextures(CTexture* texture, CList<CString> textureNames = CList<CString>());

        void SetupMesh(CMesh * mesh, int quality, CTexture* texture, CList<CString>& textureNames);

        void SetupCamera(CString name, float size, CVector position, CVector angles, CCamera* parent = nullptr);

        inline CCamera& Camera(void) {
            return m_camera;
        }

        inline void EnableCamera(void) {
            m_camera.Enable();
        }


        inline void DisableCamera(void) {
            m_camera.Disable();
        }


        virtual CTexture* GetTexture(int mood = -1) {
            return m_texture;
        }

        inline void SetTexture(CTexture* texture) {
            m_texture = texture;
        }


        inline void SetMesh(CMesh * mesh) {
            m_mesh = mesh;
        }


        inline void SetType(CString type) {
            m_type = type;
        }


        inline bool IsType(CString type) {
            return m_type == type;
        }


        inline int GetId(void) {
            return m_id;
        }


        inline CString& GetType(void) {
            return m_type;
        }


        inline bool IsPlayer(void) {
            return (m_id == 0);
        }


        inline bool IsProjectile(void) {
            return (m_id != 0);
        }


        inline bool IsViewer (void) {
            return m_isViewer;
        }


        inline CVector& GetPosition(int i = 0) {
            return m_camera.GetPosition(i);
        }


        inline bool HavePosition(int i = 0) {
            return m_camera.HavePosition(i);
        }


        inline CVector& GetOrientation(void) {
            return m_camera.GetOrientation();
        }


        void SetPosition(CVector position);


        inline void SetOrientation(CVector angles) {
                m_camera.SetOrientation(angles);
            }
        

        inline CString GetName(void) {
                return m_camera.m_name;
            }


        inline float GetSize(void) {
            return m_camera.m_size; 
        }


        inline void SetSize(float size) {
            m_camera.m_size = size; 
        }


        inline void SetScale(float scale) {
            m_scale = scale;
        }


        void SetLifeState(CActor::eLifeStates lifeState) {
            m_lifeState = lifeState;
        }


        void SetAnimation(int animation);


        inline size_t GetAnimation(void) {
            return m_animation;
        }


        inline float Radius(void) {
            return GetSize() * 0.5f;
        }


        virtual float BorderScale(void) {
            return 1.0f;
        }


        virtual int Mood(int mood = -1) {
            return 2;
        }

        virtual void Render(bool autoCamera = true);


        inline void StartCollisionHandling(void) {
            m_stationary = false;
        }


        bool MayPlayHitSound(size_t type);


        inline bool MayPlayActorHitSound(void) {
            return MayPlayHitSound(0);
        }


        inline bool MayPlayWallHitSound(void) {
            return MayPlayHitSound(1);
        }


        // move the object by a displacement vector computed in the map or actor collision handling routines
        inline void Bounce(CVector v) {
            m_camera.m_positions[0] += v;
        }


        // called when this actor has been hit by shot
        void RegisterHit(CActor* hitter = nullptr);


        // set hit points. If hit by a projectile, the player who had fired that projectile is passed in hitter
        // That player will receive a point for hitting and another point if killing the current actor (which in
        // this game will always be another player)
        // start death animation if player is killed
        void SetHitPoints(int hitPoints, CActor* hitter = nullptr);


        // The following functions are part of the death and respawn handling state engine
        // They set the timeouts for the various effects and delays and make sure the player
        // gets a new spawn position before reappearing
        // In multiplayer matches, each client only handles death and respawning for itself
        // and transmits his current status to the other players with UPDATE messages
        void Die(void);

        void Disappear(void);

        void Bury(void);

        void Hide(void);

        // start respawn animation. Signal request for a spawn position to the app
        // start immunity period
        void Resurrect(void);

        void Reappear(void);

        void Immunize(void);

        void Protect(void);

        // The state engine driver
        void UpdateLifeState(void);

        void ForceRespawn(void);

        // check whether the current actor is dead
        inline bool IsDead(void) {
            return (m_hitPoints == 0);
        }


        inline bool IsAlive(void) {
            return (m_hitPoints > 0);
        }


        inline bool IsDieing(void) {
            return (m_lifeState >= 0) and (m_lifeState <= 2);
        }


        inline bool IsHidden(void) {
            return (m_lifeState == 3) or (m_lifeState == 4);
        }


        // check whether the actor is currently respawning
        inline bool IsRespawning(void) {
            return (m_lifeState == 5);
        }


        // check whether the actor can be hit and can shoot 
        inline bool IsImmune(void) {
            // global gameData
            return (m_lifeState == 6) or (m_lifeState == 7);
        }

        virtual CMesh* GetProjectileMesh(void) {
            return nullptr;
        }

        virtual CString GetColor(void) {
            return CString("");
        }

        virtual int GetColorIndex(void) {
            return -1;
        }

        virtual CVector GetColorValue(void) {
            return CVector (1,1,1);
        }

        virtual CString GetAddress(void) {
            return CString("");
        }

        virtual uint16_t GetPort(size_t i = 0) {
            return 0;
        }

        virtual void AddScore(int points) {
            // empty placeholder
        }


        virtual void SetScore(int score) {
            // empty placeholder
        }


        virtual size_t GetScore(void) {
            return 0;
        }


        virtual void AddKill(void) {
            // empty placeholder
        }


        virtual void AddDeath(void) {
            // empty placeholder
        }


        virtual void UpdateLastMessageTime(void) {
            // empty placeholder
        }


        virtual void UpdateFrozenTime(void) {
            // empty placeholder
        }


        virtual CVector GetPlayerColorValue(void) {
            return CVector (1,1,1);
        }


        bool IsLocalActor(void);

        // heal the actor until it has full health or gets hit. Wait gameData->healDelay ms for healing by one hitpoint
        // don't heal when dead
        void Heal(void);

        inline void Delete(void) {
            m_delete = true;
        }

        void UpdateSound(void);

        // update actor status (death animation, respawn animation, healing)
        virtual void Update(float dt = 1.0f, CVector angles = CVector(0, 0, 0), CVector offset = CVector(0, 0, 0));

    };

// =================================================================================================
