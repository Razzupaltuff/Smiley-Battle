using System;
using System.Collections.Generic;

// =================================================================================================
// High level networking functions
//
// Full n x n peer to peer network
//
// Every player manages the data of his own actor && any shots he fires. This means that every player
// - detects whether his character has been hit && who hit it && will report that to all other players
// - updates all other players about his position && heading && the position && heading of all his shots
//
// This means that a client does !move around other actors than his own avatar && his own shots
// He also handles all collisions with his avatar, but only updates to his avatar's position will persist;
// Updates to other players' positions will be overriden by their update messages. For players with laggy
// connections, this can lead to objects penetrating each other. Oh well.
// 
// - The game host's only task is to accept new players, give them the list of all players already in the game 
// && assign an available color to them
// - If the game host leaves #include "the game, he will randomly make another player game host
// - If the game host gets disconnected, nobody can join a match in progress anymore, but any players in the match
//   can keep playing
//
// For scoring purposes, players are identified by their color. Projectiles are identified by their parent player's color 
// && an id that is unique on their client. The (id:color) tuple forms a match-wide unique projectile id by which projectiles
// can be unambiguously identified on each participant machine. Players always have id 0 (zero).
//
// Network message types:
//
// - Every message will be prefixed with "SMIBAT", followed by a message index, followed by an optional value list
// - Values will be semicolon separated
// Find all message below in CnetworkHandler.messageHandlers
//
// Alternatively, the game host could be taking over processing all collisions && updating all players
// with all actor positions && headings. In that case, every client would only send his data to the host,
// the host would Handle all client data && send the result to every client. The other clients would only need
// to talk to each other in case the host unexpectedly drops out of the game (e.g. disconnects), in which case
// they would need to negotiate a new game host. To negotiate a new host, each client could create a random number
// && send it to each other. The client with the lowest number becomes the new host. In case of a tie, the tied
// clients repeat this until resolution.
//
// All data will be transmitted as text (I have no idea how to transform binary data back to object data in Python).
// However, since there isn't a lot of data, this shouldn't be a problem.
// Heading && position will be transmitted as three float angles (pitch, yaw, bank) && three float coordinates (x,y,z)
// They will be packed in a string && can be parsed out of the string by the receiver.

public class NetworkHandler : UDP
{
    enum eJoinState {
        jsApply = 0,
        jsMap = 1,          // wait for the host to send map data
        jsParams = 2,       // wait for the host to send global game parameters
        jsPlayers = 3,      // wait for the host to send the list of participants
        jsProjectiles = 4,  // wait for the host to send the list of shots currently present
        jsEnter = 5,        // wait for the host to send position && heading
        jsConnected = 6     // connected, ready to play
    };

        // ========================================

        public class MessageHandler
        {
            public string m_name;
            public Func<NetworkMessage, int> m_handler;

            public MessageHandler() { }

            public MessageHandler(string name, Func<NetworkMessage, int> handler)
            {
                m_name = name;
                m_handler = handler;
            }
        }

        // ========================================

    ushort[] m_localPorts;
    public string m_hostAddress;
    ushort[] m_hostPorts;
    public List<NetworkMessage> m_messages;

    MessageHandler[] m_messageHandlers;
    Action[] m_joinStateHandlers;
    SortedDictionary<string, string> m_idMap;

    int m_fps;
    int m_frameTime;
    Timer m_updateTimer;
    Timer m_joinTimer;
    Timer m_joinStateTimer;
    int m_joinDelay;        // 5 seconds between two consecutive join attempts
    int m_joinStateDelay;
    int m_timeoutPeriod;    // seconds [ms] without message #include "a player after which the player will be removed #include "the game
    string[] m_stringMap;
    int m_mapRow;
    bool m_threadedListener;
    eJoinState m_joinState;
    string m_syncingAddress;

    // ========================================

