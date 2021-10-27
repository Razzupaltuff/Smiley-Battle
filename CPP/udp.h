#pragma once 

#include <stdint.h>

#include "SDL_net.h"
#include "cstring.h"
#include "networkmessage.h"

// =================================================================================================
// UDP based networking

class CUDPSocket {
    public:
        CString     m_localAddress;
        uint16_t    m_localPort;
        UDPsocket   m_socket;
        UDPpacket*  m_packet;
        IPaddress   m_address;
        int         m_channel;
        bool        m_isValid;

    private:
        int Bind (CString& address, uint16_t port);

        inline void Unbind(void) {
            SDLNet_UDP_Unbind(m_socket, m_channel);
        }

    public:
        CUDPSocket() : m_localAddress(CString("127.0.0.1")), m_localPort(0), m_packet(nullptr), m_channel(0), m_isValid(false) {
            memset(&m_address, 0, sizeof(m_address));
            memset(&m_socket, 0, sizeof(m_socket));
        }

        ~CUDPSocket() {
            if (m_packet) {
                SDLNet_FreePacket(m_packet);
                m_packet = nullptr;
            }
        }

        bool Open(CString localAddress, uint16_t localPort);

        void Close(void);

        bool Send(CString message, CString address, uint16_t port);


        CString Receive(CString& address, uint16_t& port);

};

// =================================================================================================

class CUDP {
    public:

        CString     m_localAddress;
        CUDPSocket  m_sockets[2];

        CUDP() : m_localAddress(CString("127.0.0.1")) {}


        bool OpenSocket(uint16_t port, int type) {     // 0: read, 1: write
            return m_sockets[type].Open(m_localAddress, port);
        }


        inline uint16_t InPort(void) {
            return m_sockets[0].m_localPort;
        }

        inline uint16_t OutPort(void) {
            return m_sockets[1].m_localPort;
        }

        bool Transmit(CString message, CString address, uint16_t port) {
            return m_sockets[1].Send(CString("SMIBAT") + message, address, port);
        }


        CMessage Receive(void);

};

// =================================================================================================
