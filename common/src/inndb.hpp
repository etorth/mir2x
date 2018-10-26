/*
 * =====================================================================================
 *
 *       Filename: inndb.hpp
 *        Created: 02/26/2016 21:48:43
 *    Description: Basic class of all integral based map cache
 *
 *                 Internal Database support for 
 *                 1. LRU
 *                 2. Double level cache
 *                 3. Easy for extension
 *
 *                 this class load resources with a external handler function
 *                 store it in a hash-table based cache, linear cache is optional
 *
 *                 to instantiation this class
 *                 1. define LoadResource()
 *                 2. define FreeResource()
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
#ifndef __INNDB_HPP__
#define __INNDB_HPP__

#include "abdlist.hpp"
#include <tuple>
#include <unordered_map>

template<typename KeyT, typename ResT, size_t ResMax> class InnDB
{
    private:
        struct ResourceEntry
        {
            ResT    Resource;
            size_t  Weight;
            size_t  DLinkOff;
        };

    private:
        std::unordered_map<KeyT, ResourceEntry> m_Cache;

    private:
        ABDList<KeyT> m_DLink;

    private:
        size_t m_ResourceSum;

    public:
        InnDB()
            : m_Cache()
            , m_DLink()
            , m_ResourceSum(0)
        {
            static_assert(std::is_unsigned<KeyT>::value, "InnDB only support unsigned intergal key");
        }

    public:
        virtual ~InnDB() = default;

    public:
        virtual std::tuple<ResT, size_t> LoadResource(KeyT  ) = 0;
        virtual void                     FreeResource(ResT &) = 0;

    public:
        void ClearCache()
        {
            for(auto &rstEntry: m_Cache){
                FreeResource(rstEntry.second.Resource);
            }
            m_Cache.clear();
            m_DLink.Clear();
        }

    protected:
        bool RetrieveResource(KeyT nKey, ResT *pResource)
        {
            if(auto p = m_Cache.find(nKey); p != m_Cache.end()){
                if(pResource){
                    *pResource = p->second.Resource;
                }
                if(ResMax > 0){
                    m_DLink.MoveHead(p->second.DLinkOff);
                }
                return true;
            }

            auto [stResource, nWeight] = LoadResource(nKey);

            if(ResMax){
                m_DLink.PushHead(nKey);
            }

            m_Cache[nKey].Resource = stResource;
            m_Cache[nKey].Weight   = nWeight;
            m_Cache[nKey].DLinkOff = (ResMax > 0) ? m_DLink.HeadOff() : 0;

            if(ResMax){
                m_ResourceSum += nWeight;
                if(m_ResourceSum > ResMax){
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
            while((ResMax > 0) && (m_ResourceSum > ResMax / 2) && !m_DLink.Empty()){
                if(auto p = m_Cache.find(m_DLink.Back()); p != m_Cache.end()){
                    FreeResource(p->second.Resource);
                    m_ResourceSum -= p->second.Weight;
                }
                m_DLink.PopBack();
            }
        }
};

#endif
