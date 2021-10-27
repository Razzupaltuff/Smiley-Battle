#include <stdlib.h>
#include <math.h>

#include "glew.h"
#include "sdl.h"
#include "sdl_net.h"
#include "sdl_ttf.h"
#include "camera.h"
#include "quad.h"
#include "player.h"
#include "argHandler.h"
#include "renderer.h"

// =================================================================================================
// basic renderer class. Initializes display && OpenGL && sets up projections && view m_projection

CRenderer::CRenderer (int width, int height, CViewer* viewer) {
    m_viewer = viewer;
    SDL_Rect rect;
    SDL_GetDisplayBounds (0, &rect);
    m_maxWidth = rect.w;
    m_maxHeight = rect.h;
    width = argHandler->IntVal ("display", 0, width);
    height = argHandler->IntVal ("display", 1, height);
    bool fullscreen;
    if (width * height > 0)
        fullscreen = false;
    else {
        width = m_maxWidth;
        height = m_maxHeight;
        fullscreen = true;
    }
    SetupDisplay (width, height, fullscreen);
    InitFont ();
    SetupOpenGL ();
    ResetProjection ();
}
        

CRenderer::~CRenderer () {
    SDL_GL_DeleteContext (m_context);
}


bool CRenderer::InitFont (void) {
    if (0 > TTF_Init ()) {
        fprintf (stderr, "Cannot initialize font system\n");
        exit (1);
    }
    if (!(m_scoreFont = TTF_OpenFont ("c:\\windows\\fonts\\cour.ttf", int (float (m_statusHeight / 3) * 0.85f)))) {
        fprintf (stderr, "Cannot load couriernew.ttf\n");
        exit (1);
    }
    TTF_SetFontStyle (m_scoreFont, TTF_STYLE_BOLD);
    return true;
}


void CRenderer::SetupDisplay (int width, int height, bool fullscreen) {
    m_width = width;
    m_height = height;
    // The status bar contains a full size smiley for the local player at the left border && up to two rows
    // of eight half size smileys for the remote players. The score width equals the width W of a full size smiley.
    // So we need horizontal room of 2 x W + 8 x W/2 + 8 x W = 4 x W/2 + 8 x W/2 + 16 x W/2 = 28 * W/2 half size smileys
    // in the status bar.
    // This means that smiley width = 2 x window width / 28 = window width / 14. This is also the status bar height,
    // since smileys cover a square area.
    auto min = [] (auto a, auto b) { return (a < b) ? a : b; };
    m_scoreWidth = m_statusHeight = min (160, m_width / 14);
    int screenType = SDL_WINDOW_OPENGL;
    if (fullscreen || (argHandler->IntVal ("fullscreen", 0, 0) == 1)) {
        screenType |= SDL_WINDOW_FULLSCREEN;
        m_height -= m_statusHeight;
    }
    else if (m_height + m_statusHeight > m_maxHeight)
        m_height = m_maxHeight - m_statusHeight;
    m_aspectRatio = float (m_width) / float (m_height);
    m_window =
        SDL_CreateWindow (
            "[C++] Smiley Battle 1.0.7 by Dietfrid Mali",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            m_width, m_height + m_statusHeight,
            screenType);
    if (!m_window) {
        fprintf (stderr, "Couldn't set screen mode (%d x %d)\n", m_width, m_height);
        exit (1);
    }
    m_context = SDL_GL_CreateContext (m_window);
    SDL_GL_SetSwapInterval (argHandler->IntVal ("vsync", 0, 1));
}


void CRenderer::SetupOpenGL (void) {
#if 1
    if (glewInit () != GLEW_OK) {
        fprintf (stderr, "Couldn't initialize OpenGL\n");
        exit (1);
    }
#endif
    glColorMask (1, 1, 1, 1);
    glDepthMask (1);
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LESS);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_ALPHA_TEST);
    glEnable (GL_CULL_FACE);
    glCullFace (GL_BACK);
    glEnable (GL_MULTISAMPLE);
    glFrontFace (GL_CW);
    glDisable (GL_POLYGON_OFFSET_FILL);
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    auto min = [] (auto a, auto b) { return (a < b) ? a : b; };
    auto max = [] (auto a, auto b) { return (a > b) ? a : b; };
    auto clampFOV = [] (auto fov) { return min (max (fov, 90), 120); };
    m_fov = clampFOV (argHandler->FloatVal ("fov", 0, 90)) / 2;
    m_zNear = 0.01f;
    m_zFar = 1000.0f;
    m_zoom = 1.0f;
    ComputeProjection ();
    SetViewport ();
}


