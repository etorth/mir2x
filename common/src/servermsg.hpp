/*
 * =====================================================================================
 *
 *       Filename: servermsg.hpp
 *        Created: 01/24/2016 19:30:45
 *    Description: net message used by client and mono-server
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
#include <cstdint>
#include <unordered_map>
#include "msgbase.hpp"
#include "actionnode.hpp"

enum SMType: uint8_t
{
    SM_NONE_0  = 0,
    SM_BEGIN   = 1,
    SM_PING    = 1,
    SM_ACCOUNT,
    SM_LOGINOK,
    SM_LOGINFAIL,
    SM_RUNTIMECONFIG,
    SM_LEARNEDMAGICLIST,
    SM_PLAYERWLDESP,
    SM_ACTION,
    SM_CORECORD,
    SM_UPDATEHP,
    SM_NOTIFYDEAD,
    SM_DEADFADEOUT,
    SM_EXP,
    SM_BUFF,
    SM_MISS,
    SM_CASTMAGIC,
    SM_OFFLINE,
    SM_PICKUPERROR,
    SM_REMOVEGROUNDITEM,
    SM_NPCXMLLAYOUT,
    SM_NPCSELL,
    SM_STARTINVOP,
    SM_STARTINPUT,
    SM_GOLD,
    SM_INVOPCOST,
    SM_STRIKEGRID,
    SM_SELLITEMLIST,
    SM_TEXT,
    SM_PLAYERNAME,
    SM_BUILDVERSION,
    SM_INVENTORY,
    SM_BELT,
    SM_UPDATEITEM,
    SM_REMOVEITEM,
    SM_REMOVESECUREDITEM,
    SM_BUYSUCCEED,
    SM_BUYERROR,
    SM_GROUNDITEMIDLIST,
    SM_GROUNDFIREWALLLIST,
    SM_EQUIPWEAR,
    SM_EQUIPWEARERROR,
    SM_GRABWEAR,
    SM_GRABWEARERROR,
    SM_EQUIPBELT,
    SM_EQUIPBELTERROR,
    SM_GRABBELT,
    SM_GRABBELTERROR,
    SM_SHOWSECUREDITEMLIST,
    SM_END,
};

#pragma pack(push, 1)
struct SMPing
{
    uint32_t Tick;
};

struct SMAccount
{
    uint32_t error;
};

struct SMLoginFail
{
    uint32_t error;
};

struct SMAction
{
    uint64_t UID;
    uint32_t mapID;
    ActionNode action;
};

struct SMCORecord
{
    uint64_t UID;
    uint32_t mapID;
    ActionNode action;

    struct _SMCORecord_Monster
    {
        uint32_t MonsterID;
    };

    struct _SMCORecord_Player
    {
        uint32_t Level;
    };

    struct _SMCORecord_NPC
    {
        uint32_t NPCID;
    };

    union
    {
        _SMCORecord_Monster Monster;
        _SMCORecord_Player  Player;
        _SMCORecord_NPC     NPC;
    };
};

struct SMUpdateHP
{
    uint64_t UID;
    uint32_t mapID;

    uint32_t HP;
    uint32_t HPMax;
};

struct SMNotifyDead
{
    uint64_t UID;
};

struct SMDeadFadeOut
{
    uint64_t UID;
    uint32_t mapID;

    uint32_t X;
    uint32_t Y;
};

struct SMExp
{
    uint32_t exp;
};

struct SMBuff
{
    uint64_t uid;
    uint32_t type;
    uint32_t state;
};

struct SMMiss
{
    uint64_t UID;
};

struct SMCastMagic
{
    uint64_t UID;
    uint32_t mapID;

    uint8_t Magic;
    uint8_t MagicParam;
    uint8_t Speed;
    uint8_t Direction;

    uint16_t X;
    uint16_t Y;
    uint16_t AimX;
    uint16_t AimY;
    uint64_t AimUID;
};

struct SMOffline
{
    uint64_t UID;
    uint32_t mapID;
};

struct SMPickUpError
{
    uint32_t failedItemID;
};

struct SMRemoveGroundItem
{
    uint16_t X;
    uint16_t Y;
    uint32_t ID;
    uint32_t DBID;
};

struct SMGold
{
    uint32_t gold;
};

struct SMInvOpCost
{
    uint32_t invOp;
    uint32_t itemID;
    uint32_t seqID;
    uint32_t cost;
};

struct SMStrikeGrid
{
    uint32_t x;
    uint32_t y;
};

struct SMPlayerName
{
    uint64_t uid;
    char name[128];
    uint32_t nameColor;
};

struct SMBuildVersion
{
    char version[128];
};

struct SMRemoveItem
{
    uint32_t itemID;
    uint32_t  seqID;
    uint16_t  count;
};

struct SMRemoveSecuredItem
{
    uint32_t itemID;
    uint32_t  seqID;
};

struct SMBuySucceed
{
    uint64_t npcUID;
    uint32_t itemID;
    uint32_t  seqID;
};

struct SMBuyError
{
    uint64_t npcUID;
    uint32_t itemID;
    uint32_t  seqID;
    uint16_t  error;
};

struct SMEquipWearError
{
    uint32_t itemID;
    uint32_t  seqID;
    uint16_t  error;
};

struct SMGrabWearError
{
    uint16_t error;
};

struct SMEquipBeltError
{
    uint32_t itemID;
    uint32_t  seqID;
    uint16_t  error;
};

struct SMGrabBeltError
{
    uint16_t error;
};
#pragma pack(pop)

class ServerMsg final: public MsgBase
{
    public:
        ServerMsg(uint8_t headCode)
            : MsgBase(headCode)
        {}

    private:
        const MsgAttribute &getAttribute() const override
        {
            static const std::unordered_map<uint8_t, MsgAttribute> s_msgAttributeTable
            {
                //  0    :     empty
                //  1    : not empty,     fixed size,     compressed
                //  2    : not empty,     fixed size, not compressed
                //  3    : not empty, not fixed size, not compressed
#define _add_server_msg_type_case(type, encodeType, length) {type, {encodeType, length, #type}},
                _add_server_msg_type_case(SM_NONE_0,              0, 0                          )
                _add_server_msg_type_case(SM_PING,                2, sizeof(SMPing)             )
                _add_server_msg_type_case(SM_ACCOUNT,             1, sizeof(SMAccount)          )
                _add_server_msg_type_case(SM_LOGINOK,             3, 0                          )
                _add_server_msg_type_case(SM_RUNTIMECONFIG,       3, 0                          )
                _add_server_msg_type_case(SM_LEARNEDMAGICLIST,    3, 0                          )
                _add_server_msg_type_case(SM_LOGINFAIL,           2, sizeof(SMLoginFail)        )
                _add_server_msg_type_case(SM_PLAYERWLDESP,        3, 0                          )
                _add_server_msg_type_case(SM_ACTION,              1, sizeof(SMAction)           )
                _add_server_msg_type_case(SM_CORECORD,            1, sizeof(SMCORecord)         )
                _add_server_msg_type_case(SM_UPDATEHP,            1, sizeof(SMUpdateHP)         )
                _add_server_msg_type_case(SM_NOTIFYDEAD,          1, sizeof(SMNotifyDead)       )
                _add_server_msg_type_case(SM_DEADFADEOUT,         1, sizeof(SMDeadFadeOut)      )
                _add_server_msg_type_case(SM_EXP,                 1, sizeof(SMExp)              )
                _add_server_msg_type_case(SM_BUFF,                1, sizeof(SMBuff)             )
                _add_server_msg_type_case(SM_MISS,                1, sizeof(SMMiss)             )
                _add_server_msg_type_case(SM_CASTMAGIC,           1, sizeof(SMCastMagic)        )
                _add_server_msg_type_case(SM_OFFLINE,             1, sizeof(SMOffline)          )
                _add_server_msg_type_case(SM_PICKUPERROR,         1, sizeof(SMPickUpError)      )
                _add_server_msg_type_case(SM_REMOVEGROUNDITEM,    1, sizeof(SMRemoveGroundItem) )
                _add_server_msg_type_case(SM_NPCXMLLAYOUT,        3, 0                          )
                _add_server_msg_type_case(SM_NPCSELL,             3, 0                          )
                _add_server_msg_type_case(SM_STARTINVOP,          3, 0                          )
                _add_server_msg_type_case(SM_STARTINPUT,          3, 0                          )
                _add_server_msg_type_case(SM_GOLD,                1, sizeof(SMGold)             )
                _add_server_msg_type_case(SM_INVOPCOST,           1, sizeof(SMInvOpCost)        )
                _add_server_msg_type_case(SM_STRIKEGRID,          1, sizeof(SMStrikeGrid)       )
                _add_server_msg_type_case(SM_SELLITEMLIST,        3, 0                          )
                _add_server_msg_type_case(SM_TEXT,                3, 0                          )
                _add_server_msg_type_case(SM_PLAYERNAME,          1, sizeof(SMPlayerName)       )
                _add_server_msg_type_case(SM_BUILDVERSION,        1, sizeof(SMBuildVersion)     )
                _add_server_msg_type_case(SM_INVENTORY,           3, 0                          )
                _add_server_msg_type_case(SM_BELT,                3, 0                          )
                _add_server_msg_type_case(SM_UPDATEITEM,          3, 0                          )
                _add_server_msg_type_case(SM_REMOVEITEM,          1, sizeof(SMRemoveItem)       )
                _add_server_msg_type_case(SM_REMOVESECUREDITEM,   1, sizeof(SMRemoveSecuredItem))
                _add_server_msg_type_case(SM_BUYSUCCEED,          1, sizeof(SMBuySucceed)       )
                _add_server_msg_type_case(SM_BUYERROR,            1, sizeof(SMBuyError)         )
                _add_server_msg_type_case(SM_GROUNDITEMIDLIST,    3, 0                          )
                _add_server_msg_type_case(SM_GROUNDFIREWALLLIST,  3, 0                          )
                _add_server_msg_type_case(SM_EQUIPWEAR,           3, 0                          )
                _add_server_msg_type_case(SM_EQUIPWEARERROR,      1, sizeof(SMEquipWearError)   )
                _add_server_msg_type_case(SM_GRABWEAR,            3, 0                          )
                _add_server_msg_type_case(SM_GRABWEARERROR,       1, sizeof(SMGrabWearError)    )
                _add_server_msg_type_case(SM_EQUIPBELT,           3, 0                          )
                _add_server_msg_type_case(SM_EQUIPBELTERROR,      1, sizeof(SMEquipBeltError)   )
                _add_server_msg_type_case(SM_GRABBELT,            3, 0                          )
                _add_server_msg_type_case(SM_GRABBELTERROR,       1, sizeof(SMGrabBeltError)    )
                _add_server_msg_type_case(SM_SHOWSECUREDITEMLIST, 3, 0                          )
#undef _add_server_msg_type_case
            };

            if(const auto p = s_msgAttributeTable.find(m_headCode); p != s_msgAttributeTable.end()){
                return p->second;
            }
            return s_msgAttributeTable.at(SM_NONE_0);
        }

    public:
        template<typename T> static T conv(const uint8_t *buf, size_t bufLen = 0)
        {
            static_assert(false
                    || std::is_same_v<T, SMPing>
                    || std::is_same_v<T, SMAccount>
                    || std::is_same_v<T, SMLoginFail>
                    || std::is_same_v<T, SMAction>
                    || std::is_same_v<T, SMCORecord>
                    || std::is_same_v<T, SMUpdateHP>
                    || std::is_same_v<T, SMNotifyDead>
                    || std::is_same_v<T, SMDeadFadeOut>
                    || std::is_same_v<T, SMExp>
                    || std::is_same_v<T, SMBuff>
                    || std::is_same_v<T, SMMiss>
                    || std::is_same_v<T, SMCastMagic>
                    || std::is_same_v<T, SMOffline>
                    || std::is_same_v<T, SMRemoveGroundItem>
                    || std::is_same_v<T, SMInvOpCost>
                    || std::is_same_v<T, SMPlayerName>
                    || std::is_same_v<T, SMBuildVersion>
                    || std::is_same_v<T, SMRemoveItem>
                    || std::is_same_v<T, SMRemoveSecuredItem>
                    || std::is_same_v<T, SMGold>
                    || std::is_same_v<T, SMStrikeGrid>
                    || std::is_same_v<T, SMBuySucceed>
                    || std::is_same_v<T, SMBuyError>
                    || std::is_same_v<T, SMPickUpError>
                    || std::is_same_v<T, SMEquipWearError>
                    || std::is_same_v<T, SMGrabWearError>
                    || std::is_same_v<T, SMEquipBeltError>
                    || std::is_same_v<T, SMGrabBeltError>);

            if(bufLen && bufLen != sizeof(T)){
                throw fflerror("invalid buffer length");
            }

            T t;
            std::memcpy(&t, buf, sizeof(t));
            return t;
        }
};
