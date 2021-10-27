using System;
using System.Collections.Generic;
using SDL2;

// =================================================================================================
// The sound handler class handles sound creation and sound channel management
// It tries to provide 128 sound channels. They are preinitialized and are kept in m_idleChannels
// (list of available channels) and busyChannels (list of channels currently used for playing back sound)
// When a new sound is to played, a channel is picked from the idleChannels list. If there are no idle
// channels available, the oldest playing channel from busyChannels will be reused. Since channels are 
// append to busyChannels in the temporal sequence they are deployed, the first channel in busyChannels
// will always be the oldest one.

public class SoundHandler
{
    SortedDictionary<string, IntPtr> m_sounds;
    List<SoundObject> m_idleChannels;
    List<SoundObject> m_busyChannels;
    int m_soundLevel;
    float m_masterVolume;
    float m_maxAudibleDistance;
    int m_channelCount;

    public SoundHandler()
    {
        m_soundLevel = Globals.argHandler.IntVal("soundlevel", 0, 1);
        m_masterVolume = Globals.argHandler.FloatVal("masterVolume", 0, 1);
        m_maxAudibleDistance = 30.0f;
        SDL_mixer.Mix_Quit();
        SDL_mixer.Mix_Init(SDL_mixer.MIX_InitFlags.MIX_INIT_MP3 | SDL_mixer.MIX_InitFlags.MIX_INIT_OGG);
        if (0 > SDL_mixer.Mix_OpenAudio(44100, SDL.AUDIO_S16SYS, 2, 4096))
            Console.Error.WriteLine("Couldn't initialize sound system ({0})", SDL.SDL_GetError());
        SDL_mixer.Mix_Volume(-1, SDL_mixer.MIX_MAX_VOLUME);
        SDL_mixer.Mix_AllocateChannels(128);
        m_channelCount = SDL_mixer.Mix_AllocateChannels(-1);
        m_idleChannels = new List<SoundObject>();
        m_busyChannels = new List<SoundObject>();
        for (int i = 0; i < m_channelCount; i++)
            m_idleChannels.Add(new SoundObject(IntPtr.Zero, i, "", i));
        LoadSounds();
    }


    // preload sound data. Sound data is kept in a dictionary. The sound name is the key to it.
    public void LoadSounds()
    {
        string[] soundNames =
        {
        "reappear", "disappear", "collide", "flare", "fusion", "hit", "hum",
        "laser", "phoenix", "pickup", "scrape", "shot", "wallhit", "wallhit-low"
    };
        m_sounds = new SortedDictionary<string, IntPtr>();
        foreach (string name in soundNames)
        {
            string fileName = Globals.gameData.m_soundFolder + name + ".wav";
            IntPtr sound = SDL_mixer.Mix_LoadWAV(fileName);
            if (sound != null)
                m_sounds.Add(name, sound);
            else
                Console.Error.WriteLine("Couldn't load sound '{0}' ({1})", name, SDL.SDL_GetError());
        }
    }


    // compute stereo panning from the angle between the viewer direction and the vector from the viewer to the sound source
    float Pan(Vector position)
    {
        // global Globals.gameItems
        Vector v = position - Globals.gameItems.m_viewer.GetPosition();
        v.Normalize();
        v = Globals.gameItems.m_viewer.m_camera.m_orientation.Rotate(v);
        return v.Dot(new Vector(1, 0, 0));       // > 0: right side of viewer, < 0: left side of viewer
    }


    void UpdateVolume(SoundObject soundObject, float d)
    {
        float volume, pan;
        if (d > m_maxAudibleDistance)
            volume = pan = 0.0f;
        else
        {
            volume = (m_maxAudibleDistance - d) / m_maxAudibleDistance;
            volume *= volume * soundObject.m_volume * m_masterVolume;
            pan = Pan(soundObject.m_position) / 2;   // use half of the volume for stereo panning
        }
        soundObject.SetPanning(Math.Abs(-0.5f + pan) * volume, (0.5f + pan) * volume);
    }


