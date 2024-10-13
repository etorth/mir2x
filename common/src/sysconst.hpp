#pragma once
#include <vector>
#include <cstdint>
#include <cstddef>
#include <type_traits>

#ifdef MIR2X_DEBUG_MODE
    constexpr bool SYS_DEBUG = true;
#else
    constexpr bool SYS_DEBUG = false;
#endif

enum SysTriggerType: int
{
    SYS_ON_BEGIN = 0,

    SYS_ON_GAINEXP = SYS_ON_BEGIN,
    SYS_ON_GAINITEM,
    SYS_ON_GAINGOLD,

    SYS_ON_ONLINE,
    SYS_ON_OFFLINE,

    SYS_ON_LEVELUP,
    SYS_ON_KILL,

    SYS_ON_TEAMUP,
    SYS_ON_TEAMDOWN,

    SYS_ON_APPEAR,

    SYS_ON_END,
};

// In code of mirx, the MAX_Y_COUNT_FOR_OBJ_H is 44, means we need to check 44 * 32 in
// height when drawing map because of the long object slice. Do some math the screen
// height is 600, then for object slice it's (44 * 32 - 600) / 32 = 25.25, means there
// are 26 cells of one object slice at most, then design data structure for object
// rendering method based on this information

constexpr int SYS_WINDOW_MIN_W = 800;
constexpr int SYS_WINDOW_MIN_H = 600;

constexpr double SYS_PI = 3.14159265359;
constexpr int SYS_DEFFPS = 10;

constexpr int SYS_TARGETRGN_GAPX = 10;
constexpr int SYS_TARGETRGN_GAPY = 8;

constexpr int SYS_MAPGRIDXP = 48;
constexpr int SYS_MAPGRIDYP = 32;

constexpr int SYS_OBJMAXW = 3;
constexpr int SYS_OBJMAXH = 25;

constexpr int SYS_MAXR         = 40;
constexpr int SYS_MAPVISIBLEW  = 60;
constexpr int SYS_MAPVISIBLEH  = 40;
constexpr int SYS_MAPVISIBLECD = 100;

constexpr int SYS_MAXPLAYERNUM = 16384;

// view range is mutual direction and universal
// a monster tracks player in range MonsterRecord::view, but it keeps records for all player in SYS_VIEWR as neighbors
// otherwise monsters will not send ACTION to players in range MonsterRecord::view < d <= SYS_VIEWR
constexpr int SYS_VIEWR = 15;

// monster targeting expire time
// CO targeted longer than the time is expired, for monster only
constexpr int SYS_TARGETSEC = 60;

constexpr int SYS_MAXDROPITEM     = 10;
constexpr int SYS_MAXDROPITEMGRID = 81;

constexpr int SYS_MINSPEED =  20;
constexpr int SYS_DEFSPEED = 100;
constexpr int SYS_MAXSPEED = 500;

constexpr int SYS_INVGRIDGW = 10;
constexpr int SYS_INVGRIDGH = 10;
constexpr int SYS_INVGRIDPW = 38;
constexpr int SYS_INVGRIDPH = 38;
constexpr int SYS_INVGRIDMAXHOLD = 99;

constexpr int SYS_MAXTARGET = 8;
constexpr int SYS_MAXACTOR  = 65521;

constexpr size_t SYS_IDSIZE = 64;
constexpr size_t SYS_PWDSIZE = 64;
constexpr size_t SYS_NAMESIZE = 64;

constexpr size_t SYS_SEFFSIZE = 16;
constexpr uint32_t SYS_MONSEFFBASE(int lookID)
{
    return 0X02000000 + lookID * SYS_SEFFSIZE;
}

constexpr uint32_t SYS_U32NIL = 0XFFFFFFFF;
constexpr uint64_t SYS_U64NIL = 0XFFFFFFFFFFFFFFFFULL;

constexpr uint32_t SYS_MAXDBID = 0XFFFFFF00;
constexpr uint32_t SYS_CHATDBID_SYSTEM = SYS_MAXDBID + 1;
constexpr uint32_t SYS_CHATDBID_GROUP  = SYS_MAXDBID + 2;
constexpr uint32_t SYS_CHATDBID_AI     = SYS_MAXDBID + 3;

