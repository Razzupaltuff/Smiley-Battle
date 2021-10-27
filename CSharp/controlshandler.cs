using System;
using System.Collections.Generic;
using SDL2;

// =================================================================================================
// handle the various input devices && set viewer rotation && movement direction && speed 
// depending on detected inputs

// =================================================================================================
// Basic data handled by controls (viewer position && angle offsets && speeds etc.)

public class ControlSpeedData
{
    public float m_base;     // base speed
    public float m_max;      // max speed (for ramping)
    public float m_ramp;     // ramp factor (acceleration)
    public float m_current;  // actual speed (when ramping)

    public ControlSpeedData(float baseSpeed = 0.0f, float maxSpeed = 0.0f, float rampFactor = 0.0f)
    {
        m_base = baseSpeed;
        m_max = maxSpeed;
        m_ramp = rampFactor;
        m_current = baseSpeed;
    }
}

// =================================================================================================

public class ControlData
{
    public uint m_fps;
    public Vector m_angles;
    public Vector m_offset;
    public float m_speedScale;
    public bool m_useRamp;
    public bool m_fire;
    public bool m_animate;
    public ControlSpeedData m_moveSpeed;
    public ControlSpeedData m_turnSpeed;


    public ControlData(bool useRamp = true)
    {
        // the controls handler will run at 100 fps to provide for enough granularity to allow the physics handler (which process controls among others) run at 60 fps
        // controls handler fps should therefore be greater than physics handler fps
        m_angles = new Vector();
        m_offset = new Vector();
        m_fps = 100;
        m_moveSpeed = new ControlSpeedData(0.05f, 0.05f, 1.0f);
        m_turnSpeed = new ControlSpeedData(1.0f, 2.0f, 1.033f);
        m_moveSpeed.m_base = Globals.argHandler.FloatVal("movespeed", 0, 1.0f) / 20.0f;
        m_moveSpeed.m_max = m_moveSpeed.m_base;
        m_turnSpeed.m_base = Globals.argHandler.FloatVal("turnspeed", 0, 1.0f);
        m_turnSpeed.m_max = m_turnSpeed.m_base * 2;
        m_speedScale = 1.0f;
        m_useRamp = Globals.argHandler.BoolVal("rampcontrols", 0, useRamp);
        m_fire = false;
        m_animate = true;
    }

    public void SetMoveSpeed(float speed)
    {
        m_moveSpeed.m_base =
        m_moveSpeed.m_max = speed;      // no ramping for movement
    }


    public float GetMoveSpeed()
    {
        return m_moveSpeed.m_base;
    }


    public void SetTurnSpeed(float speed)
    {
        m_turnSpeed.m_base = speed;
        m_turnSpeed.m_max = speed * 2;  // max. turn speed when ramping
    }


    public float GetTurnSpeed()
    {
        return m_turnSpeed.m_base;
    }


    public void ComputeSpeedScale(float fps)
    {
        m_speedScale = (float) m_fps / fps;
    }


    public float MoveSpeed(bool useRamp)
    {
        return (useRamp && (m_moveSpeed.m_ramp > 1.0)) ? m_moveSpeed.m_max * m_speedScale / 4 : m_moveSpeed.m_base * m_speedScale;
    }


    public float RotSpeed(bool useRamp)
    {
        return (useRamp && (m_turnSpeed.m_ramp > 1.0)) ? m_turnSpeed.m_max * m_speedScale / 4 : m_turnSpeed.m_base * m_speedScale;
    }


    public float MoveSpeedRamp()
    {
        return m_useRamp ? m_moveSpeed.m_ramp : 1.0f;
    }


    public float RotSpeedRamp()
    {
        return m_useRamp ? m_turnSpeed.m_ramp : 1.0f;
    }

}

// =================================================================================================
// Read descriptive data of a single joystick #include "pygame's joystick interface

public class Joystick
{
    public IntPtr m_joystick;
    public string m_name;
    public int m_axisCount;
    public int m_buttonCount;
    public int m_hatCount;

    public Joystick(int i = 0)
    {
        m_joystick = SDL.SDL_JoystickOpen(i);
        m_name = SDL.SDL_JoystickNameForIndex(i);
        m_axisCount = SDL.SDL_JoystickNumAxes(m_joystick);
        m_buttonCount = SDL.SDL_JoystickNumButtons(m_joystick);
        m_hatCount = SDL.SDL_JoystickNumHats(m_joystick);
    }
}

// =================================================================================================
// Handle joystick && gamepad inputs for a single joystick
// Only accept inputs #include "the first four axes (two sticks)
// vertical axis movement . move, horizontal axis movement . rotate
// Axis values are between 0 && 1. Only signal move || rotation if the corresponding axis value is 
// above the deadzone value

public class JoystickHandler : ControlData
{

    public List<Joystick> m_joysticks;
    public float[] m_deadZones;
    public int m_joystickCount;

