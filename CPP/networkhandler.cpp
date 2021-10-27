#include <stdint.h>

#include "networkHandler.h"
#include "controlsHandler.h"
#include "gameData.h"
#include "gameItems.h"
#include "actorHandler.h"
#include "argHandler.h"
#include "maploader.h"

// =================================================================================================

#ifdef _DEBUG
#   define LOG(msg, ...) fprintf(stderr, msg, ##__VA_ARGS__);
#else
#   define LOG(msg, ...)
#endif

// =================================================================================================

CNetworkHandler::CNetworkHandler() : CUDP() {
    m_localAddress = argHandler->StrVal("localaddress", 0, CString("127.0.0.1"));
    m_localPorts[0] = argHandler->IntVal("inport", 0, 9100);
    m_localPorts[1] = argHandler->IntVal("outport", 0, 9101);
    m_hostAddress = argHandler->StrVal("hostaddress", 0, CString("127.0.0.1"));
    m_hostPorts[0] = argHandler->IntVal("hostport", 0, 0);
    m_hostPorts[1] = 0;
    m_messageHandlers = {
        CMessageHandler("APPLY", &CNetworkHandler::HandleApply),                     // process join request sent to game host
        CMessageHandler("ACCEPT", &CNetworkHandler::HandleAccept),                   // receive position && heading #include "game host
        CMessageHandler("REQUESTMAP", &CNetworkHandler::HandleMapRequest),           // send map data to new player
        CMessageHandler("SYNCMAP", &CNetworkHandler::SyncMap),                       // process map data #include "game host
        CMessageHandler("REQUESTPARAMS", &CNetworkHandler::HandleParamsRequest),     // send game parameters to new player
        CMessageHandler("SYNCPARAMS", &CNetworkHandler::SyncParams),                 // process game parameters #include "game host
        CMessageHandler("REQUESTPLAYERS", &CNetworkHandler::HandlePlayersRequest),   // send list of all players to new player
        CMessageHandler("SYNCPLAYERS", &CNetworkHandler::SyncPlayers),               // add all players #include "player message to actor list that aren't already in it
        CMessageHandler("REQUESTSHOTS", &CNetworkHandler::HandleProjectilesRequest), // send currently live projectiles to new player
        CMessageHandler("SYNCSHOTS", &CNetworkHandler::SyncProjectiles),             // add projectiles #include "message to actor list
        CMessageHandler("ENTER", &CNetworkHandler::HandleEnter),                     // compute position && heading for new player && send them to him/her
        CMessageHandler("ANIMATION", &CNetworkHandler::HandleAnimation),             // update sending player's animation state (which determines whether && which animation sound to play)
        CMessageHandler("UPDATE", &CNetworkHandler::HandleUpdate),                   // update all local actors owned by the sending player with positions && headings #include "the message
        CMessageHandler("FIRE", &CNetworkHandler::HandleFire),                       // create a projectile fired by another player
        CMessageHandler("HIT", &CNetworkHandler::HandleHit),                         // integrate hit at a player into local player data
        CMessageHandler("DESTROY", &CNetworkHandler::HandleDestroy),                 // destroy a projectile that had hit another player
        CMessageHandler("LEAVE", &CNetworkHandler::HandleLeave),                     // remove sending player #include "player list
        CMessageHandler("REJECT", &CNetworkHandler::HandleReject)                    // react to some message sent to another player having been rejected by that player for some reason
    };

    m_idMap.SetComparator(CString::Compare);
    for (auto [id, h] : m_messageHandlers)
        m_idMap.Insert(h.m_name, CString(id) + CString("#"));

    m_joinStateHandlers = { 
        &CNetworkHandler::SendApply, &CNetworkHandler::SendSyncMap, &CNetworkHandler::SendSyncParams, 
        &CNetworkHandler::SendSyncPlayers, &CNetworkHandler::SendSyncProjectiles, &CNetworkHandler::SendEnter 
    };
    m_fps = 60;
    m_frameTime = 1000 / m_fps;             // m_fps
    m_joinDelay = 5000;             // 5 seconds between two consecutive join attempts
    m_joinStateDelay = 500;
    m_timeoutPeriod = 30 * 1000;    // seconds [ms] without message #include "a player after which the player will be removed #include "the game
    m_mapRow = -1;
    m_threadedListener = argHandler->BoolVal("multithreading", 0, true);
    m_listen = true;
    m_joinState = IamMaster() ? jsConnected : jsApply;
    m_semicolon = CString(";");
    m_colon = ":";
    m_hashtag = "#";
    m_syncingAddress = "";

}


