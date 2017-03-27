/*
 * =====================================================================================
 *
 *       Filename: monsterginfo.hpp
 *        Created: 06/02/2016 15:08:56
 *  Last Modified: 03/27/2017 14:14:49
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
#include <cassert>
#include <cstdint>

#include "message.hpp"
#include "sysconst.hpp"
#include "pngtexoffdbn.hpp"

class MonsterGInfo final
{
    protected:
        enum QueryType: int
        {
            QUERY_NA,
            QUERY_OK,
            QUERY_ERROR,
            QUERY_PENDING,
        };

        // 1. make it static so nested class can have it
        // 2. make it initialized in defination
        // 3. if(nLookID >= MAX_LOOKID){ return error; }
        const static uint32_t MAX_LOOKID { 0X00001000};

    protected:
        typedef struct _InnNetData
        {
            int CurrQuery;
            uint32_t CurrLookID;

            _InnNetData()
                : CurrQuery(QUERY_NA)
                , CurrLookID(MAX_LOOKID)
            {}

            bool Valid()
            {
                return Query() == QUERY_OK;
            }

            void Reset(uint32_t nLookID)
            {
                CurrQuery = QUERY_OK;
                CurrLookID = nLookID;
            }

            int Query()
            {
                return CurrQuery;
            }

            void Query(int nQuery)
            {
                CurrQuery = nQuery;
            }

            uint32_t LookID()
            {
                return CurrLookID;
            }
        }InnNetData;

        typedef struct _InnCacheData
        {
            int Query;
            size_t FrameCount[16][8];

            _InnCacheData()
                : Query(QUERY_NA)
            {}

            bool Valid()
            {
                return Query == QUERY_OK;
            }

            bool Load(uint32_t nLookID)
            {
                if(Valid()){ return true; }

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
                for(uint32_t nAction = 0; nAction < 16; ++nAction){
                    for(uint32_t nDirection = 0; nDirection < 8; ++nDirection){
                        // 1. calculate the key
                        uint32_t nKey = (nLookID << 12) + (nAction << 8) + (nDirection << 5);
                        extern PNGTexOffDBN *g_PNGTexOffDBN;

                        // 1. initialization of count, zero is invalid but legal
                        FrameCount[nAction][nDirection] = 0;
                        while(g_PNGTexOffDBN->Retrieve(nKey++, nullptr, nullptr)){
                            FrameCount[nAction][nDirection]++;
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
       static const MonsterGInfo &Null()
       {
           static MonsterGInfo stNullRecord(0);
           return stNullRecord;
       }

    public:
        bool Valid(uint32_t nLIDN, bool bCacheValid = false)
        {
            // 1. parameter check
            assert(nLIDN < 4);

            // 2. check the branch
            if(m_NetData[nLIDN].Valid()){
                return (bCacheValid ? m_CacheData[nLIDN].Valid() : true);
            }

            // 3. oooops now the NetData is not valid, we need to load it
            Load(nLIDN);
            return false;
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
            return m_NetData[nLIDN].LookID();
        }

    public:
        void ResetLookID(uint32_t nLIDN, uint32_t nLookID)
        {
            assert((nLIDN < 4) && (nLookID < MAX_LOOKID));
            m_NetData[nLIDN].Reset(nLookID);
        }

    protected:
        void Load(uint32_t nLIDN)
        {
            // 1. check parameter
            assert(nLIDN < 4);

            // 2. if the net data is not ready we return
            switch(m_NetData[nLIDN].Query()){
                case QUERY_NA:
                    {
                        CMQueryMonsterGInfo stCMQMGI;
                        stCMQMGI.MonsterID = m_MonsterID;
                        stCMQMGI.LookIDN   = nLIDN;

                        extern Game *g_Game;
                        g_Game->Send(CM_QUERYMONSTERGINFO, stCMQMGI);

                        m_NetData[nLIDN].Query(QUERY_PENDING);
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
        }

    public:
        size_t FrameCount(uint32_t nLIDN, uint32_t nAction, uint32_t nDirection)
        {
            // 1. check parameter, for monster:
            //      1. it can own 4 different look effect
            //      2. it can have up to 16 states (00 ~ 15) while human can have up to xxx states
            //      3. it can have up to 8 directions (0 ~7)
            if(nLIDN >= 4 || nAction >= 16 || nDirection >= 8){ return 0; }

            // 2. if the net data is not ready we return
            switch(m_NetData[nLIDN].Query()){
                case QUERY_OK:
                    {
                        goto __MONSTERGINFO_FRAMECOUNT_NETDATAVALID_LABEL_1;
                    }
                case QUERY_NA:
                    {
                        Load(nLIDN);
                        return 0;
                    }
                default:
                    {
                        return 0;
                    }
            }

__MONSTERGINFO_FRAMECOUNT_NETDATAVALID_LABEL_1:
            // 2. check cache
            if(!m_CacheData[nLIDN].Valid()){
                m_CacheData[nLIDN].Load(m_NetData[nLIDN].LookID());
            }

            // 3. ok now cache is valid
            if(m_CacheData[nLIDN].Valid()){
                return m_CacheData[nLIDN].FrameCount[nAction][nDirection];
            }

            // even we tried, but the cache loading failed
            return 0;
        }
};
