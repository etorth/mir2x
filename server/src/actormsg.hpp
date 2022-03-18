#pragma once
#include <cstdint>
#include "fixedbuf.hpp"
#include "actionnode.hpp"
#include "damagenode.hpp"
#include "actordatapackage.hpp"

enum ActorMsgPackType: int
{
    AM_NONE  = 0,
    AM_BEGIN = 1,
    AM_OK    = 1,
    AM_ERROR,
    AM_BADACTORPOD,
    AM_BADCHANNEL,
    AM_TIMEOUT,
    AM_UID,
    AM_PING,
    AM_LOGIN,
    AM_METRONOME,
    AM_TRYJUMP,
    AM_ALLOWJUMP,
    AM_REJECTJUMP,
    AM_JUMPOK,
    AM_JUMPERROR,
    AM_TRYMOVE,         // step-1. CO -> ServerMap  : co requests move
    AM_ALLOWMOVE,       // step-2. Server -> CO     : server map allows  to move
    AM_REJECTMOVE,      // step-2. Server -> CO     : server map refuses to move
    AM_MOVEOK,          // step-3. CO -> ServerMap  : co confirms to take the move permission
    AM_MOVEERROR,       // step-3. CO -> ServerMap  : co rejects  to take the move permission
    AM_TRYSPACEMOVE,
    AM_ALLOWSPACEMOVE,
    AM_REJECTSPACEMOVE,
    AM_SPACEMOVEOK,
    AM_SPACEMOVEERROR,
    AM_TRYLEAVE,        // step-1. CO -> ServerMap  : co requests to leave
    AM_ALLOWLEAVE,      // step-2. ServerMap -> CO  : server map permits to leave
    AM_REJECTLEAVE,     // step-2. ServerMap -> CO  : server map refuses to leave
    AM_LEAVEOK,         // step-3. CO -> ServerMap  : CO confirms to take the leave permission, leave has been done in CO
    AM_LEAVEERROR,      // step-3. CO -> ServerMap  : Co rejects  to take the leave permission
    AM_FINISHLEAVE,     // step-4. ServerMap -> CO  : server map has finished the leave, leave-event broadcasted, CO can broadcast for new location
    AM_LOGINOK,
    AM_LOGINQUERYDB,
    AM_SENDPACKAGE,
    AM_RECVPACKAGE,
    AM_ADDCO,
    AM_BINDCHANNEL,
    AM_ACTION,
    AM_QUERYMAPLIST,
    AM_MAPLIST,
    AM_MAPSWITCHTRIGGER,
    AM_TRYMAPSWITCH,
    AM_ALLOWMAPSWITCH,
    AM_REJECTMAPSWITCH,
    AM_MAPSWITCHOK,
    AM_MAPSWITCHERROR,
    AM_LOADMAP,
    AM_LOADMAPOK,
    AM_LOADMAPERROR,
    AM_QUERYLOCATION,
    AM_QUERYSELLITEMLIST,
    AM_LOCATION,
    AM_PATHFIND,
    AM_PATHFINDOK,
    AM_ATTACK,
    AM_UPDATEHP,
    AM_DEADFADEOUT,
    AM_QUERYCORECORD,
    AM_QUERYCOCOUNT,
    AM_QUERYPLAYERWLDESP,
    AM_COCOUNT,
    AM_ADDBUFF,
    AM_REMOVEBUFF,
    AM_EXP,
    AM_MISS,
    AM_HEAL,
    AM_QUERYHEALTH,
    AM_HEALTH,
    AM_DROPITEM,
    AM_SHOWDROPITEM,
    AM_NOTIFYDEAD,
    AM_OFFLINE,
    AM_PICKUP,
    AM_PICKUPITEMLIST,
    AM_REMOVEGROUNDITEM,
    AM_CORECORD,
    AM_NOTIFYNEWCO,
    AM_CHECKMASTER,
    AM_CHECKMASTEROK,
    AM_QUERYMASTER,
    AM_QUERYFINALMASTER,
    AM_QUERYFRIENDTYPE,
    AM_FRIENDTYPE,
    AM_CASTFIREWALL,
    AM_STRIKEFIXEDLOCDAMAGE,
    AM_QUERYNAMECOLOR,
    AM_NAMECOLOR,
    AM_MASTERKILL,
    AM_MASTERHITTED,
    AM_NPCQUERY,
    AM_NPCEVENT,
    AM_NPCERROR,
    AM_BUY,
    AM_BUYCOST,
    AM_BUYERROR,
    AM_END,
};

struct AMBadActorPod
{
    int      Type;
    uint64_t from;
    uint32_t ID;
    uint32_t Respond;
    uint64_t UID;
};

struct AMBadChannel
{
    uint32_t channID;
};

struct AMTryLeave
{
    int X;
    int Y;
};

struct AMAllowLeave
{
    int X;
    int Y;
};

struct AMLeaveOK
{
    uint64_t uid;
    uint32_t mapID;
    ActionNode action;
};

struct AMAddCharObject
{
    int type;

    int x;
    int y;
    uint32_t mapID;
    bool strictLoc;

    struct _monsterType
    {
        uint32_t monsterID;
        uint64_t masterUID;
    };

    struct _playerType
    {
        uint32_t DBID;
        uint32_t jobID;

        int level;
        int direction;
        uint32_t channID;
    };

    struct _NPCType
    {
        int direction;
        uint16_t NPCID;
    };

