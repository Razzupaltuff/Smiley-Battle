#pragma once

#include <string>

#include "cstring.h"
#include "clist.h"
#include "cavltree.h"

// =================================================================================================

class CArgValue {
    public:
        CString             m_value;
        CList<CArgValue> *  m_subValues;

        CArgValue() : m_subValues (nullptr) {}
       
        CArgValue(CString value, const char* delims = ";,:");

        CArgValue(CArgValue const& other);

        CArgValue (CArgValue&& other) noexcept;

        ~CArgValue () {
            if (m_subValues) {
                delete m_subValues;
                m_subValues = nullptr;
            }
        }

        CArgValue& Move (CArgValue& other);
            
        CArgValue& operator= (CArgValue&& other) noexcept;
                
        CArgValue& operator= (CArgValue const& other);

        CArgValue& Copy (CArgValue const& other);

        CList<CArgValue>* Parse (CString value, const char* delims);

        CString& GetVal (int i);
};

// =================================================================================================

class CArgument {
    public:
        CString      m_key;
        CArgValue    m_values;

        CArgument() {}

        CString Create (CString arg);

        CString& GetVal (int i = 0);
};

// =================================================================================================

class CArgHandler {
    public:
        CAvlTree<CString, CArgument>    m_argList;

        CArgHandler() {
            m_argList.SetComparator(CString::Compare);
        }

        static bool LineFilter (std::string line);
            
        CArgHandler(int argC, char** argV);

        void Add(CString arg);

        int LoadArgs(const char* fileName = "smileybattle.ini");

        CArgument* GetArg(const char* key);

        const CString StrVal(const char* key, int i = 0, CString defVal = CString (""));

        int IntVal(const char* key, int i = 0, int defVal = 0);

        float FloatVal(const char* key, int i = 0, float defVal = 0.0f);

        bool BoolVal(const char* key, int i = 0, bool defVal = false);
 };

extern CArgHandler* argHandler;

// =================================================================================================
