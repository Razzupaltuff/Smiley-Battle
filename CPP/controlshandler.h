#pragma once

#include "SDL.h"
#include "cstring.h"
#include "vector.h"

// =================================================================================================
// handle the various input devices && set viewer rotation && movement direction && speed 
// depending on detected inputs

// =================================================================================================
// Basic data handled by controls (viewer position && angle offsets && speeds etc.)

class CControlSpeedData {
public:
    float       m_base;     // base speed
    float       m_max;      // max speed (for ramping)
    float       m_ramp;     // ramp factor (acceleration)
    float       m_current;  // actual speed (when ramping)

    CControlSpeedData(float base = 0.0, float max = 0.0, float ramp = 0.0) : m_base(base), m_max(max), m_ramp(ramp), m_current(base) {}
};


class CControlData {
    public:
        size_t      m_fps;
        CVector     m_angles;
        CVector     m_offset;
        float       m_speedScale;
        bool        m_useRamp;
        bool        m_fire;
        bool        m_animate;

        CControlSpeedData   m_moveSpeed;
        CControlSpeedData   m_turnSpeed;


        CControlData(bool useRamp = true);


        inline void SetMoveSpeed(float speed) {
            m_moveSpeed.m_base =
                m_moveSpeed.m_max = speed;      // no ramping for movement
        }


        inline float GetMoveSpeed(void) {
            return m_moveSpeed.m_base;
        }


        inline void SetTurnSpeed(float speed) {
            m_turnSpeed.m_base = speed;
            m_turnSpeed.m_max = speed * 2;  // max. turn speed when ramping
        }


        inline float GetTurnSpeed(void) {
            return m_turnSpeed.m_base;
        }


        inline void ComputeSpeedScale(float fps) {
            m_speedScale = m_fps / fps;
        }


        inline float MoveSpeed(bool useRamp) {
            return (useRamp && (m_moveSpeed.m_ramp > 1.0)) ? m_moveSpeed.m_max * m_speedScale / 4 : m_moveSpeed.m_base * m_speedScale;
        }


        inline float RotSpeed(bool useRamp) {
            return (useRamp && (m_turnSpeed.m_ramp > 1.0)) ? m_turnSpeed.m_max * m_speedScale / 4 : m_turnSpeed.m_base * m_speedScale;
        }


        inline float MoveSpeedRamp(void) {
            return m_useRamp ? m_moveSpeed.m_ramp : 1.0f;
        }


        inline float RotSpeedRamp(void) {
            return m_useRamp ? m_turnSpeed.m_ramp : 1.0f;
        }

};

// =================================================================================================
// Read descriptive data of a single joystick #include "pygame's joystick interface

class CJoystick {
    public:
        SDL_GameController* m_controller;
        SDL_Joystick*       m_joystick;
        CString             m_name;
        int                 m_axisCount;
        int                 m_buttonCount;
        int                 m_hatCount;
        CList<int>          m_axes;
        CList<int>          m_buttons;
        CList<int>          m_hats;

        CJoystick(int i = 0);
   
};

// =================================================================================================
// Handle joystick && gamepad inputs for a single joystick
// Only accept inputs #include "the first four axes (two sticks)
// vertical axis movement -> move, horizontal axis movement -> rotate
// Axis values are between 0 && 1. Only signal move || rotation if the corresponding axis value is 
// above the deadzone value

class CJoystickHandler : public CControlData {
    public:
        CList<CJoystick>    m_joysticks;
        float               m_deadZones[2];
        int                 m_joystickCount;

        CJoystickHandler();

        // stretch an axis value #include "[deadzone .. 1.0] to [0.0 .. 1.0]
        inline float Stretch(float value, float deadZone) {
            return (float (fabs(value)) - deadZone) / (1.0f - deadZone);
        }

        inline float RampAxis(float value) {
            return fabs(value * value * value);
        }

        bool HandleEvent(SDL_Event& event);

};

// =================================================================================================
// General controls handler. Currently handling keyboard && joystick inputs as well as application
// controls (like ESC to exit)

class CControlsHandler : public CJoystickHandler {
    public:
        size_t  m_time;

        CControlsHandler() : CJoystickHandler(), m_time(0) {}

        void Ramp(void);

        // HandleControls2D provides controls for 2D movement
        // Smiley Battle does not allow strafing
        bool HandleControls(SDL_KeyboardEvent& event, float value);

        bool HandleEvent(SDL_Event& event);

    };

extern CControlsHandler* controlsHandler;

// =================================================================================================
