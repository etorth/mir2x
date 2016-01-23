/*
 * =====================================================================================
 *
 *       Filename: sourcemanager.hpp
 *        Created: 01/13/2016 08:20:29
 *  Last Modified: 01/14/2016 00:05:27
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

template<typename SourceKeyType, typename SourceType, size_t SourceCount = 100>
class SourceManager
{
    public:
        SourceManager()
        {
            std::static_assert(SourceCount > 0, "Invalid template arguments");
            m_SourceMaxCount = SourceCount;
            m_CurrentTime    = 0;
        }
        
        ~SourceManager()
        {
            ClearCache();
        }

    public:
        virtual SourceType *Load(const SourceKeyType &) = 0;
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

    public:
        SourceType &Retrieve(const SourceKeyType &stRetrieveKey)
        {
            ++m_CurrentTime;
            auto stResItor = m_SourceCache.find(stRetrieveKey);
            if(stResItor != m_SourceCache.end()){
                // find it
                stResItor->first = m_CurrentTime;
                m_AccessTimeStampQueue.emplace({m_CurrentTime, stRetrieveKey});
                return stResItor->second;
            }else{
                // didn't find source in cache
                if((size_t)m_SourceCache.size() >= m_SourceMaxCount){
                    // need to release for memory
                    while((size_t)m_SourceCache.size() >= (size_t)(m_SourceMaxCount >> 1)){
                        for(auto &stTimeKeyItor: m_AccessTimeStampQueue){
                            auto stTmpItor = m_SourceCache.find(stTimeKeyItor->second);
                            if(stTmpItor != m_SourceCache.end() && stTmpItor->first == stTimeKeyItor->first){
                                Release(stTmpItor->second);
                                m_SourceCache.erase(stTmpKey);
                            }
                        }
                        m_AccessTimeStampQueue.pop();
                    }
                }
                SourceType *pSource = Load(stRetrieveKey);
                if(pSource){
                    m_SourceCache[stRetrieveKey] = {nAccessTime, pSource};
                    if(m_AccessTimeStampQueue.back().first == nAccessTime){
                        m_AccessTimeStampQueue.back().second.insert(stRetrieveKey);
                    }else{
                        m_AccessTimeStampQueue.push({nAccessTime, {stRetrieveKey}});
                    }
                }
                return pSource;
            }
        }
};