constexpr int SYS_MAXNPCDISTANCE = 10;
constexpr char SYS_GOLDNAME[] = "金币（小）"; // always use 金币（小）to represent the gold item
constexpr char SYS_QUEST_TBL_PREFIX[] = "tbl_questdb_";

// commonly used quest variable name in fld_vars
// use key in luaTable instead of key in database table to avoid change table structure

struct SYS_QUESTFIELD
{
    constexpr static char  VARS[] = "_RSVD_NAME_QUESTFIELD::VARS__837517653";
    constexpr static char FLAGS[] = "_RSVD_NAME_QUESTFIELD::FLAGS_837517653";
    constexpr static char STATE[] = "_RSVD_NAME_QUESTFIELD::STATE_837517653";

    struct TEAM
    {
        constexpr static char     LEADER[] = "_RSVD_NAME_QUESTFIELD::TEAM::LEADER_____622091631";
        constexpr static char MEMBERLIST[] = "_RSVD_NAME_QUESTFIELD::TEAM::MEMBERLIST_694437683";
        constexpr static char   ROLELIST[] = "_RSVD_NAME_QUESTFIELD::TEAM::ROLELIST___556867549";
    };
};

constexpr char SYS_NPCERROR[] = "_RSVD_NAME_NPC_ERROR_45421406723";

constexpr char SYS_ENTER[] = "_RSVD_NAME_ENTER_90360178872";
constexpr char SYS_DONE [] = "_RSVD_NAME_DONE__06562813788";
constexpr char SYS_EXIT [] = "_RSVD_NAME_EXIT__14208236065";
constexpr char SYS_ABORT[] = "_RSVD_NAME_ABORT_72061294738";

constexpr char SYS_POSINF[] = "_RSVD_NAME_POS_INF_63583688";
constexpr char SYS_NEGINF[] = "_RSVD_NAME_NEG_INF_55461872";

constexpr char SYS_EXECDONE  [] = "_RSVD_NAME_EXEC_DONE___1952748411";
constexpr char SYS_EXECCLOSE [] = "_RSVD_NAME_EXEC_CLOSE__8553768451";
constexpr char SYS_EXECBADUID[] = "_RSVD_NAME_EXEC_BADUID_7259708252";

constexpr char SYS_HIDE        [] = "_RSVD_NAME_HIDE_3194448323";
constexpr char SYS_LABEL       [] = "_RSVD_NAME_LABEL_78921733019";
constexpr char SYS_CHECKACTIVE [] = "_RSVD_NAME_CHECK_ACTIVE_4054544333";
constexpr char SYS_ALLOWREDNAME[] = "_RSVD_NAME_ALLOW_REDNAME_761479946";

constexpr char SYS_EPDEF[] = "_RSVD_NAME_EVENT_PATH_DEF_2965316381";
constexpr char SYS_EPUID[] = "_RSVD_NAME_EVENT_PATH_UID_3623042653";
constexpr char SYS_EPQST[] = "_RSVD_NAME_EVENT_PATH_QST_6329204623";

constexpr char SYS_COOP[] = "_RSVD_NAME_COOP_293173013";

constexpr char SYS_QSTFSM[] = "_RSVD_NAME_QST_FSM_4194347313";

constexpr char SYS_FLAGVAL[] = "_RSVD_NAME_FLAG_VAL_8192362390";

constexpr inline size_t SYS_SUMEXP(uint32_t level)
{
    const size_t a =  100;
    const size_t b =  100;
    const size_t c =  100;
    const size_t d = 1000;

    return a * level * level * level + b * level * level + c * level + d;
}

constexpr size_t SYS_EXP(uint32_t level)
{
    if(level == 0){
        return SYS_SUMEXP(level);
    }
    return SYS_SUMEXP(level) - SYS_SUMEXP(level - 1);
}

constexpr inline uint32_t SYS_LEVEL(size_t exp)
{
    for(uint32_t level = 0;; ++level){
        if(SYS_SUMEXP(level) > exp){
            return level;
        }
    }
    return 0;
}
