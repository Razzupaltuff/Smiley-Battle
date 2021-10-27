import pygame
import globals

from vector import *
from renderer import *
from timer import *
from arghandler import *

# =================================================================================================
# handle the various input devices and set viewer rotation and movement direction and speed 
# depending on detected inputs

# =================================================================================================
# Basic data handled by controls (viewer position and angle offsets and speeds etc.)

class CControlSpeedData:
    def __init__ (self, base, max, ramp):
        self.base = base            # base speed
        self.max = max              # max speed (for ramping)
        self.ramp = ramp            # ramp factor (acceleration)
        self.current = self.base    # actual speed (when ramping)


class CControlData:
    def __init__ (self, useRamp : bool = True) -> None:
        # the controls handler will run at 100 fps to provide for enough granularity to allow the physics handler (which process controls among others) run at 60 fps
        # controls handler fps should therefore be greater than physics handler fps
        self.fps = 100      
        self.angles = CVector (0,0,0)
        self.offset = CVector (0,0,0)
        self.moveSpeed = CControlSpeedData (0.05, 0.05, 1.0) # 1.05)
        self.turnSpeed = CControlSpeedData (1.0, 2.0, 1.033)
        self.moveSpeed.base = globals.argHandler.FloatVal ("movespeed", 0, 1.0) / 20.0
        self.moveSpeed.max = self.moveSpeed.base
        self.turnSpeed.base = globals.argHandler.FloatVal ("turnspeed", 0, 1.0)
        self.turnSpeed.max = self.turnSpeed.base * 2
        self.sceneSpeed = CControlSpeedData (1.0, 100.0, 1.33)
        self.speedScale = 1.0
        self.useRamp = globals.argHandler.BoolVal ("rampcontrols", 0, useRamp)
        self.fire = False
        self.animate = True


    def SetMoveSpeed (self, speed : float) -> None:
        self.moveSpeed.base = speed
        self.moveSpeed.max = speed      # no ramping for movement


    def GetMoveSpeed (self) -> float:
        return self.moveSpeed.base


    def SetTurnSpeed (self, speed : float) -> None:
        self.turnSpeed.base = speed
        self.turnSpeed.max = speed * 2  # max. turn speed when ramping


    def GetTurnSpeed (self) -> float:
        return self.turnSpeed.base
        

    def ComputeSpeedScale (self, fps : float):
        self.speedScale = self.fps / fps


    def MoveSpeed (self, useRamp : bool) -> float:
        if (useRamp and (self.moveSpeed.ramp > 1.0)):
            return self.moveSpeed.max * self.speedScale / 4
        return self.moveSpeed.base * self.speedScale


    def RotSpeed (self, useRamp : bool) -> float:
        if (useRamp and (self.turnSpeed.ramp > 1.0)):
            return self.turnSpeed.max *self.speedScale / 4
        return self.turnSpeed.base * self.speedScale


    def MoveSpeedRamp (self) -> float:
        return self.moveSpeed.ramp if self.useRamp else 1.0


    def RotSpeedRamp (self) -> float:
        return self.turnSpeed.ramp if self.useRamp else 1.0

# =================================================================================================
# Read descriptive data of a single joystick from pygame's joystick interface

class CJoystick:
    def __init__ (self, i : int) -> None:
        self.joystick = pygame.joystick.Joystick (i)
        self.joystick.init ()
        self.axes = []
        self.buttons = []
        self.hats = []
        # Get the name from the OS for the controller/joystick
        self.name = self.joystick.get_name ()
        # Usually axis run in pairs, up/down for one, and left/right for the other.
        self.axisCount = self.joystick.get_numaxes ()
        for i in range (self.axisCount):
            self.axes.append ((self.joystick.get_axis (i) + 1.0) / 2.0)
        self.buttonCount = self.joystick.get_numbuttons()
        for i in range (self.buttonCount):
            self.buttons.append (self.joystick.get_button (i))
        # Hat switch. All or nothing for direction, not like joysticks.
        # Value comes back in an array.
        self.hatCount = self.joystick.get_numhats ()
        for i in range (self.hatCount):
            self.hats.append (self.joystick.get_hat (i))

