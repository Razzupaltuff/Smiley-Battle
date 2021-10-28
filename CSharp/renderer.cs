using System;
using SDL2;

// =================================================================================================
// basic renderer class. Initializes display && OpenGL && sets up projections && view m_projection

public class Renderer
{
    //public Viewer m_viewer;
    public int m_width;
    public int m_height;
    public int m_maxWidth;
    public int m_maxHeight;
    public int m_statusHeight;
    public int m_scoreWidth;
    public float m_zNear;
    public float m_zFar;
    public float m_fov;
    public float m_aspectRatio;
    public IntPtr m_window;
    public IntPtr m_context;
    public IntPtr m_scoreFont;
    public Matrix m_orientation;
    public Matrix m_projection;
    public Viewer m_viewer;


    public class Viewport
    {
        public int m_width;
        public int m_height;

        public Viewport() { }
        public Viewport(int width, int height)
        {
            m_width = width;
            m_height = height;
        }
    }


    public Renderer(int width = 1920, int height = 1080, Viewer viewer = null)
    {
        m_viewer = viewer;
        SDL.SDL_Rect rect;
        SDL.SDL_GetDisplayBounds(0, out rect);
        m_maxWidth = rect.w;
        m_maxHeight = rect.h;
        width = Globals.argHandler.IntVal("display", 0, width);
        height = Globals.argHandler.IntVal("display", 1, height);
        bool fullscreen;
        if (width * height > 0)
            fullscreen = false;
        else
        {
            width = m_maxWidth;
            height = m_maxHeight;
            fullscreen = true;
        }
        SetupDisplay(width, height, fullscreen);
        InitFont();
        SetupOpenGL();
        ResetProjection();
    }


    ~Renderer()
    {
        SDL.SDL_GL_DeleteContext(m_context);
    }


    public void SetViewer(Viewer viewer)
    {
        m_viewer = viewer;
    }

    bool InitFont()
    {
        if (0 > SDL_ttf.TTF_Init())
        {
            Console.Error.WriteLine("Cannot initialize font system");
            System.Environment.Exit(1);
        }
        m_scoreFont = SDL_ttf.TTF_OpenFont("c:\\windows\\fonts\\cour.ttf", (int)((float)(m_statusHeight / 3) * 0.85f));
        if (m_scoreFont == null)
        {
            Console.Error.WriteLine("Cannot load couriernew.ttf");
            System.Environment.Exit(1);
        }
        SDL_ttf.TTF_SetFontStyle(m_scoreFont, SDL_ttf.TTF_STYLE_BOLD);
        return true;
    }


    void SetupDisplay(int width, int height, bool fullscreen)
    {
        m_width = width;
        m_height = height;
        // The status bar contains a full size smiley for the local player at the left border && up to two rows
        // of eight half size smileys for the remote players. The score width equals the width W of a full size smiley.
        // So we need horizontal room of 2 x W + 8 x W/2 + 8 x W = 4 x W/2 + 8 x W/2 + 16 x W/2 = 28 * W/2 half size smileys
        // in the status bar.
        // This means that smiley width = 2 x window width / 28 = window width / 14. This is also the status bar height,
        // since smileys cover a square area.
        m_scoreWidth = m_statusHeight = Math.Min(160, m_width / 14);
        SDL.SDL_WindowFlags screenType = SDL.SDL_WindowFlags.SDL_WINDOW_OPENGL;
        if (fullscreen || (Globals.argHandler.IntVal("fullscreen", 0, 0) == 1))
        {
            screenType |= SDL.SDL_WindowFlags.SDL_WINDOW_FULLSCREEN;
            m_height -= m_statusHeight;
        }
        else if (m_height + m_statusHeight > m_maxHeight)
            m_height = m_maxHeight - m_statusHeight;
        m_aspectRatio = (float)m_width / (float)m_height;
        m_window =
            SDL.SDL_CreateWindow(
                "[C#] Smiley Battle 1.0.8 by Dietfrid Mali",
                SDL.SDL_WINDOWPOS_CENTERED,
                SDL.SDL_WINDOWPOS_CENTERED,
                m_width, m_height + m_statusHeight,
                screenType);
        if (m_window == null)
        {
            Console.Error.WriteLine("Couldn't set screen mode {0} x {1}", m_width, m_height);
            System.Environment.Exit(1);
        }
        m_context = SDL.SDL_GL_CreateContext(m_window);
        SDL.SDL_GL_SetSwapInterval(Globals.argHandler.IntVal("vsync", 0, 1));
    }


    void SetupOpenGL()
    {
        GL.ColorMask(1, 1, 1, 1);
        GL.DepthMask(1);
        GL.Enable(GL.DEPTH_TEST);
        GL.DepthFunc(GL.LESS);
        GL.Enable(GL.BLEND);
        GL.BlendFunc(GL.SRC_ALPHA, GL.ONE_MINUS_SRC_ALPHA);
        GL.Enable(GL.ALPHA_TEST);
        GL.Enable(GL.CULL_FACE);
        GL.CullFace(GL.BACK);
        GL.Enable(GL.MULTISAMPLE);
        GL.FrontFace(GL.CW);
        GL.Disable(GL.POLYGON_OFFSET_FILL);
        GL.Hint(GL.PERSPECTIVE_CORRECTION_HINT, GL.NICEST);
        Func<float, float> clampFOV = fov => { return Math.Min(Math.Max(fov, 90), 120); };
        m_fov = clampFOV(Globals.argHandler.FloatVal("fov", 0, 90.0f)) / 2;
        m_zNear = 0.01f;
        m_zFar = 1000.0f;
        ComputeProjection();
        SetViewport();
    }


