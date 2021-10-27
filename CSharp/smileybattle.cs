using System;
using SDL2;
using System.Runtime.InteropServices;

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

// =================================================================================================
// Main application class
// Contains all application data, initialization and main loop

// =================================================================================================

class Application
{
    public Application(string[] args = null)

    {
        InitSound();   // must be called before InitDisplay () to set proper init parameters for pygame.mixer, which is initialized in pygame.init ()
        InitDisplay();
        Globals.Create(args);
    }


    void InitSound()
    {
        SDL.SDL_Init(SDL.SDL_INIT_AUDIO);
    }


    void InitDisplay()
    {
        if (SDL.SDL_Init(SDL.SDL_INIT_EVERYTHING) != 0)
        {
            Console.Error.WriteLine("Cannot initialize display");
            System.Environment.Exit(1);
        }
    }


    void Destroy()
    {
        Globals.vaoHandler.Destroy();
        Globals.textureHandler.Destroy();
        Globals.networkHandler.Destroy();
        Globals.shaderHandler.Destroy();
        Globals.argHandler = null;
        Globals.renderer = null;
        Globals.textureHandler = null;
        Globals.gameData = null;
        Globals.actorHandler = null;
        Globals.actorHandler = null;
        Globals.soundHandler = null;
        Globals.controlsHandler = null;
        Globals.physicsHandler = null;
        Globals.networkHandler = null;
        Globals.shaderHandler = null;
        Globals.scoreBoard = null;
        Globals.effectHandler = null;
    }


    public void Quit()
    {
        Destroy();
        SDL.SDL_Quit();
        System.Environment.Exit(0);
    }


    void RenderScene()
    {
        Globals.renderer.Start();
        if (Globals.actorHandler.m_viewer.IsHidden())
            Globals.renderer.Stop();
        else
        {
            Globals.actorHandler.m_viewer.Wiggle();
            Globals.gameItems.m_map.Render();
            foreach (Actor a in Globals.actorHandler.m_actors)
                a.Render();
            Globals.renderer.Stop();
            Globals.gameItems.m_reticle.Render();
        }
        Globals.effectHandler.Fade();
        Globals.scoreBoard.Render();
    }



    bool HandleEvents()
    {
        SDL.SDL_Event sdlEvent;
        while (SDL.SDL_PollEvent(out sdlEvent) > 0)
        {
            if (sdlEvent.type == SDL.SDL_EventType.SDL_QUIT)
                return false;
            if (Globals.controlsHandler.HandleEvent(sdlEvent))
                return true;
            if ((sdlEvent.type == SDL.SDL_EventType.SDL_KEYDOWN) && (sdlEvent.key.keysym.sym == SDL.SDL_Keycode.SDLK_ESCAPE))
                return false;
        }
        return true;
    }


    void Run()
    {
        int frames = 0;
        int t0 = (int)SDL.SDL_GetTicks();
        Timer frameTime = new Timer(Globals.gameData.m_minFrameTime);

        while (HandleEvents())
        {
            if (Globals.gameData.m_suspend)
                Globals.networkHandler.Update();
            else
            {
                frames++;
                frameTime.Start();  // start frame time measurement
                Globals.gameData.m_gameTime = frameTime.m_time;
                Globals.physicsHandler.Update();
                RenderScene();
                Globals.soundHandler.Update();
                Globals.networkHandler.Update();
                Globals.actorHandler.Cleanup();
                Globals.renderer.Update();
                frameTime.Delay();  // wait until the minimum frame time has been reached or exceeded. For max fps comment this statement
            }
        }

        int t = (int)SDL.SDL_GetTicks() - t0;
        float fps = (float)(frames * 1000) / (float)t;
        Globals.networkHandler.BroadcastLeave();
        Console.Error.WriteLine("On your system, Smiley Battle runs at {0:#.00} fps.\nPress any key to continue . . .", fps);
        Console.ReadKey();
    }

    // =================================================================================================

    [DllImport("kernel32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    static extern bool AllocConsole();

    [DllImport("kernel32.dll")]
    static extern bool AttachConsole(int dwProcessId);
    private const int ATTACH_PARENT_PROCESS = -1;

    static public void Main(string[] args)
    {
        AllocConsole();
        AttachConsole(ATTACH_PARENT_PROCESS);
        Application app = new Application(args);
        app.Run();
        app.Quit();
    }

}

// =================================================================================================