# =================================================================================================
# Handle joystick and gamepad inputs for a single joystick
# Only accept inputs from the first four axes (two sticks)
# vertical axis movement -> move, horizontal axis movement -> rotate
# Axis values are between 0 and 1. Only signal move or rotation if the corresponding axis value is 
# above the deadzone value

class CJoystickHandler (CControlData):
    def __init__ (self) -> None:
        super ().__init__ ()
        self.joysticks = []
        pygame.joystick.init ()
        self.deadZones = (0.1, 0.2)
        self.joystickCount = pygame.joystick.get_count ()
        for i in range (self.joystickCount):
            self.joysticks.append (CJoystick (i))

    
    # stretch an axis value from [deadzone .. 1.0] to [0.0 .. 1.0]
    def Stretch (self, value : float, deadZone : float) -> float:
        return (np.abs (value) - deadZone) / (1.0 - deadZone)


    def RampAxis (self, value : float) -> float:
        return np.abs (value * value * value)


    def HandleEvent (self, event) -> bool:
        if event.type == pygame.JOYBUTTONDOWN:
            self.fire = True
        elif event.type == pygame.JOYBUTTONUP:
            self.fire = True
        elif event.type == pygame.JOYAXISMOTION:
            # print("Axis " + str (event.axis) + " value: " + str (event.value))
            if (event.axis > 5):
                return False
            if (event.axis > 3):
                self.fire = True
                return False
            if (event.value > 1.0):
                event.value = 1.0
            elif (event.value < -1.0):
                event.value = -1.0
            # print ("{0:1.2f}".format (event.value))
            if (event.axis == 1):
                print ("^v {0:1.2f}".format (event.value))
                if (event.value < -self.deadZones [0]):
                    self.offset.z = -self.MoveSpeed (self.useRamp) # self.RampAxis (self.Stretch (event.value, self.deadZones [0])) * self.moveSpeed.max * self.speedScale
                elif (event.value > self.deadZones [0]):
                    self.offset.z = self.MoveSpeed (self.useRamp) # RampAxis (self.Stretch (event.value, self.deadZones [0])) * self.moveSpeed.max * self.speedScalepython
                else:
                    self.offset.z = 0
            elif (event.axis == 2):
                # print ("<> {0:1.2f}".format (event.value))
                if (event.value < -self.deadZones [1]):
                    self.angles.y = self.RotSpeed (self.useRamp) # RampAxis (self.Stretch (event.value, self.deadZones [1])) * self.turnSpeed.max * self.speedScale
                elif (event.value > self.deadZones [1]):
                    self.angles.y = -self.RotSpeed (self.useRamp) # self.RampAxis (self.Stretch (event.value, self.deadZones [1])) * self.turnSpeed.max * self.speedScale
                else:
                    self.angles.y = 0
        else:
            return False
        return True


# =================================================================================================
# General controls handler. Currently handling keyboard and joystick inputs as well as application
# controls (like ESC to exit)

