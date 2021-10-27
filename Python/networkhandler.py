
import socket
import threading
import os
import globals

from timer import *
from vector import *
from actor import *
from gamedata import *
from gameitems import *
from arghandler import *

# =================================================================================================
# network data and address

class CMessage:
    '''
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
            List of single values from the payload
        numValues: 
            Number of values
        result: 
            Result of message processing (test for keyword match and required (minimum) number of values)

    Methods:
    --------
        IsValid (keyword, valueCount = 0):
            check and deconstruct a message 
    '''

    def __init__ (self, message, address, port = 0):
        '''
        Setup meaningful default values during class instance construction

        Parameters:
        -----------
            message:
                data payload as received over the UDP interface
            address:
                sender ip address
            port:
                sender udp port
        '''
        self.payload = message
        self.address = address
        self.port = port
        self.values = None
        self.numValues = 0
        self.result = 0


    def IsValid (self, valueCount = 0):
        '''
            check a message for a match with the requested keyword
            deconstruct message (split payload it into separate values)
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
        '''
        self.values = self.payload.split ("#")
        keyword = self.values [0]
        self.values = self.values [1].split (";")
        if (len (self.values [0]) > 0):
            self.numValues = len (self.values)
        else:
            self.values = []
            self.numValues = 0
        if (valueCount == 0):
            self.result = 1
            return True
        if (valueCount > 0):
            if (self.numValues == valueCount):
                self.result = 1
                return True
        elif (valueCount < 0):
            if (self.numValues >= -valueCount):
                self.result = 1
                return True
        print ("message {} has wrong number of values (expected {}, found {})".format (keyword, valueCount, self.numValues))
        self.result = -1
        return False


    def Str (self, i):
        '''
        return i-th parameter value as text

        Parameters:
        -----------
            i: Index of the requested parameter
        '''
        return self.values [i]


    def Int (self, i):
        '''
        return i-th parameter value as int

        Parameters:
        -----------
            i: Index of the requested parameter
        '''
        return int (self.values [i])


    def Float (self, i):
        '''
        return i-th parameter value as float

        Parameters:
        -----------
            i: Index of the requested parameter
        '''
        return float (self.values [i])


    # format: <x>,<y>,<z> (3 x float)
    def Vector (self, i):
        '''
        return i-th parameter value as 3D float vector

        Parameters:
        -----------
            i: Index of the requested parameter
        '''
        coords = self.values [i].split (",")
        return CVector (float (coords [0]), float (coords [1]), float (coords [2]))


    # format: <ip v4 address>":"<port>
    # <ip address> = "#.#.#.#" (# = one to three digit subnet id)
    def Address (self, i):
        '''
        return i-th parameter value as ip address:port pair

        Parameters:
        -----------
            i: Index of the requested parameter
        '''
        values = self.values [i].split (":")
        return values [0], int (values [1])

# =================================================================================================
# UDP based networking

class CUDP:
    def __init__ (self):
        self.localAddress = "127.0.0.1"
        self.localPorts = [9100, 9101]       # rx, tx
        self.sockets = [None, None]     # rx, tx


    def OpenSocket (self, port):
        if (self.localAddress == "127.0.0.1"):
            print ("UDP OpenSocket: Please specify a valid local network or internet address in the command line or ini file")
            return None
        try:
            s = socket.socket (socket.AF_INET, socket.SOCK_DGRAM) 
        except socket.error:
            return None
        try:
            s.bind ((self.localAddress, port))
        except socket.error as error:
            print ("UPD OpenSocket: Error {} ({}) on {}:{}".format (error.errno, os.strerror (error.errno), self.localAddress, port))
            return None
        s.setblocking (0)
        return s

    
    def InPort (self):
        return self.localPorts [0]


    def OutPort (self):
        return self.localPorts [1]


    def Transmit (self, message, address, port):
        if (self.sockets [1] is None):
            return False
        message = "SMIBAT" + message
        try:
            self.sockets [1].sendto (message.encode (), (address, port))
        except socket.error as error:    
            print ("UPD Transmit: Error {} ({}) on {}:{}".format (error.errno, os.strerror (error.errno), address, port))
            return False
        return True


    def Receive (self):
        if (self.sockets [0] is None):
            return False
        try:
            message, address = self.sockets [0].recvfrom (512) 
        except socket.error as error:
            if (error.errno != 10035):  # 10035 -> no data received
                print ("UPD Receive: Error {} ({})".format (error.errno, os.strerror (error.errno)))
            return None
        data = CMessage (message.decode (), address [0], address [1])
        if (data.payload.find ("SMIBAT") == 0):
            data.payload = data.payload.replace ("SMIBAT", "", 1)
            return data
        return None


