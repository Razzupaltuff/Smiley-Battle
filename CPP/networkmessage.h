#pragma once 

#include <stdint.h>

#include "cstring.h"
#include "clist.h"
#include "vector.h"

// =================================================================================================
// network data and address

class CMessage {
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

    public:
        CString         m_payload;
        CString         m_address;
        uint16_t        m_port;
        CList<CString>  m_values;
        size_t          m_numValues;
        int             m_result;

        CMessage() : m_numValues (0), m_port (0), m_result (0) {}

        CMessage(CString message, CString address, uint16_t port) {
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


        bool Empty(void) {
            return m_payload.Empty();
        }

        bool IsValid(int valueCount = 0);


        CString Str(size_t i) {
            /*
            return i-th parameter value as text

            Parameters:
            -----------
                i: Index of the requested parameter
            */
            return m_values[i];
        }



        int Int(size_t i) {
            /*
            return i-th parameter value as int

            Parameters:
            -----------
                i: Index of the requested parameter
            */
            return int(m_values[i]);
        }


        float Float(size_t i) {
            /*
            return i-th parameter value as float

            Parameters:
            -----------
                i: Index of the requested parameter
            */
            return float(m_values[i]);
        }


        // format: <x>,<y>,<z> (3 x float)
        CVector Vector(size_t i) {
            /*
            return i-th parameter value as 3D float vector

            Parameters:
            -----------
                i: Index of the requested parameter
            */
            CList<CString> coords = m_values[i].Split(',');
            return CVector(float(coords[0]), float(coords[1]), float(coords[2]));
        }



        // format: <ip v4 address>":"<port>
        // <ip address> = "//.//.//.//" (// = one to three digit subnet id)
        CString Address(size_t i, uint16_t& port) {
            /*
            return i-th parameter value as ip address:port pair

            Parameters:
            -----------
                i: Index of the requested parameter
            */
            CList<CString> values = m_values[i].Split(':');
            port = uint16_t(values[1]);
            return values[0];
        }
};

// =================================================================================================
