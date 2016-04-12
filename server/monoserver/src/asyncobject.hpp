/*
 * =====================================================================================
 *
 *       Filename: asyncobject.hpp
 *        Created: 04/09/2016 00:20:22
 *  Last Modified: 04/11/2016 20:54:20
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
#include <memory>
#include <mutex>

// async object itself won't manager a mutex pointer
// In my design, whether an ojbect is valid only depends whether you can find
// it in the object pool via MonoServer.
//
// class MonoServer take power of add/delete object to/from the pool. when
// adding a new object, MonoServer
//      1. allocate the new object
//      2. allocate the mutex
//      3. bind the mutex to the object
//      4. put object/mutex in its internal container
//
// when deleting an object
//      1. find the object and delete it
//      2. delete the mutex bound to it
//      3. remove the record in the container
//
// for outside user, there are only AddObject()/RemoveObject() available, so
// every time when refering to an object, it's always an object-mutex pair.
class AsyncObject
{
    public:
        void Unlock()
        {
            if(m_Lock){
                m_Lock->unlock();
            }
        }

        void BindLock(std::mutex *pLock)
        {
            m_Lock = pLock;
        }

        // TBD & TODO
        // I am not sure whether I need Lock() and TryLock() here
        //
        // if use p->TryLock() or p->Lock(), then p is valid and locked for sure
        // otherwise this code is ill-performed, since p can be invalidated at any time
    protected:
        std::mutex *m_Lock;
};
