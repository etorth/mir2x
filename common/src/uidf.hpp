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

    UID_END,
};
static_assert(UID_END <= 16);

namespace uidf
{
    int         getUIDType    (uint64_t);
    std::string getUIDString  (uint64_t);
    const char *getUIDTypeCStr(uint64_t);
}

namespace uidf
{
    uint64_t buildMapUID(uint32_t);
    uint64_t buildNPCUID(uint32_t);
    uint64_t buildMonsterUID(uint32_t);
    uint64_t buildReceiverUID();

    uint64_t getServiceCoreUID();
    uint64_t getMapBaseUID(uint32_t);
    uint64_t getPlayerUID(uint32_t, bool, const std::vector<int> &);
}

namespace uidf
{
    uint32_t getMapID(uint64_t);
    uint32_t getNPCID(uint64_t);
    uint32_t getMonsterID(uint64_t);

    uint32_t getMapSeq(uint64_t);
    uint32_t getNPCSeq(uint64_t);
    uint32_t getMonsterSeq(uint64_t);

    uint64_t getReceiverSeq(uint64_t);
}

namespace uidf
{
    bool     hasPlayerJob   (uint64_t, int);
    uint32_t getPlayerDBID  (uint64_t);
    bool     getPlayerGender(uint64_t);
}

namespace uidf
{
    bool isGM(uint64_t);
    bool isPlayer(uint64_t);
    bool isNPChar(uint64_t);
    bool isReceiver(uint64_t);

    bool isMonster(uint64_t);
    bool isMonster(uint64_t, uint32_t);
    bool isMonster(uint64_t, const char8_t *);

    bool isGuardMode(uint64_t);
    bool isNeutralMode(uint64_t);
}
