/*
 * =====================================================================================
 *
 *       Filename: clientmessage.hpp
 *        Created: 01/24/2016 19:30:45
 *  Last Modified: 04/24/2017 21:59:37
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
};

#pragma pack(push, 1)
typedef struct
{
    uint32_t Tick;
}CMPing;

typedef struct
{
    char ID[64];
    char Password[128];
}CMLogin;

typedef struct
{
    uint32_t MonsterID;
    uint32_t LookIDN;
}CMQueryMonsterGInfo;

typedef struct
{
    uint8_t Action;
    uint8_t ActionParam;
    uint8_t Speed;
    uint8_t Direction;

    uint16_t X;
    uint16_t Y;
    uint16_t EndX;
    uint16_t EndY;

    uint32_t ID;
}CMAction;
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

                {CM_NONE,               {0,  0,                              "CM_NONE"                   }},
                {CM_PING,               {2,  sizeof(CMPing),                 "CM_PING"                   }},
                {CM_LOGIN,              {3,  0,                              "CM_LOGIN"                  }},
                {CM_ACTION,             {1,  sizeof(CMAction),               "CM_ACTION"                 }},
                {CM_QUERYMONSTERGINFO,  {1,  sizeof(CMQueryMonsterGInfo),    "CM_QUERYMONSTERGINFO"      }},
            };

            return s_AttributeTable.at((s_AttributeTable.find(nHC) == s_AttributeTable.end()) ? (uint8_t)(CM_NONE) : nHC);
        }
};
