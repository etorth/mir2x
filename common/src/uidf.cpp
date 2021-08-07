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
#include "totype.hpp"
#include "fflerror.hpp"
#include "dbcomid.hpp"
#include "dbcomrecord.hpp"

uint64_t uidf::getMapUID(uint32_t mapID)
{
    if(!mapID){
        throw fflerror("invalid map ID: %llu", to_llu(mapID));
    }
    return (to_u64(UID_MAP) << 44) + to_u64(mapID);
}

uint64_t uidf::getServiceCoreUID()
{
    return to_u64(UID_COR) << 44;
}

uint64_t uidf::buildNPCUID(uint16_t npcId)
{
    static std::atomic<uint16_t> s_NPCSeqID {1};
    return (to_u64(UID_NPC) << 44) + (to_u64(npcId) << 16) + s_NPCSeqID.fetch_add(1);
}

static constexpr uint64_t playerUID_gender     = 0X0000080000000000ULL;
static constexpr uint64_t playerUID_jobTaoist  = 0X0000040000000000ULL;
static constexpr uint64_t playerUID_jobWarrior = 0X0000020000000000ULL;
static constexpr uint64_t playerUID_jobWizard  = 0X0000010000000000ULL;

uint64_t uidf::buildPlayerUID(uint32_t dbid, bool gender, const std::vector<int> &jobList)
{
    uint64_t jobMask = 0;
    for(int job: jobList){
        switch(job){
            case JOB_WARRIOR: jobMask |= playerUID_jobWarrior; break;
            case JOB_TAOIST : jobMask |= playerUID_jobTaoist ; break;
            case JOB_WIZARD : jobMask |= playerUID_jobWizard ; break;
            default: throw fflerror("invalid job: %d", job);
        }
    }

    if(jobMask == 0){
        throw fflerror("no job specified");
    }
    return (to_u64(UID_PLY) << 44) | (gender ? playerUID_gender : 0ULL)  | jobMask | to_u64(dbid);
}

bool uidf::isGM(uint64_t uid)
{
    return (uidf::getUIDType(uid) == UID_PLY) && (uidf::getPlayerDBID(uid) <= 10);
}

bool uidf::hasPlayerJob(uint64_t uid, int job)
{
    if(uidf::getUIDType(uid) != UID_PLY){
        throw fflerror("invalid uid type: %s", uidf::getUIDTypeCStr(uid));
    }

    switch(job){
        case JOB_WARRIOR: return uid & playerUID_jobWarrior;
        case JOB_TAOIST : return uid & playerUID_jobTaoist ;
        case JOB_WIZARD : return uid & playerUID_jobWizard ;
        default: throw fflerror("invalid job: %d", job);
    }
}

bool uidf::getPlayerGender(uint64_t uid)
{
    if(uidf::getUIDType(uid) == UID_PLY){
        return uid & playerUID_gender;
    }
    throw fflerror("bad arguments: uid = 0X%016llX, uidType = %s", to_llu(uid), uidf::getUIDTypeCStr(uid));
}

uint32_t uidf::getPlayerDBID(uint64_t uid)
{
    if(uidf::getUIDType(uid) == UID_PLY){
        return to_u32(uid & 0X00000000FFFFFFFFULL);
    }
    throw fflerror("bad arguments: uid = 0X%016llX, uidType = %s", to_llu(uid), uidf::getUIDTypeCStr(uid));
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
        throw fflerror("invalid monster id: %llu", to_llu(monsterId));
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
        return (to_u64(UID_MON) << 44) + (to_u64(monsterId) << 33) + to_u64(nSeq);
    }
    return 0;
}

static constexpr uint64_t receiverUID_mask = 0X00FF000000000000ULL;
bool uidf::isReceiver(uint64_t uid)
{
    return uid & receiverUID_mask;
}

uint64_t uidf::buildReceiverUID()
{
    static std::atomic<uint64_t> recvUIDOff{1};
    return receiverUID_mask + recvUIDOff.fetch_add(1);
}

uint64_t uidf::buildEtcUID()
{
    static std::atomic<uint32_t> s_ETCUID {1};
    if(auto nSeq = s_ETCUID.fetch_add(1); nSeq){
        return (to_u64(UID_ETC) << 44) + to_u64(nSeq);
    }
    return 0;
}

std::string uidf::getUIDString(uint64_t uid)
{
    if(!uid){
        return std::string("ZERO");
    }

    switch(getUIDType(uid)){
        case UID_PLY:
            {
                return str_printf("PLY%llu", to_llu((uid & 0XFFFFFFFFULL)));
            }
        case UID_MON:
            {
                return str_printf("MON%llu_%llu", to_llu(getMonsterID(uid)), to_llu(getMonsterSeq(uid)));
            }
        case UID_NPC:
            {
                return str_printf("NPC%llu_%llu", to_llu(uidf::getNPCID(uid)), to_llu(uidf::getNPCSeqID(uid)));
            }
        case UID_MAP:
            {
                return str_printf("MAP%llu", to_llu(uid & 0XFFFFFFFFULL));
            }
        case UID_COR:
            {
                return std::string("CORE");
            }
        case UID_ETC:
            {
                return str_printf("ETC%llu", to_llu(uid & 0XFFFFFFFFULL));
            }
        default:
            {
                return str_printf("ERR%llu", to_llu(uid));
            }
    }
}

uint32_t uidf::getMapID(uint64_t uid)
{
    if(uidf::getUIDType(uid) != UID_MAP){
        throw fflerror("invalid uid type: %s", uidf::getUIDTypeCStr(uid));
    }
    return uid & 0XFFFFFFFFULL;
}

uint16_t uidf::getNPCID(uint64_t uid)
{
    if(uidf::getUIDType(uid) != UID_NPC){
        throw fflerror("invalid uid type: %s", uidf::getUIDTypeCStr(uid));
    }
    return (uid & 0XFFFFFFFFULL) >> 16;
}

uint16_t uidf::getNPCSeqID(uint64_t uid)
{
    if(uidf::getUIDType(uid) != UID_NPC){
        throw fflerror("invalid uid type: %s", uidf::getUIDTypeCStr(uid));
    }
    return (uid & 0XFFFFULL);
}

uint16_t uidf::getLookID(uint64_t uid)
{
    switch(uidf::getUIDType(uid)){
        case UID_NPC:
            {
                return (uid >> 16) & 0XFFFFULL;
            }
        default:
            {
                throw fflerror("uid type %s doesn't support lookID", uidf::getUIDTypeCStr(uid));
            }
    }
}

bool uidf::isMonster(uint64_t uid, uint32_t monID)
{
    return uidf::isMonster(uid) && (uidf::getMonsterID(uid) == monID);
}

bool uidf::isMonster(uint64_t uid, const char8_t *name)
{
    return uidf::isMonster(uid) && (uidf::getMonsterID(uid) == DBCOM_MONSTERID(name));
}