    public JoystickHandler() : base()
    {
        SDL.SDL_Init(SDL.SDL_INIT_JOYSTICK);
        m_deadZones = new float[2] { 0.1f, 0.2f };
        m_joysticks = new List<Joystick>();
        m_joystickCount = SDL.SDL_NumJoysticks();
        for (int i = 0; i < m_joystickCount; i++)
            m_joysticks.Add(new Joystick(i));
    }


public bool HandleEvent(SDL.SDL_Event sdlEvent)
{
    if (sdlEvent.type == SDL.SDL_EventType.SDL_JOYBUTTONDOWN)
        m_fire = true;
    else if (sdlEvent.type == SDL.SDL_EventType.SDL_JOYBUTTONUP)
        m_fire = true;
    else if (sdlEvent.type == SDL.SDL_EventType.SDL_JOYAXISMOTION) 
    {
        // print("Axis " + str (sdlEvent.axis) + " value: " + str (sdlEvent.value))
        SDL.SDL_JoyAxisEvent jae = sdlEvent.jaxis;
        if (sdlEvent.jaxis.axis > 5)
                return false;
        if (sdlEvent.jaxis.axis > 3) 
        {
            m_fire = true;
            return false;
        }
        float axisValue = (float) sdlEvent.jaxis.axisValue / 32767.0f;
        if (sdlEvent.jaxis.axis == 1) 
        {
            if (axisValue < -m_deadZones[0])
                m_offset.Z = -MoveSpeed(m_useRamp); // m_RampAxis (m_Stretch (sdlEvent.value, m_deadZones [0])) * m_moveSpeed.m_max * m_speedScale
            else if (axisValue > m_deadZones[0])
                m_offset.Z = MoveSpeed(m_useRamp); // RampAxis (m_Stretch (sdlEvent.value, m_deadZones [0])) * m_moveSpeed.m_max * m_speedScale
            else
                m_offset.Z = 0.0f;
        }
        else if (sdlEvent.jaxis.axis == 2) 
        {
            if (axisValue < -m_deadZones[1])
                m_angles.Y = RotSpeed(m_useRamp); // RampAxis (m_Stretch (sdlEvent.value, m_deadZones [1])) * m_turnSpeed.m_max * m_speedScale
            else if (axisValue > m_deadZones[1])
                m_angles.Y = -RotSpeed(m_useRamp); // m_RampAxis (m_Stretch (sdlEvent.value, m_deadZones [1])) * m_turnSpeed.m_max * m_speedScale
            else
                m_angles.Y = 0.0f;
        }
    }
    else
        return false;
    return true;
}


// stretch an axis value #include "[deadzone .. 1.0] to [0.0 .. 1.0]
public float Stretch(float value, float deadZone)
{
    return (Math.Abs(value) - deadZone) / (1.0f - deadZone);
}

public float RampAxis(float value)
{
    return Math.Abs(value * value * value);
}

}

// =================================================================================================
// General controls handler. Currently handling keyboard && joystick inputs as well as application
// controls (like ESC to exit)

public class ControlsHandler : JoystickHandler
{
    public uint m_time;

    public ControlsHandler() : base()
    {
        m_time = 0;
    }

    public void Ramp()
    {
        if (m_useRamp)
        {
            if ((-m_turnSpeed.m_max * m_speedScale < m_angles.Y) && (m_angles.Y < m_turnSpeed.m_max * m_speedScale))
                m_angles *= m_turnSpeed.m_ramp;
            // if (m_angles.Y != 0.0f)
            //    fprintf (stderr, "turn speed: %1.2f\n", m_angles.Y ());
            if ((-m_moveSpeed.m_max * m_speedScale < m_offset.Z) && (m_offset.Z < m_moveSpeed.m_max * m_speedScale))
                m_offset *= m_moveSpeed.m_ramp;
             //if (m_offset.Z != 0.0f)
             //   Console.Error.WriteLine ("move speed: {0:#.00}", m_offset.Z);
        }
    }


    // HandleControls2D provides controls for 2D movement
    // Smiley Battle does not allow strafing
    public bool HandleControls(SDL.SDL_KeyboardEvent sdlEvent, float value)
    {
        switch (sdlEvent.keysym.sym)
        {

            case SDL.SDL_Keycode.SDLK_SPACE:
                if (value != 0)
                    m_fire = true;
                break;

            case SDL.SDL_Keycode.SDLK_KP_8:
            case SDL.SDL_Keycode.SDLK_w:
            case SDL.SDL_Keycode.SDLK_UP:
                if ((m_offset.Z >= 0) || (value == 0))
                    m_offset.Z = value * -MoveSpeed(m_useRamp);
                break;

            case SDL.SDL_Keycode.SDLK_KP_5:
            case SDL.SDL_Keycode.SDLK_s:
            case SDL.SDL_Keycode.SDLK_DOWN:
                if ((m_offset.Z <= 0) || (value == 0))
                    m_offset.Z = value * MoveSpeed(m_useRamp);
                break;

            case SDL.SDL_Keycode.SDLK_KP_4:
            case SDL.SDL_Keycode.SDLK_a:
            case SDL.SDL_Keycode.SDLK_LEFT:
                if ((m_angles.Y <= 0) || (value == 0))
                    m_angles.Y = value * RotSpeed(m_useRamp);
                break;

            case SDL.SDL_Keycode.SDLK_KP_6:
            case SDL.SDL_Keycode.SDLK_d:
            case SDL.SDL_Keycode.SDLK_RIGHT:
                if ((m_angles.Y >= 0) || (value == 0))
                    m_angles.Y = value * -RotSpeed(m_useRamp);
                break;

            default:
                return false;
        }
        return true;
    }


    new public bool HandleEvent(SDL.SDL_Event sdlEvent)
    {
        if (base.HandleEvent(sdlEvent))
            return true;
        else if (sdlEvent.type == SDL.SDL_EventType.SDL_KEYUP)
            return HandleControls(sdlEvent.key, 0);
        else if (sdlEvent.type == SDL.SDL_EventType.SDL_KEYDOWN)
            return HandleControls(sdlEvent.key, 1);
        else
            return false;
    }

}

// =================================================================================================
