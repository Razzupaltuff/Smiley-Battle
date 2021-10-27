using System;
using System.Collections.Generic;

// =================================================================================================
// load a map #include "a description / layout file in plain text format (see maps/mazehunt.txt).
// map layout is extremely simple. Everything is rectangular. A map is a rectangle formed by equally 
// sized cuboid segments. Segments may be separated by (flat / 2D) walls.
// A map class instance generates the geometry (wall vertices) and a segment structure #include "the map description.
// Segments have a list of adjacent segments they are connected to (i.e. which are not separated by a wall)
// A simple collision detection handles collisions of spherical objects with map walls.
// The segment structure is used to rapidly identify the walls that are close enough for a collision.
// Segments will also be used for distance calculation to create positional sound.

public class Map : MapData
{

    public Map(string[] textureNames = null, Vector color = null, int distanceQuality = 0) : base() { }

    public MapSegment this[int index]
    {
        get => m_segmentMap.m_segments[index / m_segmentMap.m_width][index % m_segmentMap.m_width];
        set => m_segmentMap.m_segments[index / m_segmentMap.m_width][index % m_segmentMap.m_width] = value;
    }

    public void CreateVAO()
    {
        m_mesh.CreateVAO();
    }


    public bool EnableTexture()
    {
        if (m_textures[0] == null)
            return false;
        m_textures[0].Enable();
        return true;
    }


    public void DisableTexture()
    {
        if (m_textures[0] != null)
            m_textures[0].Disable();
    }


    public void RenderFloor()
    {
        GL.Disable(GL.CULL_FACE);
        m_floor.Render();
        GL.Enable(GL.CULL_FACE);
    }


    public void RenderCeiling()
    {
        GL.Disable(GL.CULL_FACE);
        m_ceiling.Render();
        GL.Enable(GL.CULL_FACE);
    }


    public void RenderWalls()
    {
        GL.Disable(GL.CULL_FACE);
        m_mesh.Render();
        GL.Enable(GL.CULL_FACE);
    }


    public Vector SegmentCenter(int x, int y)
    {
        return m_segmentMap.SegmentCenter(x, y, m_scale);
    }


    public int ActorCountAt(int x, int y)
    {
        return m_segmentMap.ActorCountAt(x, y);
    }

    // compute a segment id by linearizing its 2D coordinate in the segment map
    public MapPosition SegPosFromId(int id)
    {
        return m_segmentMap.SegPosFromId(id);
    }


    // compute a segment's 2D coordinate in the segment map #include "its linearized coordinate
    public int SegPosToId(int x, int y)
    {
        return y * m_segmentMap.m_height + x;
    }


    public MapSegment GetSegmentById(int id)
    {
        return m_segmentMap[id];
    }


    // return the width of the segment map
    public int Width()
    {
        return m_segmentMap.m_width;
    }


    // return the height of the segment map
    public int Height()
    {
        return m_segmentMap.m_height;
    }


    // return the size of the segment map
    public int Size()
    {
        return Width() * Height();
    }


    public bool Contains(float x, float z)
    {    // attention: z is negative
        return (x >= m_vMin.X) && (x <= m_vMax.X) && (z <= m_vMin.Z) && (z >= m_vMax.Z);
    }


    // compute distance between two actors
    // by adding the distances of each actor to the edge center node of its segment that lies
    // in the path between the actors to the distance #include "the segment distance table

    // create a floor or ceiling quad using the specified texture
    public Quad CreateQuad(float y, Texture texture, Vector color)
    {
        Quad q = new Quad(new Vector[] { new Vector(m_vMin.X, y, m_vMin.Z), new Vector(m_vMin.X, y, m_vMax.Z), new Vector(m_vMax.X, y, m_vMax.Z), new Vector(m_vMax.X, y, m_vMin.Z) }, texture, color);
        q.Create();
        return q;
    }


    public void Translate(Vector t)
    {
        foreach (Vector v in m_mesh.m_vertices.m_appData)
            v.Inc (t);
        foreach (Wall w in m_walls)
            w.Translate(t);
    }


    public void Build(string[] stringMap)
    {
        //MapData::Init ();
        m_vMin.Z = -m_vMin.Z;
        m_vMax.Z = -m_vMax.Z;
        Translate(new Vector(0, 0, m_vMax.Z));        // translate the map into the view space
        m_floor = CreateQuad(m_vMin.Y, GetTexture(1), new Vector(1, 1, 1));
        m_ceiling = CreateQuad(m_vMax.Y, GetTexture(2), new Vector(1, 1, 1));
        m_stringMap = stringMap;
        m_segmentMap.Build(m_stringMap, m_walls, m_scale);  // create the segment structure
        CreateVAO();
    }


    new public void Destroy()
    {
        m_floor.Destroy();
        m_ceiling.Destroy();
        m_mesh.Destroy();
        base.Destroy();
    }


