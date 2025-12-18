#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <cinttypes>
#include "totype.hpp"
#include "fflerror.hpp"

// uid has type uint64_t but only uses 63 bits
// don't use highest bit because lua has difficulty to support unsigned 64-bit integers
//
//      63 : unused
// 62 - 59 : type field, supports 16 types
// 58 - 00 :

namespace uidf
{
    constexpr size_t uidTypeBits()
    {
        return 4;
    }

    constexpr size_t peerIndexBits()
    {
        return 4;
    }

    size_t peerIndex(uint64_t);
}

enum UIDType: int
{
    UID_NONE  = 0,
    UID_BEGIN = 1,

    UID_COR = UID_BEGIN,
    UID_MAP,
    UID_NPC,
    UID_MON,
    UID_PLY,
    UID_RCV,
    UID_QST,
    UID_SLO,

    UID_END,
};
static_assert(UID_END <= (1uz << uidf::uidTypeBits()));

namespace uidf
{
    int         getUIDType    (uint64_t);
    std::string getUIDString  (uint64_t);
    const char *getUIDTypeCStr(uint64_t);
}

namespace uidf
{
    uint64_t buildMapUID    (uint32_t, size_t /* peerIndex */);
    uint64_t buildNPCUID    (uint32_t, size_t /* peerIndex */);
    uint64_t buildMonsterUID(uint32_t, size_t /* peerIndex */);

    uint64_t buildReceiverUID();

    uint64_t getPeerCoreUID(size_t);
    uint64_t getServiceCoreUID();
    uint64_t getServerLuaObjectUID(uint32_t);

    uint64_t getQuestUID(uint32_t);
    uint64_t getPlayerUID(uint32_t);
    uint64_t getMapBaseUID(uint32_t, size_t /* peerIndex */);
}

namespace uidf
{
    uint32_t getPlayerDBID(uint64_t);

    uint32_t getMapID(uint64_t);
    uint32_t getNPCID(uint64_t);
    uint32_t getQuestID(uint64_t);
    uint32_t getMonsterID(uint64_t);

    uint64_t getMapSeq(uint64_t, bool);
    uint64_t getNPCSeq(uint64_t, bool);
    uint64_t getMonsterSeq(uint64_t, bool);

    uint64_t getPeerCoreSeq(uint64_t);
    uint64_t getReceiverSeq(uint64_t);
}

namespace uidf
{
    bool validUID(uint64_t);

    bool isGM(uint64_t);
    bool isPlayer(uint64_t);
    bool isQuest(uint64_t);
    bool isNPChar(uint64_t);
    bool isReceiver(uint64_t);

    bool isMap(uint64_t);
    bool isBaseMap(uint64_t);

    bool isMonster(uint64_t);
    bool isMonster(uint64_t, uint32_t);
    bool isMonster(uint64_t, const char8_t *);

    bool isGuardMode(uint64_t);
    bool isNeutralMode(uint64_t);
}
