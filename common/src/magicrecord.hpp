/*
 * =====================================================================================
 *
 *       Filename: magicrecord.hpp
 *        Created: 08/04/2017 23:00:09
 *    Description: description of magic
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
#include <string_view>
#include <initializer_list>
#include "sysconst.hpp"

enum MagicElemType: int
{
    MET_NONE  = 0,
    MET_BEGIN = 1,
    MET_FIRE  = 1,
    MET_ICE,
    MET_LIGHT,
    MET_WIND,
    MET_HOLY,
    MET_DARK,
    MET_PHANTOM,
    MET_END,
};

constexpr inline const char8_t *magicElemName(int type)
{
    switch(type){
        case MET_FIRE     : return u8"火";
        case MET_ICE      : return u8"冰";
        case MET_LIGHT    : return u8"雷";
        case MET_WIND     : return u8"风";
        case MET_HOLY     : return u8"神圣";
        case MET_DARK     : return u8"暗黑";
        case MET_PHANTOM  : return u8"幻影";
        default           : return u8"无";
    }
}

constexpr inline int magicElemID(const char8_t *type)
{
    if(type && std::u8string_view(type) == u8"火"  ) return MET_FIRE;
    if(type && std::u8string_view(type) == u8"冰"  ) return MET_ICE;
    if(type && std::u8string_view(type) == u8"雷"  ) return MET_LIGHT;
    if(type && std::u8string_view(type) == u8"风"  ) return MET_WIND;
    if(type && std::u8string_view(type) == u8"神圣") return MET_HOLY;
    if(type && std::u8string_view(type) == u8"暗黑") return MET_DARK;
    if(type && std::u8string_view(type) == u8"幻影") return MET_PHANTOM;
    return                                                  MET_NONE;
}

enum MagicStageType: int
{
    MST_NONE  = 0,
    MST_BEGIN = 1,
    MST_SPELL = 1,
    MST_START,
    MST_RUN,
    MST_DONE,
    MST_HITTED,
    MST_END,
};

constexpr inline const char8_t *magicStageName(int type)
{
    switch(type){
        case MST_SPELL  : return u8"启动";
        case MST_START  : return u8"开始";
        case MST_RUN    : return u8"运行";
        case MST_DONE   : return u8"结束";
        case MST_HITTED : return u8"挨打";
        default         : return nullptr ;
    }
}

constexpr inline int magicStageID(const char8_t *type)
{
    if(type && std::u8string_view(type) == u8"启动") return MST_SPELL;
    if(type && std::u8string_view(type) == u8"开始") return MST_START;
    if(type && std::u8string_view(type) == u8"运行") return MST_RUN;
    if(type && std::u8string_view(type) == u8"结束") return MST_DONE;
    if(type && std::u8string_view(type) == u8"挨打") return MST_HITTED;
    return                                                  MST_NONE;
}

enum MagicGfxEntryType: int
{
    MGT_NONE  = 0,
    MGT_BEGIN = 1,
    MGT_FIXED = 1,
    MGT_BOUND,
    MGT_FOLLOW,
    MGT_END,
};

constexpr inline const char8_t *magicGfxEntryName(int type)
{
    switch(type){
        case MGT_FIXED  : return u8"固定";
        case MGT_BOUND  : return u8"附着";
        case MGT_FOLLOW : return u8"跟随";
        default         : return nullptr ;
    }
}

constexpr inline int magicGfxEntryID(const char8_t *type)
{
    if(type && std::u8string_view(type) == u8"固定") return MGT_FIXED;
    if(type && std::u8string_view(type) == u8"附着") return MGT_BOUND;
    if(type && std::u8string_view(type) == u8"跟随") return MGT_FOLLOW;
    return                                                  MGT_NONE;
}

struct MagicGfxEntryRef
{
    const char8_t *name  = nullptr;
    const char8_t *stage = nullptr;
};

struct MagicGfxEntry
{
    const MagicGfxEntryRef ref {};

    const char8_t *stage = nullptr;
    const char8_t *type  = nullptr;

    const uint32_t gfxID = SYS_TEXNIL;

    const int frameCount = 0;
    const int gfxIDCount = frameCount;
    const int gfxDirType = 1;

    // gfx is on ground
    // player can walk on the magic
    const int onGround = 0;

    // motion of the magic caster
    // can only defined for EGS_INIT (u8"启动")
    // 0 : MOTION_SPELL0, use push front gestur
    // 1 : MOTION_SPELL1
    const int motion = 0;

    const int loop = 0;
    const int speed = SYS_DEFSPEED;

    constexpr operator bool () const
    {
        return stage && type && checkStage() && checkType();
    }

    constexpr bool checkStage(const char8_t *argStage) const
    {
        return std::u8string_view(argStage) == stage;
    }

    constexpr bool checkType(const char8_t *argType) const
    {
        return std::u8string_view(argType) == type;
    }

    constexpr bool checkStage() const
    {
        return false
            || checkStage(u8"启动")
            || checkStage(u8"开始")
            || checkStage(u8"运行")
            || checkStage(u8"结束")
            || checkStage(u8"挨打");
    }

    constexpr bool checkType() const
    {
        return false
            || checkType(u8"固定")
            || checkType(u8"附着")
            || checkType(u8"跟随");
    }
};

namespace
{
    constexpr MagicGfxEntry _inn_reservedEmptyMagicGfxEntry;
}

enum DCCastRangeType: int
{
    CRT_NONE = 0,
    CRT_BEGIN,

    CRT_DIR = CRT_BEGIN,
    CRT_LONG,
    CRT_LIMITED,
    CRT_END,
};

struct DCCastRange
{
    const DCCastRangeType type = CRT_NONE;
    const int distance = 0;

    operator bool () const
    {
        switch(type){
            case CRT_DIR    : return distance >= 1;
            case CRT_LONG   : return distance == 0;
            case CRT_LIMITED: return distance >= 1;
            default         : return false;
        }
    }
};

struct MagicRecord
{
    const char8_t *name = nullptr;
    const char8_t *elem = nullptr;

    const struct MagicReq
    {
        const int level[3] = {0, 0, 0};
        const int train[3] = {0, 0, 0};
        const char8_t *job = nullptr;
    }
    req {};

    const DCCastRange castRange
    {
        .type = CRT_LONG,
    };

    const int mp = 0;
    const int coolDown = 0;
    const int checkGround = 0;

    const int    power[2] = {0, 0};
    const int addPower[2] = {0, 0};

    const uint32_t icon = SYS_TEXNIL;
    const std::initializer_list<MagicGfxEntry> gfxList {};

    constexpr const MagicGfxEntry &getGfxEntry(const char8_t *stage) const
    {
        for(const auto &entry: gfxList){
            if(entry && entry.checkStage(stage)){
                return entry;
            }
        }
        return _inn_reservedEmptyMagicGfxEntry;
    }

    constexpr operator bool () const
    {
        return name && name[0];
    }
};