    void MoveChannel(List<SoundObject> source, List<SoundObject> dest, int i)
    {
        dest.Add(source[i]);
        source.RemoveAt(i);
    }

    void UseChannel(int i)
    {
        MoveChannel(m_idleChannels, m_busyChannels, i);

    }

    void ReleaseChannel(int i)
    {
        MoveChannel(m_busyChannels, m_idleChannels, i);

    }

    // get a channel for playing back a new sound
    // if all channels are busy, pick the oldest busy one
    SoundObject GetChannel()
    {
        if (m_idleChannels.Count > 0)
        {
            UseChannel(m_idleChannels.Count - 1);
        }
        else
        {
            m_busyChannels[0].Stop();
            // move from list start to list end
            MoveChannel(m_busyChannels, m_busyChannels, 0);
        }
        return m_busyChannels[m_busyChannels.Count - 1];
    }


    int FindActorSound(Actor actor, string name)
    {
        foreach (SoundObject c in m_busyChannels)
            if ((c.m_owner == actor) && (c.m_name == name))
            {
                return c.m_id;
            }
        return -1;
    }


    // play back the sound with the name 'name'. Position, viewer and DistFunc serve for computing the sound volume
    // depending on the distance of the viewer to the sound position
    public int Play(string name, Vector position = null, float volume = 1.0f, int loops = 0, Actor owner = null, int level = 1)
    {
        //return -1;
        if ((m_soundLevel == 0) || (level > m_soundLevel))
            return -1;
        if (position == null)
            return -1;

        int soundId = FindActorSound(owner, name);
        if (soundId >= 0)
            return soundId;
        float d = Globals.gameItems.m_map.Distance(position, Globals.gameItems.m_viewer.GetPosition());
        if (!m_sounds.ContainsKey(name))
            return -1;
        IntPtr sound = m_sounds[name];
        SoundObject soundObject = GetChannel();
        soundObject.m_name = name;
        soundObject.m_sound = sound; // using a copy of the sound because each channel's overall volume has to be set via the sound
        soundObject.SetVolume(volume);
        soundObject.m_volume = volume;
        soundObject.m_position = position;
        soundObject.m_startTime = Globals.gameData.m_gameTime;
        soundObject.m_owner = owner;
        soundObject.Play(loops);
        UpdateVolume(soundObject, d);
        return soundObject.m_id;
    }


    public void Stop(int id)
    {
        for (int i = m_busyChannels.Count; i > 0;)
        {
            SoundObject c = m_busyChannels[--i];
            if (c.m_id == id)
            {
                c.Stop();
                ReleaseChannel(i);
            }
        }
    }


    public void StopActorSounds(Actor actor)
    {
        for (int i = m_busyChannels.Count; i > 0;)
        {
            SoundObject c = m_busyChannels[--i];
            if (c.m_owner == actor)
            {
                c.Stop();
                ReleaseChannel(i);
            }
        }
    }


    // update all sound volumes depending on distance to viewer (viewer or sound source may have been moving)
    public void UpdateSound(SoundObject soundObject)
    {
        if (soundObject.m_owner != null)
        {
            soundObject.m_position = soundObject.m_owner.GetPosition();
            UpdateVolume(soundObject, Globals.gameItems.m_map.Distance(soundObject.m_position, Globals.gameItems.m_viewer.GetPosition()));
        }
    }


    // move all channels that are not playing back sound anymore from the busyChannels to the idleChannels list
    public void Cleanup()
    {
        for (int i = m_busyChannels.Count; i > 0;)
            if (!m_busyChannels[--i].Busy())
            {
                ReleaseChannel(i);
            }
    }


    // cleanup expired channels and update sound volumes
    public void Update()
    {
        Cleanup();
        foreach (SoundObject c in m_busyChannels)
            UpdateSound(c);
    }

}

// =================================================================================================
