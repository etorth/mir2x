/*
 * =====================================================================================
 *
 *       Filename: uidf.cpp
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
#include "uidf.hpp"
#include "strf.hpp"
#include "toll.hpp"
#include "fflerror.hpp"

uint64_t uidf::buildMapUID(uint32_t mapID)
{
    if(!mapID){
        throw fflerror("invalid map ID: %llu", toLLU(mapID));
    }
    return ((uint64_t)(UID_MAP) << 44) + (uint64_t)(mapID);
}

uint64_t uidf::buildNPCUID(uint16_t lookId)
{
    static std::atomic<uint16_t> s_NPCSeqID {1};
    return ((uint64_t)(UID_NPC) << 44) + ((uint64_t)(lookId) << 16) + s_NPCSeqID.fetch_add(1);
}

uint64_t uidf::buildPlayerUID(uint32_t dbid)
{
    return ((uint64_t)(UID_PLY) << 44) + dbid;
}

uint64_t uidf::buildMonsterUID(uint32_t monsterId)
{
    // 47-44 43-33 32 31-0
    // |  |  |  |  |  |  |
    // |  |  |  |  |  +--+---------> monster seq
    // |  |  |  |  +---------------> reserved
    // |  |  +--+------------------> monster id: 1 ~ 2047
    // +--+------------------------> UID_MON

    if(monsterId == 0 || monsterId >= 2048){
        throw fflerror("invalid monster id: %llu", toLLU(monsterId));
    }

    static auto s_monsterIDSeqPtr = []() -> std::array<std::atomic<uint32_t>, 2048> *
    {
        // have to keep this static
        // 1. copy ctor of std::atomic<T> is deleted
        // 2. avoid to allocate on heap

        static std::array<std::atomic<uint32_t>, 2048> s_atomicArray;
        for(auto &entry: s_atomicArray){
            entry.store(1);
        }
        return &s_atomicArray;
    }();

    if(uint32_t nSeq = s_monsterIDSeqPtr->at(monsterId).fetch_add(1); nSeq){
        return ((uint64_t)(UID_MON) << 44) + ((uint64_t)(monsterId) << 33) + (uint64_t)(nSeq);
    }
    return 0;
}

uint64_t uidf::buildEtcUID()
{
    static std::atomic<uint32_t> s_ETCUID {1};
    if(auto nSeq = s_ETCUID.fetch_add(1); nSeq){
        return ((uint64_t)(UID_ETC) << 44) + (uint64_t)(nSeq);
    }
    return 0;
}

uint64_t uidf::buildServiceCoreUID()
{
    return (uint64_t)(UID_COR) << 44;
}

std::string uidf::getUIDString(uint64_t uid)
{
    if(!uid){
        return std::string("ZERO");
    }

    switch(getUIDType(uid)){
        case UID_PLY:
            {
                return str_printf("PLY%llu", toLLU((uid & 0XFFFFFFFF)));
            }
        case UID_MON:
            {
                return str_printf("MON%llu_%llu", getMonsterID(uid), getMonsterSeq(uid));
            }
        case UID_NPC:
            {
                return str_printf("NPC%llu_%llu", toLLU(uidf::getLookID(uid)), toLLU(uid & 0XFFFF));
            }
        case UID_MAP:
            {
                return str_printf("MAP%llu", toLLU(uid & 0XFFFFFFFF));
            }
        case UID_COR:
            {
                return std::string("CORE");
            }
        case UID_ETC:
            {
                return str_printf("ETC%llu", toLLU(uid & 0XFFFFFFFF));
            }
        default:
            {
                return str_printf("ERR%llu", toLLU(uid));
            }
    }
}

uint32_t uidf::getMapID(uint64_t uid)
{
    if(uidf::getUIDType(uid) != UID_MAP){
        throw fflerror("invalid uid type: %s", uidf::getUIDTypeString(uid));
    }
    return uid & 0XFFFFFFFF;
}

uint16_t uidf::getLookID(uint64_t uid)
{
    switch(uidf::getUIDType(uid)){
        case UID_NPC:
            {
                return (uid & 0XFFFF0000) >> 16;
            }
        default:
            {
                throw fflerror("uid type %s doesn't support lookID", uidf::getUIDTypeString(uid));
            }
    }
}