# =================================================================================================

messageLock = threading.Semaphore (1)

class CListener (threading.Thread):
    def __init__ (self, args):
        super ().__init__ (target = self.Run, args = args)


    def Run (self, Listen):
        global messageLock

        while Listen ():
            message = globals.networkHandler.Receive ()
            if (message is None):
                time.sleep (0.005)
                continue
            messageLock.acquire ()
            globals.networkHandler.messages.append (message)
            messageLock.release ()

# =================================================================================================
# High level networking functions
#
# Full n x n peer to peer network
#
# Every player manages the data of his own actor and any shots he fires. This means that every player
# - detects whether his character has been hit and who hit it and will report that to all other players
# - updates all other players about his position and heading and the position and heading of all his shots
#
# This means that a client does not move around other actors than his own avatar and his own shots
# He also handles all collisions with his avatar, but only updates to his avatar's position will persist;
# Updates to other players' positions will be overriden by their update messages. For players with laggy
# connections, this can lead to objects penetrating each other. Oh well.
# 
# - The game host's only task is to accept new players, give them the list of all players already in the game 
# and assign an available color to them
# - If the game host leaves from the game, he will randomly make another player game host
# - If the game host gets disconnected, nobody can join a match in progress anymore, but any players in the match
#   can keep playing
#
# For scoring purposes, players are identified by their color. Projectiles are identified by their parent player's color 
# and an id that is unique on their client. The (id:color) tuple forms a match-wide unique projectile id by which projectiles
# can be unambiguously identified on each participant machine. Players always have id 0 (zero).
#
# Network message types:
#
# - Every message will be prefixed with "SMIBAT", followed by a message index, followed by an optional value list
# - Values will be semicolon separated
# Find all message below in CNetworkHandler.messageHandlers
#
# Alternatively, the game host could be taking over processing all collisions and updating all players
# with all actor positions and headings. In that case, every client would only send his data to the host,
# the host would Handle all client data and send the result to every client. The other clients would only need
# to talk to each other in case the host unexpectedly drops out of the game (e.g. disconnects), in which case
# they would need to negotiate a new game host. To negotiate a new host, each client could create a random number
# and send it to each other. The client with the lowest number becomes the new host. In case of a tie, the tied
# clients repeat this until resolution.
#
# All data will be transmitted as text (I have no idea how to transform binary data back to object data in Python).
# However, since there isn't a lot of data, this shouldn't be a problem.
# Heading and position will be transmitted as three float angles (pitch, yaw, bank) and three float coordinates (x,y,z)
# They will be packed in a string and can be parsed out of the string by the receiver.

