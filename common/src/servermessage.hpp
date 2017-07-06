/*
 * =====================================================================================
 *
 *       Filename: servermessage.hpp
 *        Created: 01/24/2016 19:30:45
 *  Last Modified: 07/06/2017 11:03:23
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
#include <cstdint>
#include <unordered_map>
#include "messagebase.hpp"

enum: uint8_t
{
    SM_NONE = 0,
    SM_PING,
    SM_LOGINOK,
    SM_LOGINFAIL,
    SM_ACTION,
    SM_CORECORD,
    SM_UPDATEHP,
    SM_DEADFADEOUT,
};

#pragma pack(push, 1)
struct SMPing
{
    uint32_t Tick;
};

struct SMLoginOK
{
    uint32_t UID;
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
};

union SMCORecord
{
    uint8_t Type;

    struct _Common
    {
        uint8_t _MemoryAlign;

        uint32_t UID;
        uint32_t MapID;

        uint8_t Action;
        uint8_t ActionParam;
        uint8_t Speed;
        uint8_t Direction;

        uint16_t X;
        uint16_t Y;
        uint16_t EndX;
        uint16_t EndY;
    }Common;

    struct _Monster
    {
        struct _Common _MemoryAlign;
        uint32_t MonsterID;
    }Monster;

    struct _Player
    {
        struct _Common _MemoryAlign;
        uint32_t DBID;
        uint32_t JobID;
        uint32_t Level;
    }Player;

    struct _NPC
    {
        struct _Common _MemoryAlign;
        uint32_t NPCID;
    }NPC;
};

struct SMUpdateHP
{
    uint32_t UID;
    uint32_t MapID;

    uint32_t HP;
    uint32_t HPMax;
};

struct SMDeadFadeOut
{
    uint32_t UID;
    uint32_t MapID;

    uint32_t X;
    uint32_t Y;
};
#pragma pack(pop)

class SMSGParam: public MessageBase
{
    public:
        SMSGParam(uint8_t nHC)
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

                {SM_NONE,        {0, 0,                     "SM_NONE"       }},
                {SM_PING,        {2, sizeof(SMPing),        "SM_PING"       }},
                {SM_LOGINOK,     {1, sizeof(SMLoginOK),     "SM_LOGINOK"    }},
                {SM_LOGINFAIL,   {2, sizeof(SMLoginFail),   "SM_LOGINFAIL"  }},
                {SM_ACTION,      {1, sizeof(SMAction),      "SM_ACTION"     }},
                {SM_CORECORD,    {1, sizeof(SMCORecord),    "SM_CORECORD"   }},
                {SM_UPDATEHP,    {1, sizeof(SMUpdateHP),    "SM_UPDATEHP"   }},
                {SM_DEADFADEOUT, {1, sizeof(SMDeadFadeOut), "SM_DEADFADEOUT"}},
            };

            return s_AttributeTable.at((s_AttributeTable.find(nHC) == s_AttributeTable.end()) ? (uint8_t)(SM_NONE) : nHC);
        }
};
