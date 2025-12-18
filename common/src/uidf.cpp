#include <map>
#include <mutex>
#include <array>
#include <atomic>
#include "uidf.hpp"
#include "strf.hpp"
#include "totype.hpp"
#include "fflerror.hpp"
#include "dbcomid.hpp"

constexpr static size_t  reservedBitWidth = 1;
constexpr static size_t   uidTypeBitWidth = uidf::uidTypeBits();
constexpr static size_t        idBitWidth = 24;
constexpr static size_t peerIndexBitWidth = uidf::peerIndexBits();
constexpr static size_t       seqBitWidth = 64 - reservedBitWidth - uidTypeBitWidth - idBitWidth - peerIndexBitWidth;

static_assert(seqBitWidth <= 32);

constexpr static size_t     uidTypeBitOff = 64 - reservedBitWidth - uidTypeBitWidth;
constexpr static size_t          idBitOff = 64 - reservedBitWidth - uidTypeBitWidth - idBitWidth;
constexpr static size_t   peerIndexBitOff = 64 - reservedBitWidth - uidTypeBitWidth - idBitWidth - peerIndexBitWidth;
constexpr static size_t         seqBitOff = 0;

constexpr static uint64_t   uidTypeBitMask = ((UINT64_C(1) <<   uidTypeBitWidth) - 1) <<   uidTypeBitOff;
constexpr static uint64_t        idBitMask = ((UINT64_C(1) <<        idBitWidth) - 1) <<        idBitOff;
constexpr static uint64_t peerIndexBitMask = ((UINT64_C(1) << peerIndexBitWidth) - 1) << peerIndexBitOff;
constexpr static uint64_t       seqBitMask = ((UINT64_C(1) <<       seqBitWidth) - 1) <<       seqBitOff;

size_t uidf::peerIndex(uint64_t uid)
{
    switch(uidf::getUIDType(uid)){
        case UID_COR:
            {
                return uidf::getPeerCoreSeq(uid);
            }
        case UID_MAP:
        case UID_NPC:
        case UID_MON:
            {
                return (uid & peerIndexBitMask) >> peerIndexBitOff;
            }
        default:
            {
                return 0;
            }
    }
}

int uidf::getUIDType(uint64_t uid)
{
    return to_d((uid & uidTypeBitMask) >> uidTypeBitOff);
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
                return str_printf("MON_%llu_%llu_%llu", to_llu(uidf::getMonsterID(uid)), to_llu(uidf::peerIndex(uid)), to_llu(uidf::getMonsterSeq(uid, false)));
            }
        case UID_NPC:
            {
                return str_printf("NPC_%llu_%llu_%llu", to_llu(uidf::getNPCID(uid)), to_llu(uidf::peerIndex(uid)), to_llu(uidf::getNPCSeq(uid, false)));
            }
        case UID_MAP:
            {
                return str_printf("MAP_%llu_%llu_%llu", to_llu(uidf::getMapID(uid)), to_llu(uidf::peerIndex(uid)), to_llu(uidf::getMapSeq(uid, false)));
            }
        case UID_COR:
            {
                return str_printf("CORE_%llu", to_llu(uidf::peerIndex(uid)));
            }
        case UID_RCV:
            {
                return str_printf("RCV_%llu", to_llu(uidf::getReceiverSeq(uid)));
            }
	case UID_QST:
	    {
		return str_printf("QST_%llu", to_llu(uidf::getQuestID(uid)));
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
        case UID_QST: return "QST";
        case UID_SLO: return "SLO";
        default     : return "ERR";
    }
}

template<UIDType uidType> static uint64_t _build_UID_helper(bool allowZeroId, uint32_t id, size_t peerIndex, uint32_t startOff, std::optional<uint32_t> seqIDOpt)
{
    if(!allowZeroId){
        fflassert(id > 0, id);
    }

    fflassert(id < (1uz << idBitWidth), id, idBitWidth);
    fflassert(peerIndex < (1uz << peerIndexBitWidth), peerIndex, peerIndexBitWidth);

    const auto seqID = seqIDOpt.value_or([id, startOff]() -> uint32_t // seq uses 32 bits at most
    {
        static std::mutex s_seqLock;
        static std::unordered_map<uint32_t, uint32_t> s_seqMap;
        {
            const std::lock_guard<std::mutex> lockGuard(s_seqLock);
            if(auto p = s_seqMap.find(id); p == s_seqMap.end()){
                return s_seqMap[id] = startOff;
            }
            else{
                return ++(p->second);
            }
        }
    }());

    fflassert(seqID < (1uz << seqBitWidth), seqID, seqBitWidth);
    return
        (to_u64(  uidType) <<   uidTypeBitOff) |
        (to_u64(peerIndex) << peerIndexBitOff) |
        (to_u64(       id) <<        idBitOff) |
        (to_u64(    seqID) <<       seqBitOff) ;
}

