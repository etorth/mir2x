/*
 * =====================================================================================
 *
 *       Filename: uidfunc.cpp
 *        Created: 08/11/2018 05:25:34
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

#include <map>
#include <mutex>
#include <array>
#include <atomic>
#include "uidfunc.hpp"

uint32_t UIDFunc::BuildUID_MON(uint32_t nMonsterID)
{
    // monster
    // data field is monster id [1, 2048)

    if(nMonsterID == 0 || nMonsterID >= 2048){
        return 0;
    }

    static auto s_MonsterIDSeqPtr = []() -> std::array<std::atomic<uint32_t>, 2048> *
    {
        // have to keep this static
        // 1. copy ctor of std::atomic<T> is deleted
        // 2. avoid to allocate on heap

        static std::array<std::atomic<uint32_t>, 2048> s_AtomicArray;
        for(auto &rstEntry: s_AtomicArray){
            rstEntry.store(1);
        }
        return &s_AtomicArray;
    }();

    uint32_t nSeq = s_MonsterIDSeqPtr->at(nMonsterID).fetch_add(1);

    // here we make a mask bit to save bits
    // for all monster id <= 255 : short id : mask = 0 , use next  8 bits as monster id
    //                 id >  255 : long  id : mask = 1 , use next 11 bits as monster id

    // 31 30 29 - 22 21 - 0
    // 31 30 29 - 19 18 - 0
    // |  |  |    |  |    |
    // |  |  |    |  +----+---------> seq id for uniqueness
    // |  |  +----+-----------------> monster id in uid for simplity
    // |  +-------------------------> bit mask as explained above
    // +----------------------------> always ZERO for monster uid

    if(nMonsterID <= 255){
        // short monster id
        // can have more seq id for longer MTBF
        if(nSeq == 0 || nSeq > 0X3FFFFF /* bits 21 ~ 0 */){
            return 0;
        }
        return (nMonsterID << 22) + nSeq;
    }

    // long monster id
    // we need to set the mask bit

    if(nSeq == 0 || nSeq > 0X07FFFF){
        return 0;
    }

    return ((uint32_t)(1) << 30) + (nMonsterID << 19) + nSeq;
}

uint32_t UIDFunc::BuildUID_PLY(bool bMale, int nJob, uint32_t nDBID)
{
    // 31-30 : 0b00
    // 29-29 : 1->male, 0->female
    // 28-27 : job id
    // 26-00 : database id

    if(nDBID == 0 || nDBID > 0XFFFFFF){
        return 0;
    }

    uint32_t nJobID = 0;
    switch(nJob){
        case JOB_WAR: nJobID = 0b00; break;
        case JOB_TAO: nJobID = 0b01; break;
        case JOB_MAG: nJobID = 0b10; break;
        default     : return 0;
    }

    uint32_t nSexID = bMale ? 1 : 0;
    return ((uint32_t)(0b10) << 30) + (nSexID << 29) + (nJobID << 27) + nDBID;
}

uint32_t UIDFunc::BuildUID_MAP(uint32_t nMapID)
{
    // 31-28 : 0b1110
    // 27-12 : map id
    // 11-00 : seq id

    if(nMapID == 0 || nMapID >= 0XFFFF){
        return 0;
    }

    static std::mutex s_MapLock;
    static std::map<uint32_t, uint32_t> s_MapUID;

    uint32_t nSeq = 0;
    {
        std::lock_guard<std::mutex> stLockGuard(s_MapLock);
        if(s_MapUID.find(nMapID) == s_MapUID.end()){
            s_MapUID[nMapID] = 1;
        }else{
            s_MapUID[nMapID]++;
        }
        nSeq = s_MapUID[nMapID];
    }

    if(nSeq > 0X0FFF){
        return 0;
    }

    return 0XE0000000 + (nMapID << 12) + nSeq;
}

uint32_t UIDFunc::BuildUID_COR()
{
    return 0XFFFC0000;
}

uint32_t UIDFunc::BuildUID_ETC()
{
    static std::atomic<uint32_t> s_ETCUID {1};
    if(auto nSeq = s_ETCUID.fetch_add(1); nSeq <= 0X00FFFFFF){
        return 0XFF000000 + nSeq;
    }
    return 0;
}

std::string UIDFunc::GetUIDString(uint32_t nUID)
{
    char szUIDString[128];
    switch(GetUIDType(nUID)){
        case UID_PLY:
            {
                std::sprintf(szUIDString, "PLY%" PRIu32, nUID & 0X7FFFFFF);
                return szUIDString;
            }
        case UID_MON:
            {
                std::sprintf(szUIDString, "MON%" PRIu32 "_%" PRIu32, GetMonsterID(nUID), GetMonsterSeq(nUID));
                return szUIDString;
            }
        case UID_MAP:
            {
                uint32_t nMapID  = (nUID & 0X0FFFF000) >> 12;
                uint32_t nMapSeq = (nUID & 0X00000FFF) >>  0;

                std::sprintf(szUIDString, "MAP%" PRIu32 "_%" PRIu32, nMapID, nMapSeq);
                return szUIDString;
            }
        case UID_COR:
            {
                std::sprintf(szUIDString, "COR");
                return szUIDString;
            }
        case UID_ETC:
            {
                std::sprintf(szUIDString, "ETC%" PRIu32, nUID & 0X00FFFFFF);
                return szUIDString;
            }
        default:
            {
                break;
            }
    }

    std::sprintf(szUIDString, "ERR%" PRIu32, nUID);
    return szUIDString;
}


const char *UIDFunc::GetUIDAddress(uint32_t nUID, char *pBuf)
{
    // google it for better solution
    // 1. shorter string 
    // 2. faster conversion speed

    if(pBuf){
        std::sprintf(pBuf, "%" PRIu32, nUID);
    }
    return pBuf;
}

std::string UIDFunc::GetUIDAddress(uint32_t nUID)
{
    char szUIDString[32];
    return std::string(GetUIDAddress(nUID, szUIDString));
}

uint32_t UIDFunc::GetUIDFromAddress(const char *szUIDString)
{
    if(szUIDString == nullptr){
        return 0;
    }
    return (uint32_t)(std::strtoull(szUIDString, nullptr, 10));
}
