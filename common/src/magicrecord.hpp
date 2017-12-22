/*
 * =====================================================================================
 *
 *       Filename: magicrecord.hpp
 *        Created: 08/04/2017 23:00:09
 *  Last Modified: 08/10/2017 13:39:19
 *
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
#include <utility>
#include "sysconst.hpp"
#include "constexprfunc.hpp"

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
    int Stage;

    int Type;
    int GfxID;
    int FrameCount;

    // motion of the caster
    // can only defined for EGS_INIT (u8"启动")
    // x : MOTION_NONE
    // 0 : MOTION_SPELL0
    // 1 : MOTION_SPELL1
    int Motion;

    int Speed;
    int Loop;
    int DirType;
    constexpr GfxEntry(const GfxEntry& other) : Stage(other.Stage), Type(other.Type), GfxID(other.GfxID), FrameCount(other.FrameCount), Motion(other.Motion), Speed(other.Speed), Loop(other.Loop), DirType(other.DirType)
    {
    }

    // as entries in MagicRecord to use gfx resource
    // should provide default parameters to GfxEntry since it supports empty entries
    constexpr GfxEntry(
            const char *szStage = u8"",
            const char *szType  = u8"",

            int nGfxID      = -1,
            int nFrameCount = -1,

            int nMotion  = -1,

            int nSpeed   = SYS_MINSPEED - 1,
            int nLoop    = -1,
            int nDirType = -1)

        : Stage(ConstExprFunc::CheckIntMap(szStage, EGS_NONE,
                    u8"启动", EGS_INIT,
                    u8"开始", EGS_START,
                    u8"运行", EGS_RUN,
                    u8"结束", EGS_DONE,
                    u8"挨打", EGS_HITTED))
        , Type(ConstExprFunc::CheckIntMap(szType, EGT_NONE,
                    u8"固定", EGT_FIXED,
                    u8"附着", EGT_BOUND,
                    u8"射击", EGT_SHOOT,
                    u8"跟随", EGT_FOLLOW))
        , GfxID(nGfxID >= -1 ? nGfxID : -1)
        , FrameCount(nFrameCount >= -1 ? nFrameCount : -1)
        , Motion(ConstExprFunc::CheckIntParam(nMotion, -1, 0, 1) ? nMotion : -1)
        , Speed(nSpeed)
        , Loop(ConstExprFunc::CheckIntParam(nLoop, -1, 0, 1) ? nLoop : -1)
        , DirType(ConstExprFunc::CheckIntParam(nDirType, -1, 1, 8, 16) ? nDirType : -1)
    {}

    constexpr operator bool () const
    {
        return true
            && ((Stage > EGS_NONE) && (Stage < EGS_MAX))
            && ((Type  > EGT_NONE) && (Type  < EGT_MAX));
    }

    constexpr bool CheckStage(const char *szStage) const
    {
        switch(Stage){
            case EGS_INIT  : return ConstExprFunc::CompareUTF8(szStage, u8"启动");
            case EGS_START : return ConstExprFunc::CompareUTF8(szStage, u8"开始");
            case EGS_RUN   : return ConstExprFunc::CompareUTF8(szStage, u8"运行");
            case EGS_DONE  : return ConstExprFunc::CompareUTF8(szStage, u8"结束");
            case EGS_HITTED: return ConstExprFunc::CompareUTF8(szStage, u8"挨打");
            default        : return false;
        }
    }

    constexpr bool CheckType(const char *szTypeStr) const
    {
        switch(Type){
            case EGT_FIXED : return ConstExprFunc::CompareUTF8(szTypeStr, u8"固定");
            case EGT_BOUND : return ConstExprFunc::CompareUTF8(szTypeStr, u8"附着");
            case EGT_SHOOT : return ConstExprFunc::CompareUTF8(szTypeStr, u8"射击");
            case EGT_FOLLOW: return ConstExprFunc::CompareUTF8(szTypeStr, u8"跟随");
            default        : return false;
        }
    }

    void Print() const
    {
    }
};

namespace
{
    // used for MagicRecord::GfxArray(u8"开始")
    // if we can't find it we have to return an empty entry
    constexpr GfxEntry _Inn_Empty_MagicAnimation;
}

class MagicRecord
{
    public:
        const char *Name;

    public:
        const GfxEntry GfxArray[8];

    public:
        template<typename... U> constexpr MagicRecord(
                const char *szName,

                // template pack for GfxEntry array
                // should put at the end of the argument list
                U&&... u)
            : Name(szName ? szName : u8"")
            , GfxArray { std::forward<U>(u)... }
        {}

    public:
        constexpr const GfxEntry &GetGfxEntry(int nEntryIndex) const
        {
            if(true
                    && nEntryIndex >= 0
                    && nEntryIndex <  (int)(sizeof(GfxArray) / sizeof(GfxArray[0]))){
                return GfxArray[nEntryIndex];
            }
            return _Inn_Empty_MagicAnimation;
        }

        constexpr const GfxEntry &GetGfxEntry(const char *szStage) const
        {
            for(size_t nIndex = 0; nIndex < sizeof(GfxArray) / sizeof(GfxArray[0]); ++nIndex){
                if(GfxArray[nIndex].CheckStage(szStage)){
                    return GfxArray[nIndex];
                }
            }
            return _Inn_Empty_MagicAnimation;
        }

    public:
        constexpr operator bool () const
        {
            return true;
        }

        void Print() const
        {
        }
};
