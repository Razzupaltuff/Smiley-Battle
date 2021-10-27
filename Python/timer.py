import pygame
import time

# =================================================================================================
# Timer functions: Measuring time, delaying program execution, etc.

class CTimer:
    def __init__ (self, duration = 0):
        self.time = 0
        self.lapTime = 0
        self.duration = duration
        self.slack = 0


    def Start (self):
        self.time = pygame.time.get_ticks ()
        return self.time


    def Lap (self):
        self.lapTime = pygame.time.get_ticks () - self.time
        return self.lapTime


    def HasPassed (self, time = 0, restart = False):
        self.Lap ()
        if (time == 0):
            time = self.duration
        if (self.time > 0) and (self.lapTime < time):
            return False
        if (restart):
            self.Start ()
        return True


    def Delay (self):
            t = self.duration - self.slack - self.Lap ()
            if (t > 0):
                # pygame.time.wait (t)
                time.sleep (t / 1000.0)
            self.slack = self.Lap () - self.duration

# =================================================================================================
