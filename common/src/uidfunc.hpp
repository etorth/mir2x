/*
 * =====================================================================================
 *
 *       Filename: uidfunc.hpp
 *        Created: 08/10/2018 21:47:18
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
#include <string>
#include <cstdint>
#include <cinttypes>

enum JobType: int
{
    JOB_ERR = 0,
    JOB_WAR = 1,
    JOB_TAO = 2,
    JOB_MAG = 3,
};

enum UIDType: int
{
    UID_ERR =  0,
    UID_MON =  1,
    UID_PLY =  2,
    UID_NPC =  3,
    UID_MAP =  4,
    UID_U05 =  5,
    UID_U06 =  6,
    UID_U07 =  7,
    UID_U08 =  8,
    UID_U09 =  9,
    UID_U10 = 10,
    UID_U11 = 11,
    UID_U12 = 12,
    UID_U13 = 13,
    UID_COR = 14,
    UID_ETC = 15,
    UID_INN = 16, // out of 4bits for inn actor type
};

namespace UIDFunc
{
    uint64_t GetMapUID(uint32_t);
}

namespace UIDFunc
{
    uint64_t BuildUID_MON(uint32_t);
    uint64_t BuildUID_PLY(bool, int, uint32_t);
    uint64_t BuildUID_MAP(uint32_t);
    uint64_t BuildUID_COR();
    uint64_t BuildUID_ETC();
}

namespace UIDFunc
{
    inline int GetUIDType(uint64_t nUID)
    {
        if(nUID & 0XFFFF000000000000){
            return UID_INN;
        }
        return (int)((nUID & 0X0000F00000000000) >> 44);
    }
}

namespace UIDFunc
{
    std::string GetUIDString(uint64_t);
}

namespace UIDFunc
{
    inline uint32_t GetMonsterID(uint32_t nUID)
    {
        if(GetUIDType(nUID) != UID_MON){
            return 0;
        }
        return (nUID & 0X00000FFE00000000) >> 32;
    }

    inline uint32_t GetMonsterSeq(uint32_t nUID)
    {
        if(GetUIDType(nUID) != UID_MON){
            return 0;
        }
        return (uint32_t)(nUID & 0XFFFFFFFF);
    }
}