    public NetworkHandler() : base()
    {
        m_updateTimer = new Timer();
        m_joinTimer = new Timer();
        m_joinStateTimer = new Timer ();
        m_localAddress = Globals.argHandler.StrVal("localaddress", 0, "127.0.0.1");
        m_localPorts = new ushort[2];
        m_localPorts[0] = (ushort)Globals.argHandler.IntVal("inport", 0, 9100);
        m_localPorts[1] = (ushort)Globals.argHandler.IntVal("outport", 0, 9101);
        m_hostAddress = Globals.argHandler.StrVal("hostaddress", 0, "127.0.0.1");
        m_hostPorts = new ushort[2];
        m_hostPorts[0] = (ushort)Globals.argHandler.IntVal("hostport", 0, 0);
        m_hostPorts[1] = 0;
        m_messages = new List<NetworkMessage>();
        m_messageHandlers = new MessageHandler[]
        {
        new MessageHandler("APPLY", HandleApply),                     // process join request sent to game host
        new MessageHandler("ACCEPT", HandleAccept),                   // receive position && heading #include "game host
        new MessageHandler("REQUESTMAP", HandleMapRequest),           // send map data to new player
        new MessageHandler("SYNCMAP", SyncMap),                       // process map data #include "game host
        new MessageHandler("REQUESTPARAMS", HandleParamsRequest),     // send game parameters to new player
        new MessageHandler("SYNCPARAMS", SyncParams),                 // process game parameters #include "game host
        new MessageHandler("REQUESTPLAYERS", HandlePlayersRequest),   // send list of all players to new player
        new MessageHandler("SYNCPLAYERS", SyncPlayers),               // add all players #include "player message to actor list that aren't already in it
        new MessageHandler("REQUESTSHOTS", HandleProjectilesRequest), // send currently live projectiles to new player
        new MessageHandler("SYNCSHOTS", SyncProjectiles),             // add projectiles #include "message to actor list
        new MessageHandler("ENTER", HandleEnter),                     // compute position && heading for new player && send them to him/her
        new MessageHandler("ANIMATION", HandleAnimation),             // update sending player's animation state (which determines whether && which animation sound to play)
        new MessageHandler("UPDATE", HandleUpdate),                   // update all local actors owned by the sending player with positions && headings #include "the message
        new MessageHandler("FIRE", HandleFire),                       // create a projectile fired by another player
        new MessageHandler("HIT", HandleHit),                         // integrate hit at a player into local player data
        new MessageHandler("DESTROY", HandleDestroy),                 // destroy a projectile that had hit another player
        new MessageHandler("LEAVE", HandleLeave),                     // remove sending player #include "player list
        new MessageHandler("REJECT", HandleReject)                    // react to some message sent to another player having been rejected by that player for some reason
        };

        m_idMap = new SortedDictionary<string, string>();
          int id = 0;
        foreach (MessageHandler h in m_messageHandlers)
            m_idMap.Add(h.m_name, Convert.ToString(id++) + "#");

        m_joinStateHandlers = new Action[]
        {
        SendApply, SendSyncMap, SendSyncParams,
        SendSyncPlayers, SendSyncProjectiles, SendEnter
        };
        m_fps = 60;
        m_frameTime = 1000 / m_fps;             // m_fps
        m_joinDelay = 5000;             // 5 seconds between two consecutive join attempts
        m_joinStateDelay = 500;
        m_timeoutPeriod = 30 * 1000;    // seconds [ms] without message #include "a player after which the player will be removed #include "the game
        m_mapRow = -1;
        m_threadedListener = Globals.argHandler.BoolVal("multithreading", 0, true);
        m_joinState = IamMaster() ? eJoinState.jsConnected : eJoinState.jsApply;
        m_syncingAddress = "";
    }


    ~NetworkHandler()
    {
        Destroy();
    }


    public void Create(string address = "", ushort port = 0)
    {
        if ((address.Length > 0) && (port != 0))
            SetHostAddress(address, port);
        OpenSocket(m_localPorts[0], 0);
        OpenSocket(m_localPorts[1], 1);
        Globals.actorHandler.m_viewer.SetAddress(m_localAddress, m_localPorts);
        if (m_threadedListener)
            NetworkListener.Start();
    }

    public void Destroy()
    {
        if (m_threadedListener)
            NetworkListener.Stop();
    }


    public void SetHostAddress(string address, ushort port)
    {
        m_hostAddress = address;
        m_hostPorts[0] = port;
    }


    // message handling helper functions ========================================

    string BuildMessage(string delim, params string[] values)
    {
        int dl = delim.Length;
        string message = "";
        for (int i = 0; i < values.Length; i++)
        {
            message += values [i];
            if ((dl > 0) && (i < values.Length - 1))
                message += delim;
        }
        return message;
    }


