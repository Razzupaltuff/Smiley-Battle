#include "udp.h"

// =================================================================================================
// UDP based networking

bool CUDPSocket::Open(CString localAddress, uint16_t localPort) {
    m_localAddress = localAddress;
    m_localPort = localPort;
    if (localAddress == "127.0.0.1") {
        fprintf(stderr, "UDP OpenSocket: Please specify a valid local network or internet address in the command line or ini file\n");
        return false;
    }
    if (!(m_socket = SDLNet_UDP_Open(localPort)))
        return false;
    m_packet = SDLNet_AllocPacket(1500);
    return m_isValid = true;
}


void CUDPSocket::Close(void) {
    if (m_isValid) {
        m_isValid = false;
        // Unbind ();
        SDLNet_UDP_Close(m_socket);
    }
}


int CUDPSocket::Bind (CString& address, uint16_t port) {
    if (0 > (m_channel = SDLNet_ResolveHost (&m_address, (char*) address, port)))
        fprintf (stderr, "Failed to resolve host '%s:%d'\n", (char*) address, port);
    else if (0 > (m_channel = SDLNet_UDP_Bind (m_socket, -1, &m_address)))
        fprintf (stderr, "Failed to bind '%s:%d' to UDP socket\n", (char*) address, port);
    else 
        return 0;
    return m_channel;
}



bool CUDPSocket::Send(CString message, CString address, uint16_t port) {
    if (!m_isValid)
        return false;
    UDPpacket packet = { Bind(address, port), (Uint8*)message.Buffer(), int (message.Length()), int (message.Size()), 0, m_address };
    if (m_channel < 0)
        return false;
    int n = SDLNet_UDP_Send(m_socket, m_channel, &packet);
    Unbind ();
    return (n > 0);
}


CString CUDPSocket::Receive(CString& address, uint16_t& port) {
    if (!m_isValid)
        return CString ("");
    if (0 > (m_packet->channel = Bind(m_localAddress, m_localPort)))
        return CString("");
    int n = SDLNet_UDP_Recv(m_socket, m_packet);
    Unbind();
    if (n <= 0)
        return CString("");
    uint8_t* p = (uint8_t*)&m_packet->address.host;
    char s[16];
    sprintf_s(s, sizeof (s), "%hu.%hu.%hu.%hu", p[0], p[1], p[2], p[3]);
    address = s;
    port = uint16_t(m_packet->address.port);
    return CString((char*)m_packet->data, m_packet->len);
}


CMessage CUDP::Receive(void) {
    CMessage data;
    data.m_payload = m_sockets[0].Receive(data.m_address, data.m_port);
    if (data.m_payload.Find("SMIBAT") == 0)
        data.m_payload = data.m_payload.Replace("SMIBAT", "", 1);
    return data;
}


// =================================================================================================
