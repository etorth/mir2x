/*
 * =====================================================================================
 *
 *       Filename: clientmessage.hpp
 *        Created: 01/24/2016 19:30:45
 *  Last Modified: 10/14/2017 18:41:45
 *
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

enum: uint8_t
{
    CM_NONE = 0,
    CM_PING,
    CM_LOGIN,
    CM_ACTION,
    CM_QUERYMONSTERGINFO,
    CM_QUERYCORECORD,

    CM_REQUESTSPACEMOVE,
    CM_PICKUP,
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
    uint32_t UID;
    uint32_t MapID;

    uint8_t Action;
    uint8_t ActionParam;
    uint8_t Speed;
    uint8_t Direction;

    uint16_t X;
    uint16_t Y;
    uint16_t AimX;
    uint16_t AimY;
    uint32_t AimUID;
};

struct CMQueryCORecord
{
    uint32_t UID;
    uint32_t MapID;

    uint16_t X;
    uint16_t Y;
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
    uint32_t UID;
    uint32_t MapID;
    uint32_t ItemID;
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

                {CM_NONE,             {0, 0,                         "CM_NONE"            }},
                {CM_PING,             {2, sizeof(CMPing),            "CM_PING"            }},
                {CM_LOGIN,            {3, 0,                         "CM_LOGIN"           }},
                {CM_ACTION,           {1, sizeof(CMAction),          "CM_ACTION"          }},
                {CM_QUERYCORECORD,    {1, sizeof(CMQueryCORecord),   "CM_QUERYCORECORD"   }},
                {CM_REQUESTSPACEMOVE, {1, sizeof(CMReqestSpaceMove), "CM_REQUESTSPACEMOVE"}},
                {CM_PICKUP,           {1, sizeof(CMPickUp),          "CM_PICKUP"          }},
            };

            return s_AttributeTable.at((s_AttributeTable.find(nHC) == s_AttributeTable.end()) ? (uint8_t)(CM_NONE) : nHC);
        }
};