    // construct a message with all update info for actor 
    string UpdateMessage(Actor actor)
    {
        if (actor.IsPlayer())
            return BuildMessage(";",
                Convert.ToString(actor.GetId()),
                Convert.ToString(actor.GetColorIndex()),
                actor.GetPosition().ToString (),
                actor.GetOrientation().ToString (),
                Convert.ToString(actor.m_hitPoints),
                Convert.ToString(actor.GetScore()),
                Convert.ToString((int) actor.m_lifeState),
                Convert.ToString(actor.m_scale),
                Convert.ToString(actor.GetPort())
                );
        else
            return BuildMessage(";",
                Convert.ToString(actor.GetId()),
                Convert.ToString(actor.GetColorIndex()),
                actor.GetPosition().ToString (),
                actor.GetOrientation().ToString ()
                );
    }


    string PlayerMessage(Player player)
    {
        return BuildMessage(";", player.GetAddress(), Convert.ToString(player.GetPort(0)), Convert.ToString(player.GetPort(1)), Convert.ToString(player.GetColorIndex()));
    }


    string IdFromName(string textId)
    {
        return (m_idMap.ContainsKey(textId)) ? m_idMap[textId] : "";
    }


    int IdFromMessage(NetworkMessage message)
    {
        string[] values = message.m_payload.Split('#');
        if (values.Length == 0)
            return -1;
        int id = Convert.ToInt32(values[0]);
        return (id < m_messageHandlers.Length) ? id : -1;
    }


    // networking helper functions ========================================

    Player FindPlayer(string address, ushort port)
    {
        foreach (Actor a in Globals.actorHandler.m_actors)
            // check for port too as there may be players in the same local network && behind the same router/firewall, sharing the same ip address && just using different ports
            if (a.IsPlayer() && (a.GetAddress() == address) && (a.GetPort(1) == port))
                return (Player)a;
        return null;
    }


    int RemotePlayerCount()
    {
        int rpc = 0;
        foreach (Actor a in Globals.actorHandler.m_actors)
            if (a.IsPlayer() && !a.IsLocalActor())
                rpc++;
        return rpc;
    }


    void UpdateLastMessageTime(NetworkMessage message)
    {
        Player player = FindPlayer(message.m_address, message.m_port);
        if (player != null)
            player.UpdateLastMessageTime();
    }


    public bool IamMaster()
    {
        return (m_hostAddress == "127.0.0.1") || (m_hostAddress == m_localAddress);
    }


    int IamConnected()
    {
        if (IamMaster())
            return 1;
        if (m_joinState == eJoinState.jsApply)
            return 0;
        Player host = FindPlayer(m_hostAddress, m_hostPorts[1]);
        if (host == null)
            return 0;
        if (TimedOut(host))
            return -1;
        return 1;
    }

    bool TimedOut(Player player)
    {
        return !player.IsLocalActor() && (Globals.gameData.m_gameTime - player.m_lastMessageTime > m_timeoutPeriod);
    }

    // send functions ========================================

    void SendApply()
    {
        Transmit(BuildMessage("", IdFromName("APPLY"), Convert.ToString(InPort())), m_hostAddress, m_hostPorts[0]);
    }


    void SendSyncMap()
    {
        m_mapRow = -1;
        m_stringMap = null;
        Transmit(BuildMessage("", IdFromName("REQUESTMAP"), Convert.ToString(Globals.actorHandler.m_viewer.GetPort(0))), m_hostAddress, m_hostPorts[0]);
    }


    void SendSyncParams()
    {
        Transmit(BuildMessage("", IdFromName("REQUESTPARAMS"), Convert.ToString(Globals.actorHandler.m_viewer.GetPort(0))), m_hostAddress, m_hostPorts[0]);
    }


    void SendSyncProjectiles()
    {
        Transmit(BuildMessage("", IdFromName("REQUESTSHOTS"), Convert.ToString(Globals.actorHandler.m_viewer.GetPort(0))), m_hostAddress, m_hostPorts[0]);
    }


    void SendSyncPlayers()
    {
        Transmit(BuildMessage("", IdFromName("REQUESTPLAYERS"), Convert.ToString(Globals.actorHandler.m_viewer.GetPort(0))), m_hostAddress, m_hostPorts[0]);
    }


    void SendEnter()
    {
        SendEnter(m_hostAddress, m_hostPorts[0]);
    }


    void SendEnter(string address, ushort port)
    {
        if (address == "")
        {
            address = m_hostAddress;
            port = m_hostPorts[0];
        }
        Transmit(BuildMessage("", IdFromName("ENTER"), Convert.ToString(Globals.actorHandler.m_viewer.m_colorIndex), ";", Convert.ToString(InPort())), address, port);
    }


