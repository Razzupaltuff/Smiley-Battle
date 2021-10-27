#pragma once

#include <stdlib.h>
#include <math.h>
#include "matrix.h"
#include "camera.h"
#include "quad.h"
#include "player.h"
#include "sdl_ttf.h"

// =================================================================================================
// basic renderer class. Initializes display and OpenGL and sets up projections and view matrix

class CRenderer {

    public:
        int             m_width;
        int             m_height;
        int             m_maxWidth;
        int             m_maxHeight;
        int             m_statusHeight;
        int             m_scoreWidth;
        float           m_aspectRatio;
        float           m_fov;
        float           m_zNear;
        float           m_zFar;
        float           m_zoom;
        CMatrix         m_projection;
        CViewer *       m_viewer;
        SDL_Window *    m_window;
        SDL_GLContext   m_context;
        TTF_Font *      m_scoreFont;

        CRenderer (int width = 1920, int height = 1080, CViewer* viewer = nullptr);

        ~CRenderer ();

        bool InitFont (void);

        void SetupDisplay (int width, int height, bool fullscreen);

        void SetupOpenGL (void);

        void ComputeProjection (void);
            
        void ComputeFrustum (float left, float right, float bottom, float top);

        void SetupProjection (void);

        void ResetProjection (void);

        void Start (void);

        void Stop (void);

        void Update (void);

        inline GLfloat* ProjectionMatrix (void) {
            return (GLfloat*) m_projection.Array ();
        }

        inline void SetViewer (CViewer* viewer) {
            m_viewer = viewer;
        }

        inline CCamera& GetCamera (void) {
            return m_viewer->m_camera;
        }

        inline CVector& GetPosition (void) {
            return GetCamera ().GetPosition ();
        }


        inline CVector& GetOrientation (void) {
            return GetCamera ().GetOrientation ();
        }


        inline void SetPosition (CVector position) {
            GetCamera ().SetPosition (position);
        }


        inline void UpdateAngles (CVector angles) {
            GetCamera ().UpdateAngles (angles);
        }

        typedef struct {
            int width, height;
        } tViewport;

        tViewport SetViewport (std::string area = "", int position = 0);

        void CheckError (const char* operation);

};

extern CRenderer* renderer;

// =================================================================================================
