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
#include "sysconst.hpp"
#include "constexprf.hpp"

enum MagicElemType: int
{
    MET_NONE  = 0,
    MET_BEGIN = 1,
    MET_FIRE  = 1,
    MET_ICE,
    MET_THUNDER,
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
        case MET_THUNDER  : return u8"雷";
        case MET_WIND     : return u8"风";
        case MET_HOLY     : return u8"神圣";
        case MET_DARK     : return u8"暗黑";
        case MET_PHANTOM  : return u8"幻影";
        default           : return u8"无";
    }
}

// use EG_ rather than GE_ here
// otherwise for GfxEntryType we get GET_XXXX

enum GfxEntryStage: int
{
    EGS_NONE = 0,       // u8""
    EGS_INIT,           // u8"启动"
    EGS_START,          // u8"开始"
    EGS_RUN,            // u8"运行"
    EGS_DONE,           // u8"结束"
    EGS_HITTED,         // u8"挨打"
    EGS_MAX,
};

enum GfxEntryType: int
{
    EGT_NONE = 0,       // u8""
    EGT_FIXED,          // u8"固定"
    EGT_BOUND,          // u8"附着"
    EGT_SHOOT,          // u8"射击"
    EGT_FOLLOW,         // u8"跟随"
    EGT_MAX,
};

struct GfxEntry
{
    const int stage;

    const int type;
    const int gfxID;
    const int frameCount;

    // motion of the caster
    // can only defined for EGS_INIT (u8"启动")
    // x : MOTION_NONE
    // 0 : MOTION_SPELL0
    // 1 : MOTION_SPELL1
    const int motion;

    const int speed;
    const int loop;
    const int dirType;

    // as entries in MagicRecord to use gfx resource
    // should provide default parameters to GfxEntry since it supports empty entries
    constexpr GfxEntry(
            const char8_t *argStage = u8"",
            const char8_t *argType  = u8"",

            int argGfxID      = -1,
            int argFrameCount = -1,

            int argMotion  = -1,

            int argSpeed   = SYS_MINSPEED - 1,
            int argLoop    = -1,
            int argDirType = -1)

        : stage(constexprf::map2Int(argStage, EGS_NONE,
                    u8"启动", EGS_INIT,
                    u8"开始", EGS_START,
                    u8"运行", EGS_RUN,
                    u8"结束", EGS_DONE,
                    u8"挨打", EGS_HITTED))
        , type(constexprf::map2Int(argType, EGT_NONE,
                    u8"固定", EGT_FIXED,
                    u8"附着", EGT_BOUND,
                    u8"射击", EGT_SHOOT,
                    u8"跟随", EGT_FOLLOW))
        , gfxID(argGfxID >= -1 ? argGfxID : -1)
        , frameCount(argFrameCount >= -1 ? argFrameCount : -1)
        , motion(constexprf::hasInt(argMotion, -1, 0, 1) ? argMotion : -1)
        , speed(argSpeed)
        , loop(constexprf::hasInt(argLoop, -1, 0, 1) ? argLoop : -1)
        , dirType(constexprf::hasInt(argDirType, -1, 1, 8, 16) ? argDirType : -1)
    {}

    constexpr operator bool () const
    {
        return true
            && ((stage > EGS_NONE) && (stage < EGS_MAX))
            && ((type  > EGT_NONE) && (type  < EGT_MAX));
    }

    constexpr bool checkStage(const char8_t *argStage) const
    {
        switch(stage){
            case EGS_INIT  : return std::u8string_view(argStage) == u8"启动";
            case EGS_START : return std::u8string_view(argStage) == u8"开始";
            case EGS_RUN   : return std::u8string_view(argStage) == u8"运行";
            case EGS_DONE  : return std::u8string_view(argStage) == u8"结束";
            case EGS_HITTED: return std::u8string_view(argStage) == u8"挨打";
            default        : return false;
        }
    }

    constexpr bool checkType(const char8_t *argType) const
    {
        switch(type){
            case EGT_FIXED : return std::u8string_view(argType) == u8"固定";
            case EGT_BOUND : return std::u8string_view(argType) == u8"附着";
            case EGT_SHOOT : return std::u8string_view(argType) == u8"射击";
            case EGT_FOLLOW: return std::u8string_view(argType) == u8"跟随";
            default        : return false;
        }
    }
};

namespace
{
    constexpr GfxEntry _inn_Empty_MagicAnimation;
}

class MagicRecord
{
    public:
        const char8_t *name;

    public:
        const int elem;
        const uint32_t icon;

    public:
        const GfxEntry gfxArray[8];

    public:
        template<typename... U> constexpr MagicRecord(const char8_t *argName, int meType, uint32_t iconGfx, U&&... u)
            : name(argName ? argName : u8"")
            , elem((meType >= MET_BEGIN && meType < MET_END) ? meType : MET_NONE)
            , icon(iconGfx)
            , gfxArray { std::forward<U>(u)... }
        {}

    public:
        constexpr const GfxEntry &getGfxEntry(int entryIndex) const
        {
            if(entryIndex >= 0 && entryIndex < (int)(std::extent<decltype(gfxArray)>::value)){
                return gfxArray[entryIndex];
            }
            return _inn_Empty_MagicAnimation;
        }

        constexpr const GfxEntry &getGfxEntry(const char8_t *stage) const
        {
            for(const auto &entry: gfxArray){
                if(entry.checkStage(stage)){
                    return entry;
                }
            }
            return _inn_Empty_MagicAnimation;
        }

    public:
        constexpr operator bool () const
        {
            return true;
        }
};
