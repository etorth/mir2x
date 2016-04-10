/*
 * =====================================================================================
 *
 *       Filename: lock.cpp
 *        Created: 04/09/2016 00:49:58
 *  Last Modified: 04/09/2016 11:38:09
 *
 *    Description: functionality for lock/unlock objects
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

#include <cstdint>
#include "monoserver.hpp"

void MonoServer::LockObject(uint32_t nID, uint32_t nAddTime)
{
    if(nID == 0 || nAddTime == 0){ return; }
    {
        std::lock_guard<std::mutex> stLockGuard<m_ObjectVLock>;
        auto p = m_ObjectV.find(nID);

        if(p == m_ObjectV.end()){
            return {nullptr, false};
        }else{
            if(nAddTime != p.second.second){
                return {nullptr, false};
            }
        }

        // ok we get it
        pObject = p.second.first;

        if(!bLock){
            return {pObject, false};

        }

        pObject->Lock();
        return {pObject, true};
    }
}

ObjectLockGuard MonoServer::CheckOut(int nID, int nAddTime, int bLock)
{
}
