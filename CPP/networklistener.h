#pragma once 

#include <stdint.h>

#include "SDL_net.h"
#include "SDL_thread.h"
#include "SDL_mutex.h"
#include "networkmessage.h"
//#include "networkHandler.h"

// =================================================================================================

class CListener {
    public:
        typedef bool (*tListen) (void);

        class CListenerData {
            public:
                SDL_Thread* m_thread;
                SDL_mutex*  m_lock;
                bool*       m_listen;

                CListenerData (bool* listen = nullptr) : m_thread (nullptr), m_lock (nullptr), m_listen (listen) {}
        };

        CListenerData m_data;

        inline void Lock(void) {
            SDL_mutexP(m_data.m_lock);
        }

        inline void Unlock(void) {
            SDL_mutexV(m_data.m_lock);
        }

        CListener (bool* listen = nullptr) {
            m_data.m_listen = listen;
            m_data.m_lock = SDL_CreateMutex();
        }

        ~CListener() {
            if (m_data.m_lock) {
                SDL_DestroyMutex(m_data.m_lock);
                m_data.m_lock = nullptr;
            }
        }
    
        bool Start(bool * listen);

        void Stop(void);

        static int Run(void* dataPtr);

};

// =================================================================================================
