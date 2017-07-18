/*
 * =====================================================================================
 *
 *       Filename: inndb.hpp
 *        Created: 02/26/2016 21:48:43
 *  Last Modified: 07/17/2017 18:20:04
 *
 *    Description: base of all Int->Tex map cache
 *
 *                 Internal Database support for 
 *                 1. FIFO
 *                 2. Double level cache
 *                 3. Easy for extension
 *
 *                 this class load resources with a external handler function
 *                 store it properly in a cache, additional linear cache is optional
 *
 *                 to instantiation this class
 *                 1. define LoadResource()
 *                 2. define FreeResource()
 *
 *                 I don't make a virtual API for Load(const char *) since if we have
 *                 a Load(), we may also need a Valid(), and RAII may also make this 
 *                 unnecessary
 *
 *                 I'm also not 100% sure of my logic, so I added a lot of comments
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
#include <utility>
#include <tuple>
#include <queue>
#include <unordered_map>
#include "cachequeue.hpp"
#include <functional>

// because for PNGTexOffDB, the offset and texture are both important
// otherwise I could only put a ``SDL_Texture *" here instead of ResT
//
// and for FontexDB, the zip archieve is not for texture but for font
// file instead, so I have to move stuff for libzip out of this file

template<typename KeyT, // key type, can only be unsigned intergal, so no reference is needed
    typename ResT,      // for res, see above why I don't use ``SDL_Texture *" directly
    size_t LCDeepN, size_t LCLenN, size_t ResMaxN>
class InnDB
{
    private:
        // linear cache
        std::array<CacheQueue<std::tuple<ResT, unsigned long, KeyT>, LCDeepN>, LCLenN> m_LCache;

        // main cache
        std::unordered_map<KeyT, std::pair<unsigned long, ResT>> m_Cache;

        // time stamp queue
        std::queue<std::tuple<unsigned long, KeyT>>  m_TimeStampQ;

        size_t        m_ResourceMaxCount;
        size_t        m_ResourceCount;
        unsigned long m_CurrentTime;

    public:
        InnDB()
            : m_ResourceMaxCount(ResMaxN)
            , m_ResourceCount(0)
            , m_CurrentTime(0)
        {
            static_assert(std::is_unsigned<KeyT>::value, "unsigned intergal type supported only please");
            static_assert(ResMaxN > LCDeepN * LCLenN, "maximal resource count must be larger than linear cache capacity please");
            static_assert(LCLenN < 8192, "don't set linear cache size to be too large please");
            static_assert(LCDeepN < 16, "don't set linear cache depth to be too large please");
        }

        virtual ~InnDB()
        {
            ClearCache();
        }

    public:

        // dtor will call ClearCache(), which can't take parameters, and no functioin handler
        // so define FreeResource() pure virtual is necessary
        //
        // function handler or pure virtual function for LoadResource() doesn't mater
        // we define it as pure virtual for conformming with FreeResource()
        //
        virtual ResT LoadResource(KeyT)   = 0;
        virtual void FreeResource(ResT &) = 0;

        void ClearLC()
        {
            if(UseLC()){
                for(auto &stQN: m_LCache){
                    // just clean it
                    // don't need FreeResource() here
                    stQN.Clear();
                }
            }
        }

        void ClearCache()
        {
            ClearLC();
            for(auto &stRecord: m_Cache){
                // TODO
                // important to make it pure virtual for derived class to define it
                FreeResource(stRecord.second.second);
            }
            m_Cache.clear();
        }

    public:

        bool UseLC() const
        {
            return LCLenN > 0 && LCDeepN > 0;
        }

        // internal retrieve function, for derived class use only
        // when retrieved successfully:
        //      when there is LC enabled, m_LCache[*pLCBucketIndex].Head() is the current result
        //      when no LC, nothing in nLCBucketIndex
        //
        // this function always return true, the pResource always return
        // a resource desc even actually it's null to prevent repeatly call LoadResource()
        //
        // outside logic will handle this, which make this null resource desc in LCache
        // sink into m_Cache quickly
        //
        //
        // caller should define the fnLinearCacheKey() function
        // we don't make LinearCacheKey() pure virtual since when UseLC() is false, we don't need 
        // it, if it pure virtual we have to define an unused function which is annoying
        //
        // parameters:
        //
        // 1. key, can only be builtin unsigned type, so no need to be const reference
        //
        // 2. return pointer, since ResT may be large
        //    but caller should never store this pointer since it may release/reload internally
        //    call should consume it before next retrieve
        //
        // 3. calculate the LCache location
        //
        // 4. may be useful, if not used just subsititute it as nullptr
        //
        bool InnRetrieve(KeyT nKey, ResT *pResource,
                const std::function<size_t(KeyT)> &fnLinearCacheKey,
                size_t *pLCBucketIndex)
        {
            // everytime when call this function
            // there must be a resource retrieved responding to nKey
            // even if it's null, so update time stamp at each time
            //
            // overflow will be handled
            m_CurrentTime++;

            // if linear cache is enabled
            // then first try to retrieve from linear cache
            //
            size_t nLCacheKey;
            size_t nLocationInLC;

            if(UseLC()){

                nLCacheKey = fnLinearCacheKey(nKey);

                if(LocateInLinearCache(nLCacheKey, nKey, &nLocationInLC)){
                    // find resource in LC, good!
                    //
                    // 1. move the record in LC to its head
                    m_LCache[nLCacheKey].SwapHead(nLocationInLC);

                    // 2. update access time stamp in LC *only*
                    //    *important*:
                    //    didn't update the access time in hash map Cache!
                    std::get<1>(m_LCache[nLCacheKey].Head()) = m_CurrentTime;

                    // 3. push the access stamp at the end of the time stamp queue
                    m_TimeStampQ.emplace(m_CurrentTime, nKey);

                    // 4. return the bucket index
                    if(pLCBucketIndex){
                        *pLCBucketIndex = nLocationInLC;
                    }

                    // 5. return the resource
                    if(pResource){
                        *pResource = std::get<0>(m_LCache[nLCacheKey].Head());
                    }

                    return true;
                }
            }

            // there is no LCache or failed to find it in LCache
            // try to retrieve from m_Cache
            //
            auto pTextureInst = m_Cache.find(nKey);
            if(pTextureInst != m_Cache.end()){
                // we find it in m_Cache, OK...

                // 1. Put a record in LC if necessary
                //
                //    but when LC is full, insertion of a record causes to drop another
                //    old record, we need to update the resource access time w.r.t the 
                //    to-drop record, since whenever there is a record in LC, access time
                //    in m_Cache won't be updated.
                if(UseLC()){

                    // since the old record is in LC, it must exist in m_Cache
                    // update its time stamp
                    //
                    if(m_LCache[nLCacheKey].Full()){
                        m_Cache[std::get<2>(m_LCache[nLCacheKey].Back())].first // in hashmap and LC
                            = std::get<1>(m_LCache[nLCacheKey].Back());         // update it's time stamp
                    }

                    // set pLCBucketIndex if necessary
                    if(pLCBucketIndex){
                        *pLCBucketIndex = nLCacheKey;
                    }

                    // now insert the record to LC
                    // if it's full, the back() is overwrited as new head
                    //
                    m_LCache[nLCacheKey].PushHead(pTextureInst->second.second, m_CurrentTime, nKey);
                }

                // 2. set access time in m_Cache
                //
                pTextureInst->second.first = m_CurrentTime;

                // 3. push the access stamp at the end of the time stamp queue
                //
                m_TimeStampQ.emplace(m_CurrentTime, nKey);

                // 4. return the resource pointer
                if(pResource){
                    *pResource = pTextureInst->second.second;
                }

                return true;
            }

            // it's not in m_Cache, too bad ...
            // we need to load it from the hard driver
            //
            // use the pure virtual function
            // always return a resource desc even actually it's null
            // to prevent repeatly call LoadResource()
            // outside logic will handle this, which make this null resource desc
            // sink into m_Cache quickly
            //
            ResT stResource = LoadResource(nKey);

            // 1. Put a record in LC if necessary
            //    same here, we need to handle when LC is full in one box
            //
            if(UseLC()){

                // since the old record is in LC, it must exist in m_Cache
                // update its time stamp
                //
                if(m_LCache[nLCacheKey].Full()){
                    m_Cache[std::get<2>(m_LCache[nLCacheKey].Back())].first
                        = std::get<1>(m_LCache[nLCacheKey].Back());
                }

                // set pLCBucketIndex if necessary
                if(pLCBucketIndex){
                    *pLCBucketIndex = nLCacheKey;
                }

                // now insert the record to LC
                m_LCache[nLCacheKey].PushHead(stResource, m_CurrentTime, nKey);
            }

            // 2. put the resource in m_Cache
            //
            m_Cache[nKey] = {m_CurrentTime, stResource};

            // 3. push the access stamp at the end of the time stamp queue
            //
            m_TimeStampQ.emplace(m_CurrentTime, nKey);

            // 4. reset the size of the cache
            // 
            m_ResourceCount++;
            Resize(fnLinearCacheKey);

            // 5. return the resource pointer
            if(pResource){
                *pResource = stResource;
            }

            // this function always return true
            return true;
        }

    private:

        // keep the size under m_ResourceMaxCount
        void Resize(const std::function<size_t(KeyT)> &fnLinearCacheKey)
        {
            while(m_ResourceCount > m_ResourceMaxCount){
                // won't happen actually
                if(m_TimeStampQ.empty()){ break; }

                // may or may not release one resource
                ReleaseResource(std::get<1>(m_TimeStampQ.front()),
                        std::get<0>(m_TimeStampQ.front()), fnLinearCacheKey); 
                // always pop front by one after trid to release resource
                m_TimeStampQ.pop();
            }
        }

        // when calling this function, the nKey
        //
        // 1. shouldn't be in linear cache
        //
        // 2. it must be in m_Cache, only exception is m_CurrentTime overflowed
        //      a) when there is no overflow, everytime when free a resource, we compare the time
        //         if the time in m_Cache != time with the record in the queue, means there is 
        //         accessing later, so we keep it. other wise we free it.
        //
        //      b) when there is overflow, think about this scenario:
        //         current node in time stamp queue is at time k, but the last accessing time of the
        //         resource is at (k + max) = k, then it's equal, and we release it incorrectly
        //         
        void ReleaseResourceByTimeStamp(KeyT nKey, unsigned long nTimeStamp)
        {
            auto pResInst = m_Cache.find(nKey);
            // to handle the overflow problem, otherwise we don't need this check
            if(pResInst != m_Cache.end()){
                if(pResInst->second.first == nTimeStamp){
                    FreeResource(pResInst->second.second);
                    m_Cache.erase(pResInst);

                    m_ResourceCount--;
                }
            }
        }

        void ReleaseResource(KeyT nKey, unsigned long nTimeStamp, const std::function<size_t(KeyT)> &fnLinearCacheKey)
        {
            // first make sure that the resource is not in LC
            // return directly if yes
            if(UseLC() && LocateInLinearCache(fnLinearCacheKey(nKey), nKey, nullptr)){ return; }

            // not in LC, release resource if (most likely) in the cache
            ReleaseResourceByTimeStamp(nKey, nTimeStamp);
        }

        // assume UseLC() is true, parameters should be checked before invocation
        bool LocateInLinearCache(int nLCKey, KeyT nKey, size_t *pLocationInLC)
        {
            for(m_LCache[nLCKey].Reset(); !m_LCache[nLCKey].Done(); m_LCache[nLCKey].Forward()){
                if(std::get<2>(m_LCache[nLCKey].Current()) == nKey){
                    if(pLocationInLC){
                        *pLocationInLC = m_LCache[nLCKey].Index();
                    }
                    return true;
                }
            }
            return false;
        }
};
