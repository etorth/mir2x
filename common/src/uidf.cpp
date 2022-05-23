#include <map>
#include <mutex>
#include <array>
#include <atomic>
#include "uidf.hpp"
#include "strf.hpp"
#include "totype.hpp"
#include "fflerror.hpp"
#include "dbcomid.hpp"

int uidf::getUIDType(uint64_t uid)
{
    return to_d((uid & 0X7800000000000000ULL) >> 59);
}

std::string uidf::getUIDString(uint64_t uid)
{
    if(!uid){
        return std::string("ZERO");
    }

    switch(getUIDType(uid)){
        case UID_PLY:
            {
                return str_printf("PLY_%llu", to_llu(uidf::getPlayerDBID(uid)));
            }
        case UID_MON:
            {
                return str_printf("MON_%llu_%llu", to_llu(uidf::getMonsterID(uid)), to_llu(uidf::getMonsterSeq(uid)));
            }
        case UID_NPC:
            {
                return str_printf("NPC_%llu_%llu", to_llu(uidf::getNPCID(uid)), to_llu(uidf::getNPCSeq(uid)));
            }
        case UID_MAP:
            {
                return str_printf("MAP_%llu_%llu", to_llu(uidf::getMapID(uid)), to_llu(uidf::getMapSeq(uid)));
            }
        case UID_COR:
            {
                return std::string("CORE");
            }
        case UID_RCV:
            {
                return str_printf("RCV_%llu", to_llu(uidf::getReceiverSeq(uid)));
            }
        default:
            {
                return str_printf("ERR_%llu", to_llu(uid));
            }
    }
}

const char *uidf::getUIDTypeCStr(uint64_t uid)
{
    fflassert(getUIDType(uid) >= UID_BEGIN, uid, getUIDType(uid));
    fflassert(getUIDType(uid) <  UID_END  , uid, getUIDType(uid));

    switch(getUIDType(uid)){
        case UID_COR: return "COR";
        case UID_MAP: return "MAP";
        case UID_NPC: return "NPC";
        case UID_MON: return "MON";
        case UID_PLY: return "PLY";
        case UID_RCV: return "RCV";
        default     : return "ERR";
    }
}

#define _def_build_UID_helper(funcName, uidType, startOff) \
uint64_t funcName(uint32_t id) \
{ \
    fflassert(id <= UINT32_C(0X07FFFFFF), id); \
    const auto seqID = [id]() -> uint32_t \
    { \
        static std::mutex s_seqLock; \
        static std::unordered_map<uint32_t, uint32_t> s_seqMap; \
        { \
            const std::lock_guard<std::mutex> lockGuard(s_seqLock); \
            if(auto p = s_seqMap.find(id); p == s_seqMap.end()){ \
                return s_seqMap[id] = startOff; \
            } \
            else{ \
                return ++(p->second); \
            } \
        } \
    }(); \
    return (to_u64(uidType) << 59) + (to_u64(id) << 32) + to_u64(seqID); \
} \

_def_build_UID_helper(uidf::buildMapUID    , UID_MAP, 2) // 1 used for map base UID
_def_build_UID_helper(uidf::buildNPCUID    , UID_NPC, 1)
_def_build_UID_helper(uidf::buildMonsterUID, UID_MON, 1)

#undef _def_build_UID_helper

uint64_t uidf::buildReceiverUID()
{
    static std::atomic<uint64_t> s_recvSeq {0};
    return (to_u64(UID_RCV) << 59) + to_u64(++s_recvSeq);
}

uint64_t uidf::getServiceCoreUID()
{
    return to_u64(UID_COR) << 59;
}

uint64_t uidf::getMapBaseUID(uint32_t mapID)
{
    return (to_u64(UID_MAP) << 59) + (to_u64(mapID) << 32) + 1;
}

// for player UID:
//
//      63: unused
//   62-59: UID_PLY
//      58: player gender
//      57: player jobTaoist
//      56: player jobWarrior
//      55: player jobWizard
//   54-32: unused
//   31-00: dbid

static constexpr uint64_t s_playerUID_gender     = 1ULL << 58;
static constexpr uint64_t s_playerUID_jobTaoist  = 1ULL << 57;
static constexpr uint64_t s_playerUID_jobWarrior = 1ULL << 56;
static constexpr uint64_t s_playerUID_jobWizard  = 1ULL << 55;

