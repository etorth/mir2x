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
};

struct AMBadActorPod
{
    int      Type;
    uint32_t ID;
    uint32_t Respond;
};

struct AMBadChannel
{
    uint32_t ChannID;
};

struct AMTryLeave
{
    uint32_t UID;
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
        uint32_t MasterUID;
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
    uint32_t UID;
    uint32_t SID;
    uint32_t MapID;
    uint64_t Key;

    int X;
    int Y;
};

struct AMTrySpaceMove
{
    uint32_t UID;

    int X;
    int Y;

    bool StrictMove;
};

struct AMSpaceMoveOK
{
    void *Data;
};

struct AMTryMove
{
    uint32_t UID;
    uint32_t MapID;

    int X;
    int Y;

    int EndX;
    int EndY;

    bool AllowHalfMove;
};

struct AMMoveOK
{
    uint32_t UID;
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
    uint32_t UID;
    uint32_t MapID;

    int Action;
    int Speed;
    int Direction;

    int X;
    int Y;
    
    int AimX;
    int AimY;

    uint32_t AimUID;
    uint32_t ActionParam;
};

struct AMPullCOInfo
{
    int X;
    int Y;
    int W;
    int H;

    uint32_t UID;
    uint32_t MapID;
};

struct AMMapList
{
    uint32_t MapList[256];
};

struct AMMapSwitch
{
    uint32_t MapID;
    uint32_t UID;

    int X;
    int Y;
};

struct AMTryMapSwitch
{
    uint32_t UID;
    uint32_t MapID;
    uint32_t MapUID;

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
    uint32_t UID;
};

struct AMMapSwitchOK
{
    void *Data;

    int X;
    int Y;
};

struct AMQueryLocation
{
    uint32_t UID;
    uint32_t MapID;
};

struct AMLocation
{
    uint32_t UID;
    uint32_t MapID;
    uint32_t RecordTime;
    
    int X;
    int Y;
    int Direction;
};

struct AMPathFind
{
    uint32_t UID;
    uint32_t MapID;

    int MaxStep;
    bool CheckCO;

    int X;
    int Y;
    int EndX;
    int EndY;
};

struct AMPathFindOK
{
    uint32_t UID;
    uint32_t MapID;

    struct _Point
    {
        int X;
        int Y;
    }Point[8];
};

struct AMAttack
{
    uint32_t UID;
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
    uint32_t UID;
    uint32_t MapID;

    int X;
    int Y;

    uint32_t HP;
    uint32_t HPMax;
};

struct AMDeadFadeOut
{
    uint32_t UID;
    uint32_t MapID;

    int X;
    int Y;
};

struct AMQueryCORecord
{
    uint32_t UID;
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
    uint32_t UIDList[128];
};

struct AMExp
{
    int Exp;
};

struct AMNewDropItem
{
    uint32_t UID;
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
    uint32_t UID;
};

struct AMOffline
{
    uint32_t UID;
    uint32_t MapID;

    int X;
    int Y;
};

struct AMPickUp
{
    uint32_t UID;
    uint32_t ID;
    uint32_t DBID;

    int X;
    int Y;
};

struct AMPickUpOK
{
    uint32_t UID;
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
        uint32_t UID;
        uint32_t MapID;

        int Action;
        int Speed;
        int Direction;

        int X;
        int Y;
        int AimX;
        int AimY;

        uint32_t AimUID;
        uint32_t ActionParam;
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
    uint32_t UID;
};
