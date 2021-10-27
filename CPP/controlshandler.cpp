#pragma once

#include "controlsHandler.h"
#include "argHandler.h"

// =================================================================================================
// handle the various input devices && set viewer rotation && movement direction && speed 
// depending on detected inputs

// =================================================================================================
// Basic data handled by controls (viewer position && angle offsets && speeds etc.)

CControlData::CControlData(bool useRamp) {
    // the controls handler will run at 100 fps to provide for enough granularity to allow the physics handler (which process controls among others) run at 60 fps
    // controls handler fps should therefore be greater than physics handler fps
    m_fps = 100;
    m_moveSpeed = CControlSpeedData(0.05f, 0.05f, 1.0f);
    m_turnSpeed = CControlSpeedData(1.0f, 2.0f, 1.033f);
    m_moveSpeed.m_base = argHandler->FloatVal("movespeed", 0, 1.0f) / 20.0f;
    m_moveSpeed.m_max = m_moveSpeed.m_base;
    m_turnSpeed.m_base = argHandler->FloatVal("turnspeed", 0, 1.0f);
    m_turnSpeed.m_max = m_turnSpeed.m_base * 2;
    m_speedScale = 1.0f;
    m_useRamp = argHandler->BoolVal("rampcontrols", 0, useRamp);
    m_fire = false;
    m_animate = true;
}



// =================================================================================================
// Read descriptive data of a single joystick #include "pygame's joystick interface

CJoystick::CJoystick(int i) {
    m_joystick = SDL_JoystickOpen (i);
    m_name = CString(SDL_JoystickNameForIndex(i));
    m_axisCount = SDL_JoystickNumAxes(m_joystick);
    m_buttonCount = SDL_JoystickNumButtons(m_joystick);
    m_hatCount = SDL_JoystickNumHats(m_joystick);
}
   
// =================================================================================================
// Handle joystick && gamepad inputs for a single joystick
// Only accept inputs #include "the first four axes (two sticks)
// vertical axis movement -> move, horizontal axis movement -> rotate
// Axis values are between 0 && 1. Only signal move || rotation if the corresponding axis value is 
// above the deadzone value

CJoystickHandler::CJoystickHandler() : CControlData() {
    SDL_Init(SDL_INIT_JOYSTICK);
    m_deadZones[0] = 0.1f;
    m_deadZones[1] = 0.2f;
    m_joystickCount = SDL_NumJoysticks();
    for (int i = 0; i < m_joystickCount; i++)
        m_joysticks.Append(CJoystick(i));
}


bool CJoystickHandler::HandleEvent(SDL_Event& event) {
    if (event.type == SDL_JOYBUTTONDOWN)
        m_fire = true;
    else if (event.type == SDL_JOYBUTTONUP)
        m_fire = true;
    else if (event.type == SDL_JOYAXISMOTION) {
        // print("Axis " + str (event.axis) + " value: " + str (event.value))
        SDL_JoyAxisEvent& jae = event.jaxis;
        if (event.jaxis.axis > 5)
            return false;
        if (event.jaxis.axis > 3) {
            m_fire = true;
            return false;
        }
        float axisValue = float(event.jaxis.value) / 32767.0f;
        // print ("{0:1.2f}".format (event.value))
        if (event.jaxis.axis == 1) {
            if (axisValue < -m_deadZones[0])
                m_offset.Z() = -MoveSpeed(m_useRamp); // m_RampAxis (m_Stretch (event.value, m_deadZones [0])) * m_moveSpeed.m_max * m_speedScale
            else if (axisValue > m_deadZones[0])
                m_offset.Z() = MoveSpeed(m_useRamp); // RampAxis (m_Stretch (event.value, m_deadZones [0])) * m_moveSpeed.m_max * m_speedScale
            else
                m_offset.Z() = 0.0f;
        }
        else if (event.jaxis.axis == 2) {
            if (axisValue < -m_deadZones[1])
                m_angles.Y() = RotSpeed(m_useRamp); // RampAxis (m_Stretch (event.value, m_deadZones [1])) * m_turnSpeed.m_max * m_speedScale
            else if (axisValue > m_deadZones[1])
                m_angles.Y() = -RotSpeed(m_useRamp); // m_RampAxis (m_Stretch (event.value, m_deadZones [1])) * m_turnSpeed.m_max * m_speedScale
            else
                m_angles.Y() = 0.0f;
        }
    }
    else
        return false;
    return true;
}

// =================================================================================================
// General controls handler. Currently handling keyboard && joystick inputs as well as application
// controls (like ESC to exit)

void CControlsHandler::Ramp(void) {
    if (m_useRamp) {
        if ((-m_turnSpeed.m_max * m_speedScale < m_angles.Y()) && (m_angles.Y() < m_turnSpeed.m_max * m_speedScale))
            m_angles *= m_turnSpeed.m_ramp;
        // if (m_angles.Y() != 0.0f)
        //    fprintf (stderr, "turn speed: %1.2f\n", m_angles.Y ());
        if ((-m_moveSpeed.m_max * m_speedScale < m_offset.Z()) && (m_offset.Z() < m_moveSpeed.m_max * m_speedScale))
            m_offset *= m_moveSpeed.m_ramp;
        //if (m_offset.Z() != 0.0f)
        //    fprintf (stderr, "move speed: %1.2f\n", m_offset.Z ());
    }
}


        // HandleControls2D provides controls for 2D movement
        // Smiley Battle does not allow strafing
bool CControlsHandler::HandleControls (SDL_KeyboardEvent& event, float value) {
    switch (event.keysym.sym) {

        case SDLK_SPACE:
            if (value != 0)
                m_fire = true;
            break;

        case SDLK_KP_8:
        case SDLK_w:
        case SDLK_UP:
            if ((m_offset.Z() >= 0) || (value == 0))
                m_offset.Z() = value * -MoveSpeed(m_useRamp);
            break;

        case SDLK_KP_5:
        case SDLK_s:
        case SDLK_DOWN:
            if ((m_offset.Z() <= 0) || (value == 0))
                m_offset.Z() = value * MoveSpeed(m_useRamp);
            break;

        case SDLK_KP_4:
        case SDLK_a:
        case SDLK_LEFT:
            if ((m_angles.Y() <= 0) || (value == 0)) 
                m_angles.Y() = value * RotSpeed(m_useRamp);
    break;

        case SDLK_KP_6:
        case SDLK_d:
        case SDLK_RIGHT:
            if ((m_angles.Y () >= 0) || (value == 0)) 
                m_angles.Y () = value * -RotSpeed (m_useRamp);
            break;

        default:
            return false;
    }
    return true;
}


bool CControlsHandler::HandleEvent(SDL_Event& event) {
    if (CJoystickHandler::HandleEvent(event))
        return true;
    else if (event.type == SDL_KEYUP)
        return HandleControls(event.key, 0);
    else if (event.type == SDL_KEYDOWN)
        return HandleControls(event.key, 1);
    else 
        return false;
}

CControlsHandler* controlsHandler = nullptr;

// =================================================================================================
