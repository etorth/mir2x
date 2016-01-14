/*
 * =====================================================================================
 *
 *       Filename: sourcemanager.hpp
 *        Created: 01/13/2016 08:20:29
 *  Last Modified: 01/13/2016 08:35:10
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
#include "timemanager.hpp"

template<typename SourceKeyType, typename SourceType>
class SourceManager final
{
    private:
        SourceManager();
        ~SourceManager();

    private:

    public:

        const SourceType &Retrieve(SourceKeyType stKey)
        {
            uint32_t nTimeMS = GetTimeManager()->Now();
            if(m_SourceCache.find(stKey) != m_SourceCache.end()){
                m_SourceCache[stKey].first = nTimeMS;
                m_TimeAccessMap[nTimeMS].insert(stKey);
                return m_SourceCache[stKey].second;
            }else{
                if
            }
        }


};
