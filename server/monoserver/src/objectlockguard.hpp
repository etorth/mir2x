/*
 * =====================================================================================
 *
 *       Filename: objectlockguard.hpp
 *        Created: 04/09/2016 00:20:22
 *  Last Modified: 04/10/2016 18:43:28
 *
 *    Description: Previously I was trying to implement AsyncObject with a mutex
 *                 and let all objects derived from it
 *
 *                 but standard says delete a locked mutex is undefined, then just
 *                 make Lock()/Unlock() API for this class, and use define how to
 *                 do lock/unlock by themself
 *
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

#pragma once
// #include "sfinaecheck.hpp"

// GENERATE_HAS_MEMBER(Lock)
// GENERATE_HAS_MEMBER(Unlock)

// here T can't be class with ``final" qualifier
// otherwise the has_member_xxx won't work
//
//
// Any function directly take m_Object without lock it is un-safe
// return pointer can be invalidate at any time
//
// Thus when take a ObjectLockGuard with bLockIt == false, the result is just for tip
// program rely on its pointer is ill-formed
//
// template<typename T, bool CheckFlag = (has_member_Lock<T>::value && has_member_Unlock<T>::value)>
// template<typename T, bool CheckFlag = (has_member_Unlock<T>::value)>
template<typename T>
class ObjectLockGuard final
{
    public:
        friend class MonoServer;

    private:
        // constructor, only MonoServer can create it from naked pointer
        //
        // pObject is locked means:
        //      1. no thread other can modify it
        //      2. no thread other can remove it from the hub
        // In one word, it always remains valid if locked
        /* explicit */ ObjectLockGuard(T *pObject, bool bLockIt)
            : m_Object(pObject)
            , m_Locked(bLockIt)
        {
            // if bLockIt is true, then pObject is already locked
            // otherwise error occurs when pObject is modified during the invocation of this ctor
        }

        // nobody can copy it, otherwise one can unlock a object many times
        ObjectLockGuard(const ObjectLockGuard &) = delete;

    public:
        ObjectLockGuard(ObjectLockGuard &&stLockGuard)
        {
            m_Object = stLockGuard.m_Object;
            m_Locked = stLockGuard.m_Locked;

            stLockGuard.m_Object = nullptr;
            stLockGuard.m_Locked = false;
        }

    public:
        // then it can be released publically
        ~ObjectLockGuard()
        {
            if(m_Locked && m_Object){
                m_Object->Unlock();
            }
        }

    protected:
        T      *m_Object;
        bool    m_Locked;

    public:
        // nobody can create it on the heap, even MonoServer can't
        void *operator new(size_t) = delete;
        void operator delete(void *) = delete;
        
        ObjectLockGuard &operator = (ObjectLockGuard stObject)
        {
            std::swap(m_Object, stObject.m_Object);
            std::swap(m_Locked, stObject.m_Locked);

            return *this;
        }

    public:
        // check whether it contains a valid object pointer
        operator bool() const
        {
            return m_Object != nullptr;
        }

        // have to return this raw pointer
        T *operator -> ()
        {
            return m_Object;
        }

        // this may cause undefined behavior, but it's ok
        T &operator * ()
        {
            return *m_Object;
        }

        T *Get()
        {
            return m_Object;
        }

        bool Locked()
        {
            return m_Locked;
        }
};
