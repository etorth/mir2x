/*
 * =====================================================================================
 *
 *       Filename: sourcemanager.hpp
 *        Created: 01/13/2016 08:20:29
 *  Last Modified: 02/06/2016 13:34:27
 *
 *    Description: 
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
#include <cstdint>
#include <type_traits>

template<typename SourceIntKeyType, typename SourceType,
    size_t SourceMaxCount = 100, bool std::is_integral<SourceIntKeyType>::value> class SourceManager;


template<typename SourceIntKeyType, typename SourceType, size_t SourceMaxCount = 100, true>
class SourceManager
{
    public:
        SourceManager()
        {
            std::static_assert(SourceMaxCount > 0, "Invalid template arguments");
            m_SourceMaxCount = SourceCount;
            m_CurrentTime    = 0;
        }
        
        ~SourceManager()
        {
            ClearCache();
        }

    public:
        virtual SourceType *Load(const SourceIntKeyType &) = 0;
        virtual void Release(SourceType *) = 0;

    public:
        void ClearCache()
        {
            for(auto &stRes: m_SourceCache){
                Release(stRes.second);
            }
        }

    private:
        size_t  m_SourceMaxCount;
        int32_t m_CurrentTime;
        
    private:
        std::unordered_map<SourceIntKeyType, SourceType> m_SourceCache;

    public:
        SourceType &Retrieve(const SourceIntKeyType &stRetrieveKey)
        {
            ++m_CurrentTime;
            auto stResItor = m_SourceCache.find(stRetrieveKey);
            if(stResItor != m_SourceCache.end()){
                // find it
                // update access time stamp
                stResItor->first = m_CurrentTime;
                m_AccessTimeStampQueue.push({m_CurrentTime, stRetrieveKey});
                return stResItor->second;
            }else{
                // don't find source in cache
                if((size_t)m_SourceCache.size() > m_SourceMaxCount){
                    // need to release for memory
                    while((size_t)m_SourceCache.size() > (size_t)(m_SourceMaxCount >> 1)){
                        auto &stTimeKeyPair = m_AccessTimeStampQueue.front();
                        auto stTmpItor = m_SourceCache.find(stTimeKeyPair.second);
                        if(stTmpItor != m_SourceCache.end() && stTmpItor->first == stTimeKeyPair.first){
                                Release(stTmpItor->second);
                                m_SourceCache.erase(stTmpItor);
                            }
                        }
                        m_AccessTimeStampQueue.pop();
                    }
                }
                SourceType *pSource = Load(stRetrieveKey);
                if(pSource){
                    m_SourceCache[stRetrieveKey] = {m_CurrentTime, pSource};
                    m_AccessTimeStampQueue.push({m_CurrentTime, stRetrieveKey});
                }
                return pSource;
            }
        }
};