void CNetworkHandler::Create(CString address, uint16_t port) {
    if (!address.Empty() && port)
        SetHostAddress(address, port);
    OpenSocket(m_localPorts[0], 0);
    OpenSocket(m_localPorts[1], 1);
    actorHandler->m_viewer->SetAddress(m_localAddress, m_localPorts);
    if (m_threadedListener)
        m_listener.Start(&m_listen);
}

    void CNetworkHandler::Destroy(void) {
        if (m_threadedListener)
            m_listener.Stop();
    }


    void CNetworkHandler::SetHostAddress(CString address, uint16_t port) {
        m_hostAddress = address;
        m_hostPorts[0] = port;
    }


    // message handling helper functions ========================================

    CString CNetworkHandler::BuildMessage(const char* delim, std::initializer_list<CString> values) {
        size_t l = 0, dl = strlen (delim);
        for (auto const& v : values)
            l += v.Length() + dl;
        CString message;
        message.Reserve(l);
        for (auto const& v : values) {
            message += v;
            if (dl)
                message += delim;
        }
        if (dl) {
            message.SetLength (message.Length () - dl);
            *message.Buffer(message.Length ()) = '\0';    // remove the last delim
        }
        return message;
    }


    CString CNetworkHandler::VectorToMessage(CVector v) {
        return BuildMessage(",", { CString(v.X()), CString(v.Y()), CString(v.Z()) });
    }


    // construct a message with all update info for actor 
    CString CNetworkHandler::UpdateMessage(CActor* actor) {
        if (actor->IsPlayer())
            return BuildMessage(";", { CString(actor->GetId()), CString(actor->GetColorIndex()), VectorToMessage(actor->GetPosition()), VectorToMessage(actor->GetOrientation()),
                                       CString(actor->m_hitPoints), CString(actor->GetScore()), CString(actor->m_lifeState), CString(actor->m_scale), CString(actor->GetPort()) });
        else
            return BuildMessage(";", { CString(actor->GetId()), CString(actor->GetColorIndex()), VectorToMessage(actor->GetPosition()), VectorToMessage(actor->GetOrientation()) });
    }


    CString CNetworkHandler::PlayerMessage(CPlayer* player) {
        return BuildMessage(";", { player->GetAddress (), CString (player->GetPort (0)), CString (player->GetPort (1)), CString (player->GetColorIndex ()) });
    }


    CString CNetworkHandler::IdFromName(const char* textId) {
        CString* id = m_idMap.Find(CString (textId));
        return id ? *id : CString("");
    }


    int CNetworkHandler::IdFromMessage(CMessage& message) {
        auto values = message.m_payload.Split('#');
		if (values.Empty())
			return -1;			
		int id = int(values[0]);
        return (id < m_messageHandlers.Length()) ? id : -1;
    }


    // networking helper functions ========================================

    CPlayer* CNetworkHandler::FindPlayer(CString & address, uint16_t port) {
        for (auto [i, a] : actorHandler->m_actors)
            // check for port too as there may be players in the same local network && behind the same router/firewall, sharing the same ip address && just using different ports
            if (a->IsPlayer () && (a->GetAddress () == address) && (a->GetPort (1) == port))
                return (CPlayer*) a;
        return nullptr;
    }


    size_t CNetworkHandler::RemotePlayerCount(void) {
        size_t rpc = 0;
        for (auto [i, a] : actorHandler->m_actors)
            if (a->IsPlayer () && !a->IsLocalActor())
                rpc++;
        return rpc;
    }


    void CNetworkHandler::UpdateLastMessageTime(CMessage& message) {
        CPlayer* player = FindPlayer(message.m_address, message.m_port);
        if (player)
            player->UpdateLastMessageTime ();
    }


    int CNetworkHandler::IamConnected(void) {
        if (IamMaster())
            return 1;
        if (m_joinState == jsApply)
            return 0;
        CPlayer* host = FindPlayer(m_hostAddress, m_hostPorts[1]);
        if (!host)
            return 0;
        if (TimedOut(host))
            return -1;
        return 1;
    }

    bool CNetworkHandler::TimedOut (CPlayer* player) {
        return !player->IsLocalActor() && (gameData->m_gameTime - player->m_lastMessageTime > m_timeoutPeriod);
    }

    // send functions ========================================

    void CNetworkHandler::SendApply(void) {
        LOG("SendApply\n")
        Transmit(BuildMessage("", { IdFromName("APPLY"), CString(InPort()) }), m_hostAddress, m_hostPorts[0]);
    }


    void CNetworkHandler::SendSyncMap(void) {
        m_mapRow = -1;
        m_stringMap.Destroy();
        LOG("SendSyncMap\n")
        Transmit(BuildMessage("", { IdFromName("REQUESTMAP"), CString(actorHandler->m_viewer->GetPort(0)) }), m_hostAddress, m_hostPorts[0]);
    }


    void CNetworkHandler::SendSyncParams(void) {
        LOG("SendSyncParams\n")
        Transmit(BuildMessage("", { IdFromName("REQUESTPARAMS"), CString(actorHandler->m_viewer->GetPort(0)) }), m_hostAddress, m_hostPorts[0]);
    }


    void CNetworkHandler::SendSyncProjectiles(void) {
        LOG("SendSyncProjectiles\n")
        Transmit(BuildMessage("", { IdFromName("REQUESTSHOTS"), CString(actorHandler->m_viewer->GetPort(0)) }), m_hostAddress, m_hostPorts[0]);
    }


    void CNetworkHandler::SendSyncPlayers(void) {
        LOG("SendSyncPlayers\n")
        Transmit(BuildMessage("", { IdFromName("REQUESTPLAYERS"), CString(actorHandler->m_viewer->GetPort(0)) }), m_hostAddress, m_hostPorts[0]);
    }


    void CNetworkHandler::SendEnter(void) {
        LOG("SendEnter\n")
        SendEnter(m_hostAddress, m_hostPorts[0]);
    }


    void CNetworkHandler::SendEnter(CString address, uint16_t port) {
        LOG("SendEnter\n")
        if (address == "") {
            address = m_hostAddress;
            port = m_hostPorts[0];
        }
        Transmit(BuildMessage("", { IdFromName("ENTER"), CString(actorHandler->m_viewer->m_colorIndex),  m_semicolon, CString(InPort()) }), address, port);
    }


    // accept tells the player requesting to join his own ip address since it it somewhat tedious to determine your own external ip address when behing a router, firewall && what not
    void CNetworkHandler::SendAccept(CPlayer * player) {
        LOG("SendAccept\n")
        CString message = BuildMessage(";", { IdFromName("ACCEPT") + CString(player->m_colorIndex), VectorToMessage(player->GetPosition()), VectorToMessage(player->GetOrientation()), player->m_address });
        Transmit(message, player->m_address, player->GetPort(0));
    }


    // format: MAP:<tag>:<text line #include "map file>
    //         tags: reversed line number (starting at line count of map - 1) => 0 for last line
    void CNetworkHandler::SendMap(CString address, uint16_t port) {
        LOG("SendMap\n")
        int l = int (gameItems->m_map->m_stringMap.Length());
        for (auto [i, s] : gameItems->m_map->m_stringMap)
            Transmit(BuildMessage (";", { IdFromName("SYNCMAP") + CString(--l), s }), address, port);
    }


    // format: PARAMS:<fire delay>;<heal delay>;<respawn delay>;<immunity duration>;<points for kill>;<projectile size>;<projectile speed>;<move speed>;<turn speed>
    void CNetworkHandler::SendParams(CString address, uint16_t port) {
        LOG("SendParams\n")
        CString message = BuildMessage(";", { IdFromName("SYNCPARAMS") + CString(gameData->m_fireDelay), CString(gameData->m_healDelay), CString(gameData->m_respawnDelay), CString(gameData->m_immunityDuration),
                                         CString(gameData->m_pointsForKill), CString(gameData->m_projectileSize), CString(gameData->m_projectileSpeed), 
                                         CString(controlsHandler->GetMoveSpeed()), CString(controlsHandler->GetTurnSpeed()) });
        Transmit(message, address, port);
    }


    // format: PLAYERS<player data>[;<player data> [...]]
    // player data: <address>:<port>;<color index>
    void CNetworkHandler::SendPlayers(CString address, uint16_t port) {
        LOG("SendPlayers\n")
        CString message;
        message.Reserve(500);
        message += IdFromName("SYNCPLAYERS");
        // add addresses of all other players to player requesting to join
        for (auto [i, a] : actorHandler->m_actors) {
            if (a->IsPlayer ()) {  // player
                if (i > 0)    // !the first entry in the list
                    message += ";";
                message += BuildMessage(";", { a->GetAddress () + ":" + CString (a->GetPort (0)), CString (a->GetPort (1)), CString (a->GetColorIndex ())});
            }
        }
        Transmit(message, address, port);
    }


    // format: SHOTS<shot data>[;<shot data> [...]]
    // shot data: <actor id>;<parent color index>
    void CNetworkHandler::SendProjectiles(CString address, uint16_t port) {
        LOG("SendProjectiles\n")
        CString message;
        message.Reserve(500);
        message += IdFromName("SYNCSHOTS");
        // add addresses of all other players to player requesting to join
        for (auto [i, a] : actorHandler->m_actors) {
            if (a->IsProjectile ()) {  // projectile
                if (i > 0)    // !the first entry in the list
                    message += ";";
                message += CString(a->m_id);
                message += ";";
                message += CString(a->GetColorIndex ());
            }
        }
        Transmit(message, address, port);
    }


    // send an update message to a single player
    void CNetworkHandler::SendUpdate(CString address, uint16_t port) {
        LOG("SendUpdate\n")
        Transmit(IdFromName("UPDATE") + UpdateMessage(actorHandler->m_viewer), address, port);
    }


    void CNetworkHandler::SendReject(CString address, uint16_t port, const char* reason) {
        LOG("SendReject\n")
        Transmit(IdFromName("REJECT") + CString (reason), address, port);
    }


    CPlayer* CNetworkHandler::AddPlayer(CString address, uint16_t ports[], int colorIndex) {
        CPlayer* player = FindPlayer(address, ports[1]);
        if (!player) {
            player = actorHandler->CreatePlayer(colorIndex, CVector (NAN, NAN, NAN), CVector (0,0,0), address, ports [0], ports [1]);
            if (!player)
                SendReject (address, ports [0], "full");
            else
                player->UpdateLastMessageTime ();
        }
        return player;
    }


    bool CNetworkHandler::OutOfSync(eJoinStates joinState, bool isFinal) {
        if (m_joinState == joinState)
            return false;
        // m_joinState = jsApply
        if (isFinal)
            SendReject(m_hostAddress, m_hostPorts[0], "out of sync");
        return true;
    }

    // incoming message processing functions ========================================

    //format: APPLY<host address>;<client rx port>
    int CNetworkHandler::HandleApply(CMessage& message) {
        if (!message.IsValid(1))
            return message.m_result;
        LOG ("HandleApply\n")
        if (m_syncingAddress == "")
            m_syncingAddress = message.m_address;
        else if (m_syncingAddress != message.m_address) {
            SendReject (message.m_address, message.Int (0), "sync in progress");
            return -1;
        }
        SendEnter(message.m_address, message.Int(0));
        return 1;
    }


    //format: ENTER<client color>;<client rx port>
    int CNetworkHandler::HandleEnter(CMessage& message) {
        if (!message.IsValid(2))
            return message.m_result;
        LOG("HandleEnter\n")
        if (OutOfSync(jsConnected, false) && OutOfSync(jsApply)) // this message is permitted when already connected
            return -1;
        int colorIndex = message.Int(0);
        if (IamMaster()) {   // reassign color if requested color is already in use
            if (!gameData->ColorIsAvailable(colorIndex))
                colorIndex = -1;
        }
        uint16_t ports[2] = { uint16_t (message.Int(1)), uint16_t (message.m_port) };
        CPlayer* player = AddPlayer(message.m_address, ports, colorIndex);  // colorIndex == -1 --> assign an available random color
        if (!player)
            return -1;
        if (IamMaster()) {
            gameItems->m_map->FindSpawnPosition(player);
            player->UpdateLastMessageTime ();
            SendAccept(player);
            m_syncingAddress = "";
        }
        else {
            if (message.m_address == m_hostAddress)
                m_hostPorts[1] = message.m_port;
        }
    if (m_joinState == jsApply)
        m_joinState = jsMap;
    return 1;
    }


    int CNetworkHandler::HandleMapRequest(CMessage& message) {
        if (!message.IsValid(1))
            return message.m_result;
        LOG("HandleMapRequest\n")
        SendMap(message.m_address, message.Int(0));
        return 1;
    }


    // format:MAP<row //>;<row text>
    int CNetworkHandler::SyncMap(CMessage& message) {
        if (!message.IsValid(2))
            return message.m_result;
        LOG("SyncMap\n")
        if (OutOfSync(jsMap))
            return -1;
        int row = message.Int(0);
        // m_mapRow < 0 ==> we're receiving the first map row
        // map row number are sent in reverse order (highest first, down to zero)
        if ((m_mapRow >= 0) && (row != m_mapRow - 1))
            return -1;   // we lost at least one map row
        m_mapRow = row;
        m_stringMap.Append(message.Str(1));
        if (row > 0)
            return 1;
        if (!gameItems->CreateMap (m_stringMap, true))
            return -1;
        m_joinState = jsParams;
        return 1;
    }


    int CNetworkHandler::HandleParamsRequest(CMessage& message) {
        if (!message.IsValid(1))
            return message.m_result;
        LOG("HandleParamsRequest\n")
        SendParams(message.m_address, message.Int(0));
        return 1;
    }


    // format: PARAMS<heal delay>;<respawn delay>;<immunity duration>;<move speed>;<turn speed>
    int CNetworkHandler::SyncParams(CMessage& message) {
        if (!message.IsValid(9))
            return message.m_result;
        LOG("SyncParams\n")
        if (OutOfSync(jsParams))
            return -1;
        gameData->m_fireDelay = message.Int(0);
        gameData->m_healDelay = message.Int(1);
        gameData->m_respawnDelay = message.Int(2);
        gameData->m_immunityDuration = message.Int(3);
        gameData->m_pointsForKill = message.Int(4);
        gameData->m_projectileSize = message.Float(5);
        gameData->m_projectileSpeed = message.Float(6);
        controlsHandler->SetMoveSpeed(message.Float(7));
        controlsHandler->SetTurnSpeed(message.Float(8));
        m_joinState = jsPlayers;
        return 1;
    }


    int CNetworkHandler::HandlePlayersRequest(CMessage& message) {
        if (!message.IsValid(1))
            return message.m_result;
        LOG("HandlePlayersRequest\n")
        SendPlayers(message.m_address, message.Int(0));
        return 1;
    }


    // add each player #include "message to actor list which isn't in it already
    int CNetworkHandler::SyncPlayers(CMessage& message) {
        if (!message.IsValid(-3))
            return message.m_result;
        LOG("SyncPlayers\n")
        if (OutOfSync(jsConnected, false) && OutOfSync(jsPlayers))  // this message is permitted when already connected
            return -1;
        for (size_t i = 0; i < message.m_numValues; i += 3) {
            int colorIndex = message.Int(i + 2);
            CPlayer* player = actorHandler->FindPlayer(colorIndex);
            if (!player) {
                uint16_t inPort;
                CString address = message.Address(i, inPort);
                uint16_t ports[2] = { inPort, uint16_t (message.Int(i + 1)) };
                player = AddPlayer(address, ports, colorIndex);
                if (!player) {    // synchronization error
                    SendReject(m_hostAddress, m_hostPorts[0], "out of sync");
                    return -1;
                }
                SendEnter(address, inPort);
            }
        }
        if (m_joinState == jsPlayers)       // this message will periodically be sent to all connected players
            m_joinState = jsProjectiles;    // so only change join state when actually joining
        return 1;
    }


    int CNetworkHandler::HandleProjectilesRequest(CMessage& message) {
        if (!message.IsValid(1))
            return message.m_result;
        LOG("HandleProjectilesRequest\n")
        SendProjectiles(message.m_address, message.Int(0));
        return 1;
    }


    // add each projectile #include "message to actor list 
    int CNetworkHandler::SyncProjectiles(CMessage& message) {
        if (!message.IsValid(0))
            return message.m_result;
        LOG("SyncProjectiles\n")
        if (OutOfSync(jsProjectiles))  // this message is permitted when already connected
            return -1;
        for (size_t i = 0; i < message.m_numValues; i += 2) {
            int colorIndex = message.Int(i + 1);
            CPlayer* parent = actorHandler->FindPlayer(colorIndex);
            if (parent)
                actorHandler->CreateActor(message.Int(i), colorIndex, parent->GetPosition(), parent->GetOrientation());
        }
        // SendEnter (m_hostAddress, m_hostPorts [0])
        m_joinState = jsEnter;
        return 1;
    }



    // format: ACCEPT<color>;<position>;<orientation>;<address>;<address>:<port>;...;<address>:<port>
    // <id> is always 0 (zero) for players && hence will be ignored
    // ip addresses && ports of all other players including the game host will be transmitted
    int CNetworkHandler::HandleAccept(CMessage& message) {
        if (!message.IsValid(4))   // need at least five values
            return message.m_result;
        LOG("HandleAccept\n")
        if (OutOfSync(jsEnter))
            return -1;
        actorHandler->m_viewer->SetColorIndex(message.Int(0), true);
        actorHandler->m_viewer->SetPosition(message.Vector(1));
        actorHandler->m_viewer->SetOrientation(message.Vector(2));
        actorHandler->m_viewer->m_address = message.Str(3);
        actorHandler->m_viewer->ForceRespawn();
        m_joinState = jsConnected;
        return 1;
    }


    // format: FIRE<projectile id>;<projectile parent color index>
    int CNetworkHandler::HandleFire(CMessage& message) {
        if (!message.IsValid(2))
            return message.m_result;
        LOG("HandleFire\n")
        CPlayer* parent = actorHandler->FindPlayer(message.Int(1));
        if (!parent)
            return -1;
        CProjectile* projectile = actorHandler->CreateProjectile(parent, message.Int(0));
        return projectile ? 1 : -1;
    }


    int CNetworkHandler::HandleAnimation(CMessage& message) {
        if (!message.IsValid(2))
            return message.m_result;
        LOG("HandleAnimation\n")
        CActor* actor = actorHandler->FindActor(0, message.Int(0));
        if (!actor)
            return 0;
        actor->SetAnimation(message.Int(1));
        return 1;
    }


    // message: UPDATE<id>;<color>;<position>;<orientation>[;<player info>]
    // position: <x>,<y>,<z> (3 x float)
    // orientation: <pitch>,<yaw>,<roll> (3 x float angles)
    // player info: <hitpoints>;<score>;<life state>;<scale> (only for players, !for projectiles)
    // an update #include "an unknown player will cause creation of that player since that player must have been accepted by the gamehost
    // || he wouldn't know this client's address. Receiving messages #include "unknown players may be the result of a previous disconnect
    int CNetworkHandler::HandleUpdate(CMessage& message) {
        if (!message.IsValid(-4))
            return message.m_result;
        // LOG("HandleUpdate\n")
        int actorId = message.Int(0);
        int colorIndex = message.Int(1);
        CActor* actor = actorHandler->FindActor(actorId, colorIndex);
        if (!actor) {
            if (actorId != 0)
                return 0;
            if (message.m_numValues < 9)
                return -1;
            uint16_t inPort = message.Int(8);
            uint16_t ports[2] = { inPort, message.m_port };
            actor = AddPlayer(message.m_address, ports, colorIndex);  // colorIndex == -1 --> assign an available random color
            if (!actor) {
                SendReject(message.m_address, inPort, "unknown");
                return 0;
            }
        }
        actor->m_camera.BumpPosition();
        actor->SetPosition(message.Vector(2));
        actor->SetOrientation(-message.Vector(3));
        actor->UpdateLastMessageTime();
        if (actorId == 0) {  // player
            if (message.m_numValues < 9)
                return -1;
            actor->SetHitPoints(message.Int(4));
            actor->SetScore(message.Int(5));
            actor->SetLifeState(CActor::eLifeStates (message.Int(6)));
            actor->SetScale(message.Float(7));
        }
        else {
            actor->UpdateFrozenTime();
        }
        return 1;
    }


    // message: HIT:<target color>;<hitter color>
    int CNetworkHandler::HandleHit(CMessage& message) {
        if (!message.IsValid(2))
            return message.m_result;
        LOG("HandleHit\n")
        CPlayer* target = actorHandler->FindPlayer(message.Int(0));
        if (!target)
            return -1;
        CPlayer* hitter = actorHandler->FindPlayer(message.Int(1));
        if (!hitter)
            return -1;
        target->RegisterHit(hitter);
        target->UpdateLastMessageTime ();
        return 1;
    }


    // message: DESTROY:<id>;<color>
    int CNetworkHandler::HandleDestroy(CMessage& message) {
        if (!message.IsValid(2))
            return message.m_result;
        LOG("HandleDestroy\n")
        return actorHandler->DeleteActor(message.Int(0), message.Int(1)) ? 1 : -1;
    }


    // message: LEAVE:<color>
    int CNetworkHandler::HandleLeave(CMessage& message) {
        if (!message.IsValid(1))
            return message.m_result;
        // values = message.payload.Split (";")
        LOG("HandleLeave\n")
        return actorHandler->DeletePlayer(message.Int(0)) ? 1 : -1;
    }


    int CNetworkHandler::HandleReject(CMessage& message) {
        if (!message.IsValid(1))
            return message.m_result;
        CString reason = message.Str(0);
        if (reason == "unknown") {
            if (!IamMaster())
                SendApply();
            return 1;
        }
        if (reason == "full") {
            fprintf (stderr, "Can't join: match is full\n");
            return 1;
        }
        if (reason == "out of sync")
            return 1;
        return -1;
    }


    void CNetworkHandler::HandleTimeouts(void) {
        for (auto [i, a] : actorHandler->m_actors) {
            if (a->IsPlayer () && !a->IsViewer() && TimedOut((CPlayer*) a)) {
                if (IamMaster())
                    BroadcastDestroy(a);
                actorHandler->DeleteActor(a->m_id, a->GetColorIndex ());
            }
        }
    }


    void CNetworkHandler::HandleDisconnect(void) {
        int connectStatus = IamConnected();
        if (connectStatus < 0) {     // i.e. had been connected, but host connection has timed out
            BroadcastLeave();
            m_joinState = jsApply;
        }
        else if (connectStatus == 0) {
            actorHandler->CleanupActors();
            m_joinState = jsApply;
        }
    }

    // broadcast functions ========================================


    void CNetworkHandler::Broadcast(CString message) {
        for (auto [i, a] : actorHandler->m_actors)
            if (a->IsPlayer () && !a->IsLocalActor()) // players have id zero
                Transmit(message, a->GetAddress (), a->GetPort (0));
    }


    // format: ANIMATION<color>;<animation>
    void CNetworkHandler::BroadcastAnimation(void) {
        Broadcast(IdFromName("ANIMATION") + CString(actorHandler->m_viewer->GetColorIndex ()) + ";" + CString(actorHandler->m_viewer->m_animation));
    }


    // tell every other player that we've been hit, && who hit us
    // format: HIT:<target color>:<hitter color>
    // will always be sent asap
    void CNetworkHandler::BroadcastHit(CActor * player, CActor * hitter) {
        LOG("BroadcastHit\n")
        Broadcast(IdFromName("HIT") + CString(player->GetColorIndex ()) + ";" + CString(hitter->GetColorIndex ()));
    }


    // tell every other player that we have fired a projectile
    // format: FIRE:<id>;<parent color>
    // will always be sent asap
    void CNetworkHandler::BroadcastFire(CProjectile * projectile) {
        LOG("BroadcastFire\n")
        Broadcast(IdFromName("FIRE") + CString(projectile->m_id) + ";" + CString(projectile->GetColorIndex()));
    }


    void CNetworkHandler::BroadcastDestroy(CActor * actor) {
        LOG("BroadcastDestroy\n")
        Broadcast(IdFromName("DESTROY") + CString(actor->m_id) + ";" + CString(actor->GetColorIndex()));
    }


    void CNetworkHandler::BroadcastEnter(void) {
        LOG("BroadcastEnter\n")
        Broadcast(IdFromName("ENTER") + CString(actorHandler->m_viewer->GetColorIndex ()));
    }


    // format: LEAVE:<color>
    void CNetworkHandler::BroadcastLeave(int colorIndex) {
        if (colorIndex < 0)
            colorIndex = actorHandler->m_viewer->GetColorIndex ();
        Broadcast(IdFromName("LEAVE") + CString(colorIndex));
        for (auto [i, a] : actorHandler->m_actors)
            if (a->IsPlayer () && !a->IsViewer()) // players have id zero
                actorHandler->DeletePlayer(a->GetColorIndex ());
    }


    // inform every other player about our position && heading && position && heading of each of our shots
    // will be sent at the network fps
    void CNetworkHandler::BroadcastUpdate(void) {
        int colorIndex = actorHandler->m_viewer->GetColorIndex ();
        for (auto [i, a] : actorHandler->m_actors)
            if (a->GetColorIndex() == colorIndex)   // actor is viewer || child of viewer
                Broadcast(IdFromName("UPDATE") + UpdateMessage(a));
    }


    void CNetworkHandler::BroadcastPlayers(void) {
        CString message = IdFromName("PLAYERS");
        // add addresses of all other players to player requesting to join
        for (auto [i, a] : actorHandler->m_actors) {
            if (a->IsPlayer ()) {
                if (i > 0)
                    message += ";";
                message += PlayerMessage((CPlayer*) a);
            }
        }
        Broadcast(message);
    }


    void CNetworkHandler::ProcessMessage(CMessage& message) {
        UpdateLastMessageTime(message);    // update last message time of player who sent this message
        int id = IdFromMessage(message);
        if (id < 0) {
            if (!message.Empty ())
                ;// LOG("Received faulty message '%s'\n", message.m_payload.Buffer ());
        }
        else {
#ifdef _DEBUG
            if (id != 12) // UPDATE
                LOG("%s\n", (char*) m_messageHandlers [id].m_name)
#endif
            if ((this->*m_messageHandlers [id].m_handler)(message) < 0)
                ; // LOG("Received faulty %s message '%s'\n", m_messageHandlers [id].m_name.Buffer (), message.m_payload.Buffer ());
        }
    }


    void CNetworkHandler::ProcessMessages(void) {
        if (!m_messages.Empty()) {
            m_listener.Lock();
            for (auto [i, m] : m_messages)
                ProcessMessage(m);
            m_messages.Destroy();
            m_listener.Unlock();
        }
    }


    bool CNetworkHandler::Joined(void) {
        if (IamMaster())
            return true;
        if (m_joinState == jsConnected)
            return true;
        // wait longer before trying to connect
        if (m_joinState == jsApply) {
            if (!m_joinTimer.HasPassed(m_joinDelay, true))
                return false;
        }
        else {
            if (!m_joinStateTimer.HasPassed(m_joinStateDelay, true))
                return false;
        }
        tJoinStateHandler stateHandler = m_joinStateHandlers[m_joinState];
        if (stateHandler)
            (this->*stateHandler)();
        return false;
    }


    void CNetworkHandler::Listen(void) {
        if (m_threadedListener)
            ProcessMessages();
        else {
            CTimer listenTimer (5);
            listenTimer.Start();
            while (!listenTimer.HasPassed(5)) {    // listen && Handle messages for max. ~ 5 ms
                CMessage message = Receive();
                if (message.Empty())
                    break;
                ProcessMessage(message);
            }
        }
    }


    void CNetworkHandler::Update(void) {
        if (m_updateTimer.HasPassed(m_frameTime, true)) {
            if (Joined()) {
                BroadcastUpdate();
                // if (IamMaster () && m_playerUpdateTimer.HasPassed (5000, true))
                //     BroadcastPlayers ();
                HandleTimeouts();
            }
            HandleDisconnect ();
            Listen ();
        }
    }

CNetworkHandler* networkHandler = nullptr;

// =================================================================================================
