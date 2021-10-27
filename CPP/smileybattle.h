#pragma once


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
// Main application class
// Contains all application data, initialization and main loop

// =================================================================================================

class CApplication {
public:
    CApplication (int argC = 0, char** argV = nullptr);

    void InitRandom (void);
        
    void InitSound (void);

    void InitDisplay (void);

    void InitNetworking (void);

    void Destroy (void);

    void Quit (void);

    void RenderScene (void);

    bool HandleEvents (void);

    void Run (void);

};

// =================================================================================================
