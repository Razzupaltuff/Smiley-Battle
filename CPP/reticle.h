#pragma once

#include "glew.h"
#include "cstring.h"
#include "clist.h"
#include "texture.h"
#include "quad.h"
#include "renderer.h"
#include "textureHandler.h"

// =================================================================================================
// Render a reticle on the scene. Requires an orthogonal (not perspective) projection.
// Reticle renderer needs to be called after the 3D renderer is done.

class CReticle : public CQuad {
public:
    CList<CString>      m_textureNames;
    CList<CTexture*>    m_textures;

    CReticle () : CQuad ({ CVector (-0.125f, -0.125f, 0.0f), CVector (-0.125f, +0.125f, 0.0f), CVector (+0.125f, +0.125f, 0.0f), CVector (+0.125f, -0.125f, 0.0f) }) {
        m_textureNames = { "reticle-darkgreen.png", "reticle-lightgreen.png" };
        CreateTextures ();
    }


    void Create (void) {
        CQuad::Create ();
        CreateTextures ();
    }


    void Destroy (void) {
        CQuad::Destroy ();
    }


    void CreateTextures (void) {
        m_textures = textureHandler->CreateTextures (m_textureNames);
    }


    void Render (void);

};

// =================================================================================================
