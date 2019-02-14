/*
 * =====================================================================================
 *
 *       Filename: actormessage.hpp
 *        Created: 05/03/2016 13:19:07
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
    MPK_BADACTORPOD,
    MPK_BADCHANNEL,
    MPK_TIMEOUT,
    MPK_UID,
    MPK_PING,
    MPK_LOGIN,
    MPK_METRONOME,
    MPK_TRYMOVE,
    MPK_TRYSPACEMOVE,
    MPK_MOVEOK,
    MPK_SPACEMOVEOK,
    MPK_TRYLEAVE,
    MPK_LOGINOK,
    MPK_ADDRESS,
    MPK_LOGINQUERYDB,
    MPK_NETPACKAGE,
    MPK_ADDCHAROBJECT,
    MPK_BINDCHANNEL,
    MPK_ACTION,
    MPK_PULLCOINFO,
    MPK_QUERYMAPLIST,
    MPK_MAPLIST,
    MPK_MAPSWITCH,
    MPK_MAPSWITCHOK,
    MPK_TRYMAPSWITCH,
    MPK_QUERYMAPUID,
    MPK_QUERYLOCATION,
    MPK_LOCATION,
    MPK_PATHFIND,
    MPK_PATHFINDOK,
    MPK_ATTACK,
    MPK_UPDATEHP,
    MPK_DEADFADEOUT,
    MPK_QUERYCORECORD,
    MPK_QUERYCOCOUNT,
    MPK_COCOUNT,
    MPK_QUERYRECTUIDLIST,
    MPK_UIDLIST,
    MPK_EXP,
    MPK_NEWDROPITEM,
    MPK_SHOWDROPITEM,
    MPK_NOTIFYDEAD,
    MPK_OFFLINE,
    MPK_PICKUP,
    MPK_PICKUPOK,
    MPK_REMOVEGROUNDITEM,
    MPK_CORECORD,
    MPK_NOTIFYNEWCO,
    MPK_CHECKMASTER,
    MPK_MAX,
};

struct AMBadActorPod
{
    int      Type;
    uint64_t From;
    uint32_t ID;
    uint32_t Respond;
    uint64_t UID;
};

struct AMBadChannel
{
    uint32_t ChannID;
};

struct AMTryLeave
{
    uint64_t UID;
    uint32_t MapID;

    int X;
    int Y;
};

union AMAddCharObject
{
    uint8_t Type;

    struct _Common
    {
        uint8_t Type;

        uint32_t MapID;
        int X;
        int Y;

        bool Random;
    }Common;

    struct _Monster
    {
        struct _Common _MemoryAlign;
        uint32_t MonsterID;
        uint64_t MasterUID;
    }Monster;

    struct _Player
    {
        struct _Common _MemoryAlign;
        uint32_t DBID;
        uint32_t JobID;
        int Level;
        int Direction;
        uint32_t ChannID;
    }Player;

    struct _NPC
    {
        struct _Common _MemoryAlign;
        uint32_t NPCID;
    }NPC;
};

struct AMLogin
{
    uint32_t DBID;
    uint64_t UID;
    uint32_t SID;
    uint32_t MapID;
    uint64_t Key;

    int X;
    int Y;
};

struct AMTrySpaceMove
{
    uint64_t UID;

    int X;
    int Y;

    bool StrictMove;
};

struct AMSpaceMoveOK
{
    void *Ptr;
};

struct AMTryMove
{
    uint64_t UID;
    uint32_t MapID;

    int X;
    int Y;

    int EndX;
    int EndY;

    bool AllowHalfMove;
};

struct AMMoveOK
{
    uint64_t UID;
    uint32_t MapID;

    int X;
    int Y;

    int EndX;
    int EndY;
};

struct AMLoginQueryDB
{
    uint32_t ChannID;

    uint32_t DBID;
    uint32_t MapID;
    int      MapX;
    int      MapY;
    int      Level;
    int      JobID;
    int      Direction;
};

struct AMNetPackage
{
    uint32_t ChannID;
    uint8_t  Type;

    uint8_t *Data;
    uint8_t  DataBuf[256];

    size_t DataLen;
};

struct AMBindChannel
{
    uint32_t ChannID;
};

struct AMAction
{
    uint64_t UID;
    uint32_t MapID;

    int Action;
    int Speed;
    int Direction;

    int X;
    int Y;
    
    int AimX;
    int AimY;

    uint64_t AimUID;
    uint64_t ActionParam;
};

struct AMPullCOInfo
{
    int X;
    int Y;
    int W;
    int H;

    uint64_t UID;
    uint32_t MapID;
};

struct AMMapList
{
    uint32_t MapList[256];
};

struct AMMapSwitch
{
    uint32_t MapID;
    uint64_t UID;

    int X;
    int Y;
};

struct AMTryMapSwitch
{
    uint64_t UID;
    uint32_t MapID;
    uint64_t MapUID;

    int X;
    int Y;

    int EndX;
    int EndY;
};

struct AMQueryMapUID
{
    uint32_t MapID;
};

struct AMUID
{
    uint64_t UID;
};

struct AMMapSwitchOK
{
    void *Ptr;

    int X;
    int Y;
};

struct AMQueryLocation
{
    uint64_t UID;
    uint32_t MapID;
};

struct AMLocation
{
    uint64_t UID;
    uint32_t MapID;
    uint32_t RecordTime;
    
    int X;
    int Y;
    int Direction;
};

struct AMPathFind
{
    uint64_t UID;
    uint32_t MapID;

    int CheckCO;
    int MaxStep;

    int X;
    int Y;
    int EndX;
    int EndY;
};

struct AMPathFindOK
{
    uint64_t UID;
    uint32_t MapID;

    struct _Point
    {
        int X;
        int Y;
    }Point[8];
};

struct AMAttack
{
    uint64_t UID;
    uint32_t MapID;

    int X;
    int Y;

    int Type;
    int Damage;
    int Element;
    int Effect[32];
};

struct AMUpdateHP
{
    uint64_t UID;
    uint32_t MapID;

    int X;
    int Y;

    uint32_t HP;
    uint32_t HPMax;
};

struct AMDeadFadeOut
{
    uint64_t UID;
    uint32_t MapID;

    int X;
    int Y;
};

struct AMQueryCORecord
{
    uint64_t UID;
};

struct AMQueryCOCount
{
    uint32_t MapID;
    struct _Check
    {
        bool NPC;
        bool Player;
        bool Monster;
    }Check;

    struct _CheckParam
    {
        // initialize and use it as needed
        // collection to put all checking paramters here
        bool     Male;
        uint32_t MonsterID;
    }CheckParam;
};

struct AMCOCount
{
    uint32_t Count;
};

struct AMQueryRectUIDList
{
    uint32_t MapID;

    int X;
    int Y;
    int W;
    int H;
};

struct AMUIDList
{
    uint64_t UIDList[128];
};

struct AMExp
{
    int Exp;
};

struct AMNewDropItem
{
    uint64_t UID;
    int X;
    int Y;

    uint32_t ID;
    int Value;
};

struct AMShowDropItem
{
    struct _CommonItem
    {
        uint32_t ID;
        uint32_t DBID;
    }IDList[16];

    int X;
    int Y;
};

struct AMNotifyDead
{
    uint64_t UID;
};

struct AMOffline
{
    uint64_t UID;
    uint32_t MapID;

    int X;
    int Y;
};

struct AMPickUp
{
    uint64_t UID;
    uint32_t ID;
    uint32_t DBID;

    int X;
    int Y;
};

struct AMPickUpOK
{
    uint64_t UID;
    uint32_t ID;
    uint32_t DBID;

    int X;
    int Y;
};

struct AMRemoveGroundItem
{
    int X;
    int Y;

    uint32_t ID;
    uint32_t DBID;
};

struct AMCORecord
{
    uint8_t COType;

    struct _Action
    {
        uint64_t UID;
        uint32_t MapID;

        int Action;
        int Speed;
        int Direction;

        int X;
        int Y;
        int AimX;
        int AimY;

        uint64_t AimUID;
        uint64_t ActionParam;
    }Action;

    // instantiation of anonymous struct is supported in C11
    // not C++11, so we define structs outside of anonymous union

    struct _AMCORecord_Monster
    {
        uint32_t MonsterID;
    };

    struct _AMCORecord_Player
    {
        uint32_t DBID;
        uint32_t JobID;
        uint32_t Level;
    };

    struct _AMCORecord_NPC
    {
        uint32_t NPCID;
    };

    union
    {
        _AMCORecord_Monster Monster;
        _AMCORecord_Player  Player;
        _AMCORecord_NPC     NPC;
    };
};

struct AMNotifyNewCO
{
    uint64_t UID;
};