class CNetworkHandler (CUDP):

    class CJoinState:
        def __init__ (self, iAmMaster):
            self.apply = 0
            self.map = 1        # wait for the host to send map data
            self.params = 2     # wait for the host to send global game parameters
            self.players = 3    # wait for the host to send the list of participants
            self.shots = 4      # wait for the host to send the list of shots currently present
            self.enter = 5      # wait for the host to send position and heading
            self.connected = 6  # connected, ready to play
            if iAmMaster:
                self.state = self.connected
            else:
                self.state = self.apply


    class CMessageHandler:
        def __init__ (self, name, handler):
            self.name = name
            self.handler = handler


    def __init__ (self):
        super ().__init__ ()
        self.localAddress = globals.argHandler.StrVal ("localaddress", 0, "127.0.0.1")
        self.localPorts = [globals.argHandler.IntVal ("inport", 0, 9100), globals.argHandler.IntVal ("outport", 0, 9101)]
        self.hostAddress = globals.argHandler.StrVal ("hostaddress", 0, "127.0.0.1")
        self.hostPorts = [globals.argHandler.IntVal ("hostport", 0, 0), 0]
        self.messageHandlers = [
            self.CMessageHandler ("APPLY", self.HandleApply),                   # process join request sent to game host
            self.CMessageHandler ("ACCEPT", self.HandleAccept),                 # receive position and heading from game host
            self.CMessageHandler ("REQUESTMAP", self.HandleMapRequest),         # send map data to new player
            self.CMessageHandler ("SYNCMAP", self.SyncMap),                     # process map data from game host
            self.CMessageHandler ("REQUESTPARAMS", self.HandleParamsRequest),   # send game parameters to new player
            self.CMessageHandler ("SYNCPARAMS", self.SyncParams),               # process game parameters from game host
            self.CMessageHandler ("REQUESTPLAYERS", self.HandlePlayersRequest), # send list of all players to new player
            self.CMessageHandler ("SYNCPLAYERS", self.SyncPlayers),             # add all players from player message to actor list that aren't already in it
            self.CMessageHandler ("REQUESTSHOTS", self.HandleShotsRequest),     # send currently live projectiles to new player
            self.CMessageHandler ("SYNCSHOTS", self.SyncShots),                 # add projectiles from message to actor list
            self.CMessageHandler ("ENTER", self.HandleEnter),                   # compute position and heading for new player and send them to him/her
            self.CMessageHandler ("ANIMATION", self.HandleAnimation),           # update sending player's animation state (which determines whether and which animation sound to play)
            self.CMessageHandler ("UPDATE", self.HandleUpdate),                 # update all local actors owned by the sending player with positions and headings from the message
            self.CMessageHandler ("FIRE", self.HandleFire),                     # create a projectile fired by another player
            self.CMessageHandler ("HIT", self.HandleHit),                       # integrate hit at a player into local player data
            self.CMessageHandler ("DESTROY", self.HandleDestroy),               # destroy a projectile that had hit another player
            self.CMessageHandler ("LEAVE", self.HandleLeave),                   # remove sending player from player list
            self.CMessageHandler ("REJECT", self.HandleReject)                  # react to some message sent to another player having been rejected by that player for some reason
            ]

        self.idMap = {}
        for id, h in enumerate (self.messageHandlers):
            self.idMap [h.name] = str (id) + "#"
        self.joinStateHandlers = [self.SendApply, self.SendSyncMap, self.SendSyncParams, self.SendSyncPlayers, self.SendSyncShots, self.SendEnter]
        self.messages = []
        self.fps = 60
        self.frameTime = 1000 // self.fps
        self.updateTimer = CTimer ()
        self.joinTimer = CTimer ()
        self.joinStateTimer = CTimer ()
        self.playerUpdateTimer = CTimer ()
        self.joinDelay = 5000              # 5 seconds between two consecutive join attempts
        self.joinStateDelay = 500
        self.timeoutPeriod = 30 * 1000     # seconds [ms] without message from a player after which the player will be removed from the game
        self.stringMap = []
        self.MapRow = -1
        self.threadedListener = globals.argHandler.BoolVal ("multithreading", 0, True)
        self.listen = True
        self.listenThread = None
        self.joinState = self.CJoinState (self.IamMaster ())
        self.syncingAddress = ""


    def Create (self, address = None, port = None):
        if (address is not None) and (port is not None):
            self.SetHostAddress (address, port)
        self.sockets [0] = self.OpenSocket (self.localPorts [0])
        self.sockets [1] = self.OpenSocket (self.localPorts [1])
        globals.actorHandler.viewer.SetAddress (self.localAddress, self.localPorts)
        if self.threadedListener:
            self.listenThread = CListener ((lambda : self.listen, ))
            self.listenThread.start ()


    def Destroy (self):
        if self.threadedListener:
            self.listen = False
            time.sleep (0.1)
            self.listenThread.join ()


    def SetHostAddress (self, address, port):
        self.hostAddress = address
        self.hostPorts [0] = port


    # message handling helper functions ========================================

    def VectorToMessage (self, v):
        return str (v.x) + "," + str (v.y) + "," + str (v.z)


    # construct a message with all update info for actor 
    def UpdateMessage (self, actor):
        message = str (actor.GetId ()) + ";" + \
                  str (actor.GetColorIndex ()) + ";" + \
                  self.VectorToMessage (actor.GetPosition ()) + ";" + \
                  self.VectorToMessage (actor.GetOrientation ()) 
        if (actor.id == 0):
            message += ";" + str (actor.hitPoints) + ";" + str (actor.score) + ";" + str (actor.lifeState) + ";" + str (actor.scale) + ";" + str (actor.ports [0])
        return message


    def PlayerMessage (self, player):
        return player.address + ":" + str (player.ports [0]) + ";" + str (player.ports [1]) + ";" + str (player.colorIndex)


    def IdFromName (self, textId):
        try:
            id = self.idMap [textId]
        except KeyError:
            print ("Invalid message id '{}'".format (textId))
            return None
        return id


    def IdFromMessage (self, message):
        values = message.payload.split ("#")
        if (len (values) > 0):
            id = int (values [0])
            if (id < len (self.messageHandlers)):
                return id
        return None


    # networking helper functions ========================================

    def FindPlayer (self, address, port):
        for a in globals.actorHandler.actors:
            # check for port too as there may be players in the same local network and behind the same router/firewall, sharing the same ip address and just using different ports
            if (a.id == 0) and (a.address == address) and (a.ports [1] == port): 
                return a
        return None


    def RemotePlayerCount (self):
        rpc = 0
        for a in globals.actorHandler.actors:
            if (a.id == 0) and not a.IsLocalActor ():
                rpc += 1
        return rpc


    def UpdateLastMessageTime (self, message):
        player = self.FindPlayer (message.address, message.port)
        if (player is not None):
            player.lastMessageTime = globals.gameData.gameTime
                

    def IamMaster (self):
        return (self.hostAddress == "127.0.0.1") or (self.hostAddress == self.localAddress)


    def IamConnected (self):
        if self.IamMaster ():
            return 1
        if (self.joinState.state == self.joinState.apply):
            return 0
        host = self.FindPlayer (self.hostAddress, self.hostPorts [1])
        if (host is None):
            return 0
        if self.TimedOut (host):
            return -1
        return 1


    def TimedOut (self, player):
        return not player.IsLocalActor () and (globals.gameData.gameTime - player.lastMessageTime > self.timeoutPeriod)

    # send functions ========================================

    def SendApply (self):
        self.Transmit (self.IdFromName ("APPLY") + str (self.InPort ()), self.hostAddress, self.hostPorts [0])


    def SendSyncMap (self):
        self.mapRow = -1
        self.stringMap = []
        self.Transmit (self.IdFromName ("REQUESTMAP") + str (globals.actorHandler.viewer.ports [0]), self.hostAddress, self.hostPorts [0])


    def SendSyncParams (self):
        self.Transmit (self.IdFromName ("REQUESTPARAMS") + str (globals.actorHandler.viewer.ports [0]), self.hostAddress, self.hostPorts [0])


    def SendSyncShots (self):
        self.Transmit (self.IdFromName ("REQUESTSHOTS") + str (globals.actorHandler.viewer.ports [0]), self.hostAddress, self.hostPorts [0])


    def SendSyncPlayers (self):
        self.Transmit (self.IdFromName ("REQUESTPLAYERS") + str (globals.actorHandler.viewer.ports [0]), self.hostAddress, self.hostPorts [0])


    def SendEnter (self, address = None, port = None):
        if (address is None):
            address = self.hostAddress
            port = self.hostPorts [0]
        self.Transmit (self.IdFromName ("ENTER") + str (globals.actorHandler.viewer.colorIndex) + ";" + str (self.InPort ()), address, port)


    # accept tells the player requesting to join his own ip address since it it somewhat tedious to determine your own external ip address when behing a router, firewall and what not
    def SendAccept (self, player):
        message = self.IdFromName ("ACCEPT") + \
                  str (player.colorIndex) + ";" + \
                  self.VectorToMessage (player.GetPosition ()) + ";" + \
                  self.VectorToMessage (player.GetOrientation ()) + ";" + \
                  player.address
        self.Transmit (message, player.address, player.ports [0])


    # format: MAP:<tag>:<text line from map file>
    #         tags: reversed line number (starting at line count of map - 1) => 0 for last line
    def SendMap (self, address, port):
        i = len (globals.gameItems.map.stringMap)
        for l in globals.gameItems.map.stringMap:
            i -= 1
            self.Transmit (self.IdFromName ("SYNCMAP") + str (i) + ";" + l, address, port)


    # format: PARAMS:<fire delay>;<heal delay>;<respawn delay>;<immunity duration>;<points for kill>;<projectile size>;<projectile speed>;<move speed>;<turn speed>
    def SendParams (self, address, port):
        self.Transmit (self.IdFromName ("SYNCPARAMS") + \
                       str (globals.gameData.fireDelay) + ";" + \
                       str (globals.gameData.healDelay) + ";" + \
                       str (globals.gameData.respawnDelay) + ";" + \
                       str (globals.gameData.immunityDuration) + ";" + \
                       str (globals.gameData.pointsForKill) + ";" + \
                       str (globals.gameData.projectileSize) + ";" + \
                       str (globals.gameData.projectileSpeed) + ";" + \
                       str (globals.controlsHandler.GetMoveSpeed ()) + ";" + \
                       str (globals.controlsHandler.GetTurnSpeed ()),
                       address, port)


    # format: PLAYERS<player data>[;<player data> [...]]
    # player data: <address>:<port>;<color index>
    def SendPlayers (self, address, port):
        message = self.IdFromName ("SYNCPLAYERS")
        # add addresses of all other players to player requesting to join
        for i, a in enumerate (globals.actorHandler.actors):
            if (a.id == 0):     # player
                if (i > 0):     # not the first entry in the list
                    message += ";"
                message += a.address + ":" + str (a.ports [0]) + ";" + str (a.ports [1]) + ";" + str (a.colorIndex)
        self.Transmit (message, address, port)


    # format: SHOTS<shot data>[;<shot data> [...]]
    # shot data: <actor id>;<parent color index>
    def SendShots (self, address, port):
        message = self.IdFromName ("SYNCSHOTS")
        # add addresses of all other players to player requesting to join
        for i, a in enumerate (globals.actorHandler.actors):
            if (a.id != 0):     # projectile
                if (i > 0):     # not the first entry in the list
                    message += ";"
                message += str (a.id) + ";" + str (a.parent.colorIndex)
        self.Transmit (message, address, port)


    # send an update message to a single player
    def SendUpdate (self, address, port):
        self.Transmit (self.IdFromName ("UPDATE") + self.UpdateMessage (globals.actorHandler.viewer), address, port)


    def SendReject (self, address, port, reason):
        self.Transmit (self.IdFromName ("REJECT") + reason, address, port)


    def AddPlayer (self, address, ports, colorIndex):
        player = self.FindPlayer (address, ports [1])
        if (player is None):
            player = globals.actorHandler.CreatePlayer (colorIndex, address = address, ports = ports)
            if (player is None):
                self.SendReject (player.address, ports [0], "full")
                return None
            player.UpdateLastMessageTime ()
        return player


    def OutOfSync (self, joinState, isFinal = True):
        if (self.joinState.state == joinState):
            return False
        # self.joinState.state = self.joinState.apply
        if (isFinal):
            self.SendReject (self.hostAddress, self.hostPorts [0], "out of sync")
        return True

    # incoming message processing functions ========================================

    #format: APPLY<host address>;<client rx port>
    def HandleApply (self, message):
        if not message.IsValid (1):
            return message.result
        if (self.syncingAddress == ""):
            self.syncingAddress = message.address
        elif (self.syncingAddress != message.address):
            self.SendReject (message.address, message.Int (0), "sync in progress")
            return -1
        self.SendEnter (message.address, message.Int (0))
        return 1


    #format: ENTER<client color>;<client rx port>
    def HandleEnter (self, message):
        if not message.IsValid (2):
            return message.result
        if self.OutOfSync (self.joinState.connected, False) and self.OutOfSync (self.joinState.apply):  # this message is permitted when already connected
            return -1
        colorIndex = message.Int (0)
        if self.IamMaster ():   # reassign color if requested color is already in use
            if (not globals.gameData.ColorIsAvailable (colorIndex)):
                colorIndex = -1
        player = self.AddPlayer (message.address, [message.Int (1), message.port], colorIndex)  # colorIndex == -1 --> assign an available random color
        if (player is None):
            return -1
        if self.IamMaster ():
            globals.gameItems.map.FindSpawnPosition (player)
            player.lastMessageTime = globals.gameData.gameTime
            self.SendAccept (player)
            self.syncingAddress = ""
        else:
            if (message.address == self.hostAddress):
                self.hostPorts [1] = message.port
        if (self.joinState.state == self.joinState.apply):
            self.joinState.state = self.joinState.map
        return 1


    def HandleMapRequest (self, message):
        if not message.IsValid (1):
            return message.result
        self.SendMap (message.address, message.Int (0))
        return 1


    # format:MAP<row #>;<row text>
    def SyncMap (self, message):
        if not message.IsValid (2):
            return message.result
        if self.OutOfSync (self.joinState.map):
            return -1
        row = message.Int (0)
        # self.mapRow < 0 ==> we're receiving the first map row
        # map row number are sent in reverse order (highest first, down to zero)
        if (self.mapRow >= 0) and (row != self.mapRow - 1):
            return -1   # we lost at least one map row
        self.mapRow = row
        self.stringMap.append (message.Str (1))
        if (row > 0):
            return 1
        if not globals.gameItems.map.CreateFromMemory (self.stringMap, True):
            return -1
        self.joinState.state = self.joinState.params
        return 1


    def HandleParamsRequest (self, message):
        if not message.IsValid (1):
            return message.result
        self.SendParams (message.address, message.Int (0))
        return 1


    # format: PARAMS<heal delay>;<respawn delay>;<immunity duration>;<move speed>;<turn speed>
    def SyncParams (self, message):
        if not message.IsValid (9):
            return message.result
        if self.OutOfSync (self.joinState.params):
            return -1
        globals.gameData.fireDelay = message.Int (0)
        globals.gameData.healDelay = message.Int (1)
        globals.gameData.respawnDelay = message.Int (2)
        globals.gameData.immunityDuration = message.Int (3)
        globals.gameData.pointsForKill = message.Int (4)
        globals.gameData.projectileSize = message.Float (5)
        globals.gameData.projectileSpeed = message.Float (6)
        globals.controlsHandler.SetMoveSpeed (message.Float (7))
        globals.controlsHandler.SetTurnSpeed (message.Float (8))
        self.joinState.state = self.joinState.players
        return 1


    def HandlePlayersRequest (self, message):
        if not message.IsValid (1):
            return message.result
        self.SendPlayers (message.address, message.Int (0))
        return 1


    # add each player from message to actor list which isn't in it already
    def SyncPlayers (self, message):
        if not message.IsValid (-3):
            return message.result
        if self.OutOfSync (self.joinState.connected, False) and self.OutOfSync (self.joinState.players):  # this message is permitted when already connected
            return -1
        for i in range (0, message.numValues, 3):
            colorIndex = message.Int (i + 2)
            player = globals.actorHandler.FindPlayer (colorIndex)
            if player is None:
                address, inPort = message.Address (i)
                player = self.AddPlayer (address, [inPort, message.Int (i + 1)], colorIndex)
                if (player is None):    # synchronization error
                    self.SendReject (self.hostAddress, self.hostPorts [0], "out of sync")
                    return -1
                self.SendEnter (address, inPort)
        if (self.joinState.state == self.joinState.players):    # this message will periodically be sent to all connected players
            self.joinState.state = self.joinState.shots         # so only change join state when actually joining
        return 1


    def HandleShotsRequest (self, message):
        if not message.IsValid (1):
            return message.result
        self.SendShots (message.address, message.Int (0))
        return 1


    # add each projectile from message to actor list 
    def SyncShots (self, message):
        if not message.IsValid (0):
            return message.result
        if self.OutOfSync (self.joinState.shots):  # this message is permitted when already connected
            return -1
        for i in range (0, message.numValues, 2):
            colorIndex = message.Int (i + 1)
            parent = globals.actorHandler.FindPlayer (colorIndex)
            if parent is not None:
                globals.actorHandler.CreateActor (message.Int (i), colorIndex, parent.GetPosition (), parent.GetOrientation ())
        # self.SendEnter (self.hostAddress, self.hostPorts [0])
        self.joinState.state = self.joinState.enter
        return 1


    # format: ACCEPT<color>;<position>;<orientation>;<address>;<address>:<port>;...;<address>:<port>
    # <id> is always 0 (zero) for players and hence will be ignored
    # ip addresses and ports of all other players including the game host will be transmitted
    def HandleAccept (self, message):
        if not message.IsValid (4):   # need at least five values
            return message.result
        if self.OutOfSync (self.joinState.enter):
            return -1
        globals.actorHandler.viewer.SetColorIndex (message.Int (0), True)
        globals.actorHandler.viewer.SetPosition (message.Vector (1))
        globals.actorHandler.viewer.SetOrientation (message.Vector (2))
        globals.actorHandler.viewer.address = message.Str (3)
        globals.actorHandler.viewer.ForceRespawn ()
        self.joinState.state = self.joinState.connected
        return 1


    # format: FIRE<projectile id>;<projectile parent color index>
    def HandleFire (self, message):
        if not message.IsValid (2):
            return message.result
        parent = globals.actorHandler.FindPlayer (message.Int (1))
        if (parent is None):
            return -1
        projectile = globals.actorHandler.CreateProjectile (parent, message.Int (0))
        if (projectile is not None):
            return 1
        return -1


    def HandleAnimation (self, message):
        if not message.IsValid (2):
            return message.result
        actor = globals.actorHandler.FindActor (0, message.Int (0))
        if (actor is None):
            return 0
        actor.SetAnimation (message.Int (1))
        return 1


    # message: UPDATE<id>;<color>;<position>;<orientation>[;<player info>]
    # position: <x>,<y>,<z> (3 x float)
    # orientation: <pitch>,<yaw>,<roll> (3 x float angles)
    # player info: <hitpoints>;<score>;<life state>;<scale> (only for players, not for projectiles)
    # an update from an unknown player will cause creation of that player since that player must have been accepted by the gamehost
    # or he wouldn't know this client's address. Receiving messages from unknown players may be the result of a previous disconnect
    def HandleUpdate (self, message):
        if not message.IsValid (-4):
            return message.result
        actorId = message.Int (0)
        colorIndex = message.Int (1)
        actor = globals.actorHandler.FindActor (actorId, colorIndex)
        if (actor is None):
            if (actorId != 0):
                return 0
            if (message.numValues < 9):
                return -1
            inPort = message.Int (8)
            actor = self.AddPlayer (message.address, [inPort, message.port], colorIndex)  # colorIndex == -1 --> assign an available random color
            if (actor is None):
                self.SendReject (message.address, inPort, "unknown")
                return 0
        actor.camera.BumpPosition ()
        actor.SetPosition (message.Vector (2))
        actor.SetOrientation (-message.Vector (3))
        if (actorId == 0):  # player
            if (message.numValues < 9):
                return -1
            actor.lastMessageTime = globals.gameData.gameTime
            actor.SetHitPoints (message.Int (4))
            actor.SetScore (message.Int (5))
            actor.SetLifeState (message.Int (6))
            actor.SetScale (message.Float (7))
        else:
            actor.UpdateFrozenTime ()
            actor.parent.lastMessageTime = globals.gameData.gameTime
        return 1


    # message: HIT:<target color>;<hitter color>
    def HandleHit (self, message):
        if not message.IsValid (2):
            return message.result
        target = globals.actorHandler.FindPlayer (message.Int (0))
        if (target is None):
            return -1
        hitter = globals.actorHandler.FindPlayer (message.Int (1))
        if (hitter is None):
            return -1
        target.RegisterHit (hitter)
        target.lastMessageTime = globals.gameData.gameTime
        return 1


    # message: DESTROY:<id>;<color>
    def HandleDestroy (self, message):
        if not message.IsValid (2):
            return message.result
        if globals.actorHandler.DeleteActor (message.Int (0), message.Int (1)):
            return 1
        return -1


    # message: LEAVE:<color>
    def HandleLeave (self, message):
        if not message.IsValid (1):
            return message.result
        # values = message.payload.split (";")
        if globals.actorHandler.DeletePlayer (message.Int (0)):
            return 1
        return -1


    def HandleReject (self, message):
        if not message.IsValid (1):
            return message.result
        reason = message.Str (0)
        if (reason == "unknown"):
            if not self.IamMaster ():
                self.SendApply ()
            return 1
        if (reason == "full"):
            print ("Cannot join: match is full")
            return 1
        if (reason == "out of sync"):
            return 1
        return -1


    def HandleTimeouts (self):
        for a in globals.actorHandler.actors:
            if (a.IsPlayer () and not a.isViewer and self.TimedOut (a)):
                if self.IamMaster ():
                    self.BroadcastDestroy (a)
                globals.actorHandler.DeleteActor (a.id, a.colorIndex)


    def HandleDisconnect (self):
        connectStatus = self.IamConnected ()
        if (connectStatus < 0):     # i.e. had been connected, but host connection has timed out
            self.BroadcastLeave ()
            self.joinState.state = self.joinState.apply
        elif (connectStatus == 0):
            globals.actorHandler.CleanupActors ()
            self.joinState.state = self.joinState.apply

    # broadcast functions ========================================


    def Broadcast (self, message):
        for a in globals.actorHandler.actors:
            if a.IsPlayer() and not a.IsLocalActor(): # players have id zero
                self.Transmit (message, a.address, a.ports [0])


    # format: ANIMATION<color>;<animation>
    def BroadcastAnimation (self):
        self.Broadcast (self.IdFromName ("ANIMATION") + str (globals.actorHandler.viewer.colorIndex) + ";" + str (globals.actorHandler.viewer.animation))


    # tell every other player that we've been hit, and who hit us
    # format: HIT:<target color>:<hitter color>
    # will always be sent asap
    def BroadcastHit (self, player, hitter):
        self.Broadcast (self.IdFromName ("HIT") + str (player.colorIndex) + ";" + str (hitter.colorIndex))


    # tell every other player that we have fired a projectile
    # format: FIRE:<id>;<parent color>
    # will always be sent asap
    def BroadcastFire (self, projectile):
        self.Broadcast (self.IdFromName ("FIRE") + str (projectile.id) + ";" + str (projectile.GetColorIndex ()))


    def BroadcastDestroy (self, actor):
        self.Broadcast (self.IdFromName ("DESTROY") + str (actor.id) + ";" + str (actor.GetColorIndex ()))


    def BroadcastEnter (self):
        self.Broadcast (self.IdFromName ("ENTER") + str (globals.actorHandler.viewer.colorIndex))


    # format: LEAVE:<color>
    def BroadcastLeave (self, colorIndex = -1):
        if (colorIndex < 0):
            colorIndex = globals.actorHandler.viewer.colorIndex
        self.Broadcast (self.IdFromName ("LEAVE") + str (colorIndex))
        for actor in globals.actorHandler.actors:
            if (actor.id == 0) and not actor.isViewer: # players have id zero
                globals.actorHandler.DeletePlayer (actor.colorIndex)


    # inform every other player about our position and heading and position and heading of each of our shots
    # will be sent at the network fps
    def BroadcastUpdate (self):
        colorIndex = globals.actorHandler.viewer.colorIndex
        for actor in globals.actorHandler.actors:
            if (actor.GetColorIndex () == colorIndex):   # actor is viewer or child of viewer
                self.Broadcast (self.IdFromName ("UPDATE") + self.UpdateMessage (actor))


    def BroadcastPlayers (self):
        message = self.IdFromName ("PLAYERS")
        # add addresses of all other players to player requesting to join
        for i, a in enumerate (globals.actorHandler.actors):
            if (a.id == 0):
                if (i > 0):
                    message += ";"
                message += self.PlayerMessage (a)
        self.Broadcast (message)


    def ProcessMessage (self, message):
        self.UpdateLastMessageTime (message)    # update last message time of player who sent this message
        id = self.IdFromMessage (message)
        if (id is None):
            print ("Received faulty message '{}'".format (message.payload))
        elif (self.messageHandlers [id].handler (message) < 0):
            print ("Received faulty {} message '{}'".format (self.messageHandlers [id].name, message.payload))


    def ProcessMessages (self):
        global messageLock
        if (len (self.messages) > 0):
            messageLock.acquire ()
            for message in self.messages:
                self.ProcessMessage (message)
            self.messages = []
            messageLock.release ()


    def Joined (self):
        if self.IamMaster ():
            return True
        if (self.joinState.state == self.joinState.connected):
            return True
        # wait longer before trying to connect
        if (self.joinState.state == self.joinState.apply):
            if not self.joinTimer.HasPassed (self.joinDelay, True):
                return False
        else:
            if not self.joinStateTimer.HasPassed (self.joinStateDelay, True):
                return False
        stateHandler = self.joinStateHandlers [self.joinState.state]
        if (stateHandler is not None):
            stateHandler ()
        return False


    def Listen (self):
        if self.threadedListener:
            self.ProcessMessages ()
        else:
            listenTimer = CTimer (5)
            listenTimer.Start ()
            while not listenTimer.HasPassed (5):    # listen and Handle messages for max. ~ 5 ms
                message = self.Receive ()
                if (message is None):
                    break
                self.ProcessMessage (message)


    def Update (self):
        if self.updateTimer.HasPassed (self.frameTime, True):
            if self.Joined ():
                self.BroadcastUpdate ()
                # if self.IamMaster () and self.playerUpdateTimer.HasPassed (5000, True):
                #     self.BroadcastPlayers ()
                self.HandleTimeouts ()
            self.HandleDisconnect ()
            self.Listen ()

# =================================================================================================
