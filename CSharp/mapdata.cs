using System;
using System.Collections.Generic;

// =================================================================================================

public class MapData
{
    public List<Wall> m_walls;
    public Quad m_floor;
    public Quad m_ceiling;
    public Mesh m_mesh;
    public Vector m_color;
    public Vector m_vMin;             // map boundaries
    public Vector m_vMax;
    public int m_vertexCount;      // total number of wall vertices
    public float m_scale;            // scale of the map (base unit is 1.0)
    public int m_distanceQuality;
    public SegmentMap m_segmentMap;       // map segments
    public string[] m_stringMap;        // map layout data
    public List<Texture> m_textures;
    public float[] m_spawnHeadings;
    public MapPosition[] m_neighbourOffsets;

    public Texture GetTexture(int i)
    {
        return (m_textures.Count > i) ? m_textures[i] : null;
    }


    public MapData()
    {
        m_scale = 3.0f;
        m_distanceQuality = 1;
        SetupTextures(new string[] { "wall.png", "floor3.png", "ceiling2.png" });
        m_distanceQuality = Globals.argHandler.IntVal("distancequality", 0, 1);
        m_spawnHeadings = new float[] { 90, 0, -90, 180 };
        m_neighbourOffsets = new MapPosition[] { new MapPosition(0, 0), new MapPosition(-1, -1), new MapPosition(1, -1), new MapPosition(1, 1), new MapPosition(-1, 1) };
        m_walls = new List<Wall>();
    }


    public void Init()
    {
        //Destroy();
        m_vMin = new Vector(1e6f, 1e6f, 1e6f);
        m_vMax = new Vector(-1e6f, -1e6f, -1e6f);
        m_vertexCount = 0;
        m_mesh = new Mesh();
        m_mesh.Init(GL.QUADS, m_textures[0]);
        m_segmentMap = new SegmentMap(m_scale, m_distanceQuality);
    }


    // create all textures required for map rendering (walls, floor, ceiling)
    public void SetupTextures(string[] textureNames)
    {
        m_textures = Globals.textureHandler.CreateTextures(textureNames);
    }


    public void Destroy()
    {
        m_segmentMap.Destroy();
        m_mesh.Destroy();
        m_walls = null;
        m_mesh = null;
        m_stringMap = null;
        m_segmentMap = null;
    }

}

// =================================================================================================
