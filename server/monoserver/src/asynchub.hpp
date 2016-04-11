/*
 * =====================================================================================
 *
 *       Filename: asynchub.hpp
 *        Created: 04/09/2016 20:51:17
 *  Last Modified: 04/10/2016 18:54:16
 *
 *    Description: An async hub is a global container which stays valid during the
 *                 whole process, but item inside may be created/released.
 *
 *                 The item inside is pointer to the resource. The resource may be
 *                 un-copyable, but it's pointer is always well-defined for copy.
 *                 so by this the underlying container resizing is transparent for
 *                 the resource.
 *
 *                 general rule:
 *
 *                  1. resource item could be delete at any time, asynchronously.
 *
 *                  2. retrieve item only by integral key, aka ID, if we return a
 *                     naked pointer of the item, then this pointer is guanteed
 *                     to stay valid untill it's un-locked.
 *
 *                  3. operation over item p via p->operation() need the assumption
 *                     that p is locked. but hub may be un-locked when calling
 *                     p->operation(). if p->operation() affects the hub, it need 
 *                     to lock the hub.
 *
 *                  4. hub maybe rehashed, resized, or even convert to another kind
 *                     of container, it should be transparent to the resource 
 *                     ponter.
 *
 *                  opreation defined:
 *                      1. when retrieve a pointer by ID from the hub
 *                          1. lock the hub
 *                          2. find the item by ID
 *                          3. lock the item
 *                          4. unlock the hub
 *                          5. return the locked item pointer
 *
 *                      2. when insert a new item:
 *                          1. lock the hub
 *                          2. create mutex for the item
 *                          3. make a record in the container
 *                          4. unlock the hub
 *                          4. return succeed
 *                      5. when delete an item by ID
 *                          1. lock the hub
 *                          2. find the item by id, lock it and delete it if succeed
 *                          3. release the mutex, delete the mutex
 *                          4. unlock the hub
 *
 *                  always-true:
 *                      1. an item always has one and  only one mutex, vise versa.
 *                      2. we can find the mutex through the item pointer, or through
 *                         the ID of the item.
 *                      3. we can only find object through its ID.
 *                      4. retrieving object through the mutex is not required.
 *                      5. the hub is the final container of the resource, if an item
 *                         is allocated but can't be found in the hub, it's resource
 *                         leakage.
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
#include <mutex>

// #include "sfinaecheck.hpp"
// GENERATE_HAS_MEMBER(BindLock)

template<typename ResType>
using AsyncResType = std::tuple<std::shared_ptr<std::mutex>, ResType *>;

// template<typename T, bool CheckBindLock = (has_member_BindLock<T>::value)>
template<typename T>
class AsyncHub
{
    private:
        std::mutex  m_Lock;
        std::unordered_map<uint64_t, AsyncResType<T>> m_ObjectHub;

    public:
        // AsyncHub()
        // {
        //     static_assert(CheckBindLock,
        //             "AsyncHub request memeber function BindLock() for template argument type");
        // }
    public:

        // return false if
        //  1. null resource pointer
        //  2. key is already used
        //      here if the key is already taken, we can lock the item w.r.t the
        //      key, and then overwrite it, but it's not necessary. since we use
        //      time as key to make sure we can tell the difference of two item
        //      with the same UID. so just return error
        //  3. throw error when memory allocatioin failed
        bool Add(uint64_t nKey, T *pRes)
        {
            if(!pRes){ return false; }
            std::lock_guard<std::mutex> stLockGuard(m_Lock);

            auto stInst = m_ObjectHub.find(nKey);
            if(stInst != m_ObjectHub.end()){ return false; }

            auto pLock = new std::mutex;
            m_ObjectHub[nKey] = {std::make_shared<std::mutex>(), pRes};

            // here we require the async resource type define a BindLock() method
            pRes->BindLock(pLock);
            return true;
        }

        // nKey maybe invalid
        // after this, guanteed there is no item w.r.t nKey
        void Remove(uint64_t nKey,
                const std::function<void(T *)> fnDelete = [](T *pRes){ delete pRes; })
        {
            std::lock_guard<std::mutex> stLockGuard(m_Lock);

            auto stInst = m_ObjectHub.find(nKey);
            if(stInst == m_ObjectHub.end()){ return; }

            {
                std::lock_guard<std::mutex> stItemLockGuard(*std::get<0>(stInst.second));
                fnDelete(std::get<1>(stInst.second));
            }

            // the mutex is owned by shared_ptr, no need for explicitly delete
            m_ObjectHub.erase(stInst);
        }

        T *Retrieve(uint64_t nKey, bool bLockIt = true)
        {
            std::lock_guard<std::mutex> stLockGuard(m_Lock);

            auto stInst = m_ObjectHub.find(nKey);
            if(stInst == m_ObjectHub.end()){ return nullptr; }

            if(bLockIt){
                std::get<0>(stInst->second)->lock();
            }
            return std::get<1>(stInst->second);
        }
};
