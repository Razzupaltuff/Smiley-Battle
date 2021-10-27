#pragma once 

#include <stdint.h>
#include <winsock.h>

#include "SDL_net.h"
#include "SDL_thread.h"
#include "SDL_mutex.h"
#include "cstring.h"
#include "clist.h"
#include "cavltree.h"
#include "timer.h"
#include "vector.h"
#include "actor.h"
#include "player.h"
#include "projectile.h"
#include "udp.h"
#include "networklistener.h"

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
// Find all message below in CnetworkHandler->messageHandlers
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

class CNetworkHandler : public CUDP {
    public:
        typedef int (CNetworkHandler::*tMessageHandler) (CMessage&);
        typedef void (CNetworkHandler::*tJoinStateHandler) (void);

        typedef enum {
            jsApply = 0,
            jsMap = 1,          // wait for the host to send map data
            jsParams = 2,       // wait for the host to send global game parameters
            jsPlayers = 3,      // wait for the host to send the list of participants
            jsProjectiles = 4,  // wait for the host to send the list of shots currently present
            jsEnter = 5,        // wait for the host to send position && heading
            jsConnected = 6     // connected, ready to play
        } eJoinStates;

        // ========================================

        class CMessageHandler {
        public:
            CString         m_name;
            tMessageHandler m_handler;

            CMessageHandler () : m_handler (nullptr) {}

            CMessageHandler(const char* name, tMessageHandler handler) : m_name(CString (name)), m_handler(handler) { }
        };

        // ========================================

    public:

        uint16_t        m_localPorts[2];
        CString         m_hostAddress;
        uint16_t        m_hostPorts[2];
        CList<CMessage> m_messages;

        CList<CMessageHandler>      m_messageHandlers;
        CArray<tJoinStateHandler>   m_joinStateHandlers;
        CAvlTree<CString, CString>  m_idMap;

        int             m_fps;
        int             m_frameTime;
        CTimer          m_updateTimer;
        CTimer          m_joinTimer;
        CTimer          m_joinStateTimer;
        CTimer          m_playerUpdateTimer;
        int             m_joinDelay;        // 5 seconds between two consecutive join attempts
        int             m_joinStateDelay;
        int             m_timeoutPeriod;    // seconds [ms] without message #include "a player after which the player will be removed #include "the game
        CList<CString>  m_stringMap;
        int             m_mapRow;
        bool            m_threadedListener;
        bool            m_listen;
        CListener       m_listener;
        eJoinStates     m_joinState;
        CString         m_semicolon;
        CString         m_colon;
        CString         m_hashtag;
        CString         m_syncingAddress;

        // ========================================

    public:

        CNetworkHandler();

        ~CNetworkHandler () {
            Destroy ();
        }

        void Create(CString address = CString(""), uint16_t port = 0);

        void Destroy(void);

        void SetHostAddress(CString address, uint16_t port);

        // message handling helper functions ========================================

        CString BuildMessage(const char* delim, std::initializer_list<CString> values);
            
        CString VectorToMessage(CVector v);

            // construct a message with all update info for actor 
        CString UpdateMessage(CActor* actor);

        CString PlayerMessage(CPlayer* player);

        CString IdFromName(const char* textId);

        int IdFromMessage(CMessage& message);

        // networking helper functions ========================================

        CPlayer* FindPlayer(CString& address, uint16_t port);

        size_t RemotePlayerCount(void);

        void UpdateLastMessageTime(CMessage& message);

        inline bool IamMaster(void) {
            return (m_hostAddress == "127.0.0.1") || (m_hostAddress == m_localAddress);
        }

        int IamConnected(void);

        bool TimedOut (CPlayer* player);

        // send functions ========================================

        void SendApply(void);

        void SendSyncMap(void);

        void SendSyncParams(void);

        void SendSyncPlayers(void);

        void SendSyncProjectiles(void);

        void SendEnter(void);

        void SendEnter(CString address, uint16_t port);

        void SendAccept(CPlayer* player);

        void SendMap(CString address, uint16_t port);

        void SendParams(CString address, uint16_t port);

        void SendPlayers(CString address, uint16_t port);

        void SendProjectiles(CString address, uint16_t port);

        void SendUpdate(CString address, uint16_t port);

        void SendReject(CString address, uint16_t port, const char * reason);

        CPlayer* AddPlayer(CString address, uint16_t ports[], int colorIndex);

        bool OutOfSync(eJoinStates joinState, bool isFinal = true);

        // incoming message processing functions ========================================

        int HandleApply(CMessage& message);

        int HandleEnter(CMessage& message);

        int HandleMapRequest(CMessage& message);

        int SyncMap(CMessage& message);

        int HandleParamsRequest(CMessage& message);

        int SyncParams(CMessage& message);

        int HandlePlayersRequest(CMessage& message);

        int SyncPlayers(CMessage& message);

        int HandleProjectilesRequest(CMessage& message);

        int SyncProjectiles(CMessage& message);

        int HandleAccept(CMessage& message);
            
        int HandleFire(CMessage& message);

        int HandleAnimation(CMessage& message);

        int HandleUpdate(CMessage& message);

        int HandleHit(CMessage& message);

        int HandleDestroy(CMessage& message);

        int HandleLeave(CMessage& message);

        int HandleReject(CMessage& message);

        void HandleTimeouts(void);

        void HandleDisconnect(void);

        // broadcast functions ========================================

        void Broadcast(CString message);

        void BroadcastAnimation(void);

        void BroadcastHit(CActor * player, CActor * hitter);

        void BroadcastFire(CProjectile* projectile);

        void BroadcastDestroy(CActor* actor);

        void BroadcastEnter(void);

         void BroadcastLeave(int colorIndex = -1);

         void BroadcastUpdate(void);

         void BroadcastPlayers(void);

         void ProcessMessage(CMessage& message);

         void ProcessMessages(void);

         bool Joined(void);

         void Listen(void);

         void Update(void);

};

extern CNetworkHandler* networkHandler;

// =================================================================================================
