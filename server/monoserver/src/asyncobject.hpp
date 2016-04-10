/*
 * =====================================================================================
 *
 *       Filename: asyncobject.hpp
 *        Created: 04/09/2016 00:20:22
 *  Last Modified: 04/09/2016 19:19:17
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
    public:
        // virtual void Lock()   = 0;
        virtual void Unlock() = 0;

        // TBD & TODO
        // I am not sure whether I need Lock() and TryLock() here
        //
        // if use p->TryLock() or p->Lock(), then p is valid and locked for sure
        // otherwise this code is ill-performed, since p can be invalidated at any time
};
