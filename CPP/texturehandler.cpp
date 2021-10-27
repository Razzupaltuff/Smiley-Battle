#pragma once

#include "gameData.h"
#include "textureHandler.h"

// =================================================================================================
// Very simply class for texture tracking
// Main purpose is to keep track of all texture objects in the game and return them to OpenGL in
// a well defined and controlled way at program termination without having to bother about releasing
// textures at a dozen places in the game

void CTextureHandler::Destroy(void) {
    for (auto [i, t] : m_textures) 
        delete t;
}


CTexture* CTextureHandler::GetTexture(void) {
    m_textures.Append(new CTexture ());
    return m_textures[-1];
}


bool CTextureHandler::Remove(CTexture* texture) {
	if (!texture)
		return false;
    m_textures.Remove(texture);
	texture->Destroy ();
	return true;
}


CCubemap* CTextureHandler::GetCubemap(void) {
    m_textures.Append(new CCubemap ());
    return (CCubemap*)m_textures[-1];
}


CList<CTexture*> CTextureHandler::Create(CList<CString>& textureNames, GLenum textureType) {
    return (textureType == GL_TEXTURE_CUBE_MAP) ? CreateCubemaps (textureNames) : CreateTextures (textureNames);
}


CList<CTexture*> CTextureHandler::CreateTextures(CList<CString>& textureNames) {
    CList<CTexture*> textures;
	CList<CString> fileNames;
	CString* fileName = fileNames.Add (-1);
    for (auto [i, n] : textureNames) {
        CTexture* t = GetTexture ();
        textures.Append (t);
        *fileName = gameData->m_textureFolder + n;
        if (!t->CreateFromFile (fileNames))
            break;
    }
    return textures;
}


CList<CTexture*> CTextureHandler::CreateCubemaps(CList<CString>& textureNames) {
    CList<CTexture*> textures;
	CList<CString> fileNames;
	CString* fileName = fileNames.Add (-1);
    for (auto [i, n] : textureNames) {
        CCubemap* t = GetCubemap ();
        textures.Append (t);
        *fileName = gameData->m_textureFolder + n;
        if (!t->CreateFromFile (fileNames))
            break;
    }
    return textures;
}


CList<CTexture*> CTextureHandler::CreateByType(CList<CString>& textureNames, GLenum textureType) {
    return Create (textureNames, textureType);
}

CTextureHandler* textureHandler = nullptr;

// =================================================================================================
