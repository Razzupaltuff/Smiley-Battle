using System;
using System.Collections.Generic;

// =================================================================================================
// Basic game object with physical properties. Can be mobile or stationary, but basically is everything 
// inside a map that is not a map and not an effect (which usually don't have physical properties relevant
// for game events, like collisions or hits)

public class Actor
{
    public enum eLifeState {
        lsAlive = -1,
        lsDie = 0,
        lsDisappear = 1,
        lsBury = 2,
        lsHide = 3,
        lsResurrect = 4,
        lsReappear = 5,
        lsImmunize = 6,
        lsProtect = 7
    };

    public int m_id;
    public Texture m_texture;
    public Mesh m_mesh;
    public Camera m_camera;
    public List<Texture> m_textures;
    public bool m_isStationary;
    public bool m_isViewer;
    public string m_type;
    public int m_timeOfDeath;
    public Actor m_hitter;
    public Vector m_angles;
    public Vector m_offset;
    public float m_size;
    public float m_scale;
    public int m_maxHitPoints;
    public int m_hitPoints;
    public int m_hitTime;
    public int m_healTime;
    public int m_deathTime;
    public int m_hiddenTime;
    public int m_respawnTime;
    public int m_immuneTime;
    public int m_animationDuration;
    public int[] m_hitSoundTime;
    public int m_hitSoundDelay;
    public int m_hitEffectTime;
    public int m_soundId;
    public int m_animation;
    public bool m_delete;
    public bool m_needPosition;
    public eLifeState m_lifeState;
    public Action[] m_lifeStateHandlers;

    public Actor(string type = "", int hitPoints = 1, bool isViewer = false)
    {
        m_isStationary = false;
        m_isViewer = isViewer;
        m_type = type;
        m_timeOfDeath = 0;
        m_hitter = null;
        m_angles = new Vector(0, 0, 0);
        m_offset = new Vector(0, 0, 0);
        m_size = 1.0f;
        m_scale = 1.0f;
        m_maxHitPoints = hitPoints;
        m_hitPoints = hitPoints;
        m_hitTime = 0;
        m_healTime = 0;
        m_deathTime = 0;
        m_hiddenTime = 0;
        m_respawnTime = 0;
        m_immuneTime = 0;
        m_animationDuration = 750;    // duration [ms] of the shrinking and growing animations after death and when respawning
        m_hitSoundTime = new int[2] { 0, 0 };
        m_hitSoundDelay = 1000;       // minimal time [ms] between two wall hit sounds to avoid the sounds to stutter during frequent wall contact
        m_hitEffectTime = 750;       // duration [ms] of taking on the hitter's color after a hit
        m_soundId = -1;
        m_lifeState = Actor.eLifeState.lsAlive;
        m_lifeStateHandlers = new Action[] { Die, Disappear, Bury, Hide, Resurrect, Reappear, Immunize, Protect };
        m_animation = 0;
        m_delete = false;
        m_needPosition = false;
    }

    // set a mesh (shape), texture, position and initial spatial orientation of the actor
    public void Create(string name, Mesh mesh, int quality, Texture texture, string[] textureNames, Vector position, Vector angles, float size, Camera parent)
    {
        m_size = size;
        SetupTextures(texture, textureNames);
        SetupMesh(mesh, quality, m_texture, textureNames);
        SetupCamera(name, size, position, angles, parent);
        m_respawnTime = Globals.gameData.m_gameTime;
        ForceRespawn();
    }


    public void Destroy()
    {
        m_mesh.Destroy();
    }


    public virtual bool SetupTextures(Texture texture, string[] textureNames = null)
    {
        m_texture = texture;
        if ((textureNames != null) && (textureNames.Length > 0))
            m_texture.CreateFromFile(textureNames);
        return true;
    }


    public void SetupMesh(Mesh mesh, int quality, Texture texture, string[] textureNames)
    {
        m_mesh = mesh;
        if (quality > 0)
            m_mesh.Create(quality, texture, textureNames);
    }


    public void SetupCamera(string name, float size, Vector position, Vector angles, Camera parent = null)
    {
        m_camera = new Camera(name, m_isViewer);
        m_camera.UpdateAngles(angles);
        m_camera.SetPosition(position);
        m_camera.SetSize(size);
        m_camera.SetParent(parent);
    }


