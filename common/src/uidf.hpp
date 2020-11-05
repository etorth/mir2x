/*
 * =====================================================================================
 *
 *       Filename: uidf.hpp
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
#include "typecast.hpp"
#include "fflerror.hpp"

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
    UID_COR = 13,
    UID_ETC = 14,
    UID_INN = 15, // put the internal out of 0 ~ 15
    UID_MAX = 16,
};

namespace uidf
{
    // based on database id to create UID
    // always model it as ``build" even some of them are pure mapping

    uint64_t buildMapUID(uint32_t);
    uint64_t buildNPCUID(uint16_t);
    uint64_t buildPlayerUID(uint32_t);
    uint64_t buildMonsterUID(uint32_t);

    uint64_t buildEtcUID();
    uint64_t buildServiceCoreUID();
}

namespace uidf
{
    constexpr int uidThreadID_Shift = 56;
    constexpr uint64_t uidThreadID_Mask = 0XFF00000000000000ULL;

    inline int getThreadID(uint64_t uid)
    {
        return static_cast<int>((uid & uidThreadID_Mask) >> uidThreadID_Shift);
    }

    inline uint64_t setThreadID(uint64_t uid, int threadId)
    {
        if(threadId < 0){
            throw fflerror("invalid uid threadID: %d", threadId);
        }

        uid = (uid & (~uidThreadID_Mask)) | ((uint64_t)(threadId) << uidThreadID_Shift);
        if(uidf::getThreadID(uid) != threadId){
            throw fflerror("can't assign threadID: %d", threadId);
        }
        return uid;
    }
}

namespace uidf
{
    inline int getUIDType(uint64_t uid)
    {
        if(uid & 0X00FF000000000000ULL){
            return UID_INN;
        }
        return (int)((uid & 0X0000F00000000000ULL) >> 44);
    }

    inline const char *getUIDTypeString(uint64_t uid)
    {
        switch(getUIDType(uid)){
            case UID_MON: return "MON";
            case UID_PLY: return "PLY";
            case UID_NPC: return "NPC";
            case UID_MAP: return "MAP";
            case UID_COR: return "COR";
            case UID_ETC: return "ETC";
            default     : return "ERR";
        }
    }

    std::string getUIDString(uint64_t);
}

namespace uidf
{
    inline uint32_t getMonsterID(uint64_t uid)
    {
        if(getUIDType(uid) != UID_MON){
            throw fflerror("not a monster uid");
        }
        return (uint32_t)((uid & 0X00000FFE00000000ULL) >> 33);
    }

    inline uint32_t getMonsterSeq(uint64_t uid)
    {
        if(getUIDType(uid) != UID_MON){
            throw fflerror("not a monster uid");
        }
        return (uint32_t)(uid & 0XFFFFFFFFULL);
    }

    uint32_t getMapID (uint64_t);
    uint16_t getLookID(uint64_t);
}

namespace uidf
{
    inline uint64_t toUID(const std::string &uidString)
    {
        try{
            return std::stoull(uidString);
        }
        catch(...){
            //
        }
        return 0;
    }

    inline uint64_t toUIDEx(const std::string &uidString)
    {
        if(const uint64_t uid = toUID(uidString)){
            return uid;
        }
        throw fflerror("failed to convert %s to a valid UID", uidString.c_str());
    }
}
