#include "maploader.h"
#include "gamedata.h"
#include "textfileloader.h"

// =================================================================================================

#ifdef _DEBUG
#   define LOG(msg, ...) fprintf(stderr, msg, ##__VA_ARGS__);
#else
#   define LOG(msg, ...)
#endif

// =================================================================================================
// load a map #include "a description / layout file in plain text format (see maps/mazehunt.txt).
// map layout is extremely simple. Everything is rectangular. A map is a rectangle formed by equally 
// sized cuboid segments. Segments may be separated by (flat / 2D) walls.
// A map class instance generates the geometry (wall vertices) and a segment structure #include "the map description.
// Segments have a list of adjacent segments they are connected to (i.e. which are not separated by a wall)
// A simple collision detection handles collisions of spherical objects with map walls.
// The segment structure is used to rapidly identify the walls that are close enough for a collision.
// Segments will also be used for distance calculation to create positional sound.


bool CMapLoader::CreateFromFile (CString fileName, CList<CString>& stringMap) {
    if (!Load (fileName, stringMap))
        return false;
    if (!CreateFromMemory (stringMap, false))
        return false;
    return true;
}


bool CMapLoader::CreateFromMemory (CList<CString>& stringMap, bool isPrepared) {
    LOG ("init map")
    m_map->Init ();
    LOG ("parsing map\n")
        if (!Parse (stringMap, isPrepared))   // load && parse the layout data
        return false;
    //m_map->m_stringMap.Move (stringMap);
    LOG ("building map\n")
        m_map->Build ();
    LOG ("building map done\n")
        return true;
}


// add a vertex to the vertex list && update map boundaries
void CMapLoader::AddVertex (CVector v) {
    m_map->m_mesh.m_vertices.Append (v);
    m_map->m_vMin.Minimize (v);
    m_map->m_vMax.Maximize (v);
    m_map->m_vertexCount++;
}


void CMapLoader::AddTexCoords (void) {
    m_map->m_mesh.m_texCoords.m_appData += m_quadTexCoords;
}


// add wall coordinates && corresponding plane data (normal && additional data for fast collision computation)
// the wall position consists of the row && column values of the map cell the wall sits in
// it is needed to find the walls around each segment
CWall* CMapLoader::AddWall (float x1, float z1, float x2, float z2, CMapPosition position) {
    x1 *= m_map->m_scale;
    x2 *= m_map->m_scale;
    z1 *= m_map->m_scale;
    z2 *= m_map->m_scale;
    CWall * w = m_map->m_walls.Add (-1);
    w->Init ({ CVector (x1, 0, z1), CVector (x1, m_map->m_scale / 2, z1), CVector (x2, m_map->m_scale / 2, z2), CVector (x2, 0, z2) }, position);
    for (auto v : w->m_vertices)
        AddVertex (*v);
    AddTexCoords ();
    return w;
}


void CMapLoader::ParseError (CString& rowString, int row, int col, const char* expected, const char found) {
    for (int i = 0; i < rowString.Length (); i++)
        if (rowString [i] == ' ')
            rowString [i] = '.';
    fprintf (stderr, "%s\n", rowString.Buffer ());
    char fmt [20];
    sprintf_s (fmt, sizeof (fmt), "%%%dc\n", col);
    fprintf (stderr, fmt, '^');
    fprintf (stderr, "invalid map data in line %d (expected %s, found '%c')\n", row + 1, expected, found);
}


// parse horizontal walls #include "the map layout. A wall is basically a minus character between two plus characters ('+-+')
// a blank between two pluses means there is no wall
// the map has to be surrounded by a solid line of walls
bool CMapLoader::ParseHorizontalWalls (CString& rowString, int row, int rowCount) {
    char p = '\0';
    int l = int (rowString.Length ());
    for (int col = 0; col < l; col++) {
        char c = rowString [col];
        if (col % 2 == 0) {
            if ((c != '+') && (c != ' ')) {
                ParseError (rowString, row * 2, col + 1, "' ' or '+'", c);
                return false;
            }
            if (col > 0) {
                if (p == '-') {
                    CWall* w = AddWall (float (col / 2 - 1), float (row), float (col / 2), float (row), CMapPosition (col - 1, 2 * row));
                    w->m_isBoundary = (row == 0) || (row == rowCount);
                }
                else if (p != ' ') {
                    ParseError (rowString, row * 2, col, "' '", p);
                    return false;
                }
            }
        }
        else {
            if (c == '\n')
                return true;
            if ((c != ' ') && (c != '-')) {
                ParseError (rowString, row * 2, col + 1, "' ' || '-'", c);
                return false;
            }
        }
        p = c;
    }
    return true;
}


