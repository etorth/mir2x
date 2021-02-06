/*
 * =====================================================================================
 *
 *       Filename: protocoldef.hpp
 *        Created: 06/03/2016 11:40:51
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
#include "motion.hpp"

// define of directioin
//
//               0
//            7     1
//          6    +--> 2
//            5  |  3
//               V
//               4
//
enum DirectionType: int
{
    DIR_NONE  = 0,
    DIR_BEGIN = 1,
    DIR_UP    = 1,
    DIR_UPRIGHT,
    DIR_RIGHT,
    DIR_DOWNRIGHT,
    DIR_DOWN,
    DIR_DOWNLEFT,
    DIR_LEFT,
    DIR_UPLEFT,
    DIR_END,
};

inline bool directionValid(int direction)
{
    return direction >= DIR_BEGIN && direction < DIR_END;
}

inline const char *directionName(int direction)
{
    switch(direction){
        case DIR_UP       : return "DIR_UP";
        case DIR_UPRIGHT  : return "DIR_UPRIGHT";
        case DIR_RIGHT    : return "DIR_RIGHT";
        case DIR_DOWNRIGHT: return "DIR_DOWNRIGHT";
        case DIR_DOWN     : return "DIR_DOWN";
        case DIR_DOWNLEFT : return "DIR_DOWNLEFT";
        case DIR_LEFT     : return "DIR_LEFT";
        case DIR_UPLEFT   : return "DIR_UPLEFT";
        default           : return "UNKNOWN";
    }
}

enum ActExtType: int
{
    ACTEXT_NONE = 0,

    ACTEXT_FLYIN,
    ACTEXT_FLYOUT,

    ACTEXT_FADEIN,
    ACTEXT_FADEOUT,
};

enum LookIDType: int
{
    LID_BEGIN = 0,
    LID_END   = LID_BEGIN + 2048,
};

enum DCType: int
{
    DC_NONE = 0,

    DC_PHY             = (1 << 8),
    DC_PHY_PLAIN       = (1 << 8) | 0,
    DC_PHY_WIDESWORD   = (1 << 8) | 1,
    DC_PHY_FIRESWORD   = (1 << 8) | 2,

    DC_MAG             = (2 << 8),
    DC_MAG_FIRE        = (2 << 8) | 0,
    DC_MAG_EXPLODE     = (2 << 8) | 1,

    DC_PHY_MON         = (3 << 8),
    DC_PHY_MON_PLAIN   = (3 << 8) | 0,
    DC_PHY_MON_DUALAXE = (3 << 8) | 1,

    DC_MAG_MON         = (4 << 8),
    DC_MAG_MON_FIRE    = (4 << 8) | 0,
};

enum ECType: int
{
    EC_NONE = 0,
    EC_PLAIN,
    EC_FIRE,
    EC_ICE,
    EC_LIGHT,
    EC_WIND,
    EC_HOLY,
    EC_DARK,
    EC_PHANTOM,
    EC_MAX,
};

enum EffectType: int
{
    EFF_NONE = 0,

    EFF_HPSTEAL = (1 << 8),
    EFF_MPSTEAL = (2 << 8),
    EFF_PUNCH   = (3 << 8),
    EFF_STONE   = (4 << 8),
};

enum GenType: int
{
    GT_NONE = 0,
    GT_MALE,
    GT_FEMALE,
};

enum MonsterAttackType: int
{
    MA_NONE = 0,
    MA_NORMAL,
    MA_DOGZ,
    MA_ATTACKALL,
};

enum WeaponIDType: int
{
    WEAPON_NONE  = 0,
    WEAPON_BEGIN = 1,
    WEAPON_END   = 256,
};

enum HelmetIDType: int
{
    HELMET_NONE  = 0,
    HELMET_BEGIN = 1,
    HELMET_END   = 256,
};

enum DressIDType : int
{
    DRESS_NONE  = 0,
    DRESS_BEGIN = 1,
    DRESS_END   = 256,
};

enum HairIDType: int
{
    HAIR_NONE  = 0,
    HAIR_BEGIN = 1,
    HAIR_END   = 256,
};

enum NPCErrorType: int
{
    NPCE_NONE = 0,
    NPCE_TOOFAR,
    NPCE_BADEVENTID,
};

enum JobType: int
{
    JOB_NONE    = 0,
    JOB_BEGIN   = 1,
    JOB_WARRIOR = 1,
    JOB_TAOIST,
    JOB_MAGE,
    JOB_END,
};

inline bool jobValid(int job)
{
    return job >= JOB_BEGIN && job < JOB_END;
}

inline const char * jobName(int job)
{
    switch(job){
        case JOB_WARRIOR: return "WARRIOR";
        case JOB_TAOIST : return "TAOIST" ;
        case JOB_MAGE   : return "MAGE"   ;
        default         : return "UNKNOWN";
    }
}

// keep it POD
// used in actor/server/client message
// this only includes player look to draw, no necklace, rings etc
struct PlayerLook
{
    uint32_t hair;
    uint32_t hairColor;

    uint32_t helmet;
    uint32_t helmetColor;

    uint32_t dress;
    uint32_t dressColor;

    uint32_t weapon;
    uint32_t weaponColor;
};

struct PlayerWear
{
    uint32_t necklace;
    uint32_t armring[2];
    uint32_t ring[2];

    uint32_t shoes;
    uint32_t torch;
    uint32_t charm;
};

enum WearLookGridType: int
{
    WLG_NONE  = 0,
    WLG_BEGIN = 1,

    WLG_L_BEGIN = WLG_BEGIN,
    WLG_DRESS   = WLG_L_BEGIN,
    WLG_HELMET,
    WLG_WEAPON,
    WLG_L_END,

    WLG_W_BEGIN = WLG_L_END,
    WLG_SHOES   = WLG_W_BEGIN,
    WLG_NECKLACE,
    WLG_ARMRING0,
    WLG_ARMRING1,
    WLG_RING0,
    WLG_RING1,
    WLG_TORCH,
    WLG_CHARM,
    WLG_W_END,

    WLG_END = WLG_W_END,
};
