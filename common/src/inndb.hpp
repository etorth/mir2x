/*
 * =====================================================================================
 *
 *       Filename: inndb.hpp
 *        Created: 02/26/2016 21:48:43
 *    Description: Basic class of all integral based map cache
 *
 *                 Internal Database support for 
 *                 1. LRU
 *                 2. Easy for extension
 *
 *                 this class load resources with a external handler function
 *                 store it in a hash-table based cache
 *
 *                 to instantiation this class
 *                 1. define loadResource()
 *                 2. define freeResource()
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
#include "abdlist.hpp"
#include <tuple>
#include <unordered_map>

template<typename KeyT, typename ResT> class innDB
{
    private:
        struct ResourceEntry
        {
            ResT    Resource;
            size_t  Weight;
            size_t  DLinkOff;
        };

    private:
        std::unordered_map<KeyT, ResourceEntry> m_cache;

    private:
        ABDList<KeyT> m_DLink;

    private:
        size_t m_resSum;

    private:
        const size_t m_resMax;

    public:
        innDB(size_t nResMax)
            : m_resSum(0)
            , m_resMax(nResMax)
        {
            static_assert(std::is_unsigned<KeyT>::value, "innDB only support unsigned intergal key");
        }

    public:
        virtual ~innDB() = default;

    public:
        virtual std::tuple<ResT, size_t> loadResource(KeyT  ) = 0;
        virtual void                     freeResource(ResT &) = 0;

    public:
        void ClearCache()
        {
            for(auto &rstEntry: m_cache){
                freeResource(rstEntry.second.Resource);
            }
            m_cache.clear();
            m_DLink.Clear();
        }

    protected:
        bool RetrieveResource(KeyT nKey, ResT *pResource)
        {
            if(auto p = m_cache.find(nKey); p != m_cache.end()){
                if(pResource){
                    *pResource = p->second.Resource;
                }
                if(m_resMax > 0){
                    m_DLink.MoveHead(p->second.DLinkOff);
                }
                return true;
            }

            auto [stResource, nWeight] = loadResource(nKey);

            if(m_resMax){
                m_DLink.PushHead(nKey);
            }

            ResourceEntry stEntry;
            stEntry.Resource = stResource;
            stEntry.Weight   = nWeight;
            stEntry.DLinkOff = (m_resMax > 0) ? m_DLink.HeadOff() : 0;

            m_cache[nKey] = stEntry;

            if(m_resMax){
                m_resSum += nWeight;
                if(m_resSum > m_resMax){
                    Resize();
                }
            }

            if(pResource){
                *pResource = stResource;
            }
            return true;
        }

    private:
        void Resize()
        {
            while((m_resMax > 0) && (m_resSum > m_resMax / 2) && !m_DLink.Empty()){
                if(auto p = m_cache.find(m_DLink.Back()); p != m_cache.end()){
                    freeResource(p->second.Resource);
                    m_resSum -= p->second.Weight;
                    m_cache.erase(p);
                }
                m_DLink.PopBack();
            }
        }
};
