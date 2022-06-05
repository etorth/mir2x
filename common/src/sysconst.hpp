#pragma once
#include <vector>
#include <cstdint>
#include <type_traits>

#ifdef MIR2X_DEBUG_MODE
    constexpr bool SYS_DEBUG = true;
#else
    constexpr bool SYS_DEBUG = false;
#endif

// In code of mirx, the MAX_Y_COUNT_FOR_OBJ_H is 44, means we need to check 44 * 32 in
// height when drawing map because of the long object slice. Do some math the screen
// height is 600, then for object slice it's (44 * 32 - 600) / 32 = 25.25, means there
// are 26 cells of one object slice at most, then design data structure for object
// rendering method based on this information

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

constexpr int SYS_MAXPLAYERNUM = 8192;

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
constexpr int SYS_MAXNPCDISTANCE = 10;

constexpr char SYS_NPCINIT [] = "RSVD_NPC_INIT__2967391362393263";
constexpr char SYS_NPCDONE [] = "RSVD_NPC_DONE__6381083734343264";
constexpr char SYS_NPCQUERY[] = "RSVD_NPC_QUERY_8619263917692639";
constexpr char SYS_NPCERROR[] = "RSVD_NPC_ERROR_8619263917692639";

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
