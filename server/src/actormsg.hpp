#pragma once
#include <cstdint>
#include "staticbuffer.hpp"
#include "actionnode.hpp"
#include "damagenode.hpp"
#include "actordatapackage.hpp"

enum ActorMsgPackType: int
{
    AM_NONE = 0,

    AM_SYS_BEGIN     = 1,
    AM_SYS_PEERINDEX = 1,
    AM_SYS_LAUNCH,
    AM_SYS_LAUNCHED,
    AM_SYS_SLAVEPEERPORT,
    AM_SYS_SLAVEPEERLIST,
    AM_SYS_END,

    AM_BEGIN = AM_SYS_END,
    AM_OK    = AM_BEGIN,
    AM_ERROR,
    AM_TRUE,
    AM_FALSE,
    AM_BADACTORPOD,
    AM_BADCHANNEL,
    AM_SDBUFFER,
    AM_REMOTECALL,
    AM_UID,
    AM_UIDLIST,
    AM_TIMEOUT,
    AM_CANCEL,
    AM_PING,
    AM_ACTIVATE,
    AM_WAITACTIVATED,
    AM_WAITACTIVATEDOK,
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
    AM_SENDPACKAGE,
    AM_RECVPACKAGE,
    AM_ADDCO,
    AM_ADDMAP,
    AM_ADDMAPOK,
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
    AM_PEERLOADMAP,
    AM_PEERLOADMAPOK,
    AM_LOADMAP,
    AM_LOADMAPOK,
    AM_QUERYLOCATION,
    AM_QUERYSELLITEMLIST,
    AM_LOCATION,
    AM_PATHFIND,
    AM_PATHFINDOK,
    AM_ATTACK,
    AM_UPDATEHP,
    AM_DEADFADEOUT,
    AM_QUERYUIDBUFF,
    AM_QUERYCORECORD,
    AM_QUERYCOCOUNT,
    AM_QUERYPLAYERNAME,
    AM_QUERYPLAYERWLDESP,
    AM_COCOUNT,
    AM_PEERCONFIG,
    AM_ADDBUFF,
    AM_REMOVEBUFF,
    AM_QUERYDEAD,
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
    AM_NPCEVENT,
    AM_NPCERROR,
    AM_BUY,
    AM_BUYCOST,
    AM_BUYERROR,
    AM_MODIFYQUESTTRIGGERTYPE,
    AM_QUERYQUESTUID,
    AM_QUERYQUESTUIDLIST,
    AM_QUERYQUESTTRIGGERLIST,
    AM_QUERYTEAMMEMBERLIST,
    AM_QUERYTEAMPLAYER,
    AM_TEAMPLAYER,
    AM_TEAMUPDATE,
    AM_TEAMMEMBERLIST,
    AM_SENDNOTIFY,
    AM_REGISTERQUEST,
    AM_REQUESTJOINTEAM,
    AM_REQUESTLEAVETEAM,
    AM_END,
};

