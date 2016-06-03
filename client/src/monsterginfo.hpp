/*
 * =====================================================================================
 *
 *       Filename: monsterginfo.hpp
 *        Created: 06/02/2016 15:08:56
 *  Last Modified: 06/02/2016 23:32:14
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
#include "pngtexoffdbn.hpp"

class MonsterGInfo final
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
            uint32_t R;

            _InnNetData()
                : Query(QUERY_NA)
            {}

            bool Valid()
            {
                return Query == QUERY_OK;
            }

            void Reset(uint32_t nR)
            {
                R = nR;
                Query = QUERY_OK;
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
                // TODO didn't check since it's expensive
                return Query == QUERY_OK;
            }

            bool Load(uint32_t nLookID)
            {
                if(nLookID > 0X00000FFF){ return false; }
                // 1. create retrieving key, using body frame count
                //    0X FF FF FF FF
                //       ^^ ^^ ^^ ^^
                //       || || || ||
                //       || || || ++---+ bit 7 ~ 5 for direction, bit 4 ~ 0 for frame, up to 32 frames
                //       || || |+------+ for state, up to 16 different states for monster
                //       || ++-+-------+ for monster id, upto 4096 monster look effects
                //       ++------------+ 00 for monster body ID
                //
                for(uint32_t nState = 0; nState < 16; ++nState){
                    for(uint32_t nDirection = 0; nDirection < 8; ++nDirection){
                        // 1. calculate the key
                        uint32_t nKey = (nLookID << 12) + (nState << 8) + (nDirection << 5);
                        extern PNGTexOffDBN *g_PNGTexOffDBN;

                        // 1. initialization of count, zero is invalid but legal
                        FrameCount[nState][nDirection] = 0;
                        while(g_PNGTexOffDBN->Retrieve(nKey++, nullptr, nullptr)){
                            FrameCount[nState][nDirection]++;
                        }
                    }
                }

                Query = QUERY_OK;
                return true;
            }
        }InnCacheData;

    protected:
        uint32_t     m_MonsterID;
        uint32_t     m_LookIDV[4];

        InnNetData   m_NetData[4];
        InnCacheData m_CacheData[4];

    public:
        MonsterGInfo(uint32_t nMonsterID = 0)
            : m_MonsterID(nMonsterID)
        {}

        ~MonsterGInfo() = default;

    public:
        bool Valid(uint32_t nLIDN, bool bCacheValid = false)
        {
            // 1. parameter check
            if(nLIDN >= 4){ return false; }

            // 2. check the branch
            return m_NetData[nLIDN].Valid()
                && (bCacheValid ? m_CacheData[nLIDN].Valid() : true);
        }

        bool Valid(bool bCacheValid = false)
        {
            return true
                && Valid(0, bCacheValid) && Valid(1, bCacheValid)
                && Valid(2, bCacheValid) && Valid(3, bCacheValid);
        }

    public:
        uint32_t LookID(uint32_t nLIDN)
        {
            return m_LookIDV[nLIDN];
        }

    public:
        template<typename... T> void ResetLookID(uint32_t nLIDN, uint32_t nLookID, T&&... stT)
        {
            // 1. parameter check
            if(nLIDN >= 4 || nLookID >= 0X00001000){ return; }

            // 2. set the LookIDV
            m_LookIDV[nLIDN] = nLookID;

            // 3. set the NetData
            m_NetData[nLIDN].Reset(std::forward<T>(stT)...);
        }

    public:
        size_t FrameCount(uint32_t nLIDN, uint32_t nState, uint32_t nDirection)
        {
            // 1. check parameter, for monster:
            //      1. it can own 4 different look effect
            //      2. it can have up to 16 states (00 ~ 15) while human can have up to xxx states
            //      3. it can have up to 8 directions (0 ~7)
            if(nLIDN >= 4 || nState >= 16 || nDirection >= 8){ return 0; }

            // 2. check cache
            if(!m_CacheData[nLIDN].Valid()){
                m_CacheData[nLIDN].Load(m_LookIDV[nLIDN]);
            }

            // 3. ok now cache is valid
            if(m_CacheData[nLIDN].Valid()){
                return m_CacheData[nLIDN].FrameCount[nState][nDirection];
            }

            // even we tried, but the cache loading failed
            return 0;
        }
};
