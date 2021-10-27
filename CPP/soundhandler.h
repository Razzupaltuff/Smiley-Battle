#pragma once

#include <math.h>

#include "SDL.h"
#include "SDL_mixer.h"
#include "vector.h"
#include "actor.h"

#include "carray.h"
#include "clist.h"
#include "cavltree.h"

// =================================================================================================

class CSoundObject {
    public:
        int         m_id;
        CString     m_name;
        Mix_Chunk * m_sound;
        int         m_channel;
        float       m_volume;
        CVector     m_position;
        CActor *    m_owner;
        size_t      m_startTime;

        CSoundObject(int id = -1, CString name = CString(""), int channel = -1, Mix_Chunk * sound = nullptr, CVector position = CVector(0, 0, 0), float volume = 1.0f)
            : m_id(id), m_name(name), m_channel(channel), m_sound(sound), m_position(position), m_owner (nullptr), m_volume(volume), m_startTime (0)
        {}

        ~CSoundObject () {
            if (Busy ())
                Stop ();
        }

        void Play (int loops = 1);

        void Stop (void);

        void SetPanning (float left, float right);

        void SetVolume (float volume);

        bool Busy (void);
    };

    // =================================================================================================
// The sound handler class handles sound creation and sound channel management
// It tries to provide 128 sound channels. They are preinitialized and are kept in m_idleChannels
// (list of available channels) and busyChannels (list of channels currently used for playing back sound)
// When a new sound is to played, a channel is picked from the idleChannels list. If there are no idle
// channels available, the oldest playing channel from busyChannels will be reused. Since channels are 
// append to busyChannels in the temporal sequence they are deployed, the first channel in busyChannels
// will always be the oldest one.

class CSoundHandler {
    public:
        CAvlTree<CString, Mix_Chunk*>   m_sounds;
        CList<CSoundObject>             m_idleChannels;
        CList<CSoundObject>             m_busyChannels;
        int                             m_soundLevel;
        float                           m_masterVolume;
        float                           m_maxAudibleDistance;
        int                             m_channelCount;

        CSoundHandler();

        int FindActorSound(CActor* actor, CString name);

        // play back the sound with the name 'name'. Position, viewer and DistFunc serve for computing the sound volume
        // depending on the distance of the viewer to the sound position
        int Play(CString name, CVector position = CVector(NAN, NAN, NAN), float volume = 1.0f, int loops = 0, CActor* owner = nullptr, int level = 1);

        void Stop(int id);

        void StopActorSounds(CActor* actor);

        // update all sound volumes depending on distance to viewer (viewer or sound source may have been moving)
        void UpdateSound(CSoundObject& soundObject);

        // move all channels that are not playing back sound anymore from the busyChannels to the idleChannels list
        void Cleanup(void);

        // cleanup expired channels and update sound volumes
        void Update(void);

    private:

        // preload sound data. Sound data is kept in a dictionary. The sound name is the key to it.
        void LoadSounds(void);

        // compute stereo panning from the angle between the viewer direction and the vector from the viewer to the sound source
        float Pan(CVector& position);

        void UpdateVolume(CSoundObject& soundObject, float d);

        // get a channel for playing back a new sound
        // if all channels are busy, pick the oldest busy one
        CSoundObject& GetChannel(void);

};

extern CSoundHandler* soundHandler;

// =================================================================================================
