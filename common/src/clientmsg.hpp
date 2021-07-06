/*
 * =====================================================================================
 *
 *       Filename: clientmsg.hpp
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
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <unordered_map>
#include "msgbase.hpp"
#include "actionnode.hpp"

enum CMType: uint8_t
{
    // CM_NONE already used on windows
    // outside of this file use zero directly for invalid value
    CM_NONE_0 = 0,
    CM_BEGIN  = 1,
    CM_PING   = 1,
    CM_LOGIN,
    CM_ACTION,
    CM_SETMAGICKEY,
    CM_QUERYCORECORD,
    CM_REQUESTKILLPETS,
    CM_REQUESTSPACEMOVE,
    CM_REQUESTMAGICDAMAGE,
    CM_PICKUP,
    CM_QUERYGOLD,
    CM_QUERYPLAYERWLDESP,
    CM_ACCOUNT,
    CM_NPCEVENT,
    CM_QUERYSELLITEMLIST,
    CM_DROPITEM,
    CM_CONSUMEITEM,
    CM_BUY,
    CM_REQUESTEQUIPWEAR,
    CM_REQUESTGRABWEAR,
    CM_REQUESTEQUIPBELT,
    CM_REQUESTGRABBELT,
    CM_END,
};

#pragma pack(push, 1)
struct CMPing
{
    uint32_t Tick;
};

struct CMLogin
{
    char id[64];
    char password[64];
};

struct CMSetMagicKey
{
    uint32_t magicID;
    uint8_t key;
};

struct CMAction
{
    uint64_t UID;
    uint32_t mapID;
    ActionNode action;
};

struct CMQueryCORecord
{
    uint64_t AimUID;
};

struct CMRequestSpaceMove
{
    uint32_t mapID;
    uint16_t X;
    uint16_t Y;
};

struct CMRequestMagicDamage
{
    uint32_t magicID;
    uint64_t aimUID;
};

struct CMPickUp
{
    uint16_t x;
    uint16_t y;
    uint32_t mapID;
};

struct CMQueryPlayerWLDesp
{
    uint64_t uid;
};

struct CMAccount
{
    char id[64];
    char password[128];
};

struct CMNPCEvent
{
    uint64_t uid;
    char event[ 64];
    char value[256];
};

struct CMQuerySellItemList
{
    uint64_t npcUID;
    uint32_t itemID;
};

struct CMDropItem
{
    uint32_t itemID;
    uint32_t seqID;
    uint16_t count;
};

struct CMConsumeItem
{
    uint32_t itemID;
    uint32_t seqID;
    uint16_t count;
};

struct CMBuy
{
    uint64_t npcUID;
    uint32_t itemID;
    uint32_t seqID;
    uint32_t count;
};

struct CMRequestEquipWear
{
    uint32_t itemID;
    uint32_t seqID;
    uint16_t wltype;
};

struct CMRequestGrabWear
{
    uint16_t wltype;
};

struct CMRequestEquipBelt
{
    uint32_t itemID;
    uint32_t seqID;
    uint16_t slot;
};

struct CMRequestGrabBelt
{
    uint16_t slot;
};
#pragma pack(pop)

// I was using class name ClientMessage
// unfortunately this name has been taken by Xlib
// and ClientMessage seems like a real message but actually this class only give general information

class ClientMsg final: public MsgBase
{
    public:
        ClientMsg(uint8_t headCode)
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
#define _add_client_msg_type_case(type, encodeType, length) {type, {encodeType, length, #type}},
                _add_client_msg_type_case(CM_NONE_0,              0, 0                            )
                _add_client_msg_type_case(CM_PING,                2, sizeof(CMPing)               )
                _add_client_msg_type_case(CM_LOGIN,               1, sizeof(CMLogin)              )
                _add_client_msg_type_case(CM_ACTION,              1, sizeof(CMAction)             )
                _add_client_msg_type_case(CM_SETMAGICKEY,         1, sizeof(CMSetMagicKey)        )
                _add_client_msg_type_case(CM_QUERYCORECORD,       1, sizeof(CMQueryCORecord)      )
                _add_client_msg_type_case(CM_REQUESTKILLPETS,     0, 0                            )
                _add_client_msg_type_case(CM_REQUESTSPACEMOVE,    1, sizeof(CMRequestSpaceMove)   )
                _add_client_msg_type_case(CM_REQUESTMAGICDAMAGE,  1, sizeof(CMRequestMagicDamage) )
                _add_client_msg_type_case(CM_PICKUP,              1, sizeof(CMPickUp)             )
                _add_client_msg_type_case(CM_QUERYGOLD,           0, 0                            )
                _add_client_msg_type_case(CM_QUERYPLAYERWLDESP,   1, sizeof(CMQueryPlayerWLDesp)  )
                _add_client_msg_type_case(CM_ACCOUNT,             1, sizeof(CMAccount)            )
                _add_client_msg_type_case(CM_NPCEVENT,            1, sizeof(CMNPCEvent)           )
                _add_client_msg_type_case(CM_QUERYSELLITEMLIST,   1, sizeof(CMQuerySellItemList)  )
                _add_client_msg_type_case(CM_DROPITEM,            1, sizeof(CMDropItem)           )
                _add_client_msg_type_case(CM_CONSUMEITEM,         1, sizeof(CMConsumeItem)        )
                _add_client_msg_type_case(CM_BUY,                 1, sizeof(CMBuy)                )
                _add_client_msg_type_case(CM_REQUESTEQUIPWEAR,    1, sizeof(CMRequestEquipWear)   )
                _add_client_msg_type_case(CM_REQUESTGRABWEAR,     1, sizeof(CMRequestGrabWear)    )
                _add_client_msg_type_case(CM_REQUESTEQUIPBELT,    1, sizeof(CMRequestEquipBelt)   )
                _add_client_msg_type_case(CM_REQUESTGRABBELT,     1, sizeof(CMRequestGrabBelt)    )
#undef _add_client_msg_type_case
            };

            if(const auto p = s_msgAttributeTable.find(m_headCode); p != s_msgAttributeTable.end()){
                return p->second;
            }
            return s_msgAttributeTable.at(CM_NONE_0);
        }

    public:
        template<typename T> static T conv(const uint8_t *buf, size_t bufLen = 0)
        {
            static_assert(false
                    || std::is_same_v<T, CMPing>
                    || std::is_same_v<T, CMLogin>
                    || std::is_same_v<T, CMAction>
                    || std::is_same_v<T, CMQueryCORecord>
                    || std::is_same_v<T, CMRequestSpaceMove>
                    || std::is_same_v<T, CMRequestMagicDamage>
                    || std::is_same_v<T, CMPickUp>
                    || std::is_same_v<T, CMSetMagicKey>
                    || std::is_same_v<T, CMQueryPlayerWLDesp>
                    || std::is_same_v<T, CMAccount>
                    || std::is_same_v<T, CMNPCEvent>
                    || std::is_same_v<T, CMQuerySellItemList>
                    || std::is_same_v<T, CMDropItem>
                    || std::is_same_v<T, CMConsumeItem>
                    || std::is_same_v<T, CMBuy>
                    || std::is_same_v<T, CMRequestEquipWear>
                    || std::is_same_v<T, CMRequestGrabWear>
                    || std::is_same_v<T, CMRequestEquipBelt>
                    || std::is_same_v<T, CMRequestGrabBelt>);

            if(bufLen && bufLen != sizeof(T)){
                throw fflerror("invalid buffer length");
            }

            T t;
            std::memcpy(&t, buf, sizeof(t));
            return t;
        }
};