    // accept tells the player requesting to join his own ip address since it it somewhat tedious to determine your own external ip address when behing a router, firewall && what not
    void SendAccept(Player player)
    {
        Transmit(
            BuildMessage(";", 
                IdFromName("ACCEPT") + Convert.ToString(player.m_colorIndex), 
                player.GetPosition().ToString(), 
                player.GetOrientation().ToString(), 
                player.m_address), 
            player.m_address, 
            player.GetPort(0));
    }


    // format: MAP:<tag>:<text line #include "map file>
    //         tags: reversed line number (starting at line count of map - 1) => 0 for last line
    void SendMap(string address, ushort port)
    {
        int l = Globals.gameItems.m_map.m_stringMap.Length;
        foreach (string s in Globals.gameItems.m_map.m_stringMap)
            Transmit(BuildMessage(";", IdFromName("SYNCMAP") + Convert.ToString(--l), s), address, port);
    }


    // format: PARAMS:<fire delay>;<heal delay>;<respawn delay>;<immunity duration>;<points for kill>;<projectile size>;<projectile speed>;<move speed>;<turn speed>
    void SendParams(string address, ushort port)
    {
        string message = BuildMessage(";",
            IdFromName("SYNCPARAMS") +
            Convert.ToString(Globals.gameData.m_fireMode),
            Convert.ToString(Globals.gameData.m_fireDelay),
            Convert.ToString(Globals.gameData.m_healDelay),
            Convert.ToString(Globals.gameData.m_respawnDelay),
            Convert.ToString(Globals.gameData.m_immunityDuration),
            Convert.ToString(Globals.gameData.m_pointsForKill),
            Convert.ToString(Globals.gameData.m_projectileSize),
            Convert.ToString(Globals.gameData.m_projectileSpeed),
            Convert.ToString(Globals.controlsHandler.GetMoveSpeed()),
            Convert.ToString(Globals.controlsHandler.GetTurnSpeed()));
        Transmit(message, address, port);
    }


    // format: PLAYERS<player data>[;<player data> [...]]
    // player data: <address>:<port>;<color index>
    void SendPlayers(string address, ushort port)
    {
        string message = IdFromName("SYNCPLAYERS");
        // add addresses of all other players to player requesting to join
        int i = 0;
        foreach (Actor a in Globals.actorHandler.m_actors)
        {
            if (a.IsPlayer())
            {  // player
                if (i > 0)    // !the first entry in the list
                    message += ";";
                message += BuildMessage(";", a.GetAddress() + ":" + Convert.ToString(a.GetPort(0)), Convert.ToString(a.GetPort(1)), Convert.ToString(a.GetColorIndex()));
            }
            i++;
        }
        Transmit(message, address, port);
    }


    // format: SHOTS<shot data>[;<shot data> [...]]
    // shot data: <actor id>;<parent color index>
    void SendProjectiles(string address, ushort port)
    {
        string message = IdFromName("SYNCSHOTS");
        // add addresses of all other players to player requesting to join
        int i = 0;
        foreach (Actor a in Globals.actorHandler.m_actors)
        {
            if (a.IsProjectile())
            {  // projectile
                if (i > 0)    // !the first entry in the list
                    message += ";";
                message += Convert.ToString(a.m_id);
                message += ";";
                message += Convert.ToString(a.GetColorIndex());
            }
            i++;
        }
        Transmit(message, address, port);
    }


    // send an update message to a single player
    void SendUpdate(string address, ushort port)
    {
        Transmit(IdFromName("UPDATE") + UpdateMessage(Globals.actorHandler.m_viewer), address, port);
    }


    void SendReject(string address, ushort port, string reason)
    {
        Transmit(IdFromName("REJECT") + reason, address, port);
    }


    Player AddPlayer(string address, ushort[] ports, int colorIndex)
    {
        Player player = FindPlayer(address, ports[1]);
        if (player == null)
        {
            player = Globals.actorHandler.CreatePlayer(colorIndex, new Vector(float.NaN, float.NaN, float.NaN), new Vector(0, 0, 0), address, ports[0], ports[1]);
            if (player == null)
                SendReject(address, ports[0], "full");
            else
                player.UpdateLastMessageTime();
        }
        return player;
    }


    bool OutOfSync(eJoinState joinState, bool isFinal = true)
    {
        if (m_joinState == joinState)
            return false;
        // m_joinState = eJoinState.jsApply
        if (isFinal)
            SendReject(m_hostAddress, m_hostPorts[0], "out of sync");
        return true;
    }

