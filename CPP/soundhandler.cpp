#include "soundHandler.h"
#include "gameItems.h"
#include "gameData.h"
#include "argHandler.h"

// =================================================================================================

void CSoundObject::Play (int loops) {
    if (0 > Mix_PlayChannel (m_channel, m_sound, loops))
        fprintf (stderr, "Couldn't play sound '%s' (%s)\n", m_name.Buffer (), Mix_GetError ());
#if 0
    else if (Busy ())
        fprintf (stderr, "playing '%s' on channel %d (%d loops)\n", m_name.Buffer (), m_channel, loops);
    else
        fprintf (stderr, "Couldn't play sound '%s' (%s)\n", m_name.Buffer (), Mix_GetError ());
#endif
}

void CSoundObject::Stop (void) {
    Mix_HaltChannel (m_channel);
}

void CSoundObject::SetPanning (float left, float right) {
    Mix_SetPanning (m_channel, Uint8 (left * 255), Uint8 (right * 255));
}

void CSoundObject::SetVolume (float volume) {
    Mix_Volume (m_channel, int (MIX_MAX_VOLUME * volume));
}

bool CSoundObject::Busy (void) {
    return bool (Mix_Playing (m_channel));
}

// =================================================================================================
// The sound handler class handles sound creation and sound channel management
// It tries to provide 128 sound channels. They are preinitialized and are kept in m_idleChannels
// (list of available channels) and busyChannels (list of channels currently used for playing back sound)
// When a new sound is to played, a channel is picked from the idleChannels list. If there are no idle
// channels available, the oldest playing channel from busyChannels will be reused. Since channels are 
// append to busyChannels in the temporal sequence they are deployed, the first channel in busyChannels
// will always be the oldest one.

CSoundHandler::CSoundHandler () {
    m_sounds.SetComparator(CString::Compare);
    m_soundLevel = argHandler->IntVal("soundlevel", 0, 1);
    m_masterVolume = argHandler->FloatVal("masterVolume", 0, 1);
    m_maxAudibleDistance = 30.0f;
    Mix_Quit();
    Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);
    if (0 > Mix_OpenAudio (44100, AUDIO_S16SYS, 2, 4096))
        fprintf (stderr, "Couldn't initialize sound system (%s)\n", Mix_GetError ());
    Mix_Volume (-1, MIX_MAX_VOLUME);
    Mix_AllocateChannels(128);
    m_channelCount = Mix_AllocateChannels(-1);
    for (int i = 0; i < m_channelCount; i++)
        m_idleChannels.Append(CSoundObject(i, CString(""), i));
    LoadSounds ();
}


// preload sound data. Sound data is kept in a dictionary. The sound name is the key to it.
void CSoundHandler::LoadSounds(void) {
    CList<CString> soundNames = {
        CString("reappear"), CString("disappear"), CString("collide"), CString("flare"), CString("fusion"), CString("hit"), CString("hum"),
        CString("laser"), CString("phoenix"), CString("pickup"), CString("scrape"), CString("shot"), CString("wallhit"), CString("wallhit-low")
    };
    for (auto [i, name] : soundNames) {
        CString fileName = gameData->m_soundFolder + *name + ".wav";
        Mix_Chunk* sound = Mix_LoadWAV (fileName.Buffer ());
        if (sound)
            m_sounds.Insert(name, sound);
        else
            fprintf (stderr, "Couldn't load sound '%s' (%s)\n", name.Buffer (), Mix_GetError ());
    }
}


// compute stereo panning from the angle between the viewer direction and the vector from the viewer to the sound source
float CSoundHandler::Pan(CVector& position) {
    // global gameItems
    CVector v = position - gameItems->m_viewer->GetPosition();
    v.Normalize();
    v = gameItems->m_viewer->m_camera.m_orientation.Rotate(v);
    return v.Dot(CVector(1, 0, 0));       // > 0: right side of viewer, < 0: left side of viewer
}


