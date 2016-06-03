/*
 * =====================================================================================
 *
 *       Filename: monsterginfo.hpp
 *        Created: 06/02/2016 15:08:56
 *  Last Modified: 06/02/2016 17:38:36
 *
 *    Description: monster global info
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
#include "sysconst.hpp"

class MonsterGInfo
{
    protected:
        enum QueryType: int{
            QUERY_NA,
            QUERY_OK,
            QUERY_ERROR,
            QUERY_PENDING,
        };

    protected:
        typedef struct _InnNetData{
            int Query;  // query the server
            int R;
            uint32_t LookIDV[4];

            _InnNetData()
                : Query(QUERY_NA)
            {}

            bool Valid()
            {
                // all set and we can use this record immediately
                return Query == QUERY_OK
                    && R >= 0                  // TODO
                    && LookIDV[0] < 0X1000     // no harm to do more check here
                    && LookIDV[1] < 0X1000     // maybe for release version I can remove it
                    && LookIDV[2] < 0X1000
                    && LookIDV[3] < 0X1000;
            }
        }InnNetData;

        typedef struct _InnCacheData{
            int Query;          // query the local database
            size_t FrameCount[16][8];

            _InnCacheData()
                : Query(QUERY_NA)
            {}

            bool Valid()
            {
                return Query == QUERY_OK;
            }

            size_t FrameCount(uint32_t nState, uint32_t nDirection)
            {
                // 1. check parameter, for monster:
                //      1. it can have up to 16 states (00 ~ 15) while human can have up to xxx states
                //      2. it can have up to 8 directions (0 ~7)
                if(nState >= 16 || nDirection >= 8){ return 0; }

                // 2. check cache
                if(Valid()){ return FrameCount[nState][nDirection]; }

                // 3. no cache, so we have to retrieve it, internal logic check
                if(m_LookIDV[nLookIDIndex] > 0X00000FFF){ return 0; }

                // 2. create retrieving key, using body frame count
                //    0X FF FF FF FF
                //       ^^ ^^ ^^ ^^
                //       || || || ||
                //       || || || ++---+ bit 7 ~ 5 for direction, bit 4 ~ 0 for frame, up to 32 frames
                //       || || |+------+ for state, up to 16 different states for monster
                //       || ++-+-------+ for monster id, upto 4096 monster look effects
                //       ++------------+ 00 for monster body ID
                //

                uint32_t nKey = (m_LookIDV[nLookIDIndex]<< 12) + (nState << 8) + (nDirection << 5);
                extern PNGTexOffDBN *g_PNGTexOffDBN;

                // TODO
                // this code is based on the assumption
                // 1. there should be a empty record between different animations
                // 2. 
                m_CacheData.FrameCount[nLookIDIndex] = 0;
                while(g_PNGTexOffDBN->Retrieve(nKey++, nullptr, nullptr)){
                    m_CacheData.FrameCount[nLookIDIndex]++;
                }

                return (size_t)(m_CacheData.FrameCount[nLookIDIndex]);
            }
        }InnCacheData;

    protected:
        uint32_t     m_MonsterID;
        InnNetData   m_NetData;
        InnCacheData m_CacheData;


    public:
        MonsterGInfo()
            : m_MonsterID(0)
        {}

        MonsterGInfo(
                uint32_t nMonsterID,
                uint32_t nLookID0,          // default look id, 0X0000 ~ 0X0FFF
                uint32_t nLookID1,          // 1
                uint32_t nLookID2,          // 2
                uint32_t nLookID3,          // 3
                uint32_t nLookID0,
                uint32_t nR                // size
                )
            : m_MonsterID(nMonsterID)
              , m_LookIDV {nLookID0, nLookID1, nLookID2, nLookID3}
        , m_R(nR)
        {}

        size_t FrameCount(uint32_t nLookIDIndex, uint32_t nState, uint32_t nDirection)
        {
            // 1. check parameter, for monster:
            //      1. it can own 4 different look effect
            //      2. it can have up to 16 states (00 ~ 15) while human can have up to xxx states
            //      3. it can have up to 8 directions (0 ~7)
            if(nLookIDIndex >= 4 || nState >= 16 || nDirection >= 8){ return 0; }

            // 2. check cache
            if(m_CacheData.FrameCount[nLookIDIndex] >= 0){
                return (size_t)(m_CacheData.FrameCount[nLookIDIndex]);
            }

            // 3. no cache, so we have to retrieve it, internal logic check
            if(m_LookIDV[nLookIDIndex] > 0X00000FFF){ return 0; }

            // 2. create retrieving key, using body frame count
            //    0X FF FF FF FF
            //       ^^ ^^ ^^ ^^
            //       || || || ||
            //       || || || ++---+ bit 7 ~ 5 for direction, bit 4 ~ 0 for frame, up to 32 frames
            //       || || |+------+ for state, up to 16 different states for monster
            //       || ++-+-------+ for monster id, upto 4096 monster look effects
            //       ++------------+ 00 for monster body ID
            //

            uint32_t nKey = (m_LookIDV[nLookIDIndex]<< 12) + (nState << 8) + (nDirection << 5);
            extern PNGTexOffDBN *g_PNGTexOffDBN;

            // TODO
            // this code is based on the assumption
            // 1. there should be a empty record between different animations
            // 2. 
            m_CacheData.FrameCount[nLookIDIndex] = 0;
            while(g_PNGTexOffDBN->Retrieve(nKey++, nullptr, nullptr)){
                m_CacheData.FrameCount[nLookIDIndex]++;
            }

            return (size_t)(m_CacheData.FrameCount[nLookIDIndex]);
        }

    protected:
};
