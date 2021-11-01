#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <cinttypes>
#include "totype.hpp"
#include "fflerror.hpp"

enum UIDType: int
{
    UID_ERR =  0,
    UID_COR =  1,
    UID_NPC =  2,
    UID_MAP =  3,
    UID_PLY =  4,
    UID_MON =  5,
    UID_U06 =  6,
    UID_U07 =  7,
    UID_U08 =  8,
    UID_U09 =  9,
    UID_U10 = 10,
    UID_U11 = 11,
    UID_U12 = 12,
    UID_U13 = 13,
    UID_ETC = 14,
    UID_INN = 15, // put the internal out of 0 ~ 15
    UID_MAX = 16,
};

namespace uidf
{
    uint64_t buildEtcUID();
    uint64_t buildNPCUID(uint16_t);
    uint64_t buildMonsterUID(uint32_t);

    uint64_t getMapUID(uint32_t);
    uint64_t getServiceCoreUID();
    uint64_t getPlayerUID(uint32_t, bool, const std::vector<int> &);
}

namespace uidf
{
    bool isReceiver(uint64_t);
    uint64_t buildReceiverUID();
}

namespace uidf
{
    inline int getUIDType(uint64_t uid)
    {
        if(uid & 0X00FF000000000000ULL){
            return UID_INN;
        }
        return to_d((uid & 0X0000F00000000000ULL) >> 44);
    }

    inline const char *getUIDTypeCStr(uint64_t uid)
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
    bool isGM(uint64_t);
    bool hasPlayerJob(uint64_t, int);
    bool getPlayerGender(uint64_t);
    uint32_t getPlayerDBID(uint64_t);
}

namespace uidf
{
    inline uint32_t getMonsterID(uint64_t uid)
    {
        fflassert(getUIDType(uid) == UID_MON);
        return to_u32((uid & 0X00000FFE00000000ULL) >> 33);
    }

    inline uint32_t getMonsterSeq(uint64_t uid)
    {
        fflassert(getUIDType(uid) == UID_MON);
        return to_u32(uid & 0XFFFFFFFFULL);
    }

    uint32_t getMapID(uint64_t);
    uint16_t getLookID(uint64_t);

    uint16_t getNPCID(uint64_t);
    uint16_t getNPCSeqID(uint64_t);
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
        throw fflerror("invalid uid string: %s", to_cstr(uidString));
    }
}

namespace uidf
{
    inline bool isPlayer(uint64_t uid)
    {
        return uidf::getUIDType(uid) == UID_PLY;
    }

    inline bool isNPChar(uint64_t uid)
    {
        return uidf::getUIDType(uid) == UID_NPC;
    }

    inline bool isMonster(uint64_t uid)
    {
        return uidf::getUIDType(uid) == UID_MON;
    }

    bool isMonster(uint64_t, uint32_t);
    bool isMonster(uint64_t, const char8_t *);

    bool isGuardMode(uint64_t);
    bool isNeutralMode(uint64_t);
}
