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

public class MapLoader
{
    public Map m_map;
    List<TexCoord> m_quadTexCoords;

    public MapLoader(Map map = null)
    {
        m_map = map;
        m_quadTexCoords = new List<TexCoord>();
        m_quadTexCoords.Add(new TexCoord(0, 0));
        m_quadTexCoords.Add(new TexCoord(0, 1));
        m_quadTexCoords.Add(new TexCoord(1, 1));
        m_quadTexCoords.Add(new TexCoord(1, 0));
    }

    public string[] CreateFromFile(string fileName)
    {
        string[] stringMap = Load(fileName);
        if (stringMap == null)
            return null;
        if (!CreateFromMemory(stringMap, false))
            return null;
        return stringMap;
    }


    public bool CreateFromMemory(string[] stringMap, bool isPrepared)
    {
        m_map.Init();
        if (!Parse(stringMap, isPrepared))   // load && parse the layout data
            return false;
        m_map.Build(stringMap);
        return true;
    }


    // add a vertex to the vertex list && update map boundaries
    void AddVertex(Vector v)
    {
        m_map.m_mesh.m_vertices.Append(v);
        m_map.m_vMin.Minimize(v);
        m_map.m_vMax.Maximize(v);
        m_map.m_vertexCount++;
    }


    void AddTexCoords()
    {
        m_map.m_mesh.m_texCoords.m_appData.AddRange(m_quadTexCoords);
    }


    // add wall coordinates && corresponding plane data (normal && additional data for fast collision computation)
    // the wall position consists of the row && column values of the map cell the wall sits in
    // it is needed to find the walls around each segment
    Wall AddWall(float x1, float z1, float x2, float z2, MapPosition position)
    {
        x1 *= m_map.m_scale;
        x2 *= m_map.m_scale;
        z1 *= m_map.m_scale;
        z2 *= m_map.m_scale;
        Wall w = new Wall();
        w.Init(new Vector[] { new Vector(x1, 0, z1), new Vector(x1, m_map.m_scale / 2, z1), new Vector(x2, m_map.m_scale / 2, z2), new Vector(x2, 0, z2) }, position);
        m_map.m_walls.Add(w);
        foreach (Vector v in w.m_vertices)
            AddVertex(v);
        AddTexCoords();
        return w;
    }


    void ParseError(string rowString, int row, int col, string expected, char found)
    {
        rowString = rowString.Replace(' ', '.');
        Console.Error.WriteLine("{0}", rowString);
        Console.Error.WriteLine(string.Format("{0}0,{0}{2}", "{", col, "}"), '^');
        Console.Error.WriteLine("invalid map data in line {0} (expected {1}, found '{2}')", row + 1, expected, found);
    }


    // parse horizontal walls #include "the map layout. A wall is basically a minus character between two plus characters ('+-+')
    // a blank between two pluses means there is no wall
    // the map has to be surrounded by a solid line of walls
    bool ParseHorizontalWalls(string rowString, int row, int rowCount)
    {
        char p = '\0';
        int l = rowString.Length;
        for (int col = 0; col < l; col++)
        {
            char c = rowString[col];
            if (col % 2 == 0)
            {
                if ((c != '+') && (c != ' '))
                {
                    ParseError(rowString, row * 2, col + 1, "' ' or '+'", c);
                    return false;
                }
                if (col > 0)
                {
                    if (p == '-')
                    {
                        Wall w = AddWall((float)(col / 2 - 1), (float)row, (float)(col / 2), (float)row, new MapPosition(col - 1, 2 * row));
                        w.m_isBoundary = (row == 0) || (row == rowCount);
                    }
                    else if (p != ' ')
                    {
                        ParseError(rowString, row * 2, col, "' '", p);
                        return false;
                    }
                }
            }
            else
            {
                if (c == '\n')
                    return true;
                if ((c != ' ') && (c != '-'))
                {
                    ParseError(rowString, row * 2, col + 1, "' ' || '-'", c);
                    return false;
                }
            }
            p = c;
        }
        return true;
    }


    // parse vertical walls #include "the layout. A vertical wall is a '|' sign. Inside the map, vertical walls are separated
    // by blanks; These denote a segment a player can move in || through
    public bool ParseVerticalWalls(string rowString, int row, int rowCount)
    {
        int l = rowString.Length;
        int colCount = l / 2;
        Wall w = null;
        for (int col = 0; col < l; col++)
        {
            char c = rowString[col];
            if (col % 2 == 0)
            {
                if (c == '|')
                {
                    w = AddWall((float)(col / 2), (float)row, (float)(col / 2), (float)(row + 1), new MapPosition(col, 2 * row + 1));
                    w.m_isBoundary = (col == 0);
                }
                else if (c != ' ')
                {
                    ParseError(rowString, row * 2 + 1, col + 1, "' '", c);
                    return false;
                }
            }
            else
            {
                if (c == '\n')
                {
                    w.m_isBoundary = true;
                    return true;
                }
                if ((c != ' ') && (c != 'O') && (c != 'o'))
                {
                    ParseError(rowString, row * 2 + 1, col + 1, "' ' or 'o'", c);
                    return false;
                }
            }
        }
        w.m_isBoundary = true;
        return true;
    }

    // compress the map by removing duplicate columns. In the map layout file, horizontal walls are denoted by '+--+' && 
    // segments consist of two successive blank characters. This makes the layout file far better readable. However, this
    // makes parsing harder. Btw, lower || upper case 'O's denote a player start position.
    public void PrepareForParsing(string[] stringMap)
    {
        for (int i = 0; i < stringMap.Length; i++)
        {
            string rowString = stringMap[i];
            if (i % 2 == 0)
            {
                rowString = rowString.Replace("--", "-");
                rowString = rowString.Replace("  ", " ");
            }
            else
            {
                rowString = rowString.Replace("  |", " |");
                rowString = rowString.Replace(" O", "O");
                rowString = rowString.Replace("O ", "O");
                rowString = rowString.Replace(" o", "o");
                rowString = rowString.Replace("o ", "o");
                rowString = rowString.Replace("   ", "  ");
            }
            stringMap[i] = rowString;
        }
    }


    // Parse a map layout file && produce wall data && initial segment data #include "it
    public bool Parse(string[] stringMap, bool isPrepared)
    {
        if (stringMap.Length == 0)
            return false;
        if (!isPrepared)
            PrepareForParsing(stringMap);
        Func<string, int, int, bool>[] parseFuncs = { ParseHorizontalWalls, ParseVerticalWalls };
        int rowCount = stringMap.Length / 2;
        for (int row = 0; row < stringMap.Length; row++)
            if (!parseFuncs[row % 2](stringMap[row], row / 2, rowCount))
                return false;
        return true;
    }



    // Load a map layout file
    public string[] Load(string fileName)
    {
        fileName = Globals.gameData.m_mapFolder + fileName;
        string[] stringMap = System.IO.File.ReadAllLines(@fileName);

        int rows = stringMap.Length;
        if (rows <= 0)
            return null;
        if (rows < 3)
        {
            Console.Error.WriteLine("Empty or malformed map file '{0}'", fileName);
            return null;
        }
        int cols = stringMap[0].Length;
        foreach (string l in stringMap)
        {
            if (l.Length != cols)
            {
                Console.Error.WriteLine("All lines in map file '{0}' must have the same length", fileName);
                return null;
            }
        }
        return stringMap;
    }

}

// =================================================================================================