    // incoming message processing functions ========================================

    //format: APPLY<host address>;<client rx port>
    int HandleApply(NetworkMessage message)
    {
        if (!message.IsValid(1))
            return message.m_result;
        if (m_syncingAddress == "")
            m_syncingAddress = message.m_address;
        else if (m_syncingAddress != message.m_address)
        {
            SendReject(message.m_address, message.UShort(0), "sync in progress");
            return -1;
        }
        SendEnter(message.m_address, message.UShort(0));
        return 1;
    }


    //format: ENTER<client color>;<client rx port>
    int HandleEnter(NetworkMessage message)
    {
        if (!message.IsValid(2))
            return message.m_result;
        if (OutOfSync(eJoinState.jsConnected, false) && OutOfSync(eJoinState.jsApply)) // this message is permitted when already connected
            return -1;
        int colorIndex = message.Int(0);
        if (IamMaster())
        {   // reassign color if requested color is already in use
            if (!Globals.gameData.ColorIsAvailable(colorIndex))
                colorIndex = -1;
        }
        ushort[] ports = { message.UShort(1), message.m_port };
        Player player = AddPlayer(message.m_address, ports, colorIndex);  // colorIndex == -1 -. assign an available random color
        if (player == null)
            return -1;
        if (IamMaster())
        {
            Globals.gameItems.m_map.FindSpawnPosition(player);
            player.UpdateLastMessageTime();
            SendAccept(player);
            m_syncingAddress = "";
        }
        else
        {
            if (message.m_address == m_hostAddress)
                m_hostPorts[1] = message.m_port;
        }
        if (m_joinState == eJoinState.jsApply)
            m_joinState = eJoinState.jsMap;
        return 1;
    }


    int HandleMapRequest(NetworkMessage message)
    {
        if (!message.IsValid(1))
            return message.m_result;
        SendMap(message.m_address, message.UShort(0));
        return 1;
    }


    // format:MAP<row //>;<row text>
    int SyncMap(NetworkMessage message)
    {
        if (!message.IsValid(2))
            return message.m_result;
        if (OutOfSync(eJoinState.jsMap))
            return -1;
        int row = message.Int(0);
        // m_mapRow < 0 ==> we're receiving the first map row
        // map row number are sent in reverse order (highest first, down to zero)
        if (m_mapRow < 0)
            m_stringMap = new string[row + 1];
        else if (row != m_mapRow - 1)
            return -1;   // we lost at least one map row
        m_mapRow = row;
        // map line numbers arrive in reverse order; range: [0 .. stringMap.Length - 1]
        m_stringMap [m_stringMap.Length - row - 1] = message.Str(1);
        if (row > 0) // more rows to come
            return 1;
        if (!Globals.gameItems.CreateMap(m_stringMap, true))
            return -1;
        m_joinState = eJoinState.jsParams;
        return 1;
    }


    int HandleParamsRequest(NetworkMessage message)
    {
        if (!message.IsValid(1))
            return message.m_result;
        SendParams(message.m_address, message.UShort(0));
        return 1;
    }


    // format: PARAMS<heal delay>;<respawn delay>;<immunity duration>;<move speed>;<turn speed>
    int SyncParams(NetworkMessage message)
    {
        if (!message.IsValid(9))
            return message.m_result;
        if (OutOfSync(eJoinState.jsParams))
            return -1;
        Globals.gameData.m_fireMode = message.Int(0);
        Globals.gameData.m_fireDelay = message.Int(1);
        Globals.gameData.m_healDelay = message.Int(2);
        Globals.gameData.m_respawnDelay = message.Int(3);
        Globals.gameData.m_immunityDuration = message.Int(4);
        Globals.gameData.m_pointsForKill = message.Int(5);
        Globals.gameData.m_projectileSize = message.Float(6);
        Globals.gameData.m_projectileSpeed = message.Float(7);
        Globals.controlsHandler.SetMoveSpeed(message.Float(8));
        Globals.controlsHandler.SetTurnSpeed(message.Float(9));
        m_joinState = eJoinState.jsPlayers;
        return 1;
    }


    int HandlePlayersRequest(NetworkMessage message)
    {
        if (!message.IsValid(1))
            return message.m_result;
        SendPlayers(message.m_address, message.UShort(0));
        return 1;
    }


