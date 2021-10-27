#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "textfileloader.h"

#include "argHandler.h"
#include "cavltree.h"

// =================================================================================================

CArgValue::CArgValue(CString value, const char* delims) {
    m_value = value;
    m_subValues = Parse (value, delims);
}


CArgValue::CArgValue(CArgValue const& other) {
    Copy (other);
}


CArgValue& CArgValue::operator= (CArgValue const& other) {
    return Copy (other);
}


CArgValue& CArgValue::Copy (CArgValue const& other) {
    m_value = other.m_value;
    if (other.m_subValues) {
        m_subValues = new CList<CArgValue>;
        *m_subValues = *other.m_subValues;
    }
    else
        m_subValues = nullptr;
    return *this;
}


CArgValue::CArgValue (CArgValue&& other) noexcept {
    Move (other);
}


CArgValue& CArgValue::operator= (CArgValue&& other) noexcept {
    return Move (other);
}


CArgValue& CArgValue::Move (CArgValue& other) {
    m_value = other.m_value;
    m_subValues = other.m_subValues;
    other.m_subValues = nullptr;
    return *this;
}


// The argument parser accepts arguments delimited with ';', ',' or ':'
// Their hierarchical order is ";.:". 
// Example: test=a:b:c,d:e:f,g:h:i;1:2:3,4:5:6 would be 
// "a:b:c,d:e:f,g:h:i" and "1:2:3,4:5:6" on the highest level
// "a:b:c", "d:e:f", "g:h:i" and "1:2:3", "4:5:6" on the next level
// arguments are stored recursively
// delimiters can be skipped (you can e.g. use ';' and ':', but not exchanged)

CList<CArgValue>* CArgValue::Parse(CString value, const char* delims) {
    size_t l = strlen(delims);
    if (l == 0)
        return nullptr;
    CList<CArgValue>* subValues = new CList<CArgValue>;
    for (int i = 0; i < l; i++) {
        CList<CString> values = value.Split(delims[i]);
        if (values.Length() > 1) {
            for (const auto [h, v] : values) {
                CArgValue argV(v, delims + i + 1);
                if (argV.m_value.Length ())
                    subValues->Append(argV);
            }
            return subValues;
        }
    }
    delete subValues;
    return nullptr;
}


CString& CArgValue::GetVal (int i) {
    if (!m_subValues || m_subValues->Empty ())
        return m_value;
    return (*m_subValues)[i].m_value;
}

// =================================================================================================

CString CArgument::Create(CString arg) {
    arg = arg.Split('#')[0];
    CList<CString> values = arg.Replace("= ", "=", 1).Replace(" =", "=", 1).Replace("\n", "", 1).Split('=');
    m_key = values[0].Lower();
    if (values.Length() > 1)
        m_values = CArgValue(values[1]);
    else
        m_values = CArgValue (CString ("0"));
    return m_key;
}
        

CString& CArgument::GetVal(int i) {
    return m_values.GetVal(i);
}

    // split all arguments into one uniform value array
    // void Parse (value, delims) {
    //     values = value.split (delims [0])
    //     if (len (delims) == 1) {
    //         return values
    //     subValues = []
    //     for v in values:
    //         subValues += m_Parse (v, delims [1:])
    //     return subValues

// =================================================================================================

CArgHandler::CArgHandler(int argC, char** argV) {
    m_argList.SetComparator (CString::Compare);
    while (argC--)
        Add(CString (*(argV++)));
}


void CArgHandler::Add(CString arg) {
    CArgument a;
    CString key = a.Create(arg);
    m_argList.Insert(key, a);
}


bool CArgHandler::LineFilter (std::string line) {
    return (line [0] != '#') && (line [0] != ';');
}


int CArgHandler::LoadArgs(const char* fileName) {
    CTextFileLoader f;
    CList<CString>  fileLines;

    int argC = f.ReadLines (fileName, fileLines, LineFilter);
    if (argC > 0)
        for (auto [i, line] : fileLines)
            Add(line);
    return argC;
}


CArgument* CArgHandler::GetArg(const char* key) {
    return m_argList.Find(CString(key));
}


const CString CArgHandler::StrVal(const char* key, int i, CString defVal) {
    CArgument* a = GetArg(key);
    return a ? a->GetVal(i) : defVal;
}


int CArgHandler::IntVal(const char* key, int i, int defVal) {
    CArgument* a = GetArg(key);
    return a ? int (a->GetVal(i)) : defVal;
}


float CArgHandler::FloatVal(const char* key, int i, float defVal) {
    CArgument* a = GetArg(key);
    return a ? float (a->GetVal(i)) : defVal;
}


bool CArgHandler::BoolVal(const char* key, int i, bool defVal) {
    return bool(IntVal(key, i, int(defVal)));
}

CArgHandler* argHandler = nullptr;

// =================================================================================================