uint64_t uidf::getPlayerUID(uint32_t dbid, bool gender, const std::vector<int> &jobList)
{
    uint64_t jobMask = 0;
    for(int job: jobList){
        switch(job){
            case JOB_WARRIOR: jobMask |= s_playerUID_jobWarrior; break;
            case JOB_TAOIST : jobMask |= s_playerUID_jobTaoist ; break;
            case JOB_WIZARD : jobMask |= s_playerUID_jobWizard ; break;
            default: throw fflerror("invalid job: %d", job);
        }
    }

    if(jobMask == 0){
        throw fflerror("no job specified");
    }
    return (to_u64(UID_PLY) << 59) | (gender ? s_playerUID_gender : 0ULL)  | jobMask | to_u64(dbid);
}

bool uidf::hasPlayerJob(uint64_t uid, int job)
{
    fflassert(uidf::getUIDType(uid) == UID_PLY, uid, uidf::getUIDString(uid));
    switch(job){
        case JOB_WARRIOR: return uid & s_playerUID_jobWarrior;
        case JOB_TAOIST : return uid & s_playerUID_jobTaoist ;
        case JOB_WIZARD : return uid & s_playerUID_jobWizard ;
        default: throw fflvalue(uid, uidf::getUIDString(uid), job);
    }
}

uint32_t uidf::getPlayerDBID(uint64_t uid)
{
    fflassert(uidf::getUIDType(uid) == UID_PLY, uid, uidf::getUIDString(uid));
    return to_u32(uid);
}

bool uidf::getPlayerGender(uint64_t uid)
{
    fflassert(uidf::getUIDType(uid) == UID_PLY, uid, uidf::getUIDString(uid));
    return uid & s_playerUID_gender;
}

#define _def_get_UID_id_helper(funcName, uidType) \
uint32_t funcName(uint64_t uid) \
{ \
    fflassert(uidf::getUIDType(uid) == uidType, uid, uidf::getUIDString(uid)); \
    return to_u32((uid & 0X07FFFFFF00000000ULL) >> 32); \
} \

_def_get_UID_id_helper(uidf::getMapID    , UID_MAP)
_def_get_UID_id_helper(uidf::getNPCID    , UID_NPC)
_def_get_UID_id_helper(uidf::getMonsterID, UID_MON)

#undef _def_get_UID_id_helper

#define _def_get_UID_seq_helper(funcName, uidType) \
uint32_t funcName(uint64_t uid) \
{ \
    fflassert(uidf::getUIDType(uid) == uidType, uid, uidf::getUIDString(uid)); \
    return to_u32(uid); \
} \

_def_get_UID_seq_helper(uidf::getMapSeq    , UID_MAP)
_def_get_UID_seq_helper(uidf::getNPCSeq    , UID_NPC)
_def_get_UID_seq_helper(uidf::getMonsterSeq, UID_MON)

#undef _def_get_UID_seq_helper

uint64_t uidf::getReceiverSeq(uint64_t uid)
{
    return uid & 0X07FFFFFFFFFFFFFFULL;
}

bool uidf::isGM(uint64_t uid)
{
    return (uidf::getUIDType(uid) == UID_PLY) && (uidf::getPlayerDBID(uid) <= 10);
}

bool uidf::isPlayer(uint64_t uid)
{
    return uidf::getUIDType(uid) == UID_PLY;
}

bool uidf::isNPChar(uint64_t uid)
{
    return uidf::getUIDType(uid) == UID_NPC;
}

bool uidf::isReceiver(uint64_t uid)
{
    return getUIDType(uid) == UID_RCV;
}

bool uidf::isMonster(uint64_t uid)
{
    return uidf::getUIDType(uid) == UID_MON;
}

bool uidf::isMonster(uint64_t uid, uint32_t monID)
{
    return uidf::isMonster(uid) && (uidf::getMonsterID(uid) == monID);
}

bool uidf::isMonster(uint64_t uid, const char8_t *name)
{
    return uidf::isMonster(uid) && (uidf::getMonsterID(uid) == DBCOM_MONSTERID(name));
}

bool uidf::isGuardMode(uint64_t uid)
{
    return uidf::isMonster(uid) && (DBCOM_MONSTERRECORD(uidf::getMonsterID(uid)).behaveMode == BM_GUARD);
}

bool uidf::isNeutralMode(uint64_t uid)
{
    return uidf::isMonster(uid) && (DBCOM_MONSTERRECORD(uidf::getMonsterID(uid)).behaveMode == BM_NEUTRAL);
}
