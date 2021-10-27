#pragma once

#include "glew.h"
#include "reticle.h"
#include "gameItems.h"
#include "renderer.h"

// =================================================================================================
// Render a reticle on the scene. Requires an orthogonal (not perspective) projection.
// Reticle renderer needs to be called after the 3D renderer is done.

void CReticle::Render (void) {
    // global renderer
    if (m_textures.Empty ())
        return;
    SetTexture (m_textures [int (gameItems->m_viewer->ReadyToFire ())]);
    glDepthFunc (GL_ALWAYS);
    glDisable (GL_CULL_FACE);
    glPushMatrix ();
    glTranslatef (0.5, 0.5, 0);
    glScalef (0.125f / renderer->m_aspectRatio, 0.125f, 1.0f);
    CQuad::Render ();
    glPopMatrix ();
    glEnable (GL_CULL_FACE);
    glDepthFunc (GL_LESS);
}

// =================================================================================================
