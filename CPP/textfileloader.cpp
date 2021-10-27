#include "textfileloader.h"

#include <iostream>
#include <fstream>
#include <string>

// =================================================================================================

int CTextFileLoader::ReadLines(const char * fileName, CList<CString>& fileLines, tLineFilter filter) {
    std::ifstream f(fileName);
    if (!f.is_open())
        return -1;
    std::string line;
    int lineC = 0;
    while (std::getline(f, line))
        if ((*filter) (line)) {
            lineC++;
            fileLines.Append (CString (line.c_str ()));
        }
    return int (fileLines.Length ());
}

// =================================================================================================
