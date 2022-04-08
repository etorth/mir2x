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
#include <tuple>
#include <utility>
#include <string_view>
#include <initializer_list>
#include "sysconst.hpp"
#include "motion.hpp"
#include "colorf.hpp"

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
    const char8_t *name = nullptr;
    const char8_t *stage = nullptr;
    const uint32_t modColor = colorf::RGBA(0XFF, 0XFF, 0XFF, 0XFF);

    constexpr operator bool () const
    {
        return name && name[0] && stage && stage[0];
    }

    constexpr bool checkStage(const char8_t *argStage) const
    {
        return argStage && argStage[0] && stage && stage[0] && (std::u8string_view(argStage) == stage);
    }
};

struct MagicGfxEntry
{
    const MagicGfxEntryRef ref {};

    const char8_t *stage = nullptr;
    const char8_t *type  = nullptr;

    const uint32_t gfxID = SYS_U32NIL;
    const uint32_t modColor = colorf::RGBA(0XFF, 0XFF, 0XFF, 0XFF);

    const int frameCount = 0;
    const int gfxIDCount = frameCount;
    const int gfxDirType = 1;

    // target offset
    // only needed for magic type: 跟随
    // targetOffList.size() should be zero or equal to gfxDirType
    const std::initializer_list<std::tuple<int, int>> targetOffList {};
    const struct MagicGfxEntrySoundEffect
    {
        // common cases of sound indexing
        // 1. magic has no sound at all, like 精神力战法，基本剑术，etc:
        //
        //      leave MagicRecord::seffBase empty as default
        //      leave MagicGfxEntry::seff::absSeffID empty
        //
        // 2. magic entry has sound effect and is by default sound indexing: 0X04000000 + MagicRecord::seff * 16 + offset_2_stage, mostly common
        //
        //      assign MagicRecord::seffBase value as in King_Magic.csv
        //      leave MagicGfxEntry::seff::absSeffID empty
        //
        // 3. magic entry has sound effect but sound index can not be figured out directly:
        //
        //      ignore MagicRecord::seffBase value
        //      assign MagicGfxEntry::seff::absSeffID with the absolute seffID in SoundEffectDB
        //
        // 4. magic has sound for other gfxEntry, but *this* entry has no sound, very common
        //
        //      do same as 2
        //      client tries to find the sound file in SoundEffectDB, but it doesn't exist
        //
        //    TODO this case and case-2 are mostly common for the magic configuration
        //         I shall keep their configuration as simple as possible, mute a MagicGfxEntry in a gentle way takes me some time
        //
        //         for current implementation:
        //         pros: simple
        //         cons: SoundEffectDB either loads this non-existing file everytime trying to play it, or a null place-holder needed
        //
        //         for implementation that add a flag as: MagicGfxEntry::seff::mute
        //         pros:
        //         cons: case-2 and case-4 are inverse
        //               this means for either case-2 or case-4 I have to explicitly assign this .mute a value to overwrite the default
        //
        //         for implementation that assign MagicGfxEntry::seff::absSeffID as SYS_U32NIL
        //         pros: skip the loading at all
        //         cons: 1. need to explicit assign this value if to disable the GfxEntry
        //               2. I am trying to get rid of this SYS_U32MIL and SYS_U64NIL, most of time an empty std::optional can be used to replace magic number
        //                  but here I can not because the empty std::optional means to used default sound indexing rule

        // for a magic gfx entry, priority of sound effect:
        // 1. use sound effect of ref if ref is defined
        // 2. use absSeffID if defined
        // 3. use 0X04000000 + MagicRecord::seff * 16 + offset_2_stage

        const struct MagicGfxEntrySoundEffectRef
        {
            const std::u8string_view name  {};
            const std::u8string_view stage {};

            constexpr operator bool () const
            {
                return !name.empty() && stage.empty();
            }
        }
        ref {};

        // absolute index in SoundEffectDB
        // assign it to SYS_U32NIL to mute *this* MagicGfxEntry
        const std::optional<uint32_t> absSeffID {};
    }
    seff {};

    // gfx is on ground
    // player can walk on the magic
    const int onGround = 0;

    // motion of the magic caster
    // can only defined for EGS_INIT (u8"启动")
    // * : MOTION_SPELL0, use push front gesture
    // * : MOTION_SPELL1
    // * : MOTION_ATTACKMODE, for 铁布衫 and 破血狂杀
    const int motion = MOTION_SPELL0;

    const int loop = 0;
    const int speed = SYS_DEFSPEED;

    constexpr operator bool () const
    {
        return (stage && stage[0] && checkStage()) && (type && type[0] && checkType());
    }

    constexpr bool checkStage(const char8_t *argStage) const
    {
        return argStage && argStage[0] && stage && stage[0] && (std::u8string_view(argStage) == stage);
    }

    constexpr bool checkType(const char8_t *argType) const
    {
        return argType && argType[0] && type && type[0] && (std::u8string_view(argType) == type);
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

        const char8_t *job   = nullptr;
        const char8_t *prior = nullptr;
    }
    req {};

    const DCCastRange castRange
    {
        .type = CRT_LONG,
    };

    const int castDelay   = 0;
    const int coolDown    = 0;
    const int checkGround = 0;

    const int mp = 0;
    const int mpInc = 0;

    const int    power[2] = {0, 0};
    const int addPower[2] = {0, 0};

    // seffBase is MagID in readme/sql2csv/King_Magic.csv
    // MagicGfxEntry can assign value to absSeffID to give seperate sound effect for specific entry
    const std::optional<int> seffBase {};
    const std::initializer_list<MagicGfxEntry> gfxList {};

    constexpr operator bool () const
    {
        return name && name[0];
    }

    // need to be non-constexpr
    // because to support magic gfx entry reference
    std::pair<const MagicGfxEntry &, const MagicGfxEntryRef &> getGfxEntry(const char8_t *) const;
};