class CControlsHandler (CJoystickHandler):
    def __init__ (self) -> None:
        super ().__init__ ()
        self.time = 0


    def Ramp (self) -> None:
        if (self.useRamp):
            if (-self.turnSpeed.max * self.speedScale < self.angles.y) and (self.angles.y < self.turnSpeed.max * self.speedScale):
                self.angles.Scale (self.turnSpeed.ramp)
            # if self.angles.y != 0.0:
            #     print ("turn speed: " + str (self.angles.y))
            if (-self.moveSpeed.max * self.speedScale < self.offset.z) and (self.offset.z < self.moveSpeed.max * self.speedScale):
                self.offset.Scale (self.moveSpeed.ramp)


    # HandleControls2D provides controls for 2D movement
    # Smiley Battle does not allow strafing
    def HandleControls2D (self, event, value : int) -> bool:
        if (event.key == pygame.K_SPACE):
            if (value == 0):
                self.fire = True
        elif (event.key == pygame.K_KP8) or \
             (event.key == pygame.K_w) or \
             (event.key == pygame.K_UP):
            if (self.offset.z >= 0) or (value == 0):
                self.offset.z = value * -self.MoveSpeed (self.useRamp)
        elif (event.key == pygame.K_KP5) or \
             (event.key == pygame.K_s) or \
             (event.key == pygame.K_DOWN):
            if (self.offset.z <= 0) or (value == 0):
                self.offset.z = value * self.MoveSpeed (self.useRamp)
        elif (event.key == pygame.K_KP4) or \
             (event.key == pygame.K_a) or \
             (event.key == pygame.K_LEFT):
            if (self.angles.y <= 0) or (value == 0):
                self.angles.y = value * self.RotSpeed (self.useRamp)
        elif (event.key == pygame.K_KP6) or \
             (event.key == pygame.K_d) or \
             (event.key == pygame.K_RIGHT):
            if (self.angles.y >= 0) or (value == 0):
                self.angles.y = value * -self.RotSpeed (self.useRamp)
        else:
            return False
        return True


    # this control set is meant to control free flight in a fully 3D environment
    # it allows for movement along all three spatial axes including strafing
    def HandleControls3D (self, event, value : float) -> bool:
        if (event.key == pygame.K_SPACE):
            if (value == 0):
                self.animate = not self.animate
        elif (event.key == pygame.K_KP_PLUS):
            if (value == 0):
                self.sceneSpeed.current *= self.sceneSpeed.ramp
        elif (event.key == pygame.K_KP_MINUS):
            if (value == 0):
                self.sceneSpeed.current /= self.sceneSpeed.ramp
        elif (event.key == pygame.K_KP7):
            if (self.offset.y >= 0) or (value == 0):
                self.offset.y = value * self.MoveSpeed (self.useRamp)
        elif (event.key == pygame.K_KP9):
            if (self.offset.y >= 0) or (value == 0):
                self.offset.y = value * -self.MoveSpeed (self.useRamp)
        elif (event.key == pygame.K_KP8):
            if (self.offset.z >= 0) or (value == 0):
                self.offset.z = value * -self.MoveSpeed (self.useRamp) * 2
        elif (event.key == pygame.K_KP5):
            if (self.offset.z <= 0) or (value == 0):
                self.offset.z = value * self.MoveSpeed (self.useRamp) * 2
        elif (event.key == pygame.K_KP6):
            if (self.offset.x <= 0) or (value == 0):
                self.offset.x = value * self.MoveSpeed (self.useRamp)
        elif (event.key == pygame.K_KP4):
            if (self.offset.x >= 0) or (value == 0):
                self.offset.x = value * -self.MoveSpeed (self.useRamp)
        elif (event.key == pygame.K_UP):
            if (self.angles.x <= 0) or (value == 0):
                self.angles.x = value * 2 * self.RotSpeed (self.useRamp)
        elif (event.key == pygame.K_DOWN):
            if (self.angles.x >= 0) or (value == 0):
                self.angles.x = value * -2 * self.RotSpeed (self.useRamp)
        elif (event.key == pygame.K_LEFT):
            if (self.angles.y <= 0) or (value == 0):
                self.angles.y = value * 2 * self.RotSpeed (self.useRamp)
        elif (event.key == pygame.K_RIGHT):
            if (self.angles.y >= 0) or (value == 0):
                self.angles.y = value * -2 * self.RotSpeed (self.useRamp)
        elif (event.key == pygame.K_KP1):
            if (self.angles.z <= 0) or (value == 0):
                self.angles.z = value * 2 * self.RotSpeed (self.useRamp)
        elif (event.key == pygame.K_KP3):
            if (self.angles.z >= 0) or (value == 0):
                self.angles.z = value * -2 * self.RotSpeed (self.useRamp)
        else:
            return False
        return True


    def HandleControls (self, event, value : float, mode : int) -> bool:
        if (mode == 0):
            return self.HandleControls2D (event, value)
        else:
            return self.HandleControls3D (event, value)


    def HandleEvent (self, event, mode : int) -> bool:
        if super ().HandleEvent (event):
            return True
        elif (event.type == pygame.KEYUP):
            return self.HandleControls (event, 0, mode)
        elif (event.type == pygame.KEYDOWN):
            return self.HandleControls (event, 1, mode)
        return False

# =================================================================================================