    public void SetPosition(Vector position)
    {
        m_camera.SetPosition(position);
        if (!m_camera.HavePosition(1))
            m_camera.SetPosition(position, 1);
        m_needPosition = false;
    }


    public Camera Camera()
    {
        return m_camera;
    }

    public void EnableCamera()
    {
        m_camera.Enable();
    }


    public void DisableCamera()
    {
        m_camera.Disable();
    }


    public virtual Texture GetTexture(int mood = -1)
    {
        return m_texture;
    }

    public void SetTexture(Texture texture)
    {
        m_texture = texture;
    }


    public void SetMesh(Mesh mesh)
    {
        m_mesh = mesh;
    }


    public void SetType(string type)
    {
        m_type = type;
    }


    public bool IsType(string type)
    {
        return m_type == type;
    }


    public int GetId()
    {
        return m_id;
    }


    public new string GetType()
    {
        return m_type;
    }


    public bool IsPlayer()
    {
        return (m_id == 0);
    }


    public bool IsProjectile()
    {
        return (m_id != 0);
    }


    public bool IsViewer()
    {
        return m_isViewer;
    }


    public Vector GetPosition(int i = 0)
    {
        return m_camera.GetPosition(i);
    }


    public bool HavePosition(int i = 0)
    {
        return m_camera.HavePosition(i);
    }


    public Vector GetOrientation()
    {
        return m_camera.GetOrientation();
    }


    public void SetOrientation(Vector angles)
    {
        m_camera.SetOrientation(angles);
    }


    public string GetName()
    {
        return m_camera.m_name;
    }


    public float GetSize()
    {
        return m_camera.m_size;
    }


    public void SetSize(float size)
    {
        m_camera.m_size = size;
    }


    public void SetScale(float scale)
    {
        m_scale = scale;
    }


    public void SetLifeState(eLifeState lifeState)
    {
        m_lifeState = lifeState;
    }


    public int GetAnimation()
    {
        return m_animation;
    }


    public float Radius()
    {
        return GetSize() * 0.5f;
    }


    public virtual float BorderScale()
    {
        return 1.0f;
    }


    public virtual int Mood(int mood = -1)
    {
        return 2;
    }

    public void StartCollisionHandling()
    {
        m_isStationary = false;
    }


    public bool MayPlayHitSound(int type)
    {
        if (Globals.gameData.m_gameTime < m_hitSoundTime[type])
            return false;
        m_hitSoundTime[type] = Globals.gameData.m_gameTime + m_hitSoundDelay;
        return true;
    }

    public bool MayPlayActorHitSound()
    {
        return MayPlayHitSound(0);
    }


    public bool MayPlayWallHitSound()
    {
        return MayPlayHitSound(1);
    }


    // move the object by a displacement vector computed in the map or actor collision handling routines
    public void Bounce(Vector v)
    {
        m_camera.m_positions[0] += v;
    }


    // check whether the current actor is dead
    public bool IsDead()
    {
        return (m_hitPoints == 0);
    }


    public bool IsAlive()
    {
        return (m_hitPoints > 0);
    }


    public bool IsDieing()
    {
        return (m_lifeState >= Actor.eLifeState.lsDie) && (m_lifeState <= Actor.eLifeState.lsBury);
    }


    public bool IsHidden()
    {
        return (m_lifeState == Actor.eLifeState.lsHide) || (m_lifeState == Actor.eLifeState.lsResurrect);
    }


    // check whether the actor is currently respawning
    public bool IsRespawning()
    {
        return (m_lifeState == Actor.eLifeState.lsReappear);
    }


    // check whether the actor can be hit and can shoot 
    public bool IsImmune()
    {
        // global gameData
        return (m_lifeState == Actor.eLifeState.lsImmunize) || (m_lifeState == Actor.eLifeState.lsProtect);
    }

    public virtual Mesh GetProjectileMesh()
    {
        return null;
    }

    public virtual string GetColor()
    {
        return "";
    }

    public virtual int GetColorIndex()
    {
        return -1;
    }

