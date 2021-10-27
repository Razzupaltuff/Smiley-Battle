#pragma once

#include <chrono>

#include "glew.h"
#include "smileybattle.h"
#include "gamedata.h"
#include "gameitems.h"
#include "actorhandler.h"
#include "controlshandler.h"
#include "physicshandler.h"
#include "soundhandler.h"
#include "networkhandler.h"
#include "shaders.h"
#include "texturehandler.h"
#include "arghandler.h"
#include "effecthandler.h"
#include "scoreboard.h"
#include "renderer.h"

// =================================================================================================
// Smiley Battle is a remake of Midimaze, which was probably the first first person multiplayer shooter 
// with fluid movement. It ran on a ring network of up two 16 Atari STs which were connected via their 
// midi out and midi in ports. Players roam a maze like map with their smiley avatars and throw
// color bags at other players. Receiving three hits within a sufficiently short time puts a player in
// "limbo" for a short time before he respawns at a random position. Scoring a hit yields one point,
// sending a player to limbo yields another point. Players start with three hitpoints. When hit,
// hitpoints regenerate after a short time unless the player is hit again.
//
// I wrote Smiley Battle as a small Python exercise to explore the language and learn its strengths
// and weaknesses. It uses very limited OpenGL features (the most advanced are VAOs, VBOs and two 
// very simple shaders to render cubemaps on spheres.) It still probably won't run on integrated
// graphics.
//
// Smiley battle requires numpy, pygame, pyopengl and pyopengl-accelerate. To build pyopengl-accelerate,
// you will unfortunately have to install MS Visual Studio and its C++ application building components.
// That's however all you need to do, it just needs to be present on your computer to install
// pyopengl-accelerate (which will be built #include "C source during installation.)
//
// Smiley battle has a simple UDP based networking interface to allow multiplayer matches over the
// internet. The networking model is peer to peer (see networkhandler.py for a brief documentation).
//
// Other "fancy" stuff is a DACS based shortest path search to quickly compute distances between
// movement nodes the application's map handler can create. Right now, the only purpose is to approximate
// the distance between the viewer and sound sources for proper sound volumes. Unfortunately, this
// is slow with Python even on a fast computer.
// 
// The only shape used in this game (besides some sprites) is a sphere, which is generated #include "a cube. 
// Smiley Battle has proper collision handling, so you can bump into walls and push other players around.
//
// Smiley Battle supports gamepads and keyboard. Control movement with W-A-S-D, arrow keys or numpad 
// 4-5-6-8 keys. Fire with spacebar. Alternatively, control movement with the gamepad sticks and 
// shoot with its fire buttons. 

// =================================================================================================

#ifdef _DEBUG
#   define LOG(msg, ...) fprintf(stderr, msg, ##__VA_ARGS__);
#else
#   define LOG(msg, ...)
#endif

// =================================================================================================
// Main application class
// Contains all application data, initialization and main loop

// =================================================================================================

CApplication::CApplication (int argC, char** argV) {
    InitRandom ();
    InitSound ();   // must be called before InitDisplay () to set proper init parameters for pygame.mixer, which is initialized in pygame.init ()
    LOG ("InitDisplay")
    InitDisplay ();
    LOG ("InitNetworking")
        InitNetworking ();

    LOG ("argHandler\n")
    argHandler = new CArgHandler (argC, argV);
    argHandler->LoadArgs ("smileybattle.ini");
    LOG ("renderer\n")
    renderer = new CRenderer (1920, 1080);
    LOG ("textureHandler\n")
        textureHandler = new CTextureHandler ();
    LOG ("gameData\n")
        gameData = new CGameData ();
    LOG ("actorHandler\n")
        actorHandler = new CActorHandler ();
    LOG ("gameItems\n")
        gameItems = new CGameItems ();
    LOG ("soundHandler\n")
        soundHandler = new CSoundHandler ();
    LOG ("controlsHandler\n")
        controlsHandler = new CControlsHandler ();
    LOG ("physicsHandler\n")
        physicsHandler = new CPhysicsHandler ();
    LOG ("networkHandler\n")
        networkHandler = new CNetworkHandler ();
    LOG ("shaderHandler\n")
        shaderHandler = new CShaderHandler ();
    LOG ("scoreBoard\n")
        scoreBoard = new CScoreBoard ();
    LOG ("effectHandler\n")
        effectHandler = new CEffectHandler ();
    LOG ("gameItems->Create\n")
        gameItems->Create ();
    LOG ("networkHandler->Create\n")
        networkHandler->Create ();
}