    //m_projection will receive the calculated perspective m_projection.
    //You would have to upload to your shader
    // or use glLoadMatrixf if you aren't using shaders.
    void ComputeProjection()
    {
        //m_projection.Create (16);
        float yMax = m_zNear * (float)Math.Tan(m_fov * Math.PI / 360.0f);
        float xMax = yMax * m_aspectRatio;
        ComputeFrustum(-xMax, xMax, -yMax, yMax);
    }


    void ComputeFrustum(float left, float right, float bottom, float top)
    {
        float nearPlane = 2.0f * m_zNear;
        float depth = m_zFar - m_zNear;

        float width = right - left;
        float height = top - bottom;
        m_projection =
            new Matrix(
                new Vector(nearPlane / width, 0.0f, (left + right) / width, 0.0f),
                new Vector(0.0f, nearPlane / height, (top + bottom) / height, 0.0f),
                new Vector(0.0f, 0.0f, -(m_zFar + m_zNear) / depth, -1.0f),
                new Vector(0.0f, 0.0f, (-nearPlane * m_zFar) / depth, 0.0f)
            );
        m_projection.AsArray();
    }

    public void SetupProjection()
    {
        SetViewport("game");
        GL.MatrixMode(GL.PROJECTION);
        GL.LoadMatrix(m_projection.AsArray());
    }


    public void ResetProjection()
    {
        GL.MatrixMode(GL.PROJECTION);
        GL.LoadIdentity();
        GL.Ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
        GL.MatrixMode(GL.MODELVIEW);
        GL.LoadIdentity();
    }

    public void Start()
    {
        GL.Clear(GL.COLOR_BUFFER_BIT | GL.DEPTH_BUFFER_BIT);
        SetupProjection();
        GL.MatrixMode(GL.MODELVIEW);
        GL.LoadIdentity();
        GL.Color(1, 1, 1);
        m_viewer.m_camera.Enable();
    }

    public void Stop()
    {
        m_viewer.m_camera.Disable();
        ResetProjection();
    }


    public void Update()
    {
        SDL.SDL_GL_SwapWindow(m_window);
    }


    public Viewport SetViewport(string area = "", int position = 0)
    {
        int width, height;
        if (area == "game")
        {
            width = m_width;
            height = m_height;
            GL.Viewport(0, 0, m_width, m_height);
        }
        else if (area == "status")
        {
            if (position == 0)
            {
                width = m_statusHeight;
                height = m_statusHeight;
                GL.Viewport(0, m_height, m_statusHeight, m_statusHeight);
            }
            else
            {
                // split status area right of viewer's status icon in two rows && eight columns
                // row = ((position - 1) / 8)
                int l = m_statusHeight + m_scoreWidth;  // horizontal space of the viewer's status icon
                int h = m_statusHeight / 2;             // half of the vertical space, for two rows
                int w = (m_width - l) / 8;              // total horizontal space for a non viewer's status (icon + score)
                position--;
                width = h;
                height = h;
                GL.Viewport(l + (position % 8) * w, m_height + (1 - (position / 8)) * h, width, height);
            }
        }
        else if (area == "score")
        {
            if (position == 0)
            {
                width = m_scoreWidth;
                height = m_statusHeight / 3;
                GL.Viewport(m_statusHeight, m_height + 2 * height, width, height);
            }
            else
            {
                // split status area right of viewer's status icon in two rows && eight columns
                // row = ((position - 1) / 8);
                int l = m_statusHeight + m_scoreWidth;  // horizontal space of the viewer's status icon && score area
                int h = m_statusHeight / 2;
                height = m_statusHeight / 3;            // a third of the total status height, like for viewer scores
                int w = (m_width - l) / 8;              // total horizontal space for a non viewer's status (icon + score)
                position -= 1;
                width = m_scoreWidth;
                GL.Viewport(l + (position % 8) * w + h, m_height + (1 - (position / 8)) * h + m_statusHeight / 12, width, height);
            }
        }
        else if (area == "kills")
        {
            width = m_scoreWidth;
            height = m_statusHeight / 3;
            GL.Viewport(m_statusHeight, m_height + height, width, height);
        }
        else if (area == "deaths")
        {
            width = m_scoreWidth;
            height = m_statusHeight / 3;
            GL.Viewport(m_statusHeight, m_height, width, height);
        }
        else if (area == "scoreboard")
        {
            width = m_width;
            height = m_statusHeight;
            GL.Viewport(0, m_height, m_width, m_statusHeight);
        }
        else
        {
            width = m_width;
            height = m_height + m_statusHeight;
            GL.Viewport(0, 0, m_width, height);
        }
        ResetProjection();

        return new Viewport (width, height);
    }


    public void CheckError(string operation)
    {
        uint glError = GL.GetError();
        if (glError != 0)
        {
            Console.Error.WriteLine("OpenGL Error {0} ({1})", glError, operation);
            //System.Environment.Exit(1);
        }
    }

}

// =================================================================================================