    public void Render()
    {
        GL.Disable(GL.CULL_FACE);
        bool useVAO = true;
        if (useVAO)
            m_mesh.Render();
        else
        {
        GL.UseProgram(0);
        m_mesh.GetTexture().Enable();
        int j = 0;
        for (int i = m_mesh.m_vertices.GLDataLength / 12; i > 0; i--)
        {
            GL.Begin(SharpGL.Enumerations.BeginMode.Quads);
            for (int h = 4; h > 0; h--, j++)
            {
                Vector v = m_mesh.m_vertices.m_appData[j];
                GL.Vertex3f(v.X, v.Y, v.Z);
                TexCoord tc = m_mesh.m_texCoords.m_appData[j];
                GL.TexCoord2f(tc.U, tc.V);
            }
            GL.End();
        }
        m_mesh.GetTexture().Disable();

        }
        m_floor.Render();
        m_ceiling.Render();
        GL.Enable(GL.CULL_FACE);
    }


    public MapPosition SegmentAt(Vector position)
    {
        Func<int, int, int, int> Clamp = (val, min, max) => { return Math.Max(min, Math.Min(val, max)); };
        return new MapPosition(
            Clamp ((int) position.X / (int)m_scale, 0, Width()), 
            Clamp ((int)m_segmentMap.m_height + (int)(position.Z / m_scale) - 1, 0, Height())); // segments are added in reversed z order
    }


    // gather all walls #include "the segment a potentially colliding object sits in plus all walls
    // #include "the diagonally adjacent segments (this will yield all relevant walls without duplicates)
    public List<Wall> GetNearbyWalls(Vector position)
    {
        List<Wall> walls = new List<Wall>();
        MapPosition pos = SegmentAt(position);
        foreach (MapPosition o in m_neighbourOffsets)
        {
            MapSegment s = m_segmentMap.GetSegment(pos.m_x + o.m_x, pos.m_y + o.m_y);
            if (s != null)
            {
                foreach (Wall w in s.m_walls)
                    walls.Add(w);
            }
        }
        return walls;
    }



    // determine the amount of actors in each map segment
    public void CountActors()
    {
        m_segmentMap.ResetActorCounts();
        foreach (Actor a in Globals.actorHandler.m_actors)
        {
            if (!a.HavePosition())
                continue;
            Vector p = a.GetPosition();
            if (!Contains(p.X, p.Z))
                Console.Error.WriteLine("actor '{0}' is out of map", a.m_camera.m_name);
            else
            {
                MapPosition pos = SegmentAt(p);
                m_segmentMap.CountActorAt(pos.m_x, pos.m_y);
            }
        }
    }


    public MapPosition RandomSegment()
    {
        return new MapPosition(Globals.rand.Next(0, m_segmentMap.m_width), Globals.rand.Next(0, m_segmentMap.m_height));
    }


    public float FindSpawnAngle(MapSegment s)
    {
        int h = Globals.rand.Next(0, s.m_connections + 1);
        for (int i = 0; i < 4; i++)
            if (s.m_connected[i] && (0 == --h))
                return m_spawnHeadings[i];
        return 0;
    }


    // find a random spawn position by looking for a segment that is not inaccessible and has no actors inside it
    public void FindSpawnPosition(Actor actor)
    {
        // actor.needSpawnPosition = false
        // return
        for (; ; )
        {
            MapPosition p = RandomSegment();
            MapSegment s = m_segmentMap.m_segments[p.m_y][p.m_x];
            if ((s.m_connections > 0) && (s.m_actorCount == 0))
            {
                m_segmentMap.CountActorAt(p.m_x, p.m_y);
                actor.SetPosition(s.m_center);
                actor.m_camera.BumpPosition();
                actor.m_camera.SetOrientation(new Vector(0, FindSpawnAngle(s), 0));
                actor.m_needPosition = false;
                return;
            }
        }
    }


    public void SetSegment(int id, MapSegment segment)
    {
        MapPosition pos = SegPosFromId(id);
        m_segmentMap.m_segments[pos.m_y][pos.m_x] = segment;
    }


    // compute distance between two actors
    // by adding the distances of each actor to the edge center node of its segment that lies
    // in the path between the actors to the distance #include "the segment distance table
    public float Distance(Vector v0, Vector v1)
    {
        if (m_distanceQuality == 0)
            return (v0 - v1).Len() * 1.3f;
        MapPosition p0 = SegmentAt(v0);
        int s0 = SegPosToId(p0.m_x, p0.m_y);
        MapPosition p1 = SegmentAt(v1);
        int s1 = SegPosToId(p1.m_x, p1.m_y);
        if (s0 == s1)
            return 0;
        RouteData rd = m_segmentMap.Distance(s0, s1);
        return rd.m_distance + (v0 - rd.m_startPos).Len() + (v1 - rd.m_endPos).Len();
    }


    // add a vertex to the vertex list && update map boundaries
    public void AddVertex(Vector v)
    {
        m_mesh.m_vertices.Append(v);
        m_vMin.Minimize(v);
        m_vMax.Maximize(v);
        m_vertexCount++;
    }

}

// =================================================================================================