void CSoundHandler::UpdateVolume(CSoundObject& soundObject, float d) {
    float volume, pan;
    if (d > m_maxAudibleDistance)
        volume = pan = 0.0f;
    else {
        volume = (m_maxAudibleDistance - d) / m_maxAudibleDistance;
        volume *= volume * soundObject.m_volume * m_masterVolume;
        pan = Pan(soundObject.m_position) / 2;   // use half of the volume for stereo panning
    }
    soundObject.SetPanning(abs(-0.5f + pan) * volume, (0.5f + pan) * volume);
}


// get a channel for playing back a new sound
// if all channels are busy, pick the oldest busy one
CSoundObject& CSoundHandler::GetChannel(void) {
    if (!m_idleChannels.Empty())
        m_busyChannels.Append(m_idleChannels.Pop(-1));
    else {
        m_busyChannels[0].Stop();
        m_busyChannels.Append(m_busyChannels.Pop(0));
    }
    return m_busyChannels[-1];
}


int CSoundHandler::FindActorSound(CActor* actor, CString name) {
    for (auto [i, c] : m_busyChannels)
        if ((c.m_owner == actor) && (c.m_name == name)) {
            return c.m_id;
        }
    return -1;
}


// play back the sound with the name 'name'. Position, viewer and DistFunc serve for computing the sound volume
// depending on the distance of the viewer to the sound position
int CSoundHandler::Play(CString name, CVector position, float volume, int loops, CActor* owner, int level) {
    //return -1;
    if ((m_soundLevel == 0) || (level > m_soundLevel))
        return -1;
    if (!position.IsValid())
        return -1;

    int soundId = FindActorSound(owner, name);
    if (soundId >= 0)
        return soundId;
    float d = gameItems->m_map->Distance(position, gameItems->m_viewer->GetPosition());
    Mix_Chunk** sound = m_sounds.Find(name);
    if (!sound)
        return -1;
    CSoundObject& soundObject = GetChannel();
    soundObject.m_name = name;
    soundObject.m_sound = *sound; // using a copy of the sound because each channel's overall volume has to be set via the sound
    soundObject.SetVolume(volume);
    soundObject.m_volume = volume;
    soundObject.m_position = position;
    soundObject.m_startTime = gameData->m_gameTime;
    soundObject.m_owner = owner;
    soundObject.Play(loops);
    UpdateVolume(soundObject, d);
    return soundObject.m_id;
}


void CSoundHandler::Stop(int id) {
    for (int i = int (m_busyChannels.Length()); i;) {
        CSoundObject& c = m_busyChannels[--i];
        if (c.m_id == id) {
            c.Stop();
            m_idleChannels.Append(m_busyChannels.Pop(i));
        }
    }
}


void CSoundHandler::StopActorSounds(CActor* actor) {
    for (int i = int (m_busyChannels.Length()); i; ) {
        CSoundObject& c = m_busyChannels[--i];
        if (c.m_owner == actor) {
            c.Stop();
            m_idleChannels.Append(m_busyChannels.Pop(i));
        }
    }
}


// update all sound volumes depending on distance to viewer (viewer or sound source may have been moving)
void CSoundHandler::UpdateSound(CSoundObject& soundObject) {
    // global gameItems
    if (soundObject.m_owner) {
        soundObject.m_position = soundObject.m_owner->GetPosition();
        UpdateVolume(soundObject, gameItems->m_map->Distance(soundObject.m_position, gameItems->m_viewer->GetPosition()));
    }
}

    
// move all channels that are not playing back sound anymore from the busyChannels to the idleChannels list
void CSoundHandler::Cleanup(void) {
    for (int i = int (m_busyChannels.Length()); i;)
        if (!m_busyChannels[--i].Busy())
            m_idleChannels.Append(m_busyChannels.Pop(i));
}


// cleanup expired channels and update sound volumes
void CSoundHandler::Update(void) {
    Cleanup();
    for (auto [i, so] : m_busyChannels)
        UpdateSound(so);
}

CSoundHandler* soundHandler = nullptr;

// =================================================================================================
