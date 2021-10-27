#pragma once

#include "vector.h"
#include "plane.h"
#include "texcoord.h"
#include "mapdata.h"
#include "mapsegments.h"
#include "map.h"

// =================================================================================================
// load a map #include "a description / layout file in plain text format (see maps/mazehunt.txt).
// map layout is extremely simple. Everything is rectangular. A map is a rectangle formed by equally 
// sized cuboid segments. Segments may be separated by (flat / 2D) walls.
// A map class instance generates the geometry (wall vertices) and a segment structure #include "the map description.
// Segments have a list of adjacent segments they are connected to (i.e. which are not separated by a wall)
// A simple collision detection handles collisions of spherical objects with map walls.
// The segment structure is used to rapidly identify the walls that are close enough for a collision.
// Segments will also be used for distance calculation to create positional sound.

class CMapLoader {
    public:
        CMap* m_map;
        CList<CTexCoord>   m_quadTexCoords;

        CMapLoader (CMap* map = nullptr) : m_map (map) {
            m_quadTexCoords = { CTexCoord (0,0), CTexCoord (0,1), CTexCoord (1,1), CTexCoord (1,0) };
        }

        bool CreateFromFile (CString fileName, CList<CString>& stringMap);

        bool CreateFromMemory (CList<CString>& stringMap, bool isPrepared = false);

        // add a vertex to the vertex list && update map boundaries
        void AddVertex (CVector v);

        void AddTexCoords (void);

        // add wall coordinates && corresponding plane data (normal && additional data for fast collision computation)
        // the wall position consists of the row && column values of the map cell the wall sits in
        // it is needed to find the walls around each segment
        CWall* AddWall (float x1, float z1, float x2, float z2, CMapPosition position);

        void ParseError (CString& rowString, int row, int col, const char* expected, const char found);

        // parse horizontal walls #include "the map layout. A wall is basically a minus character between two plus characters ('+-+')
        // a blank between two pluses means there is no wall
        // the map has to be surrounded by a solid line of walls
        bool ParseHorizontalWalls (CString& rowString, int row, int rowCount);

        // parse vertical walls #include "the layout. A vertical wall is a '|' sign. Inside the map, vertical walls are separated
        // by blanks; These denote a segment a player can move in || through
        bool ParseVerticalWalls (CString& rowString, int row, int rowCount);

        // compress the map by removing duplicate columns. In the map layout file, horizontal walls are denoted by '+--+' && 
        // segments consist of two successive blank characters. This makes the layout file far better readable. However, this
        // makes parsing harder. Btw, lower || upper case 'O's denote a player start position.
        void PrepareForParsing (CList<CString>& stringMap);

        // Parse a map layout file && produce wall data && initial segment data #include "it
        bool Parse (CList<CString>& stringMap, bool isPrepared = false);

        static bool LineFilter (std::string line);

        // Load a map layout file
        bool Load (CString fileName, CList<CString>& stringMap);

};

// =================================================================================================
