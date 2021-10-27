#pragma once

#include "glew.h"
#include "carray.h"
#include "cstring.h"
#include "SDL.h"
#include "SDL_image.h"

// =================================================================================================
// texture handling classes

class CAbstractTexture {
    public:  
        virtual bool Create(void) = 0;

        virtual void Destroy (void) = 0;

        virtual bool Available (void) = 0;

        virtual void Bind (void) = 0;

        virtual void Release (void) = 0;

        virtual void SetParams (void) = 0;

        virtual void Deploy (int bufferIndex = 0) = 0;

        virtual void Enable (int tmu = 0) = 0;

        virtual void Disable (void) = 0;

        virtual bool Load (CList<CString>& fileNames, bool flipVertically = true) = 0;
};

// =================================================================================================
// texture data buffer handling

    class CTextureBuffer {
        public:
            typedef enum {
                eClone,
                eMove,
                eCopy
            } eCopyMode;


            int     m_width;
            int     m_height;
            int     m_componentCount;
            GLenum  m_internalFormat;
            GLenum  m_format;
            int     m_dataSize;
            void *  m_data;
            int     m_isAlias;

        CTextureBuffer () 
            : m_width (0), m_height (0), m_componentCount (0), m_internalFormat (0), m_format (0), m_dataSize (0), m_data (nullptr), m_isAlias (false)
        {}

        ~CTextureBuffer () {
            if (m_data) {
                if (m_isAlias)
                    m_isAlias = false;
                else
                    free (m_data);
                m_data = nullptr;
            }
        }

        CTextureBuffer (CTextureBuffer const& other);

        CTextureBuffer (CTextureBuffer&& other) noexcept;

        CTextureBuffer (SDL_Surface* image);

        CTextureBuffer& Create (SDL_Surface* image);

        CTextureBuffer& operator= (CTextureBuffer other);

        // CTextureBuffer& operator= (CTextureBuffer&& other);

        CTextureBuffer& Copy (CTextureBuffer& other, eCopyMode mode = eClone);

        // CTextureBuffer(CTextureBuffer&& other) = default;            // move construct

        // CTextureBuffer& operator=(CTextureBuffer && other) = default; // move assignment
    };

// =================================================================================================
// texture handling: Loading from file, parameterization and sending to OpenGL driver, 
// enabling for rendering
// Base class for higher level textures (e.g. cubemaps)

class CTexture : public CAbstractTexture {
    public:
        GLuint                  m_handle;
        CList<CTextureBuffer>   m_buffers;
        CList<CString>          m_fileNames;
        int                     m_type;
        int                     m_wrapMode;
        int                     m_useMipMaps;

        CTexture (int type = GL_TEXTURE_2D, int wrapMode = GL_CLAMP_TO_EDGE) 
            : m_handle (0), m_type (type), m_wrapMode (wrapMode), m_useMipMaps (false) 
            {}

        ~CTexture () {
            Destroy ();
        }

        inline bool operator== (CTexture const& other) {
            return m_handle == other.m_handle;
        }

        virtual bool Create(void);

        virtual void Destroy(void);

        virtual bool Available(void);

        virtual void Bind(void);

        virtual void Release(void);

        void SetParams(void);

        void Wrap(void);

        virtual void Enable(int tmu = 0);

        virtual void Disable(void);

        virtual void Deploy(int bufferIndex = 0);

        virtual bool Load(CList<CString>& fileNames, bool flipVertically);

        bool CreateFromFile(CList<CString>& fileNames, bool flipVertically = true);

        bool CreateFromSurface(SDL_Surface* surface);

        inline size_t TextureCount(void) {
            return m_buffers.Length ();
        }

        inline int GetWidth(int i = 0) {
            return m_buffers[i].m_width;
        }

        inline int GetHeight(int i = 0) {
            return m_buffers[i].m_height;
        }

        inline int Type(void) {
            return m_type;
        }

        inline int WrapMode(void) {
            return m_wrapMode;
        }

    };

// =================================================================================================
