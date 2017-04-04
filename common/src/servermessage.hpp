/*
 * =====================================================================================
 *
 *       Filename: servermessage.hpp
 *        Created: 01/24/2016 19:30:45
 *  Last Modified: 04/04/2017 14:41:18
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

enum: uint8_t
{
    SM_NONE,
    SM_OK,
    SM_ERROR,
    SM_PING,
    SM_LOGINOK,
    SM_LOGINFAIL,
    SM_SERVERFULL,
    SM_STATE,
    SM_ACTION,
    SM_MONSTERGINFO,
    SM_CORECORD,
};

#pragma pack(push, 1)

// TODO
// for this kind of package with str, enable compress
// 1. send byte HC =  SMLoginOK
// 2. send byte Compress flag:
//      0: uncompressed
//  not 0: compressed, then check the length
//          0X80 & byte == 0: length = (0X7F & byte)
//                      != 0: length = (length << 7) + (0X7F & byte)
// 3. read message body
//
// currently just always use uncompressed package
typedef struct{
    uint32_t GUID;
    uint32_t UID;
    uint32_t AddTime;
    uint32_t Male;
    uint32_t JobID;
    uint32_t Level;
    uint32_t X;
    uint32_t Y;
    uint32_t MapID;
    uint32_t Direction;
}SMLoginOK;

typedef struct
{
    uint32_t Tick;
}SMPing;

typedef struct
{
    uint8_t Type;
    
    struct _Common
    {
        uint8_t _Type;

        uint32_t UID;

        uint32_t CurrX;
        uint32_t CurrY;
        uint32_t MapID;
    }Common;

    struct _Attack
    {
        struct _Common _MemoryAlign;
    }Attack;

    struct _Move
    {
        struct _Common _MemoryAlign;

        uint16_t X;
        uint16_t Y;

        uint8_t Param;
    }Move;

    uint8_t Action;
    uint8_t ActionParam;
    uint8_t Direction;

    uint32_t Speed;
}SMAction;

typedef struct{
    uint32_t MonsterID;
    uint32_t LookIDN;
    uint32_t LookID;
    uint32_t R;
}SMMonsterGInfo;

typedef union
{
    uint8_t Type;

    struct _Common
    {
        uint8_t Type;

        uint32_t UID;
        uint32_t AddTime;

        uint32_t R;
        uint32_t MapX;
        uint32_t MapY;
        uint32_t MapID;

        uint32_t Action;
        uint32_t Direction;

        uint32_t Speed;
    }Common;

    struct _Monster
    {
        struct _Common _MemoryAlign;
        uint32_t MonsterID;
    }Monster;

    struct _Player
    {
        struct _Common _MemoryAlign;
        uint32_t GUID;
        uint32_t JobID;
        uint32_t Level;
    }Player;

    struct _NPC
    {
        struct _Common _MemoryAlign;
        uint32_t NPCID;
    }NPC;
}SMCORecord;
#pragma pack(pop)
