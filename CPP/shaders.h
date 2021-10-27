#pragma once

#include "glew.h"
#include "vector.h"
#include "clist.h"
#include "texture.h"

// =================================================================================================
// Some basic shader handling: Compiling, enabling, setting shader variables

class CShader {
    public:
        GLuint          m_handle;
        CArray<float>   m_glData;

        CShader() : m_handle(0) {}

        CShader(CShader const& other) {
            m_handle = other.m_handle;
        }

        CShader (CShader&& other) noexcept {
            m_handle = other.m_handle;
            other.m_handle = 0;
        }

        ~CShader () {
            Destroy ();
        }

        CShader& operator=(CShader&& other) noexcept {
            m_handle = other.m_handle;
            other.m_handle = 0;
        }

        CString GetInfoLog (GLuint handle, bool isProgram = false);
            
        GLuint Compile(const char* code, GLuint type);

        GLuint Link(GLuint vsHandle, GLuint fsHandle);

        CShader& Create(const char* vsCode, const char* fsCode);

        void Destroy(void);

        void SetUniformMatrix(const char* name, CArray<GLfloat>& data);

        void SetUniformVector(const char* name, CVector data);

        void SetUniformFloat(const char* name, float data);

        CArray<float>& GetFloatData(GLenum id, size_t size);

        inline void Enable(void) {
            glUseProgram(m_handle);
        }

        inline void Disable(void) {
            glUseProgram(0);
        }

    };

// =================================================================================================

class CShaderHandler {
    public:
        CList<CShader>   m_shaders;

        CShaderHandler() {
            CreateShaders();
        }

        int SelectShader(bool useShader, CTexture* texture);

        void SetupShader(int shaderId);

        void CreateShaders(void);

        void CreateCubemapShader(CShader * shader);

        void CreateTextureShader(CShader* shader);

        void CreateColorShader(CShader* shader);

        inline CShader& Shader(int i) {
            return m_shaders[i];
        }

};

extern CShaderHandler * shaderHandler;

// =================================================================================================
