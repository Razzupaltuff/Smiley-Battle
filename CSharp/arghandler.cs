using System;
using System.Collections.Generic;

// =================================================================================================

public class ArgValue
{
    public string m_value;
    public List<ArgValue> m_subValues;

    public ArgValue(string value, string delims = ";,:")
    {
        m_value = value;
        m_subValues = Parse(value, delims);
    }

    public bool IsValid()
    {
        return m_value.Length > 0;
    }


    // The argument parser accepts arguments delimited with ';', ',' or ':'
    // Their hierarchical order is ";.:". 
    // Example: test=a:b:c,d:e:f,g:h:i;1:2:3,4:5:6 would be 
    // "a:b:c,d:e:f,g:h:i" and "1:2:3,4:5:6" on the highest level
    // "a:b:c", "d:e:f", "g:h:i" and "1:2:3", "4:5:6" on the next level
    // arguments are stored recursively
    // delimiters can be skipped (you can e.g. use ';' and ':', but not exchanged)

    public List<ArgValue> Parse(string value, string delims)
    {
        List<ArgValue> subValues = new List<ArgValue>();
        int l = delims.Length;
        if (l == 0)
            return new List<ArgValue>();
        for (int i = 0; i < l; i++)
        {
            string[] values = value.Split(delims[i]);
            if (values.Length > 1)
            {
                foreach (string s in values)
                {
                    ArgValue argV = new ArgValue(s, delims.Substring(1));
                    if (argV.m_value.Length > 0)
                        subValues.Add(argV);
                }
                return subValues;
            }
        }
        return subValues;
    }


    public string GetVal(int i)
    {
        return (m_subValues.Count == 0) ? m_value : m_subValues[i].m_value;
    }

}

// =================================================================================================

public class Argument
{
    public string m_key;
    public ArgValue m_values;

    public Argument() { }

    public bool IsValid ()
        {
            return (m_values != null) && m_values.IsValid();
        }

    public string Create(string arg)
    {
        arg = arg.Split('#')[0];
        string[] values = arg.Replace("= ", "=").Replace(" =", "=").Replace("\n", "").Split('=');
        m_key = values[0].ToLower();
        if (values.Length > 1)
            m_values = new ArgValue(values[1]);
        else
            m_values = new ArgValue("0");
        return m_key;
    }


    public string GetVal(int i)
    {
        return m_values.GetVal(i);
    }

}

// =================================================================================================

public class ArgHandler
{
    public Dictionary<string, Argument> m_argList;

    public ArgHandler()
    {
        m_argList = new Dictionary<string, Argument>();
    }


    public void Add(string arg)
    {
        if (arg.IndexOf("#") == 0)
            return;
        arg = arg.Split('#')[0];
        Argument a = new Argument();
        string key = a.Create(arg);
        m_argList.Add(key, a);
    }


    public bool LineFilter(string line)
    {
        return (line[0] != '#') && (line[0] != ';');
    }


    public int LoadArgs(string[] args)
    {
        if (args == null)
            return 0;
        for (int i = 0; i < args.Length; i++)
            Add(args[i++]);
        return args.Length;
    }


    public int LoadArgs(string fileName)
    {
        string[] fileLines = System.IO.File.ReadAllLines(@fileName);

        int argC = fileLines.Length;
        if (argC > 0)
            foreach (string line in fileLines)
                Add(line);
        return argC;
    }


    public Argument GetArg(string key)
    {
        return m_argList.ContainsKey(key) ? m_argList[key] : new Argument();
    }


    public string StrVal(string key, int i, string defVal)
    {
        Argument a = GetArg(key);
        return (a.m_values.m_value.Length > 0) ? a.GetVal(i) : defVal;
    }


    public int IntVal(string key, int i, int defVal)
    {
        Argument a = GetArg(key);
        return a.IsValid() ? Convert.ToInt32(a.GetVal(i)) : defVal;
    }


    public float FloatVal(string key, int i, float defVal)
    {
        Argument a = GetArg(key);
        return a.IsValid() ? Convert.ToSingle(a.GetVal(i), System.Globalization.CultureInfo.InvariantCulture) : defVal;
    }


    public bool BoolVal(string key, int i, bool defVal)
    {
        return Convert.ToBoolean(IntVal(key, i, Convert.ToInt32(defVal)));
    }
}

// =================================================================================================
