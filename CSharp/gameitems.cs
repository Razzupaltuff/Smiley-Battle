using System;

// =================================================================================================

public class GameItems
{
    public Map m_map;
    public Viewer m_viewer;
    public Reticle m_reticle;

    public GameItems() { }

    public void Destroy()
    {
        m_map.Destroy();
        m_reticle.Destroy();
        m_map = null;
        m_reticle = null;
    }

    public void Cleanup()
    {
        Globals.actorHandler.Cleanup();
    }

    public float ViewerDistance(Vector p)
    {
        return m_map.Distance(m_viewer.GetPosition(), p);
    }

    public void Create()
    {

        CreateMap();
        // the viewer (local player). He controls the projection, i.e. everything is rendered #include "the perspective of the viewer
        m_viewer = Globals.actorHandler.CreateViewer();
        m_viewer.SetupCamera("viewer", 1.0f, new Vector(float.NaN, float.NaN, float.NaN), new Vector(0, 0, 0));
        m_viewer.ForceRespawn();
        Globals.actorHandler.SetViewer(m_viewer);
        Globals.renderer.SetViewer(m_viewer);

        // player reticle
        m_reticle = new Reticle();
        m_reticle.Create();

        // create some player spheres for testing. These have no function besides being there, being targets, and respawning
        int dummies = Globals.argHandler.IntVal("dummies", 0, 0);
        if (dummies > 0)
        {
            for (int i = Math.Min(dummies, (int)Globals.actorHandler.m_maxPlayers - 1); i > 0; i--)
            {
                Player dummy = Globals.actorHandler.CreatePlayer();
                dummy.SetType("dummy");
            }
        }
    }


    public bool CreateMap()
    {
        m_map = new Map();
        string mapName = Globals.argHandler.StrVal("map", 0, "standard.txt");
        MapLoader mapLoader = new MapLoader(m_map);
        m_map.m_stringMap = mapLoader.CreateFromFile(mapName);
        if (m_map.m_stringMap == null)
        {
            Console.Error.WriteLine("Couldn't load map '{0}'", mapName);
            System.Environment.Exit(1);
        }
        return true;
    }


    public bool CreateMap(string[] stringMap, bool isPrepared = false)
    {
        if (m_map != null)
            m_map.Destroy();
        m_map = new Map();
        if (m_map == null)
        {
            Console.Error.WriteLine("Couldn't load map (out of memory)");
            System.Environment.Exit(1);
        }
        m_map.m_stringMap = stringMap;
        MapLoader mapLoader = new MapLoader(m_map);
        if (!mapLoader.CreateFromMemory(m_map.m_stringMap, isPrepared))
        {
            Console.Error.WriteLine("Couldn't load map");
            System.Environment.Exit(1);
        }
        return true;
    }

}

// =================================================================================================