// parse vertical walls #include "the layout. A vertical wall is a '|' sign. Inside the map, vertical walls are separated
// by blanks; These denote a segment a player can move in || through
bool CMapLoader::ParseVerticalWalls (CString& rowString, int row, int rowCount) {
    int l = int (rowString.Length ());
    int colCount = l / 2;
    CWall* w = nullptr;
    for (int col = 0; col < l; col++) {
        char c = rowString [col];
        if (col % 2 == 0) {
            if (c == '|') {
                w = AddWall (float (col / 2), float (row), float (col / 2), float (row + 1), CMapPosition (col, 2 * row + 1));
                w->m_isBoundary = (col == 0);
            }
            else if (c != ' ') {
                ParseError (rowString, int (row * 2 + 1), int (col + 1), "' '", c);
                return false;
            }
        }
        else {
            if (c == '\n') {
                w->m_isBoundary = true;
                return true;
            }
            if ((c != ' ') && (c != 'O') && (c != 'o')) {
                ParseError (rowString, int (row * 2 + 1), int (col + 1), "' ' or 'o'", c);
                return false;
            }
        }
    }
    w->m_isBoundary = true;
    return true;
}

// compress the map by removing duplicate columns. In the map layout file, horizontal walls are denoted by '+--+' && 
// segments consist of two successive blank characters. This makes the layout file far better readable. However, this
// makes parsing harder. Btw, lower || upper case 'O's denote a player start position.
void CMapLoader::PrepareForParsing (CList<CString>& stringMap) {
    for (auto [i, rowString] : stringMap) {
        if (i % 2 == 0) {
            rowString = rowString.Replace ("--", "-");
            rowString = rowString.Replace ("  ", " ");
        }
        else {
            rowString = rowString.Replace ("  |", " |");
            rowString = rowString.Replace (" O", "O");
            rowString = rowString.Replace ("O ", "O");
            rowString = rowString.Replace (" o", "o");
            rowString = rowString.Replace ("o ", "o");
            rowString = rowString.Replace ("   ", "  ");
        }
        stringMap [i] = rowString;
    }
}


// Parse a map layout file && produce wall data && initial segment data #include "it
bool CMapLoader::Parse (CList<CString>& stringMap, bool isPrepared) {

    typedef bool (CMapLoader::* tParseFunc) (CString&, int, int);

    if (stringMap.Empty ())
        return false;
    if (!isPrepared)
        PrepareForParsing (stringMap);
    tParseFunc parseFuncs[] = { &CMapLoader::ParseHorizontalWalls, &CMapLoader::ParseVerticalWalls };
    int rowCount = int (stringMap.Length () / 2);
    for (auto [row, rowString] : stringMap)
        if (!(this->*parseFuncs [row % 2]) (rowString, int (row / 2), rowCount))
            return false;
    return true;
}



bool CMapLoader::LineFilter (std::string line) {
    return true;
}


// Load a map layout file
bool CMapLoader::Load (CString fileName, CList<CString>& stringMap) {
    CTextFileLoader f;
    CList<CString>  fileLines;

    stringMap.Destroy ();
    fileName = gameData->m_mapFolder + fileName;
    int rows = f.ReadLines (fileName.Buffer (), stringMap, LineFilter);
    if (rows < 0)
        return false;
    if (rows < 3) {
        fprintf (stderr, "Empty or malformed map file '%s'\n", fileName.Buffer ());
        stringMap.Destroy ();
        return false;
    }
    size_t cols = stringMap [0].Length ();
    for (auto [i, l] : stringMap) {
        if (l.Length () != cols) {
            fprintf (stderr, "All lines in map file '%s' must have the same length\n", fileName.Buffer ());
            return false;
            }
        }
    return true;
}

// =================================================================================================
