/*
 * =====================================================================================
 *
 *       Filename: actormessage.hpp
 *        Created: 05/03/2016 13:19:07
 *  Last Modified: 03/26/2017 18:04:36
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
enum MessagePackType: int{
    MPK_UNKNOWN = 0,
    MPK_DUMMY,
    MPK_OK,
    MPK_ERROR,
    MPK_BADARG,
    MPK_NA,
    MPK_PENDING,
    MPK_HI,
    MPK_CHECKCOVEROK,
    MPK_PING,
    MPK_LOGIN,
    MPK_REFUSE,
    MPK_MOVE,
    MPK_HELLO,
    MPK_ACTIVATE,
    MPK_METRONOME,
    MPK_LEAVE,
    MPK_ADDMONSTER,
    MPK_NEWPLAYER,
    MPK_NEWCONNECTION,
    MPK_PLAYERPHATOM,
    MPK_QUERYLOCATION,
    MPK_TRYMOVE,
    MPK_TRYSPACEMOVE,
    MPK_MOVEOK,
    MPK_COMMITMOVE,
    MPK_LOCATIION,
    MPK_MASTERPERSONA,
    MPK_INITREGIONMONITOR,
    MPK_MAPID,
    MPK_READY,
    MPK_REGIONMONITORREADY,
    MPK_NEIGHBOR,
    MPK_NEWMONSTER,
    MPK_LOGINOK,
    MPK_FORWARDCM,
    MPK_CHECKCOVER,
    MPK_QUERYRMADDRESS,
    MPK_QUERYPOINTRMADDRESS,
    MPK_QUERYSCADDRESS,
    MPK_QUERYMAPADDRESS,
    MPK_ADDRESS,
    MPK_LOGINQUERYDB,
    MPK_NETPACKAGE,
    MPK_CHAROBJECTINFO,
    MPK_ADDCHAROBJECT,
    MPK_BINDSESSION,
    MPK_ACTIONSTATE,
    MPK_UPDATECOINFO,
    MPK_QUERYMONSTERGINFO,
    MPK_PULLCOINFO,
};

typedef struct
{
    uintptr_t This;

    int X;
    int Y;
}AMLeave;

typedef struct{
    uint32_t GUID;
    uint32_t UID;
    uint32_t AddTime;
    int X;
    int Y;
    int R;
    void *Data;
}AMNewMonster;

typedef struct{
    int  LocX;
    int  LocY;
}AMRegionMonitorReady;

typedef struct{
    int  X;
    int  Y;
}AMRequestMove;

typedef struct{
    int  X;
    int  Y;
    int  W;
    int  H;
    uint32_t MapID;
    int  LocX;
    int  LocY;
}AMRegion;

typedef struct{
    uint32_t MonsterIndex;
    uint32_t MapID;

    bool Strict;
    int  X;
    int  Y;
}AMMasterPersona;

typedef union {
    uint8_t Type;

    struct _Common{
        uint8_t Type;

        uint32_t MapID;
        int MapX;
        int MapY;
    }Common;

    struct _Monster{
        struct _Common _MemoryAlign;
        uint32_t MonsterID;
    }Monster;

    struct _Player{
        struct _Common _MemoryAlign;
        uint32_t GUID;
        uint32_t JobID;
        int Level;
        int Direction;
        uint32_t SessionID;
    }Player;

    struct _NPC{
        struct _Common _MemoryAlign;
        uint32_t NPCID;
    }NPC;
}AMAddCharObject;

typedef struct{
    uint32_t GUID;
    uint32_t MapID;
    uint32_t UID;
    uint32_t AddTime;

    bool Strict;
    int  X;
    int  Y;
    int  R;
}AMAddMonster;

typedef struct{
    void *Data;
}AMNewPlayer;

typedef struct{
    uint32_t GUID;
    uint32_t UID;
    uint32_t SID;
    uint32_t AddTime;
    uint32_t MapID;
    uint64_t Key;

    int X;
    int Y;
}AMLogin;

typedef struct{
    uint32_t GUID;
}AMPlayerPhantom;

typedef struct{
    uint32_t MapID;
    uint32_t UID;
    uint32_t AddTime;

    int CurrX;
    int CurrY;

    int X;
    int Y;

    int R;
}AMTrySpaceMove;

typedef struct{
    uint32_t MapID;
    uint32_t UID;
    uint32_t AddTime;

    int X;
    int Y;

    int R;
}AMTryMove;

typedef struct{
    int X;
    int Y;
    int OldX;
    int OldY;
}AMMoveOK;

typedef struct{
    int X;
    int Y;
    int OldX;
    int OldY;
}AMCommitMove;

typedef struct{
    int X;
    int Y;
    int OldX;
    int OldY;
}AMLocation;

typedef struct{
    uint32_t UID;
    uint32_t AddTime;
    int X;
    int Y;
    int R;
}AMCheckCover;

typedef struct{
    int MapX;
    int MapY;
    uint32_t MapID;
}AMQueryRMAddress;

typedef struct{
    uint32_t SessionID;

    uint32_t GUID;
    uint32_t MapID;
    int      MapX;
    int      MapY;
    int      Level;
    int      JobID;
    int      Direction;
}AMLoginQueryDB;

typedef struct{
    uint32_t SessionID;
    uint8_t Type;
    uint8_t *Data; // don't make it const, otherwise we need explicit when Free(void *)
    size_t DataLen;
}AMNetPackage;

typedef struct{
    uint32_t MapID;
    uint32_t MapX;
    uint32_t MapY;
    uint32_t R;
}AMNewCharObject;

typedef union{
    uint8_t Type;

    struct _Player{
        uint8_t Type;
        uint32_t GUID;
        uint32_t UID;
        uint32_t AddTime;
    }Player;
}AMCharObjectInfo;

typedef struct{
    uint32_t SessionID;
}AMBindSession;

typedef struct{
    uint32_t UID;
    uint32_t AddTime;

    int X;
    int Y;
    int R;
    uint32_t MapID;

    uint8_t Action;
    uint8_t Direction;

    uint32_t Speed;
}AMActionState;

typedef struct{
    uint32_t UID;
    uint32_t AddTime;
    uint32_t SessionID;
    int X;
    int Y;
    uint32_t MapID;
}AMUpdateCOInfo;

typedef struct{
    uint32_t MonsterID;
    int      LookIDN;
    uint32_t SessionID;
}AMQueryMonsterGInfo;

typedef struct
{
    uint32_t SessionID;
}AMPullCOInfo;
