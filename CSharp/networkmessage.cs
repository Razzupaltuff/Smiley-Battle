using System;
using System.Collections.Generic;

// =================================================================================================
// network data and address

public class NetworkMessage
{
    /*
    Network message container class

    Attributes:
    -----------
        payload:
            the net message data containing application parameters
        address:
            ip address of the sender
        port:
            udp port of the sender
        values:
            List of single values #include "the payload
        numValues:
            Number of values
        result:
            Result of message processing (test for keyword match and required (minimum) number of values)

    Methods:
    --------
        IsValid (keyword, valueCount = 0):
            check and deconstruct a message
    */

    public string m_payload;
    public string m_address;
    public ushort m_port;
    public string[] m_values;
    public int m_numValues;
    public int m_result;

    public NetworkMessage() { }

    public NetworkMessage(string message, string address, ushort port)
    {
        /*
        Setup meaningful default values during class instance construction

        Parameters:
        -----------
            message:
                data payload as received over the UDP interface
            address:
                sender ip address
            port:
                sender udp port
        */
        m_payload = message;
        m_address = address;
        m_port = port;
        m_numValues = 0;
        m_result = 0;
    }


    public bool Empty()
    {
        return m_payload.Length == 0;
    }

    public bool IsValid(int valueCount = 0)
    {
        /*
            check a message for a match with the requested keyword
            deconstruct message (Split payload it into separate values)
            check message correctness (value count)

        Parameters:
        -----------
            keyword:
                the keyword to check the message for
            valueCount:
                number of application parameters that the message payload should contain.
                > 0: specifies the exact required number of parameters
                < 0: specifies the required minimum number of parameters
                == 0: don't check parameter count
        */
        m_values = m_payload.Split('#');
        string keyword = m_values[0];
        m_values = m_values[1].Split(';');
        if (m_values[0].Length > 0)
            m_numValues = m_values.Length;
        else
        {
            m_values = null;
            m_numValues = 0;
        }
        if (valueCount == 0)
        {
            m_result = 1;
            return true;
        }
        if (valueCount > 0)
        {
            if (m_numValues == valueCount)
            {
                m_result = 1;
                return true;
            }
        }
        else if (valueCount < 0)
        {
            if (m_numValues >= -valueCount)
            {
                m_result = 1;
                return true;
            }
        }
        Console.Error.WriteLine("message {0} has wrong number of values (expected {1}, found {2})", keyword, valueCount, m_numValues);
        m_result = -1;
        return false;
    }



    public string Str(int i)
    {
        /*
        return i-th parameter value as text

        Parameters:
        -----------
            i: Index of the requested parameter
        */
        return m_values[i];
    }



    public int Int(int i)
    {
        /*
        return i-th parameter value as int

        Parameters:
        -----------
            i: Index of the requested parameter
        */
        return Convert.ToInt32(m_values[i]);
    }


    public ushort UShort (int i)
    {
        /*
        return i-th parameter value as int

        Parameters:
        -----------
            i: Index of the requested parameter
        */
        return Convert.ToUInt16(m_values[i]);
    }



    public float Float(int i)
    {
        /*
        return i-th parameter value as float

        Parameters:
        -----------
            i: Index of the requested parameter
        */
        return Convert.ToSingle(m_values[i], System.Globalization.CultureInfo.InvariantCulture);
    }


    // format: <x>,<y>,<z> (3 x float)
    public Vector Vector(int i)
    {
        /*
        return i-th parameter value as 3D float vector

        Parameters:
        -----------
            i: Index of the requested parameter
        */
        return new Vector ().FromString (m_values[i]);
    }



    // format: <ip v4 address>":"<port>
    // <ip address> = "//.//.//.//" (// = one to three digit subnet id)
    public string Address(int i, out ushort port)
    {
        /*
        return i-th parameter value as ip address:port pair

        Parameters:
        -----------
            i: Index of the requested parameter
        */
        string[] values = m_values[i].Split(':');
        port = Convert.ToUInt16(values[1]);
        return values[0];
    }
}

// =================================================================================================
