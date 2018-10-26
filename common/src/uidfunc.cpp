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

uint64_t UIDFunc::BuildMonsterUID(uint32_t nMonsterID)
{
    // 47-44 43-33 32 31-0
    // |  |  |  |  |  |  |
    // |  |  |  |  |  +--+---------> monster seq
    // |  |  |  |  +---------------> reserved
    // |  |  +--+------------------> monster id: 1 ~ 2047
    // +--+------------------------> UID_MON

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

    if(uint32_t nSeq = s_MonsterIDSeqPtr->at(nMonsterID).fetch_add(1); nSeq){
        return ((uint64_t)(UID_MON) << 44) + ((uint64_t)(nMonsterID) << 33) + (uint64_t)(nSeq);
    }
    return 0;
}

uint64_t UIDFunc::BuildEtcUID()
{
    static std::atomic<uint32_t> s_ETCUID {1};
    if(auto nSeq = s_ETCUID.fetch_add(1); nSeq){
        return ((uint64_t)(UID_ETC) << 44) + (uint64_t)(nSeq);
    }
    return 0;
}

std::string UIDFunc::GetUIDString(uint64_t nUID)
{
    if(!nUID){
        return std::string("ZERO");
    }

    char szUIDString[128];
    switch(GetUIDType(nUID)){
        case UID_PLY:
            {
                std::sprintf(szUIDString, "PLY%" PRIu32, (uint32_t)(nUID & 0XFFFFFFFF));
                return szUIDString;
            }
        case UID_MON:
            {
                std::sprintf(szUIDString, "MON%" PRIu32 "_%" PRIu32, GetMonsterID(nUID), GetMonsterSeq(nUID));
                return szUIDString;
            }
        case UID_MAP:
            {
                std::sprintf(szUIDString, "MAP%" PRIu32, (uint32_t)(nUID & 0XFFFFFFFF));
                return szUIDString;
            }
        case UID_COR:
            {
                std::sprintf(szUIDString, "CORE");
                return szUIDString;
            }
        case UID_ETC:
            {
                std::sprintf(szUIDString, "ETC%" PRIu32, (uint32_t)(nUID & 0XFFFFFFFF));
                return szUIDString;
            }
        default:
            {
                break;
            }
    }

    std::sprintf(szUIDString, "ERR%" PRIu64, nUID);
    return szUIDString;
}