void CApplication::InitRandom (void) {
    time_t t;
    std::time (&t);
    srand (int (t));
}


void CApplication::InitSound (void) {
    SDL_Init (SDL_INIT_AUDIO);
}


void CApplication::InitDisplay (void) {
    if (SDL_Init (SDL_INIT_EVERYTHING) != 0) {
        fprintf (stderr, "Cannot initialize display\n");
        exit (1);
    }
}


void CApplication::InitNetworking (void) {
    if (SDLNet_Init () != 0) {
        fprintf (stderr, "Cannot initialize networking\n");
        exit (1);
    }
}


void CApplication::Destroy (void) {
    delete argHandler;
    argHandler = nullptr;
    delete renderer;
    renderer = nullptr;
    delete textureHandler;
    textureHandler = nullptr;
    delete gameData;
    gameData = nullptr;
    delete actorHandler;
    actorHandler = nullptr;
    delete gameItems;
    gameItems = nullptr;
    delete soundHandler;
    soundHandler = nullptr;
    delete controlsHandler;
    controlsHandler = nullptr;
    delete physicsHandler;
    physicsHandler = nullptr;
    delete networkHandler;
    networkHandler = nullptr;
    delete shaderHandler;
    shaderHandler = nullptr;
    delete scoreBoard;
    scoreBoard = nullptr;
    delete effectHandler;
    effectHandler = nullptr;
}


void CApplication::Quit (void) {
    Destroy ();
    SDL_Quit ();
    exit (0);
}


void CApplication::RenderScene (void) {
    renderer->Start ();
    if (actorHandler->m_viewer->IsHidden ())
        renderer->Stop ();
    else {
        actorHandler->m_viewer->Wiggle ();
        gameItems->m_map->Render ();
        for (auto [i, a] : actorHandler->m_actors)
           a->Render ();
        renderer->Stop ();
        gameItems->m_reticle.Render ();
    }
    effectHandler->Fade ();
    scoreBoard->Render ();
}
            


bool CApplication::HandleEvents (void) {
    SDL_Event event;
    while (SDL_PollEvent (&event)) {
        if (event.type == SDL_QUIT)
            return false;
        if (controlsHandler->HandleEvent (event))
            return true;
        if ((event.type == SDL_KEYDOWN) && (event.key.keysym.sym == SDLK_ESCAPE))
            return false;
    }
    return true;
}


void CApplication::Run (void) {
    size_t frames = 0;
    size_t t0 = SDL_GetTicks ();
    CTimer frameTime (gameData->m_minFrameTime);

    while (HandleEvents ()) {
        if (gameData->m_suspend)
            networkHandler->Update ();
        else {
            frames++;
            frameTime.Start ();  // start frame time measurement
            gameData->m_gameTime = frameTime.m_time;
            physicsHandler->Update ();
            RenderScene ();
            soundHandler->Update ();
            networkHandler->Update ();
            actorHandler->Cleanup ();
            renderer->Update ();
            frameTime.Delay ();  // wait until the minimum frame time has been reached or exceeded. For max fps comment this statement
        }
    }

    size_t t = SDL_GetTicks () - t0;
    float fps = float (frames * 1000) / float (t);
    networkHandler->BroadcastLeave ();
    fprintf (stderr, "On your system, Smiley Battle runs at %1.2f fps.\n", fps);
    system ("pause");
}

// =================================================================================================

int main (int argC, char** argV) {
    CApplication app (argC, argV);
    app.Run ();
    app.Quit ();
    return 0;
}

// =================================================================================================
