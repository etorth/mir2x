/*
 * =====================================================================================
 *
 *       Filename: asynchub.hpp
 *        Created: 04/09/2016 20:51:17
 *  Last Modified: 04/13/2016 20:25:03
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
 *                  5. object hub has no concept of object life circle, in hub the
 *                     object is valid or not only for the memory purpose
 *
 *                  6. inserting object into hub only by pointer, delete object
 *                     out of hub only by ID
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
#include "objectlockguard.hpp"

// #include "sfinaecheck.hpp"
// GENERATE_HAS_MEMBER(BindLock)

// 1. the mutex
// 2. the res pointer
// 3. the handler of how to clear the handler
using ResKeyType = uint64_t;
template<typename ResType>
using AsyncResType = std::tuple<
    std::shared_ptr<std::mutex>, ResType *, std::function<void(ResType *)>>;

// template<typename T, bool CheckBindLock = (has_member_BindLock<T>::value)>
template<typename T>
class AsyncHub
{
    private:
        std::mutex  m_Lock;
        std::unordered_map<ResKeyType, AsyncResType<T>> m_ObjectHub;

    public:
        // AsyncHub()
        // {
        //     static_assert(CheckBindLock,
        //             "AsyncHub request memeber function BindLock() for template argument type");
        // }
    public:

        // give the ownship fully to the hub, from then on, nobody except this hub can this object.
        //  1. input pointer should be non-null, unlocked
        //  2. input key should be unique, caller should maintain the uniqueness
        //  3. this function **won't** lock the object
        //
        // this function won't and can't check whether the object is unlocked, if passing a locked
        // pointer, behavior is undefined, return false if
        //  1. null resource pointer
        //  2. key is already used
        //      here if the key is already taken, we can lock the item w.r.t the
        //      key, and then overwrite it, but it's not necessary. since we use
        //      time as key to make sure we can tell the difference of two item
        //      with the same UID. so just return error
        //  3. throw if allocatioin for std::shared_ptr<std::mutex> failed
        //
        // object hub has no concept of object life circle.
        // this is the only interface of inserting an new object to the hub, since now hub has
        // the fully ownship of the res, we use fnDelete to define how to clear this res, but
        // it's invoked by the hub only
        //
        bool Add(ResKeyType nKey, T *pRes, 
                const std::function<void(T *)> fnDelete = [](T *pRes){ delete pRes; })
        {
            if(!pRes){ return false; }
            std::lock_guard<std::mutex> stLockGuard(m_Lock);

            auto stInst = m_ObjectHub.find(nKey);
            if(stInst != m_ObjectHub.end()){ return false; }

            auto pLock = std::make_shared<std::mutex>();
            m_ObjectHub[nKey] = {pLock, pRes, fnDelete};

            // here we require the async resource type define a BindLock() method
            return pRes->BindLock(pLock.get());
        }

        // this is the only interface to remove an object out of the hub
        // input nKey maybe invalid
        // after this, guanteed there is no item w.r.t nKey
        void Remove(ResKeyType nKey)
        {
            std::lock_guard<std::mutex> stLockGuard(m_Lock);

            auto stInst = m_ObjectHub.find(nKey);
            if(stInst == m_ObjectHub.end()){ return; }

            {
                std::lock_guard<std::mutex> stItemLockGuard(*std::get<0>(stInst.second));
                try{
                    (std::get<2>(stInst.second))(std::get<1>(stInst.second));
                }catch(...){
                    extern Log *g_Log;
                    g_Log->AddLog(LOGTYPE_WARNING,
                            "unexpected exception in clear handler for object ID = %d", nKey);
                }
            }

            // the mutex is owned by shared_ptr, no need for explicitly delete
            m_ObjectHub.erase(stInst);
        }

        // return the naked pointer of the object
        // object is locked that any other thread can't grub it if bLockIt is true
        T *Retrieve(ResKeyType nKey, bool bLockIt = true)
        {
            std::lock_guard<std::mutex> stLockGuard(m_Lock);

            auto stInst = m_ObjectHub.find(nKey);
            if(stInst == m_ObjectHub.end()){ return nullptr; }

            if(bLockIt){
                std::get<0>(stInst->second)->lock();
            }
            return std::get<1>(stInst->second);
        }

        template<bool bLockIt> ObjectLockGuard<T> CheckOut(ResKeyType nKey)
        {
            return {Retrieve(nKey, bLockIt), bLockIt};
        }
};
