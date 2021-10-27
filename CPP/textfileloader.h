#pragma once

#include <clist.h>
#include <cstring.h>

// =================================================================================================

class CTextFileLoader {
    public:

        typedef bool (*tLineFilter) (std::string);

        int ReadLines (const char * fileName, CList<CString>& fileLines, tLineFilter filter);
};

// =================================================================================================
