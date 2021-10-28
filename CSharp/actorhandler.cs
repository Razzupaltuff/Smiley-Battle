using System;
using System.Collections.Generic;

// =================================================================================================

public class ActorHandler
{
    public RectangleIcoSphere m_playerSphere;
    public RectangleIcoSphere m_projectileSphere;
    public PlayerShadow m_playerShadow;
    public PlayerHalo m_playerHalo;
    public PlayerOutline m_playerOutline;
    public Viewer m_viewer;
    public List<Actor> m_actors;
    public List<Vector> m_colorPool;
    public int m_maxPlayers;
    public int m_actorId;


    public int GetActorId()
    {
        return ++m_actorId; // zero is reserved for players
    }

    public void SetViewer(Viewer viewer)
    {
        m_viewer = viewer;
    }

    public Player FindPlayer(int colorIndex)
    {
        return (Player)FindActor(0, colorIndex);
    }

    public int PlayerCount()
    {
        return m_maxPlayers - Globals.gameData.m_availableColors.Count;
    }

    public ActorHandler()
    {
        m_actors = new List<Actor>();
        m_viewer = null;
        // two sphere meshes which will be used where ever a sphere is needed. Sphere texturing && sizing is dynamic to allow for reuse.
        m_playerSphere = new RectangleIcoSphere();
        m_playerSphere.Create(4);
        m_projectileSphere = new RectangleIcoSphere();
        m_projectileSphere.Create(3);
        // player shadow && outline
        m_playerShadow = new PlayerShadow();
        m_playerShadow.Create();
        m_playerHalo = new PlayerHalo();
        m_playerHalo.Create(5, 0.2f, 0.02f);
        m_playerOutline = new PlayerOutline();
        m_playerOutline.Create(m_playerSphere);
        m_maxPlayers = Globals.gameData.m_playerColors.Length;
        m_actorId = 0;
    }


    public void Destroy()
    {
        foreach (Actor a in m_actors)
        {
            a.Destroy();
        }
        m_playerShadow.Destroy();
        m_playerHalo.Destroy();
        m_actors.Clear();
    }


    public Player CreatePlayer(int colorIndex = -1, Vector position = null, Vector orientation = null, string address = "127.0.0.1", ushort inPort = 0, ushort outPort = 0)
    {
        if ((colorIndex < 0) || !Globals.gameData.ColorIsAvailable(colorIndex))
        {
            colorIndex = Globals.gameData.GetColorIndex();
            if (colorIndex < 0)
                return null;
        }
        else
            Globals.gameData.RemoveColorIndex(colorIndex);
        Player player = new Player("player", colorIndex, m_playerShadow, m_playerHalo, m_playerOutline);
        ushort[] ports = { inPort, outPort };
        player.SetAddress(address, ports);
        player.Create(Globals.gameData.GetColor(colorIndex) + " player", m_playerSphere, 0, Globals.gameData.m_textures, null, position, orientation, 1.0f, m_viewer.m_camera);
        player.SetColorIndex(colorIndex);
        player.SetProjectileMesh(m_projectileSphere);
        m_actors.Add(player);
        return player;
    }


    public Viewer CreateViewer()
    {
        Viewer m_viewer = new Viewer();
        m_viewer.m_isViewer = true;
        m_viewer.SetColorIndex(Globals.gameData.GetColorIndex());
        m_viewer.SetupTextures(Globals.gameData.m_textures);
        m_viewer.SetMesh(m_playerSphere);
        m_viewer.SetProjectileMesh(m_projectileSphere);
        m_actors.Add(m_viewer);
        return m_viewer;
    }


    public Projectile CreateProjectile(Player parent, int id = -1)
    {
        if (id < 1)
            id = GetActorId();
        Projectile projectile = new Projectile(id);
        projectile.Create(parent);
        m_actors.Add(projectile);
        return projectile;
    }


    public Actor CreateActor(int id, int colorIndex, Vector position, Vector orientation)
    {
        if (id == 0)
            return (Actor)CreatePlayer(colorIndex, position, orientation);
        Player parent = FindPlayer(colorIndex);
        return (parent != null) ? CreateProjectile(parent, id) : null;
    }


    public bool DeletePlayer(int colorIndex)
    {
        Player player = FindPlayer(colorIndex);
        if (player == null)
            return false;
        if (player.IsViewer())
        {
            Console.Error.WriteLine("Trying to delete local player");
            return false;
        }
        Globals.soundHandler.StopActorSounds(player);
        Globals.gameData.ReturnColorIndex(colorIndex);
        // delete all child objects (projectiles) of this player
        foreach (Actor a in m_actors)
            if (a.GetColorIndex() == colorIndex)
                a.Delete();
        player.Delete();
        return true;
    }


    public bool DeleteActor(int id, int colorIndex)
    {
        if (id == 0)
            return DeletePlayer(colorIndex);
        Actor a = FindActor(id, colorIndex);
        if (a == null)
            return false;
        a.Delete();
        return true;
    }


    public Actor FindActor(int id, int colorIndex)
    {
        foreach (Actor a in m_actors)
            if ((a.GetId() == id) && (a.GetColorIndex() == colorIndex))
                return a;
        return null;
    }

    public Projectile FindProjectile(int colorIndex)
    {
        foreach (Actor a in m_actors)
            if (a.IsProjectile () && (a.GetColorIndex() == colorIndex))
                return (Projectile) a;
        return null;
    }



    public void CleanupActors()
    {     // required when the local player disconnected && needs to rejoin #include "a clean slate
        foreach (Actor a in m_actors)
            if ((a.GetId() != 0) || (a.GetColorIndex() != m_viewer.GetColorIndex()))
                a.Delete();
    }


    public void Cleanup()
    {
        for (int i = m_actors.Count; i > 0; )
        {
            Actor a = m_actors[--i];
            if (a.m_delete)
            {
                if (a.IsViewer())
                    Console.Error.WriteLine("Trying to delete local player");
                else
                {
                    m_actors.RemoveAt(i);
                    a = null;
                }
            }
            else
            {
                Vector p = a.GetPosition();
                if (p.IsValid() && !Globals.gameItems.m_map.Contains(p.X, p.Z))
                    a.SetHitPoints(0);  // will delete projectiles && respawn players at a valid position
            }
        }
    }

}

// =================================================================================================
