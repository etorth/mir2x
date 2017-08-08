/*
 * =====================================================================================
 *
 *       Filename: magicrecord.hpp
 *        Created: 08/04/2017 23:00:09
 *  Last Modified: 08/08/2017 16:48:24
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

enum EffectGfxType: int
{
    EGT_NONE = 0,       // u8""
    EGT_FIXED,          // u8"固定"
    EGT_BOUND,          // u8"附着"
    EGT_SHOOT,          // u8"射击"
    EGT_FOLLOW,         // u8"跟随"
};

struct GfxEntry
{
    const char *Name;

    int Type;
    int GfxID;
    int FrameCount;

    // only defined for stage: u8"启动"
    // motion when current GfxEntry takes place
    // x : MOTION_NONE
    // 0 : MOTION_SPELL0
    // 1 : MOTION_SPELL1
    int Motion;

    int Speed;
    int Loop;
    int DirType;

    // as entries in MagicRecord to use gfx resource
    // should provide default parameters to GfxEntry since it supports empty entries
    constexpr GfxEntry(
            const char *szName = u8"",
            const char *szType = u8"",

            int nGfxID      = -1,
            int nFrameCount = -1,

            int nMotion  = -1,

            int nSpeed   = SYS_MINSPEED - 1,
            int nLoop    = -1,
            int nDirType = -1)

        : Name(szName ? szName : u8"")
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
        return !ConstExprFunc::CompareUTF8(Name, u8"");
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

        constexpr const GfxEntry &GetGfxEntry(const char *szName) const
        {
            for(size_t nIndex = 0; nIndex < sizeof(GfxArray) / sizeof(GfxArray[0]); ++nIndex){
                if(ConstExprFunc::CompareUTF8(GfxArray[nIndex].Name, szName)){
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