    // add each player #include "message to actor list which isn't in it already
    int SyncPlayers(NetworkMessage message)
    {
        if (!message.IsValid(-3))
            return message.m_result;
        if (OutOfSync(eJoinState.jsConnected, false) && OutOfSync(eJoinState.jsPlayers))  // this message is permitted when already connected
            return -1;
        for (int i = 0; i < message.m_numValues; i += 3)
        {
            int colorIndex = message.Int(i + 2);
            Player player = Globals.actorHandler.FindPlayer(colorIndex);
            if (player != null)
            {
                ushort inPort;
                string address = message.Address(i, out inPort);
                ushort[] ports = { inPort, message.UShort(i + 1) };
                player = AddPlayer(address, ports, colorIndex);
                if (player == null)
                {    // synchronization error
                    SendReject(m_hostAddress, m_hostPorts[0], "out of sync");
                    return -1;
                }
                SendEnter(address, inPort);
            }
        }
        if (m_joinState == eJoinState.jsPlayers)       // this message will periodically be sent to all connected players
            m_joinState = eJoinState.jsProjectiles;    // so only change join state when actually joining
        return 1;
    }


    int HandleProjectilesRequest(NetworkMessage message)
    {
        if (!message.IsValid(1))
            return message.m_result;
        SendProjectiles(message.m_address, message.UShort(0));
        return 1;
    }


    // add each projectile #include "message to actor list 
    int SyncProjectiles(NetworkMessage message)
    {
        if (!message.IsValid(0))
            return message.m_result;
        if (OutOfSync(eJoinState.jsProjectiles))  // this message is permitted when already connected
            return -1;
        for (int i = 0; i < message.m_numValues; i += 2)
        {
            int colorIndex = message.Int(i + 1);
            Player parent = Globals.actorHandler.FindPlayer(colorIndex);
            if (parent != null)
                Globals.actorHandler.CreateActor(message.Int(i), colorIndex, parent.GetPosition(), parent.GetOrientation());
        }
        // SendEnter (m_hostAddress, m_hostPorts [0])
        m_joinState = eJoinState.jsEnter;
        return 1;
    }



    // format: ACCEPT<color>;<position>;<orientation>;<address>;<address>:<port>;...;<address>:<port>
    // <id> is always 0 (zero) for players && hence will be ignored
    // ip addresses && ports of all other players including the game host will be transmitted
    int HandleAccept(NetworkMessage message)
    {
        if (!message.IsValid(4))   // need at least five values
            return message.m_result;
        if (OutOfSync(eJoinState.jsEnter))
            return -1;
        Globals.actorHandler.m_viewer.SetColorIndex(message.Int(0), true);
        Globals.actorHandler.m_viewer.SetPosition(message.Vector(1));
        Globals.actorHandler.m_viewer.SetOrientation(message.Vector(2));
        Globals.actorHandler.m_viewer.m_address = message.Str(3);
        Globals.actorHandler.m_viewer.ForceRespawn();
        m_joinState = eJoinState.jsConnected;
        return 1;
    }


    // format: FIRE<projectile id>;<projectile parent color index>
    int HandleFire(NetworkMessage message)
    {
        if (!message.IsValid(2))
            return message.m_result;
        Player parent = Globals.actorHandler.FindPlayer(message.Int(1));
        if (parent == null)
            return -1;
        return (Globals.actorHandler.CreateProjectile(parent, message.Int(0)) != null) ? 1 : -1;
    }


    int HandleAnimation(NetworkMessage message)
    {
        if (!message.IsValid(2))
            return message.m_result;
        Actor actor = Globals.actorHandler.FindActor(0, message.Int(0));
        if (actor == null)
            return 0;
        actor.SetAnimation(message.Int(1));
        return 1;
    }


