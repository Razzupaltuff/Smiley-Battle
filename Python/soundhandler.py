import numpy as np
import pygame
import globals

from vector import *
from gameitems import *
from arghandler import *

# =================================================================================================

class CSoundObject:
    def __init__ (self, id, name, channel, sound, position, volume = 1.0):
        self.id = id
        self.name = name
        self.sound = sound
        self.channel = channel
        self.volume = volume
        self.position = position
        self.owner = None
        self.startTime = 0
        # self.channel.set_endevent (pygame.USEREVENT + self.id + 1)


    def Play (self):
        self.channel.play (self.sound)

# =================================================================================================
# The sound handler class handles sound creation and sound channel management
# It tries to provide 128 sound channels. They are preinitialized and are kept in self.idleChannels
# (list of available channels) and busyChannels (list of channels currently used for playing back sound)
# When a new sound is to played, a channel is picked from the idleChannels list. If there are no idle
# channels available, the oldest playing channel from busyChannels will be reused. Since channels are 
# append to busyChannels in the temporal sequence they are deployed, the first channel in busyChannels
# will always be the oldest one.

class CSoundHandler:
    def __init__ (self):
        self.sounds = {}
        self.LoadSounds ()
        self.idleChannels = []
        self.busyChannels = []
        self.soundLevel = globals.argHandler.IntVal ("soundlevel", 0, 1)
        self.masterVolume = globals.argHandler.FloatVal ("masterVolume", 0, 1)
        self.maxAudibleDistance = 30.0
        pygame.mixer.quit ()
        pygame.mixer.init (channels = 2)
        pygame.mixer.set_num_channels (128)
        self.channelCount = pygame.mixer.get_num_channels ()
        for i in range (self.channelCount):
            self.idleChannels.append (CSoundObject (i, None, None, None, CVector (0,0,0)))
            

    # preload sound data. Sound data is kept in a dictionary. The sound name is the key to it.
    def LoadSounds (self):
        soundNames = ["reappear", "disappear", "collide", "flare", "fusion", "hit", "hum", "laser", "phoenix", "pickup", "scrape", "shot", "wallhit", "wallhit-low"]
        for name in soundNames:
            self.sounds [name] = pygame.mixer.Sound (globals.gameData.soundFolder + name + ".wav")


    # compute stereo panning from the angle between the viewer direction and the vector from the viewer to the sound source
    def Pan (self, position):
        # global gameItems
        v = position - globals.gameItems.viewer.GetPosition ()
        v.Normalize ()
        v = globals.gameItems.viewer.camera.orientation.Rotate (v)
        return v.Dot (CVector (1,0,0))       # > 0: right side of viewer, < 0: left side of viewer


    def UpdateVolume (self, soundObject, d):
        if (d > self.maxAudibleDistance):
            volume = 0
            pan = 0
        else:
            volume = (self.maxAudibleDistance - d) / self.maxAudibleDistance
            volume *= volume * soundObject.volume * self.masterVolume
            pan = self.Pan (soundObject.position) / 2   # use half of the volume for stereo panning
        soundObject.channel.set_volume (abs (-0.5 + pan) * volume, (0.5 + pan) * volume)
        # soundObject.channel.set_volume (self.Abs (-0.5 + pan) * volume, (0.5 + pan) * volume) # 3 * volume - pan * volume, 3 * volume + pan * volume)
        #   print ("{0: 3d} volumes: {1:1.2f}, {2:1.2f}, pan: {3:1.2f}. SDL: {4:1.2f}".format (soundObject.id, (0.5 + pan) * volume, np.abs (-0.5 + pan) * volume, pan, soundObject.channel.get_volume ()))
        # print ("position: {0:1.2f} / {1:1.2f} / {2:1.2f}".format (soundObject.position.x, soundObject.position.y, soundObject.position.z))
        # print ("  viewer: {0:1.2f} / {1:1.2f} / {2:1.2f}".format (viewer.camera.positions [0].x, viewer.camera.positions [0].y, viewer.camera.positions [0].z))


    # get a channel for playing back a new sound
    # if all channels are busy, pick the oldest busy one
    def GetChannel (self):
        if (len (self.idleChannels) > 0):
            self.busyChannels.append (self.idleChannels.pop (-1))
        else:
            self.busyChannels [0].channel.stop ()
            self.busyChannels.append (self.busyChannels.pop (0))
        return self.busyChannels [-1]


    def FindActorSound (self, actor, name):
        for c in self.busyChannels:
            if (c.owner == actor) and (c.name == name):
                return c
        return None


    # play back the sound with the name 'name'. Position, viewer and DistFunc serve for computing the sound volume
    # depending on the distance of the viewer to the sound position
    def Play (self, name, position, volume = 1.0, loops = 0, owner = None, level = 1):
        if (self.soundLevel == 0) or (level > self.soundLevel):
            return -1
        if (position is None):
            return -1

        c = self.FindActorSound (owner, name)
        if (c is not None):
            return c.id
        d = globals.gameItems.map.Distance (position, globals.gameItems.viewer.GetPosition ())
        try:
            sound = self.sounds [name]
        except KeyError:
            return -1
        soundObject = self.GetChannel ()
        soundObject.name = name
        soundObject.channel = pygame.mixer.find_channel ()
        soundObject.sound = pygame.mixer.Sound (sound.get_raw ()) # using a copy of the sound because each channel's overall volume has to be set via the sound
        soundObject.sound.set_volume (volume)
        # soundObject.sound.set_volume (0.1)
        soundObject.volume = volume # / 5.0
        soundObject.position = position
        soundObject.startTime = pygame.time.get_ticks ()
        soundObject.owner = owner
        soundObject.channel.play (soundObject.sound, loops)
        self.UpdateVolume (soundObject, d)
        return soundObject.id


    def Stop (self, id):
        for i, c in enumerate (self.busyChannels):
            if (c.id == id):
                c.channel.stop ()
                self.idleChannels.append (self.busyChannels.pop (i))


    def StopActorSounds (self, actor):
        for i, c in enumerate (self.busyChannels):
            if (c.owner == actor):
                c.channel.stop ()
                self.idleChannels.append (self.busyChannels.pop (i))


    # update all sound volumes depending on distance to viewer (viewer or sound source may have been moving)
    def UpdateSound (self, soundObject):
        # global gameItems
        if (soundObject.owner):
            soundObject.position = soundObject.owner.GetPosition ()
        self.UpdateVolume (soundObject, globals.gameItems.map.Distance (soundObject.position, globals.gameItems.viewer.GetPosition ()))

    
    # move all channels that are not playing back sound anymore from the busyChannels to the idleChannels list
    def Cleanup (self):
        i = len (self.busyChannels)
        while (i > 0):
            i -= 1
            if not self.busyChannels [i].channel.get_busy ():
                self.idleChannels.append (self.busyChannels.pop (i))


    # cleanup expired channels and update sound volumes
    def Update (self):
        self.Cleanup ()
        for soundObject in self.busyChannels:
            self.UpdateSound (soundObject)

# =================================================================================================
