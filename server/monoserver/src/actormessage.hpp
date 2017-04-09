/*
 * =====================================================================================
 *
 *       Filename: actormessage.hpp
 *        Created: 05/03/2016 13:19:07
 *  Last Modified: 04/09/2017 01:57:35
 *
 *    Description: 
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
enum MessagePackType: int
{
    MPK_NONE = 0,
    MPK_OK,
    MPK_ERROR,
    MPK_NA,
    MPK_PENDING,
    MPK_HI,
    MPK_PING,
    MPK_LOGIN,
    MPK_METRONOME,
    MPK_LEAVE,
    MPK_TRYMOVE,
    MPK_TRYSPACEMOVE,
    MPK_LOGINOK,
    MPK_ADDRESS,
    MPK_LOGINQUERYDB,
    MPK_NETPACKAGE,
    MPK_ADDCHAROBJECT,
    MPK_BINDSESSION,
    MPK_ACTION,
    MPK_STATE,
    MPK_QUERYMONSTERGINFO,
    MPK_PULLCOINFO,
    MPK_NEWCONNECTION,
    MPK_QUERYMAPLIST,
    MPK_MAPLIST,
};

typedef struct
{
    void *This;

    int X;
    int Y;
}AMLeave;

typedef union
{
    uint8_t Type;

    struct _Common
    {
        uint8_t Type;

        uint32_t MapID;
        int MapX;
        int MapY;
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
        int Level;
        int Direction;
        uint32_t SessionID;
    }Player;

    struct _NPC
    {
        struct _Common _MemoryAlign;
        uint32_t NPCID;
    }NPC;
}AMAddCharObject;

typedef struct
{
    uint32_t GUID;
    uint32_t UID;
    uint32_t SID;
    uint32_t MapID;
    uint64_t Key;

    int X;
    int Y;
}AMLogin;

typedef struct
{
    void *This;

    uint32_t MapID;
    uint32_t UID;

    int X;
    int Y;

    int CurrX;
    int CurrY;
}AMTrySpaceMove;

typedef struct
{
    void *This;

    uint32_t MapID;
    uint32_t UID;

    int X;
    int Y;

    int CurrX;
    int CurrY;
}AMTryMove;

typedef struct
{
    uint32_t SessionID;

    uint32_t GUID;
    uint32_t MapID;
    int      MapX;
    int      MapY;
    int      Level;
    int      JobID;
    int      Direction;
}AMLoginQueryDB;

typedef struct
{
    uint32_t SessionID;
    uint8_t Type;
    uint8_t *Data;
    size_t DataLen;
}AMNetPackage;

typedef struct
{
    uint32_t SessionID;
}AMBindSession;

typedef struct
{
    uint32_t UID;
    uint32_t MapID;

    int X;
    int Y;
}AMState;

typedef struct
{
    uint32_t UID;
    uint32_t MapID;

    int Action;
    int ActionParam;

    int Speed;
    int Direction;

    int X;
    int Y;
    
    int EndX;
    int EndY;
}AMAction;

typedef struct
{
    uint32_t MonsterID;
    int      LookIDN;
    uint32_t SessionID;
}AMQueryMonsterGInfo;

typedef struct
{
    uint32_t SessionID;
}AMPullCOInfo;

typedef struct
{
    uint32_t SessionID;
}AMNewConnection;

typedef struct
{
    uint32_t MapList[256];
}AMMapList;
