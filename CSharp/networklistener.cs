using System;
using System.Threading;

// =================================================================================================

static public class NetworkListener
{
    public static Thread m_thread;
    public static Semaphore m_lock;
    public static bool m_listen;

    static NetworkListener() 
    {
        m_listen = false;
        m_lock = new Semaphore(0, 1);
        Unlock();
        m_thread = new Thread(new ThreadStart (NetworkListener.Run));
    }

    public static void Lock()
    {
        m_lock.WaitOne ();
    }

    public static void Unlock()
    {
        m_lock.Release();
    }

    public static void Run()
    {
        while (m_listen && (Globals.networkHandler != null))
        {
            NetworkMessage message = Globals.networkHandler.Receive();
            if (message.Empty())
                Thread.Sleep(5);
            else
            {
                Lock ();
                Globals.networkHandler.m_messages.Add (message);
                Unlock ();
            }
        }
    }


    public static void Start()
    {
        m_listen = true;
        m_thread.Start();
    }


    public static void Stop()
    {
        m_listen = false;
        //m_thread.Interrupt();
    }

}

// =================================================================================================
