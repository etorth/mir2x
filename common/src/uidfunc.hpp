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
#include <cstdint>
#include <cinttypes>

enum JobType: int
{
    JOB_WAR,
    JOB_TAO,
    JOB_MAG,
};

// define 0~16 types for uid conversion
// for newer class, get one type and rename it

enum UIDType: int
{
    UID_ERR =  0,
    UID_MON =  1,   // 0xxxxxxxxxxxxxxx
    UID_PLY =  2,   // 10xxxxxxxxxxxxxx
    UID_NPC =  3,   // 110xxxxxxxxxxxxx
    UID_MAP =  4,   // 1110xxxxxxxxxxxx
    UID_U05 =  5,   // 11110xxxxxxxxxxx
    UID_U06 =  6,   // 111110xxxxxxxxxx
    UID_U07 =  7,   // 1111110xxxxxxxxx
    UID_U08 =  8,   // 11111110xxxxxxxx
    UID_U09 =  9,   // 111111110xxxxxxx
    UID_U10 = 10,   // 1111111110xxxxxx
    UID_U11 = 11,   // 11111111110xxxxx
    UID_U12 = 12,   // 111111111110xxxx
    UID_U13 = 13,   // 1111111111110xxx
    UID_U14 = 14,   // 11111111111110xx
    UID_COR = 15,   // 111111111111110x
    UID_ETC = 16,   // 1111111111111110
};

namespace UIDFunc
{
    uint32_t BuildUID_MON(uint32_t);
    uint32_t BuildUID_PLY(bool, int, uint32_t);
    uint32_t BuildUID_MAP(uint32_t);
    uint32_t BuildUID_COR();
    uint32_t BuildUID_ETC();
}

namespace UIDFunc
{
    inline int GetUIDType(uint32_t nUID)
    {
        if((nUID & ((uint32_t)(1) << 31)) == 0){ return UID_MON; }
        if((nUID & ((uint32_t)(1) << 30)) == 0){ return UID_PLY; }
        if((nUID & ((uint32_t)(1) << 29)) == 0){ return UID_NPC; }
        if((nUID & ((uint32_t)(1) << 28)) == 0){ return UID_MAP; }
        if((nUID & ((uint32_t)(1) << 27)) == 0){ return UID_U05; }
        if((nUID & ((uint32_t)(1) << 26)) == 0){ return UID_U06; }
        if((nUID & ((uint32_t)(1) << 25)) == 0){ return UID_U07; }
        if((nUID & ((uint32_t)(1) << 24)) == 0){ return UID_U08; }
        if((nUID & ((uint32_t)(1) << 23)) == 0){ return UID_U09; }
        if((nUID & ((uint32_t)(1) << 22)) == 0){ return UID_U10; }
        if((nUID & ((uint32_t)(1) << 21)) == 0){ return UID_U11; }
        if((nUID & ((uint32_t)(1) << 20)) == 0){ return UID_U12; }
        if((nUID & ((uint32_t)(1) << 19)) == 0){ return UID_U13; }
        if((nUID & ((uint32_t)(1) << 18)) == 0){ return UID_U14; }
        if((nUID & ((uint32_t)(1) << 17)) == 0){ return UID_COR; }
        if((nUID & ((uint32_t)(1) << 16)) == 0){ return UID_ETC; }
        return UID_ERR;
    }
}

namespace UIDFunc
{
    // for debug usage
    // create a human readable string for log/print
    std::string GetUIDString(uint32_t);

    // to create one-to-one map actor address
    // 1. shorter string
    // 2. faster conversion speed
    std::string GetUIDAddress(uint32_t);
    const char *GetUIDAddress(uint32_t, char *);

    // to get uid from the address string
    // for null/invalid string return ZERO
    uint32_t GetUIDFromAddress(const char *);
}

namespace UIDFunc
{
    inline uint32_t GetMonsterID(uint32_t nUID)
    {
        if(GetUIDType(nUID) != UID_MON){
            return 0;
        }

        if(nUID & ((uint32_t)(1) << 30)){
            return (nUID & (uint32_t)(0X3FFFFFFF)) >> 19;
        }else{
            return (nUID & (uint32_t)(0X3FFFFFFF)) >> 22;
        }
    }

    inline uint32_t GetMonsterSeq(uint32_t nUID)
    {
        if(GetUIDType(nUID) != UID_MON){
            return 0;
        }

        if(nUID & ((uint32_t)(1) << 30)){
            return nUID & (uint32_t)(0X07FFFF);
        }else{
            return nUID & (uint32_t)(0X3FFFFF);
        }
    }
}