    // message: UPDATE<id>;<color>;<position>;<orientation>[;<player info>]
    // position: <x>,<y>,<z> (3 x float)
    // orientation: <pitch>,<yaw>,<roll> (3 x float angles)
    // player info: <hitpoints>;<score>;<life state>;<scale> (only for players, !for projectiles)
    // an update #include "an unknown player will cause creation of that player since that player must have been accepted by the gamehost
    // || he wouldn't know this client's address. Receiving messages #include "unknown players may be the result of a previous disconnect
    int HandleUpdate(NetworkMessage message)
    {
        if (!message.IsValid(-4))
            return message.m_result;
        int actorId = message.Int(0);
        int colorIndex = message.Int(1);
        Actor actor = Globals.actorHandler.FindActor(actorId, colorIndex);
        if (actor == null)
        {
            if (actorId != 0)
                return 0;
            if (message.m_numValues < 9)
                return -1;
            ushort inPort = message.UShort(8);
            ushort[] ports = { inPort, message.m_port };
            actor = AddPlayer(message.m_address, ports, colorIndex);  // colorIndex == -1 -. assign an available random color
            if (actor == null)
            {
                SendReject(message.m_address, inPort, "unknown");
                return 0;
            }
        }
        actor.m_camera.BumpPosition();
        actor.SetPosition(message.Vector(2));
        actor.SetOrientation(-message.Vector(3));
        actor.UpdateLastMessageTime();
        if (actorId == 0)
        {  // player
            if (message.m_numValues < 9)
                return -1;
            actor.SetHitPoints(message.Int(4));
            actor.SetScore(message.Int(5));
            actor.SetLifeState((Actor.eLifeState)message.Int(6));
            actor.SetScale(message.Float(7));
        }
        else
        {
            actor.UpdateFrozenTime();
        }
        return 1;
    }


    // message: HIT:<target color>;<hitter color>
    int HandleHit(NetworkMessage message)
    {
        if (!message.IsValid(2))
            return message.m_result;
        Player target = Globals.actorHandler.FindPlayer(message.Int(0));
        if (target == null)
            return -1;
        Player hitter = Globals.actorHandler.FindPlayer(message.Int(1));
        if (hitter == null)
            return -1;
        target.RegisterHit(hitter);
        target.UpdateLastMessageTime();
        return 1;
    }


    // message: DESTROY:<id>;<color>
    int HandleDestroy(NetworkMessage message)
    {
        if (!message.IsValid(2))
            return message.m_result;
        return Globals.actorHandler.DeleteActor(message.Int(0), message.Int(1)) ? 1 : -1;
    }


    // message: LEAVE:<color>
    int HandleLeave(NetworkMessage message)
    {
        if (!message.IsValid(1))
            return message.m_result;
        return Globals.actorHandler.DeletePlayer(message.Int(0)) ? 1 : -1;
    }


    int HandleReject(NetworkMessage message)
    {
        if (!message.IsValid(1))
            return message.m_result;
        string reason = message.Str(0);
        if (reason == "unknown")
        {
            if (!IamMaster())
                SendApply();
            return 1;
        }
        if (reason == "full")
        {
            Console.Error.WriteLine("Can't join: match is full");
            return 1;
        }
        if (reason == "out of sync")
            return 1;
        return -1;
    }


    void HandleTimeouts()
    {
        foreach (Actor a in Globals.actorHandler.m_actors)
        {
            if (a.IsPlayer() && !a.IsViewer() && TimedOut((Player)a))
            {
                if (IamMaster())
                    BroadcastDestroy(a);
                Globals.actorHandler.DeleteActor(a.m_id, a.GetColorIndex());
            }
        }
    }


    void HandleDisconnect()
    {
        int connectStatus = IamConnected();
        if (connectStatus < 0)
        {     // i.e. had been connected, but host connection has timed out
            BroadcastLeave();
            m_joinState = eJoinState.jsApply;
        }
        else if (connectStatus == 0)
        {
            Globals.actorHandler.CleanupActors();
            m_joinState = eJoinState.jsApply;
        }
    }

    // broadcast functions ========================================


    void Broadcast(string message)
    {
        foreach (Actor a in Globals.actorHandler.m_actors)
            if (a.IsPlayer() && !a.IsLocalActor()) // players have id zero
                Transmit(message, a.GetAddress(), a.GetPort(0));
    }


    // format: ANIMATION<color>;<animation>
    public void BroadcastAnimation()
    {
        Broadcast(IdFromName("ANIMATION") + Convert.ToString(Globals.actorHandler.m_viewer.GetColorIndex()) + ";" + Convert.ToString(Globals.actorHandler.m_viewer.m_animation));
    }


    // tell every other player that we've been hit, && who hit us
    // format: HIT:<target color>:<hitter color>
    // will always be sent asap
    public void BroadcastHit(Actor player, Actor hitter)
    {
        Broadcast(IdFromName("HIT") + Convert.ToString(player.GetColorIndex()) + ";" + Convert.ToString(hitter.GetColorIndex()));
    }


    // tell every other player that we have fired a projectile
    // format: FIRE:<id>;<parent color>
    // will always be sent asap
    public void BroadcastFire(Projectile projectile)
    {
        Broadcast(IdFromName("FIRE") + Convert.ToString(projectile.m_id) + ";" + Convert.ToString(projectile.GetColorIndex()));
    }


