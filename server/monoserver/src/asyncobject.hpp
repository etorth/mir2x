/*
 * =====================================================================================
 *
 *       Filename: asyncobject.hpp
 *        Created: 04/09/2016 00:20:22
 *  Last Modified: 04/10/2016 02:25:21
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

class AsyncObject
{
    protected:
        std::mutex *m_Lock;

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
};

enum ObjectType: uint8_t{
    OT_CHAROBJECT,
    OT_ITEMOBJECT,
    OT_EVENTOBJECT,
};

typedef struct{
    uint32_t UID;
    uint32_t AddTime;
}OBJECTRECORD;
