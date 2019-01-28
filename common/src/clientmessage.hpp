/*
 * =====================================================================================
 *
 *       Filename: clientmessage.hpp
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
#include <unordered_map>
#include "messagebase.hpp"

enum CMType: uint8_t
{
    // CM_NONE already used on windows
    // outside of this file use zero directly for invalid value
    CM_NONE_0 = 0,

    CM_PING,
    CM_LOGIN,
    CM_ACTION,
    CM_QUERYMONSTERGINFO,
    CM_QUERYCORECORD,

    CM_REQUESTSPACEMOVE,
    CM_PICKUP,
    CM_QUERYGOLD,
    CM_ACCOUNT,
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

    uint8_t Action;
    uint8_t Speed;
    uint8_t Direction;

    uint16_t X;
    uint16_t Y;
    uint16_t AimX;
    uint16_t AimY;

    uint64_t AimUID;
    uint64_t ActionParam;
};

struct CMQueryCORecord
{
    uint64_t AimUID;
};

struct CMReqestSpaceMove
{
    uint32_t MapID;
    uint16_t X;
    uint16_t Y;
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
#pragma pack(pop)

// I was using class name ClientMessage
// unfortunately this name has been taken by Xlib
// and ClientMessage seems like a real message but actually this class only give general information

class CMSGParam: public MessageBase
{
    public:
        CMSGParam(uint8_t nHC)
            : MessageBase(nHC)
        {}

    private:
        const MessageAttribute &GetAttribute(uint8_t nHC) const
        {
            static const std::unordered_map<uint8_t, MessageAttribute> s_AttributeTable
            {
                //  0    :     empty
                //  1    : not empty,     fixed size,     compressed
                //  2    : not empty,     fixed size, not compressed
                //  3    : not empty, not fixed size, not compressed

                {CM_NONE_0,           {0, 0,                         "CM_NONE"            }},
                {CM_PING,             {2, sizeof(CMPing),            "CM_PING"            }},
                {CM_LOGIN,            {1, sizeof(CMLogin),           "CM_LOGIN"           }},
                {CM_ACTION,           {1, sizeof(CMAction),          "CM_ACTION"          }},
                {CM_QUERYCORECORD,    {1, sizeof(CMQueryCORecord),   "CM_QUERYCORECORD"   }},
                {CM_REQUESTSPACEMOVE, {1, sizeof(CMReqestSpaceMove), "CM_REQUESTSPACEMOVE"}},
                {CM_PICKUP,           {1, sizeof(CMPickUp),          "CM_PICKUP"          }},
                {CM_QUERYGOLD,        {0, 0,                         "CM_QUERYGOLD"       }},
                {CM_ACCOUNT,          {1, sizeof(CMAccount),         "CM_ACCOUNT"         }},
            };

            return s_AttributeTable.at((s_AttributeTable.find(nHC) == s_AttributeTable.end()) ? (uint8_t)(CM_NONE_0) : nHC);
        }
};