    public void BroadcastDestroy(Actor actor)
    {
        Broadcast(IdFromName("DESTROY") + Convert.ToString(actor.m_id) + ";" + Convert.ToString(actor.GetColorIndex()));
    }


    public void BroadcastEnter()
    {
        Broadcast(IdFromName("ENTER") + Convert.ToString(Globals.actorHandler.m_viewer.GetColorIndex()));
    }


    // format: LEAVE:<color>
    public void BroadcastLeave(int colorIndex = -1)
    {
        if (colorIndex < 0)
            colorIndex = Globals.actorHandler.m_viewer.GetColorIndex();
        Broadcast(IdFromName("LEAVE") + Convert.ToString(colorIndex));
        foreach (Actor a in Globals.actorHandler.m_actors)
            if (a.IsPlayer() && !a.IsViewer()) // players have id zero
                Globals.actorHandler.DeletePlayer(a.GetColorIndex());
    }


    // inform every other player about our position && heading && position && heading of each of our shots
    // will be sent at the network fps
    public void BroadcastUpdate()
    {
        int colorIndex = Globals.actorHandler.m_viewer.GetColorIndex();
        foreach (Actor a in Globals.actorHandler.m_actors)
            if (a.GetColorIndex() == colorIndex)   // actor is viewer || child of viewer
                Broadcast(IdFromName("UPDATE") + UpdateMessage(a));
    }


    public void BroadcastPlayers()
    {
        string message = IdFromName("PLAYERS");
        // add addresses of all other players to player requesting to join
        int i = 0;
        foreach (Actor a in Globals.actorHandler.m_actors)
        {
            if (a.IsPlayer())
            {
                if (i > 0)
                    message += ";";
                message += PlayerMessage((Player)a);
            }
            i++;
        }
        Broadcast(message);
    }


    void ProcessMessage(NetworkMessage message)
    {
        UpdateLastMessageTime(message);    // update last message time of player who sent this message
        int id = IdFromMessage(message);
        if (id < 0)
        {
            if (!message.Empty())
#pragma warning disable CS0642 // Possible mistaken empty statement
                ;// LOG("Received faulty message '%s'\n", message.m_payload.Buffer ());
#pragma warning restore CS0642 // Possible mistaken empty statement
        }
        else
        {
            if (id != 12) // UPDATE
                Console.Error.WriteLine("{0}", m_messageHandlers[id].m_name);
            if (m_messageHandlers[id].m_handler(message) < 0)
#pragma warning disable CS0642 // Possible mistaken empty statement
                ; // Console.Error.WriteLine("Received faulty %s message '%s'\n", m_messageHandlers [id].m_name, message.m_payload);
#pragma warning restore CS0642 // Possible mistaken empty statement
        }
    }


    void ProcessMessages()
    {
        if (m_messages.Count > 0)
        {
            NetworkListener.Lock();
            foreach (NetworkMessage m in m_messages)
                ProcessMessage(m);
            m_messages.Clear();
            NetworkListener.Unlock();
        }
    }


    bool Joined()
    {
        if (IamMaster())
            return true;
        if (m_joinState == eJoinState.jsConnected)
            return true;
        // wait longer before trying to connect
        if (m_joinState == eJoinState.jsApply)
        {
            if (!m_joinTimer.HasPassed(m_joinDelay, true))
                return false;
        }
        else
        {
            if (!m_joinStateTimer.HasPassed(m_joinStateDelay, true))
                return false;
        }
        m_joinStateHandlers[(int)m_joinState]();
        return false;
    }


    void Listen()
    {
        if (m_threadedListener)
            ProcessMessages();
        else
        {
            Timer listenTimer = new Timer(5);
            listenTimer.Start();
            while (!listenTimer.HasPassed(5))
            {    // listen && Handle messages for max. ~ 5 ms
                NetworkMessage message = Receive();
                if (message.Empty())
                    break;
                ProcessMessage(message);
            }
        }
    }


    public void Update()
    {
        if (m_updateTimer.HasPassed(m_frameTime, true))
        {
            if (Joined())
            {
                BroadcastUpdate();
                // if (IamMaster () && m_playerUpdateTimer.HasPassed (5000, true))
                //     BroadcastPlayers ();
                HandleTimeouts();
            }
            HandleDisconnect();
            Listen();
        }
    }

}

// =================================================================================================
