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
    CM_QUERYCORECORD,
    CM_REQUESTKILLPETS,
    CM_REQUESTSPACEMOVE,
    CM_REQUESTMAGICDAMAGE,
    CM_PICKUP,
    CM_QUERYGOLD,
    CM_QUERYPLAYERLOOK,
    CM_QUERYPLAYERWEAR,
    CM_ACCOUNT,
    CM_NPCEVENT,
    CM_QUERYSELLITEM,
    CM_END,
};

#pragma pack(push, 1)
struct CMPing
{
    uint32_t Tick;
};

struct CMLogin
{
    char ID[64];
    char Password[128];
};

struct CMAction
{
    uint64_t UID;
    uint32_t MapID;
    ActionNode action;
};

struct CMQueryCORecord
{
    uint64_t AimUID;
};

struct CMRequestSpaceMove
{
    uint32_t MapID;
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
    uint16_t X;
    uint16_t Y;
    uint64_t UID;
    uint32_t MapID;
    uint32_t ID;
    uint32_t DBID;
};

struct CMQueryPlayerLook
{
    uint64_t uid;
};

struct CMQueryPlayerWear
{
    uint64_t uid;
};

struct CMAccount
{
    // register operation for the account
    // 0 : validate this account
    // 1 : create account
    // 2 : login
    uint8_t Operation;

    char ID[64];
    char Password[128];
};

struct CMNPCEvent
{
    uint64_t uid;
    char event[32];
    char value[32];
};

struct CMQuerySellItem
{
    uint64_t npcUID;
    uint32_t itemID;
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
                _add_client_msg_type_case(CM_NONE_0,             0, 0                           )
                _add_client_msg_type_case(CM_PING,               2, sizeof(CMPing)              )
                _add_client_msg_type_case(CM_LOGIN,              1, sizeof(CMLogin)             )
                _add_client_msg_type_case(CM_ACTION,             1, sizeof(CMAction)            )
                _add_client_msg_type_case(CM_QUERYCORECORD,      1, sizeof(CMQueryCORecord)     )
                _add_client_msg_type_case(CM_REQUESTKILLPETS,    0, 0                           )
                _add_client_msg_type_case(CM_REQUESTSPACEMOVE,   1, sizeof(CMRequestSpaceMove)  )
                _add_client_msg_type_case(CM_REQUESTMAGICDAMAGE, 1, sizeof(CMRequestMagicDamage))
                _add_client_msg_type_case(CM_PICKUP,             1, sizeof(CMPickUp)            )
                _add_client_msg_type_case(CM_QUERYGOLD,          0, 0                           )
                _add_client_msg_type_case(CM_QUERYPLAYERLOOK,    1, sizeof(CMQueryPlayerLook)   )
                _add_client_msg_type_case(CM_QUERYPLAYERWEAR,    1, sizeof(CMQueryPlayerWear)   )
                _add_client_msg_type_case(CM_ACCOUNT,            1, sizeof(CMAccount)           )
                _add_client_msg_type_case(CM_NPCEVENT,           1, sizeof(CMNPCEvent)          )
                _add_client_msg_type_case(CM_QUERYSELLITEM,      1, sizeof(CMQuerySellItem)     )
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
                    || std::is_same_v<T, CMQueryPlayerLook>
                    || std::is_same_v<T, CMQueryPlayerWear>
                    || std::is_same_v<T, CMAccount>
                    || std::is_same_v<T, CMNPCEvent>
                    || std::is_same_v<T, CMQuerySellItem>);

            if(bufLen && bufLen != sizeof(T)){
                throw fflerror("invalid buffer length");
            }

            T t;
            std::memcpy(&t, buf, sizeof(t));
            return t;
        }
};