inline const char *mpkName(int type)
{
#define _add_mpk_type_case(t) case t: return #t;
    switch(type){
        _add_mpk_type_case(AM_NONE)
        _add_mpk_type_case(AM_SYS_PEERINDEX)
        _add_mpk_type_case(AM_SYS_LAUNCH)
        _add_mpk_type_case(AM_SYS_LAUNCHED)
        _add_mpk_type_case(AM_SYS_SLAVEPEERPORT)
        _add_mpk_type_case(AM_SYS_SLAVEPEERLIST)
        _add_mpk_type_case(AM_OK)
        _add_mpk_type_case(AM_ERROR)
        _add_mpk_type_case(AM_TRUE)
        _add_mpk_type_case(AM_FALSE)
        _add_mpk_type_case(AM_BADACTORPOD)
        _add_mpk_type_case(AM_BADCHANNEL)
        _add_mpk_type_case(AM_SDBUFFER)
        _add_mpk_type_case(AM_REMOTECALL)
        _add_mpk_type_case(AM_UID)
        _add_mpk_type_case(AM_UIDLIST)
        _add_mpk_type_case(AM_TIMEOUT)
        _add_mpk_type_case(AM_CANCEL)
        _add_mpk_type_case(AM_PING)
        _add_mpk_type_case(AM_ACTIVATE)
        _add_mpk_type_case(AM_WAITACTIVATED)
        _add_mpk_type_case(AM_WAITACTIVATEDOK)
        _add_mpk_type_case(AM_TRYJUMP)
        _add_mpk_type_case(AM_ALLOWJUMP)
        _add_mpk_type_case(AM_REJECTJUMP)
        _add_mpk_type_case(AM_JUMPOK)
        _add_mpk_type_case(AM_JUMPERROR)
        _add_mpk_type_case(AM_TRYMOVE)
        _add_mpk_type_case(AM_ALLOWMOVE)
        _add_mpk_type_case(AM_REJECTMOVE)
        _add_mpk_type_case(AM_MOVEOK)
        _add_mpk_type_case(AM_MOVEERROR)
        _add_mpk_type_case(AM_TRYSPACEMOVE)
        _add_mpk_type_case(AM_ALLOWSPACEMOVE)
        _add_mpk_type_case(AM_REJECTSPACEMOVE)
        _add_mpk_type_case(AM_SPACEMOVEOK)
        _add_mpk_type_case(AM_SPACEMOVEERROR)
        _add_mpk_type_case(AM_TRYLEAVE)
        _add_mpk_type_case(AM_ALLOWLEAVE)
        _add_mpk_type_case(AM_REJECTLEAVE)
        _add_mpk_type_case(AM_LEAVEOK)
        _add_mpk_type_case(AM_LEAVEERROR)
        _add_mpk_type_case(AM_FINISHLEAVE)
        _add_mpk_type_case(AM_LOGINOK)
        _add_mpk_type_case(AM_SENDPACKAGE)
        _add_mpk_type_case(AM_RECVPACKAGE)
        _add_mpk_type_case(AM_ADDCO)
        _add_mpk_type_case(AM_ADDMAP)
        _add_mpk_type_case(AM_ADDMAPOK)
        _add_mpk_type_case(AM_BINDCHANNEL)
        _add_mpk_type_case(AM_ACTION)
        _add_mpk_type_case(AM_QUERYMAPLIST)
        _add_mpk_type_case(AM_MAPLIST)
        _add_mpk_type_case(AM_MAPSWITCHTRIGGER)
        _add_mpk_type_case(AM_TRYMAPSWITCH)
        _add_mpk_type_case(AM_ALLOWMAPSWITCH)
        _add_mpk_type_case(AM_REJECTMAPSWITCH)
        _add_mpk_type_case(AM_MAPSWITCHOK)
        _add_mpk_type_case(AM_MAPSWITCHERROR)
        _add_mpk_type_case(AM_PEERLOADMAP)
        _add_mpk_type_case(AM_PEERLOADMAPOK)
        _add_mpk_type_case(AM_LOADMAP)
        _add_mpk_type_case(AM_LOADMAPOK)
        _add_mpk_type_case(AM_QUERYLOCATION)
        _add_mpk_type_case(AM_QUERYSELLITEMLIST)
        _add_mpk_type_case(AM_LOCATION)
        _add_mpk_type_case(AM_PATHFIND)
        _add_mpk_type_case(AM_PATHFINDOK)
        _add_mpk_type_case(AM_ATTACK)
        _add_mpk_type_case(AM_UPDATEHP)
        _add_mpk_type_case(AM_DEADFADEOUT)
        _add_mpk_type_case(AM_QUERYUIDBUFF)
        _add_mpk_type_case(AM_QUERYCORECORD)
        _add_mpk_type_case(AM_QUERYCOCOUNT)
        _add_mpk_type_case(AM_QUERYPLAYERNAME)
        _add_mpk_type_case(AM_QUERYPLAYERWLDESP)
        _add_mpk_type_case(AM_COCOUNT)
        _add_mpk_type_case(AM_PEERCONFIG)
        _add_mpk_type_case(AM_ADDBUFF)
        _add_mpk_type_case(AM_REMOVEBUFF)
        _add_mpk_type_case(AM_QUERYDEAD)
        _add_mpk_type_case(AM_EXP)
        _add_mpk_type_case(AM_MISS)
        _add_mpk_type_case(AM_HEAL)
        _add_mpk_type_case(AM_QUERYHEALTH)
        _add_mpk_type_case(AM_HEALTH)
        _add_mpk_type_case(AM_DROPITEM)
        _add_mpk_type_case(AM_SHOWDROPITEM)
        _add_mpk_type_case(AM_NOTIFYDEAD)
        _add_mpk_type_case(AM_OFFLINE)
        _add_mpk_type_case(AM_PICKUP)
        _add_mpk_type_case(AM_PICKUPITEMLIST)
        _add_mpk_type_case(AM_REMOVEGROUNDITEM)
        _add_mpk_type_case(AM_CORECORD)
        _add_mpk_type_case(AM_NOTIFYNEWCO)
        _add_mpk_type_case(AM_CHECKMASTER)
        _add_mpk_type_case(AM_CHECKMASTEROK)
        _add_mpk_type_case(AM_QUERYMASTER)
        _add_mpk_type_case(AM_QUERYFINALMASTER)
        _add_mpk_type_case(AM_QUERYFRIENDTYPE)
        _add_mpk_type_case(AM_FRIENDTYPE)
        _add_mpk_type_case(AM_CASTFIREWALL)
        _add_mpk_type_case(AM_STRIKEFIXEDLOCDAMAGE)
        _add_mpk_type_case(AM_QUERYNAMECOLOR)
        _add_mpk_type_case(AM_NAMECOLOR)
        _add_mpk_type_case(AM_MASTERKILL)
        _add_mpk_type_case(AM_MASTERHITTED)
        _add_mpk_type_case(AM_NPCEVENT)
        _add_mpk_type_case(AM_NPCERROR)
        _add_mpk_type_case(AM_BUY)
        _add_mpk_type_case(AM_BUYCOST)
        _add_mpk_type_case(AM_BUYERROR)
        _add_mpk_type_case(AM_MODIFYQUESTTRIGGERTYPE)
        _add_mpk_type_case(AM_QUERYQUESTUID)
        _add_mpk_type_case(AM_QUERYQUESTUIDLIST)
        _add_mpk_type_case(AM_QUERYQUESTTRIGGERLIST)
        _add_mpk_type_case(AM_QUERYTEAMMEMBERLIST)
        _add_mpk_type_case(AM_QUERYTEAMPLAYER)
        _add_mpk_type_case(AM_TEAMPLAYER)
        _add_mpk_type_case(AM_TEAMUPDATE)
        _add_mpk_type_case(AM_TEAMMEMBERLIST)
        _add_mpk_type_case(AM_SENDNOTIFY)
        _add_mpk_type_case(AM_REGISTERQUEST)
        _add_mpk_type_case(AM_REQUESTJOINTEAM)
        _add_mpk_type_case(AM_REQUESTLEAVETEAM)
        default: return "AM_UNKNOWN";
    }
#undef _add_mpk_type_case
}

