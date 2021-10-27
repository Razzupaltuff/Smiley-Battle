# =================================================================================================
# global variables
# to have changes to these made in one module be visible in other modules, use "import globals"
# (rather than "from globals import *") and qualify the global variables below with "global."
# (e.g. "global.gameData")
# "from globals import *" will import everything in globals in the local namespace of the module
# globals is imported to, resulting in other modules also importing globals not sharing changes
# made to the global variables in the current module.

gameData = None
gameItems = None
actorHandler = None
physicsHandler = None
soundHandler = None
controlsHandler = None
networkHandler = None
shaderHandler = None
textureHandler = None
mapHandler = None
argHandler = None
effectHandler = None
scoreBoard = None

# =================================================================================================
