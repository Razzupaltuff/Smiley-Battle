import pygame

def InitDisplay ():
    pygame.init ()
    if not pygame.display.get_init ():
        print ("Couldn't initialize display")
        exit ()

def SetupDisplay (width, height):
    try:
        screen = pygame.display.set_mode ((width, height), pygame.DOUBLEBUF | pygame.OPENGL)
    except pygame.error:
        print ("Couldn't set screen mode: {0}".format (pygame.get_error ()))
        exit ()
    print ("GL_CONTEXT_MAJOR_VERSION = {0}".format (pygame.display.gl_get_attribute (pygame.GL_CONTEXT_MAJOR_VERSION)))
    print ("GL_CONTEXT_MINOR_VERSION = {0}".format (pygame.display.gl_get_attribute (pygame.GL_CONTEXT_MINOR_VERSION)))
    return screen

def HandleEvents ():
    for event in pygame.event.get ():
        if (event.type == pygame.QUIT):
            return False
        if (event.type == pygame.KEYDOWN) and \
           (event.key == pygame.K_ESCAPE):
            return False
    return True


InitDisplay ()
screen = SetupDisplay (800, 600)
while HandleEvents ():
    pass