    public virtual Vector GetColorValue()
    {
        return new Vector(1, 1, 1);
    }

    public virtual string GetAddress()
    {
        return "";
    }

    public virtual ushort GetPort(int i = 0)
    {
        return 0;
    }

    public virtual void AddScore(int points)
    {
        // empty placeholder
    }


    public virtual void SetScore(int score)
    {
        // empty placeholder
    }


    public virtual int GetScore()
    {
        return 0;
    }


    public virtual void AddKill()
    {
        // empty placeholder
    }


    public virtual void AddDeath()
    {
        // empty placeholder
    }


    public virtual void UpdateLastMessageTime()
    {
        // empty placeholder
    }


    public virtual void UpdateFrozenTime()
    {
        // empty placeholder
    }


    public virtual Vector GetPlayerColorValue()
    {
        return new Vector(1, 1, 1);
    }


    public void Delete()
    {
        m_delete = true;
    }


    public virtual void Render(bool autoCamera = true)
    {
        if (IsViewer())
            return;
        if (autoCamera)
            EnableCamera();
        GL.PushMatrix();
        float size = m_camera.m_size / BorderScale();
        GL.Scale(size, size, size);
        if (m_hitter == null)
            m_mesh.PushTexture(m_texture);
        else
        {
            m_mesh.PushTexture(m_hitter.GetTexture(Mood()));
            m_mesh.PushColor(m_hitter.GetPlayerColorValue());
        }
        m_mesh.Render();
        m_mesh.PopTexture();
        if (m_hitter != null)
            m_mesh.PopColor();
        GL.PopMatrix();
        if (autoCamera)
            DisableCamera();
    }


    // called when this actor has been hit by shot
    public void RegisterHit(Actor hitter)
    {
        if (m_hitPoints > 0)
        {
            SetHitPoints(m_hitPoints - 1, hitter);
            if (IsLocalActor() & (hitter != null))
                Globals.networkHandler.BroadcastHit(this, hitter);
        }
    }


    public void SetAnimation(int animation)
    {
        m_animation = animation;
        if (IsViewer ())
            Globals.networkHandler.BroadcastAnimation();
    }


    // set hit points. If hit by a projectile, the player who had fired that projectile is passed in hitter
    // That player will receive a point for hitting and another point if killing the current actor (which in
    // this game will always be another player)
    // start death animation if player is killed
    public void SetHitPoints(int hitPoints, Actor hitter = null)
    {
        // global gameData
        if (m_hitPoints == hitPoints)
            return;
        m_hitPoints = hitPoints;
        m_hitTime = Globals.gameData.m_gameTime;
        m_healTime = 0;       // restart healing process when hit
        if (hitter != null)
            hitter.AddScore(1); // one point for the hit
        m_hitter = hitter;
        if (m_hitPoints == 0)
        {
            if (m_id == 0)
            {
                AddDeath();
                if (hitter != null)
                {
                    hitter.AddScore(Globals.gameData.m_pointsForKill);         // another point for the kill
                    hitter.AddKill();
                }
            }
            m_lifeState = Actor.eLifeState.lsDie;
        }
    }


    // The following functions are part of the death and respawn handling state engine
    // They set the timeouts for the various effects and delays and make sure the player
    // gets a new spawn position before reappearing
    // In multiplayer matches, each client only handles death and respawning for itself
    // and transmits his current status to the other players with UPDATE messages
    void Die()
    {
        SetAnimation(1);
        if (this == Globals.actorHandler.m_viewer)
            Globals.effectHandler.StartFade(new Vector(0, 0, 0), m_animationDuration, false);
        m_deathTime = m_hitTime;   // start death animation
        m_lifeState = Actor.eLifeState.lsDisappear;
    }


    void Disappear()
    {
        int dt = Globals.gameData.m_gameTime - m_deathTime;
        if (dt <= m_animationDuration)
            m_scale = 1.0f - (float) dt / (float) m_animationDuration;
        else
            m_lifeState = Actor.eLifeState.lsBury;
    }


    void Bury()
    {
        m_hitter = null;
        m_hiddenTime = Globals.gameData.m_gameTime + Globals.gameData.m_respawnDelay;
        m_scale = 0.0f;
        m_lifeState = Actor.eLifeState.lsHide;
    }