uint64_t uidf::buildMapUID    (uint32_t id, size_t peerIndex) { return _build_UID_helper<UID_MAP>(false, id, peerIndex, 2, std::nullopt); } // 1 used for map base UID
uint64_t uidf::buildNPCUID    (uint32_t id, size_t peerIndex) { return _build_UID_helper<UID_NPC>(true , id, peerIndex, 1, std::nullopt); }
uint64_t uidf::buildMonsterUID(uint32_t id, size_t peerIndex) { return _build_UID_helper<UID_MON>(false, id, peerIndex, 1, std::nullopt); }

uint64_t uidf::buildReceiverUID()
{
    static std::atomic<uint64_t> s_recvSeq {0};
    return (to_u64(UID_RCV) << uidTypeBitOff) + to_u64(++s_recvSeq);
}

uint64_t uidf::getPeerCoreUID(size_t peerIndex)
{
    fflassert(peerIndex < (1uz << peerIndexBitWidth), peerIndex, peerIndexBitWidth);
    return (to_u64(UID_COR) << uidTypeBitOff) | peerIndex;
}

uint64_t uidf::getServiceCoreUID()
{
    return uidf::getPeerCoreUID(0);
}

uint64_t uidf::getServerLuaObjectUID(uint32_t luaObjIndex)
{
    return (to_u64(UID_SLO) << uidTypeBitOff) + luaObjIndex;
}

uint64_t uidf::getQuestUID(uint32_t questID)
{
    return (to_u64(UID_QST) << uidTypeBitOff) + questID;
}

uint64_t uidf::getMapBaseUID(uint32_t mapID, size_t peerIndex)
{
    return _build_UID_helper<UID_MAP>(false, mapID, peerIndex, 2, 1);
}

uint64_t uidf::getPlayerUID(uint32_t dbid)
{
    return (to_u64(UID_PLY) << uidTypeBitOff) | to_u64(dbid);
}

uint32_t uidf::getPlayerDBID(uint64_t uid)
{
    fflassert(uidf::getUIDType(uid) == UID_PLY, uid, uidf::getUIDString(uid));
    return to_u32(uid);
}

#define _def_get_UID_id_helper(funcName, uidType) \
uint32_t funcName(uint64_t uid) \
{ \
    fflassert(uidf::getUIDType(uid) == uidType, uid, uidf::getUIDString(uid)); \
    return to_u32((uid & idBitMask) >> idBitOff); \
} \

_def_get_UID_id_helper(uidf::getMapID    , UID_MAP)
_def_get_UID_id_helper(uidf::getNPCID    , UID_NPC)
_def_get_UID_id_helper(uidf::getMonsterID, UID_MON)

#undef _def_get_UID_id_helper

#define _def_get_UID_seq_helper(funcName, uidType) \
uint64_t funcName(uint64_t uid, bool andPeerIndex) \
{ \
    fflassert(uidf::getUIDType(uid) == uidType, uid, andPeerIndex, uidf::getUIDString(uid), uidf::getUIDTypeCStr(uidType)); \
    return uid & ((andPeerIndex ? peerIndexBitMask : UINT64_C(0)) | seqBitMask); \
} \

_def_get_UID_seq_helper(uidf::getMapSeq    , UID_MAP)
_def_get_UID_seq_helper(uidf::getNPCSeq    , UID_NPC)
_def_get_UID_seq_helper(uidf::getMonsterSeq, UID_MON)

#undef _def_get_UID_seq_helper

uint64_t uidf::getReceiverSeq(uint64_t uid) { return uid & ((UINT64_C(1) << uidTypeBitOff) - 1); }
uint64_t uidf::getPeerCoreSeq(uint64_t uid) { return uid & ((UINT64_C(1) << uidTypeBitOff) - 1); }
uint32_t uidf::getQuestID    (uint64_t uid) { return uid & ((UINT64_C(1) << uidTypeBitOff) - 1); }

bool uidf::validUID(uint64_t uid)
{
    if(!uid){
        return false;
    }

    if(const auto type = uidf::getUIDType(uid); type < UID_BEGIN || type >= UID_END){
        return false;
    }
    else switch(type){
        case UID_COR: return true;

        case UID_MAP: return uidf::getMapSeq    (uid, false) > 0;
        case UID_NPC: return uidf::getNPCSeq    (uid, false) > 0;
        case UID_MON: return uidf::getMonsterSeq(uid, false) > 0;

        case UID_PLY: return uidf::getPlayerDBID (uid) > 0;
        case UID_RCV: return uidf::getReceiverSeq(uid) > 0;
        case UID_QST: return uidf::getQuestID    (uid) > 0;

        case UID_SLO: return true;
        default     : return false;
    }
}

bool uidf::isGM(uint64_t uid)
{
    return (uidf::getUIDType(uid) == UID_PLY) && (uidf::getPlayerDBID(uid) <= 10);
}

bool uidf::isMap(uint64_t uid)
{
    return (uidf::getUIDType(uid) == UID_MAP) && (uidf::getMapID(uid) > 0);
}

bool uidf::isBaseMap(uint64_t uid)
{
    return uidf::isMap(uid) && (uidf::getMapSeq(uid, false) == 1);
}

bool uidf::isPlayer(uint64_t uid)
{
    return uidf::getUIDType(uid) == UID_PLY;
}

bool uidf::isQuest(uint64_t uid)
{
    return uidf::getUIDType(uid) == UID_QST;
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
