/*
 * =====================================================================================
 *
 *       Filename: netpod.hpp
 *        Created: 05/24/2016 22:29:29
 *  Last Modified: 05/24/2016 22:45:54
 *
 *    Description: wrapper for Session
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include "netpodhub.hpp"

class NetPod
{
    private:
        std::mutex *m_Lock;
        Session    *m_Session;

    public:
        NetPod(uint32_t nNetID)
        {
            if(g_NetPodHub->Get)
        }

    public:
        friend class AsyncHub<T>;

    private:
        // constructor, only AsyncHub can create it from naked pointer
        //
        // pObject is locked means:
        //      1. no thread other can modify it
        //      2. no thread other can remove it from the hub
        // In one word, it always remains valid if locked
        /* explicit */ NetPod(T *pObject, bool bLockIt)
            : m_Object(pObject)
            , m_Locked(bLockIt)
        {
            // if bLockIt is true, then pObject is already locked
            // otherwise error occurs when pObject is modified during the invocation of this ctor
        }

    private:
    // public:
        // we can make it's public, this is ok since no object created
        // this is to enable following pattern:
        //
        // NetPod pGuard;
        // if(...){
        //      pGuard = f();
        // }else{
        //      pGuard = g();
        // }
        NetPod()
            : m_Object(nullptr)
            , m_Locked(false)
        {}


        // nobody can copy it, otherwise one can unlock a object many times
        NetPod(const NetPod &) = delete;

    public:
        NetPod(NetPod &&stLockGuard)
        {
            m_Object = stLockGuard.m_Object;
            m_Locked = stLockGuard.m_Locked;

            stLockGuard.m_Object = nullptr;
            stLockGuard.m_Locked = false;
        }

    public:
        // then it can be released publically
        ~NetPod()
        {
            if(m_Locked && m_){
                m_Object->Unlock();
            }
        }

    protected:
        Session *m_Session;
        bool     m_Locked;

    public:
        void *operator new(size_t) = delete;
        void operator delete(void *) = delete;
        
        NetPod &operator = (NetPod stObject)
        {
            std::swap(m_Object, stObject.m_Object);
            std::swap(m_Locked, stObject.m_Locked);

            return *this;
        }

    public:
        // check whether it contains a valid object pointer
        operator bool() const
        {
            return m_Session != nullptr;
        }

        template<typename... Args> void Send(Args args...)
        {
            if(m_Session){
                m_Session->Send(std::forward<Args>(args)...);
            }
        }

        bool Locked()
        {
            return m_Locked;
        }
};
