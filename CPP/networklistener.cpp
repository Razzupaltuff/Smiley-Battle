#include "networklistener.h"
#include "networkHandler.h"

// =================================================================================================

int CListener::Run(void* dataPtr) {
    CListenerData& data = *((CListenerData*) dataPtr);

    while (*data.m_listen) {
        CMessage message = networkHandler->Receive();
        if (message.Empty ())
            Sleep(5);
        else {
            SDL_mutexP (data.m_lock);
            networkHandler->m_messages.Append(message);
            SDL_mutexV (data.m_lock);
        }
    }
    return 0;
}


bool CListener::Start(bool *listen) {
    m_data.m_listen = listen;
    m_data.m_thread = SDL_CreateThread(CListener::Run, "network listener", &m_data);
    return (m_data.m_thread != nullptr);
}


void CListener::Stop(void) {
    *m_data.m_listen = false;
    SDL_WaitThread(m_data.m_thread, nullptr);

}

// =================================================================================================