//m_projection will receive the calculated perspective m_projection.
//You would have to upload to your shader
// or use glLoadMatrixf if you aren't using shaders.
void CRenderer::ComputeProjection (void)
{
    //m_projection.Create (16);
    float yMax = m_zNear * tanf (m_fov * M_PI / 360.0f);
    //ymin = -ymax;
    //xmin = -ymax * aspectRatio;
    float xMax = yMax * m_aspectRatio;
    ComputeFrustum (-xMax, xMax, -yMax, yMax);
}


void CRenderer::ComputeFrustum (float left, float right, float bottom, float top)
{
    float nearPlane = 2.0f * m_zNear;
    float depth = m_zFar - m_zNear;

#if 0
    // symmetric frustum, i.e. left == -right and bottom == -top
    m_projection =
        CMatrix (
            CVector (m_zNear / right, 0.0f, 0.0f, 0.0f),
            CVector (0.0f, m_zNear / top, 0.0f, 0.0f),
            CVector (0.0f, 0.0f, -(m_zFar + m_zNear) / depth, -1.0f),
            CVector (0.0f, 0.0f, (-nearPlane * m_zFar) / depth, 0.0f)
        );
#else
    float width = right - left;
    float height = top - bottom;
    m_projection =
        CMatrix (
            CVector (nearPlane / width, 0.0f, (left + right) / width, 0.0f),
            CVector (0.0f, nearPlane / height, (top + bottom) / height, 0.0f),
            CVector (0.0f, 0.0f, -(m_zFar + m_zNear) / depth, -1.0f),
            CVector (0.0f, 0.0f, (-nearPlane * m_zFar) / depth, 0.0f)
        );
#endif
    m_projection.AsArray ();
}


void CRenderer::SetupProjection (void) {
    SetViewport ("game");
    glMatrixMode (GL_PROJECTION);
    glLoadMatrixf (m_projection.Array ());
    }


void CRenderer::ResetProjection (void) {
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glOrtho (0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    }


void CRenderer::Start (void) {
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SetupProjection ();
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    glColor3f (1, 1, 1);
    m_viewer->m_camera.Enable ();
    }


void CRenderer::Stop (void) {
    m_viewer->m_camera.Disable ();
    ResetProjection ();
    }


void CRenderer::Update (void) {
    SDL_GL_SwapWindow (m_window);
}


CRenderer::tViewport CRenderer::SetViewport (std::string area, int position) {
    int width, height;
    if (area == "game") {
        width = m_width;
        height = m_height;
        glViewport (0, 0, m_width, m_height);
    }
    else if (area == "status") {
        if (position == 0) {
            width = m_statusHeight;
            height = m_statusHeight;
            glViewport (0, m_height, m_statusHeight, m_statusHeight);
        }
        else {
            // split status area right of viewer's status icon in two rows && eight columns
            // row = ((position - 1) / 8)
            int l = m_statusHeight + m_scoreWidth;  // horizontal space of the viewer's status icon
            int h = m_statusHeight / 2;             // half of the vertical space, for two rows
            int w = (m_width - l) / 8;              // total horizontal space for a non viewer's status (icon + score)
            position--;
            width = h;
            height = h;
            glViewport (l + (position % 8) * w, m_height + (1 - (position / 8)) * h, width, height);
        }
    }
    else if (area == "score") {
        if (position == 0) {
            width = m_scoreWidth;
            height = int (m_statusHeight / 3);
            glViewport (m_statusHeight, m_height + 2 * height, width, height);
        }
        else {
            // split status area right of viewer's status icon in two rows && eight columns
            // row = ((position - 1) / 8);
            int l = m_statusHeight + m_scoreWidth;  // horizontal space of the viewer's status icon && score area
            int h = m_statusHeight / 2;
            height = m_statusHeight / 3;            // a third of the total status height, like for viewer scores
            int w = (m_width - l) / 8;              // total horizontal space for a non viewer's status (icon + score)
            position -= 1;
            width = m_scoreWidth;
            glViewport (l + (position % 8) * w + h, m_height + (1 - (position / 8)) * h + m_statusHeight / 12, width, height);
        }
    }
    else if (area == "kills") {
        width = m_scoreWidth;
        height = m_statusHeight / 3;
        glViewport (m_statusHeight, m_height + height, width, height);
    }
    else if (area == "deaths") {
        width = m_scoreWidth;
        height = m_statusHeight / 3;
        glViewport (m_statusHeight, m_height, width, height);
    }
    else if (area == "scoreboard") {
        width = m_width;
        height = m_statusHeight;
        glViewport (0, m_height, m_width, m_statusHeight);
    }
    else {
        width = m_width;
        height = m_height + m_statusHeight;
        glViewport (0, 0, m_width, height);
    }
    ResetProjection ();

    return tViewport { width, height };
}


void CRenderer::CheckError (const char* operation) {
    GLenum glError = glGetError ();
    if (glError) {
        fprintf (stderr, "OpenGL Error %d (%s)\n", glError, operation);
        exit (1);
    }
}

CRenderer* renderer = nullptr;

// =================================================================================================