    void Hide()
    {
        if (Globals.gameData.m_gameTime >= m_hiddenTime)
        {
            SetAnimation(2);
            if (IsLocalActor())
                Globals.gameItems.m_map.FindSpawnPosition(this);
            else
                m_needPosition = true;
            m_lifeState = Actor.eLifeState.lsResurrect;
        }
    }


    // start respawn animation. Signal request for a spawn position to the app
    // start immunity period
    void Resurrect()
    {
        if (m_needPosition)     // don't respawn without a valid spawn position
            return;
        m_respawnTime = Globals.gameData.m_gameTime + m_animationDuration;
        if (this == Globals.actorHandler.m_viewer)
            Globals.effectHandler.StartFade(new Vector(0, 0, 0), m_animationDuration, true);
        m_lifeState = Actor.eLifeState.lsReappear;
    }


    void Reappear()
    {
        int dt = m_respawnTime - Globals.gameData.m_gameTime;
        if (dt > 0)
            m_scale = (float) (m_animationDuration - dt) / (float) (m_animationDuration);
        else
        {
            m_scale = 1.0f;
            m_lifeState = Actor.eLifeState.lsImmunize;
        }
    }


    void Immunize()
    {
        m_immuneTime = Globals.gameData.m_gameTime + Globals.gameData.m_immunityDuration;
        m_lifeState = Actor.eLifeState.lsProtect;
    }


    void Protect()
    {
        if (Globals.gameData.m_gameTime > m_immuneTime)
        {
            m_hitPoints = m_maxHitPoints;
            m_lifeState = Actor.eLifeState.lsAlive;
        }
    }


    // The state engine driver
    void UpdateLifeState()
    {
        eLifeState oldState = Actor.eLifeState.lsAlive;
        while ((m_lifeState >= 0) && (oldState != m_lifeState))
        {
            oldState = m_lifeState;
            m_lifeStateHandlers[(int) m_lifeState]();
        }
    }


    public void ForceRespawn()
    {
        m_hitPoints = 0;
        m_hiddenTime = 0;
        m_lifeState = Actor.eLifeState.lsHide;
    }


    // check whether the current actor is dead
    public bool IsLocalActor()
    {
        if (Globals.networkHandler.m_localAddress == "127.0.0.1")
            return true;
        return (GetColorIndex() == Globals.actorHandler.m_viewer.GetColorIndex()) ||
               (GetAddress() == Globals.networkHandler.m_localAddress) ||
               (GetAddress() == "127.0.0.1");
    }


    // heal the actor until it has full health or gets hit. Wait gameData.healDelay ms for healing by one hitpoint
    // don't heal when dead
    public void Heal()
    {
        // global gameData
        if (m_hitPoints == 0)
            return;
        if (m_hitPoints == m_maxHitPoints)
        {
            m_healTime = 0;
            return;
        }
        if (m_healTime == 0)
        {
            m_healTime = Globals.gameData.m_gameTime;    // start over
            return;
        }
        else if (Globals.gameData.m_gameTime - m_healTime < Globals.gameData.m_healDelay)
            return;
        m_hitPoints++;
        m_healTime = 0;
    }


    public void UpdateSound()
    {
        // global soundHandler
        if (m_id != 0)
            return;
        if (IsAlive() || IsImmune()) {
            if (IsViewer() & (m_soundId < 0))
                m_soundId = Globals.soundHandler.Play("hum", GetPosition(), 0.1f, -1, this, 4);
        }
    else
        {
            if (m_soundId >= 0)
            {
                Globals.soundHandler.Stop(m_soundId);
                m_soundId = -1;
            }
        }
    }


    // update actor status (death animation, respawn animation, healing)
    public virtual void Update(float dt = 1.0f, Vector angles = null, Vector offset = null)
    {
        // global gameData
        if (IsLocalActor())
        {
            UpdateLifeState();
            Heal();
        }
        if (m_hitter == null)
            return;
        if (Globals.gameData.m_gameTime - m_hitTime < m_hitEffectTime)
            return;
        m_hitter = null;
        m_hitTime = 0;
    }

}

// =================================================================================================
