using System;
using System.Net.Sockets;
using System.Net;
using System.Text;

// =================================================================================================
// UDP based networking

public class UDPSocket
{
    public string m_localAddress;
    public ushort m_localPort;
    public IPAddress m_address;
    public bool m_isValid;
    public UdpClient m_udpClient;


    public UDPSocket() 
    {
        m_localAddress = "127.0.0.1";
    }

    ~UDPSocket()
    {
    }

    public bool Open(string localAddress, ushort localPort)
    {
        m_localAddress = localAddress;
        m_localPort = localPort;
        if (localAddress == "127.0.0.1")
        {
            Console.Error.WriteLine ("UDP OpenSocket: Please specify a valid local network or internet address in the command line or ini file");
            return false;
        }
        m_udpClient = new UdpClient(localPort);
        return true;
    }


    public void Close()
    {
        if (m_udpClient != null)
        {
            m_udpClient.Close();
            m_udpClient = null;
        }
    }


    public bool IsValid ()
    {
        return m_udpClient != null;
    }


    public bool Send(string message, string address, ushort port)
    {
        if (!IsValid())
            return false;
        byte[] bytesToSend = Encoding.ASCII.GetBytes(message);
        return (0 < m_udpClient.Send(bytesToSend, bytesToSend.Length, new IPEndPoint (IPAddress.Parse (address), port)));
    }


    public string Receive(out string address, out ushort port)
    {
        if (!(IsValid () && (m_udpClient.Available > 0))) {
            address = "";
            port = 0;
            return "";
        }
        IPEndPoint ipEndPoint = new IPEndPoint(IPAddress.Any, m_localPort);
        byte[] bytesReceived = m_udpClient.Receive(ref ipEndPoint);
        string[] ipAddress = ipEndPoint.ToString().Split (':');
        address = ipAddress[0];
        port = (ushort) Convert.ToUInt16 (ipAddress[1]);
        return Encoding.ASCII.GetString(bytesReceived);
    }


};

// =================================================================================================

public class UDP
{
    public string m_localAddress;
    public UDPSocket[] m_sockets;

    public UDP() 
    {
        m_localAddress = "127.0.0.1";
        m_sockets = new UDPSocket[2] { new UDPSocket(), new UDPSocket() };
    }


    public bool OpenSocket(ushort port, int type)
    {     // 0: read, 1: write
        return m_sockets[type].Open(m_localAddress, port);
    }


    public ushort InPort()
    {
        return m_sockets[0].m_localPort;
    }

    public ushort OutPort()
    {
        return m_sockets[1].m_localPort;
    }

    public bool Transmit(string message, string address, ushort port)
    {
        return m_sockets[1].Send("SMIBAT" + message, address, port);
    }


    public NetworkMessage Receive()
    {
        NetworkMessage data = new NetworkMessage();
        data.m_payload = m_sockets[0].Receive(out data.m_address, out data.m_port);
        if (data.m_payload.IndexOf("SMIBAT") == 0)
            data.m_payload = data.m_payload.Substring (6);
        return data;
    }

}

// =================================================================================================
