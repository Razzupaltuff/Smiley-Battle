from typing import List

import sys

# =================================================================================================

class CArgValue:
    def __init__ (self, value : str, delims : str = ";,:") -> None:
        self.value = value
        self.subValues = self.Parse (value, delims)

    # The argument parser accepts arguments delimited with ';', ',' or ':'
    # Their hierarchical order is ";.:". 
    # Example: test=a:b:c,d:e:f,g:h:i;1:2:3,4:5:6 would be 
    # "a:b:c,d:e:f,g:h:i" and "1:2:3,4:5:6" on the highest level
    # "a:b:c", "d:e:f", "g:h:i" and "1:2:3", "4:5:6" on the next level
    # arguments are stored recursively
    # delimiters can be skipped (you can e.g. use ';' and ':', but not exchanged)

    def Parse (self, value : str, delims : str) -> list:
        l = len (delims)
        if (l == 0):
            return None
        i = 0
        subValues = []
        while (i < l):
            values = value.split (delims [i])
            if (len (values) > 1):
                for v in values:
                    argV = CArgValue (v, delims [i + 1:])
                    if (argV is not None):
                        subValues.append (argV)
                return subValues
            i += 1
        return None


    def GetVal (self, i : int) -> str:
        return self.value if (self.subValues is None) else self.subValues [i].value

# =================================================================================================

class CArgument ():
    def __init__ (self) -> None:
        self.key = ""
        self.values = None


    def Create (self, arg : str) -> str:
        arg = arg.split ("#")
        values = arg [0].replace ("= ", "=", 1).replace (" =", "=", 1).replace ("\n", "", 1).split ("=")
        self.key = values [0].lower ()
        if (len (values) > 1):
            self.values = CArgValue (values [1])
            # print ("CArgument ({}, {})".format (values [0], values [1]))
        else:
            self.values = CArgValue ("0")
        return self.key
        

    def GetVal (self, i : int = 0) -> str:
        return self.values.GetVal (i)

    # split all arguments into one uniform value array
    # def Parse (self, value, delims):
    #     values = value.split (delims [0])
    #     if (len (delims) == 1):
    #         return values
    #     subValues = []
    #     for v in values:
    #         subValues += self.Parse (v, delims [1:])
    #     return subValues

# =================================================================================================

class CArgHandler ():
    def __init__ (self) -> None:
        self.argList = {}
        for arg in sys.argv:
            self.Add (arg)


    def Add (self, arg):
        a = CArgument ()
        key = a.Create (arg)
        self.argList [key] = a


    def LoadArgs (self, fileName : str = "smileybattle.ini") -> int:
        try:
            with open (fileName, 'r') as f:
                args = f.readlines ()
                f.close ()
        except:
            return -1
        argC : int = 0
        for arg in args:
            if (arg [0] != '#') and (arg [0] != ';'):
                self.Add (arg)
                argC += 1
        return argC


    def GetArg (self, key : str) -> CArgument:
        try:
            a = self.argList [key]
        except KeyError:
            return None
        return a


    def StrVal (self, key, i : int = 0, default : str = None) -> str:
        a = self.GetArg (key)
        if (a is None):
            return default
        return a.GetVal (i)



    def IntVal (self, key, i : int = 0, default : int = None) -> int:
        a = self.GetArg (key)
        if (a is None):
            return default
        return int (a.GetVal (i))


    def FloatVal (self, key, i : int = 0, default : float = None) -> float:
        a = self.GetArg (key)
        if (a is None):
            return default
        return float (a.GetVal (i))


    def BoolVal (self, key, i : int = 0, default : bool = None) -> bool:
        a = self.GetArg (key)
        if (a is None):
            return default
        return bool (int (a.GetVal (i)))

# =================================================================================================