    union
    {
        _NPCType NPC;
        _playerType player;
        _monsterType monster;
    };

    FixedBuf<256> buf;
};

struct AMLogin
{
    uint32_t DBID;
    uint64_t UID;
    uint32_t SID;
    uint32_t mapID;
    uint64_t Key;

    int X;
    int Y;
};

struct AMTrySpaceMove
{
    int X;
    int Y;
    int EndX;
    int EndY;

    bool StrictMove;
};

struct AMAllowSpaceMove
{
    int X;
    int Y;
    int EndX;
    int EndY;
};

struct AMSpaceMoveOK
{
    uint64_t uid;
    uint32_t mapID;
    ActionNode action;
};

struct AMTryMove
{
    uint64_t UID;
    uint32_t mapID;

    int X;
    int Y;

    int EndX;
    int EndY;

    bool AllowHalfMove;
    bool RemoveMonster;
};

struct AMAllowMove
{
    uint64_t UID;
    uint32_t mapID;

    int X;
    int Y;

    int EndX;
    int EndY;
};

struct AMMoveOK
{
    uint64_t uid;
    uint32_t mapID;
    ActionNode action;
};

struct AMTryJump
{
    int X;
    int Y;

    int EndX;
    int EndY;
};

struct AMAllowJump
{
    int X;
    int Y;

    int EndX;
    int EndY;
};

struct AMJumpOK
{
    uint64_t uid;
    uint32_t mapID;
    ActionNode action;
};

struct AMLoginQueryDB
{
    uint32_t channID;

    uint32_t DBID;
    uint32_t mapID;
    int      MapX;
    int      MapY;
    int      Level;
    int      JobID;
    int      Direction;
};

struct AMSendPackage
{
    ActorDataPackage package;
};

struct AMRecvPackage
{
    uint32_t channID;
    ActorDataPackage package;
};

struct AMBindChannel
{
    uint32_t channID;
};

struct AMAction
{
    uint64_t UID;
    uint32_t mapID;
    ActionNode action;
};

struct AMMapList
{
    uint32_t MapList[256];
};

struct AMMapSwitchTrigger
{
    uint32_t mapID;
    uint64_t UID;

    int X;
    int Y;
};

struct AMTryMapSwitch
{
    int X;
    int Y;
    bool strictMove;
};

struct AMAllowMapSwitch
{
    void *Ptr;

    int X;
    int Y;
};

struct AMMapSwitchOK
{
    uint64_t uid;
    uint32_t mapID;
    ActionNode action;
};

struct AMLoadMap
{
    uint32_t mapID;
};

struct AMUID
{
    uint64_t UID;
};

struct AMQueryLocation
{
    uint64_t UID;
    uint32_t mapID;
};

struct AMQuerySellItemList
{
    uint32_t itemID;
};

struct AMLocation
{
    uint64_t UID;
    uint32_t mapID;

    int X;
    int Y;
    int Direction;
};

struct AMPathFind
{
    uint64_t UID;
    uint32_t mapID;

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
    uint32_t mapID;

    struct _Point
    {
        int X;
        int Y;
    }Point[8];
};

struct AMAttack
{
    uint64_t UID;
    uint32_t mapID;

    int X;
    int Y;

    DamageNode damage;
};

struct AMUpdateHP
{
    uint64_t UID;
    uint32_t mapID;

    int X;
    int Y;

    uint32_t HP;
    uint32_t HPMax;
};

struct AMDeadFadeOut
{
    uint64_t UID;
    uint32_t mapID;

    int X;
    int Y;
};

struct AMQueryCORecord
{
    uint64_t UID;
};

struct AMQueryCOCount
{
    uint32_t mapID;
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

struct AMAddBuff
{
    int id;
    uint32_t fromBuff;
    uint64_t from;
};

struct AMRemoveBuff
{
    uint64_t fromUID;
    uint64_t fromBuffSeq;
};

struct AMExp
{
    int exp;
};

struct AMMiss
{
    uint64_t UID;
};

struct AMHeal
{
    uint32_t mapID;
    int x;
    int y;

    int addHP;
    int addMP;
};

struct AMNotifyDead
{
    uint64_t UID;
};

struct AMOffline
{
    uint64_t UID;
    uint32_t mapID;

    int X;
    int Y;
};

struct AMPickUp
{
    int x;
    int y;
    uint32_t availableWeight;
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
    uint64_t UID;
    uint32_t mapID;
    ActionNode action;

    // instantiation of anonymous struct is supported in C11
    // not C++11, so we define structs outside of anonymous union

    struct _AMCORecord_Monster
    {
        uint32_t MonsterID;
    };

    struct _AMCORecord_Player
    {
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

struct AMCheckMasterOK
{
    int dc[2];
    int mc[2];
    int sc[2];

    int  ac[2];
    int mac[2];
};

struct AMCastFireWall
{
    int x;
    int y;

    int minDC;
    int maxDC;
    int mcHit;

    int duration;
    int dps;
};

struct AMStrikeFixedLocDamage
{
    int x;
    int y;

    DamageNode damage;
};

struct AMQueryFriendType
{
    uint64_t UID;
};

struct AMFriendType
{
    int Type;
};

struct AMNameColor
{
    int Color;
};

struct AMNPCQuery
{
    char query[128];
};

struct AMNPCError
{
    int errorID;
};

struct AMBuy
{
    uint32_t itemID;
    uint32_t seqID;
    size_t   count;
};

struct AMBuyError
{
    int error;
};