struct AMBadActorPod
{
    int      Type;
    uint64_t from;
    uint64_t ID;
    uint64_t Respond;
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
    uint64_t mapUID;
    ActionNode action;
};

struct AMAddCharObject
{
    int type;

    uint64_t mapUID;
    int x;
    int y;

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

    StaticBuffer<256> buf;
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
    uint64_t mapUID;
    ActionNode action;
};

struct AMTryMove
{
    uint64_t UID;
    uint64_t mapUID;

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
    uint64_t mapUID;

    int X;
    int Y;

    int EndX;
    int EndY;
};

struct AMMoveOK
{
    uint64_t uid;
    uint64_t mapUID;
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
    uint64_t mapUID;
    ActionNode action;
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
    uint64_t mapUID;
    ActionNode action;
};

struct AMMapList
{
    uint32_t MapList[256];
};

struct AMMapSwitchTrigger
{
    uint64_t mapUID;
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
    int X;
    int Y;
};

struct AMMapSwitchOK
{
    ActionNode action;
};

struct AMPeerLoadMap
{
    uint64_t mapUID;
    uint8_t  waitActivated;
};

struct AMPeerLoadMapOK
{
    uint8_t newLoad;
};

struct AMLoadMap
{
    uint64_t mapUID;
    uint8_t  waitActivated;
};

struct AMLoadMapOK
{
    uint8_t newLoad;
};

struct AMUID
{
    uint64_t uid;
};

struct AMQueryLocation
{
    uint64_t UID;
    uint64_t mapUID;
};

struct AMQuerySellItemList
{
    uint32_t itemID;
};

struct AMLocation
{
    uint64_t UID;
    uint64_t mapUID;

    int X;
    int Y;
    int Direction;
};

struct AMPathFind
{
    uint64_t UID;
    uint64_t mapUID;

    int CheckCO;
    int MaxStep;

    int X;
    int Y;
    int direction;

    int EndX;
    int EndY;
};

struct AMPathFindOK
{
    uint64_t UID;
    uint64_t mapUID;

    struct _Point
    {
        int X;
        int Y;
    }Point[8];
};

struct AMAttack
{
    uint64_t UID;
    uint64_t mapUID;

    int X;
    int Y;

    DamageNode damage;
};

struct AMUpdateHP
{
    uint64_t UID;
    uint64_t mapUID;

    int X;
    int Y;

    uint32_t HP;
    uint32_t HPMax;
};

struct AMDeadFadeOut
{
    uint64_t UID;
    uint64_t mapUID;

    int X;
    int Y;
};

struct AMQueryCORecord
{
    uint64_t UID;
};

struct AMQueryCOCount
{
    uint64_t mapUID;
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
    size_t Count;
};

struct AMAddBuff
{
    int id;
    uint64_t fromUID;
    uint64_t fromBuffSeq;
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
    uint64_t mapUID;
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
    uint64_t mapUID;

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
    uint64_t mapUID;
    ActionNode action;

    // instantiation of anonymous struct is supported in C11
    // not C++11, so we define structs outside of anonymous union

    struct _AMCORecord_Monster
    {
        uint32_t MonsterID;
    };

    struct _AMCORecord_Player
    {
        uint8_t gender : 1;
        uint8_t job    : 3;
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

struct AMModifyQuestTriggerType
{
    int  type;
    bool enable;
};

struct AMQueryQuestTriggerList
{
    int type;
};
