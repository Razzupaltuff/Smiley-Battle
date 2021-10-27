using System;
using SDL2;

// =================================================================================================

public class SoundObject
{
    public int m_id;
    public string m_name;
    public IntPtr m_sound;
    public int m_channel;
    public float m_volume;
    public Vector m_position;
    public Actor m_owner;
    public int m_startTime;

    public SoundObject(IntPtr sound, int id = -1, string name = "", int channel = -1, Vector position = null, float volume = 1.0f)
    {
        m_id = id;
        m_name = name;
        m_channel = channel;
        m_sound = sound;
        m_position = position;
        m_owner = null;
        m_volume = volume;
        m_startTime = 0;
    }

    ~SoundObject()
    {
        if (Busy())
            Stop();
    }

    public void Play(int loops = 1)
    {
        if (0 > SDL_mixer.Mix_PlayChannel(m_channel, m_sound, loops))
        Console.Error.WriteLine("Couldn't play sound '{0}' ({1})", m_name, SDL.SDL_GetError());
    }

    public void Stop()
    {
        SDL_mixer.Mix_HaltChannel(m_channel);
    }

    public void SetPanning(float left, float right)
    {
        SDL_mixer.Mix_SetPanning(m_channel, (byte) (left * 255), (byte) (right * 255));
    }

    public void SetVolume(float volume)
    {
        SDL_mixer.Mix_Volume(m_channel, (int) (SDL_mixer.MIX_MAX_VOLUME * volume));
    }

    public bool Busy()
    {
        return SDL_mixer.Mix_Playing(m_channel) != 0;
    }

}

// =================================================================================================
