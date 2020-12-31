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
    SM_ACTION,
    SM_CORECORD,
    SM_UPDATEHP,
    SM_NOTIFYDEAD,
    SM_DEADFADEOUT,
    SM_EXP,
    SM_MISS,
    SM_SHOWDROPITEM,
    SM_CASTMAGIC,
    SM_SPACEMOVE,
    SM_OFFLINE,
    SM_PICKUPOK,
    SM_REMOVEGROUNDITEM,
    SM_NPCXMLLAYOUT,
    SM_NPCSELL,
    SM_GOLD,
    SM_TEXT,
    SM_MAX,
};

#pragma pack(push, 1)
struct SMPing
{
    uint32_t Tick;
};

struct SMAccount
{
    // register operation for the account
    // 0 : validate this account
    // 1 : create account
    // 2 : login
    uint8_t Operation;
    uint8_t Respond;

    char ID[64];
    char Password[128];
};

struct SMLoginOK
{
    uint64_t UID;
    uint32_t DBID;
    uint32_t MapID;
    uint16_t X;
    uint16_t Y;

    uint8_t Male;
    uint8_t Direction;

    uint32_t JobID;
    uint32_t Level;
};

struct SMLoginFail
{
    uint32_t FailID;
};

struct SMAction
{
    uint64_t UID;
    uint32_t MapID;
    ActionNode action;
};

struct SMCORecord
{
    uint64_t UID;
    uint32_t MapID;
    ActionNode action;

    struct _SMCORecord_Monster
    {
        uint32_t MonsterID;
    };

    struct _SMCORecord_Player
    {
        uint32_t DBID;
        uint32_t JobID;
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
    uint32_t MapID;

    uint32_t HP;
    uint32_t HPMax;
};

struct SMDeadFadeOut
{
    uint64_t UID;
    uint32_t MapID;

    uint32_t X;
    uint32_t Y;
};

struct SMNotifyDead
{
    uint64_t UID;
};

struct SMExp
{
    uint32_t Exp;
};

struct SMMiss
{
    uint64_t UID;
};

struct SMShowDropItem
{
    struct _CommonItem
    {
        uint32_t ID;
        uint32_t DBID;
    }IDList[16];

    uint16_t X;
    uint16_t Y;
};

struct SMCastMagic
{
    uint64_t UID;
    uint32_t MapID;

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
    uint32_t MapID;
};

struct SMRemoveGroundItem
{
    uint16_t X;
    uint16_t Y;
    uint32_t ID;
    uint32_t DBID;
};

struct SMPickUpOK
{
    uint16_t X;
    uint16_t Y;
    uint32_t ID;
    uint32_t DBID;
};

struct SMGold
{
    uint32_t Gold;
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
                _add_server_msg_type_case(SM_NONE_0,           0, 0                         )
                _add_server_msg_type_case(SM_PING,             2, sizeof(SMPing)            )
                _add_server_msg_type_case(SM_ACCOUNT,          1, sizeof(SMAccount)         )
                _add_server_msg_type_case(SM_LOGINOK,          1, sizeof(SMLoginOK)         )
                _add_server_msg_type_case(SM_LOGINFAIL,        2, sizeof(SMLoginFail)       )
                _add_server_msg_type_case(SM_ACTION,           1, sizeof(SMAction)          )
                _add_server_msg_type_case(SM_CORECORD,         1, sizeof(SMCORecord)        )
                _add_server_msg_type_case(SM_UPDATEHP,         1, sizeof(SMUpdateHP)        )
                _add_server_msg_type_case(SM_NOTIFYDEAD,       1, sizeof(SMNotifyDead)      )
                _add_server_msg_type_case(SM_DEADFADEOUT,      1, sizeof(SMDeadFadeOut)     )
                _add_server_msg_type_case(SM_EXP,              1, sizeof(SMExp)             )
                _add_server_msg_type_case(SM_MISS,             1, sizeof(SMMiss)            )
                _add_server_msg_type_case(SM_SHOWDROPITEM,     1, sizeof(SMShowDropItem)    )
                _add_server_msg_type_case(SM_CASTMAGIC,        1, sizeof(SMCastMagic)       )
                _add_server_msg_type_case(SM_OFFLINE,          1, sizeof(SMOffline)         )
                _add_server_msg_type_case(SM_PICKUPOK,         1, sizeof(SMPickUpOK)        )
                _add_server_msg_type_case(SM_REMOVEGROUNDITEM, 1, sizeof(SMRemoveGroundItem))
                _add_server_msg_type_case(SM_NPCXMLLAYOUT,     3, 0                         )
                _add_server_msg_type_case(SM_NPCSELL,          3, 0                         )
                _add_server_msg_type_case(SM_GOLD,             1, sizeof(SMGold)            )
                _add_server_msg_type_case(SM_TEXT,             3, 0                         )
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
                    || std::is_same_v<T, SMLoginOK>
                    || std::is_same_v<T, SMLoginFail>
                    || std::is_same_v<T, SMAction>
                    || std::is_same_v<T, SMCORecord>
                    || std::is_same_v<T, SMUpdateHP>
                    || std::is_same_v<T, SMNotifyDead>
                    || std::is_same_v<T, SMDeadFadeOut>
                    || std::is_same_v<T, SMExp>
                    || std::is_same_v<T, SMMiss>
                    || std::is_same_v<T, SMShowDropItem>
                    || std::is_same_v<T, SMCastMagic>
                    || std::is_same_v<T, SMOffline>
                    || std::is_same_v<T, SMPickUpOK>
                    || std::is_same_v<T, SMRemoveGroundItem>
                    || std::is_same_v<T, SMGold>);

            if(bufLen && bufLen != sizeof(T)){
                throw fflerror("invalid buffer length");
            }

            T t;
            std::memcpy(&t, buf, sizeof(t));
            return t;
        }
};
