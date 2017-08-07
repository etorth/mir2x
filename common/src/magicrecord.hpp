/*
 * =====================================================================================
 *
 *       Filename: magicrecord.hpp
 *        Created: 08/04/2017 23:00:09
 *  Last Modified: 08/06/2017 17:32:27
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
#include "constexprfunc.hpp"
struct GfxEntry
{
    const char *Name;

    int GfxID;
    int FrameCount;

    int Loop;
    int DirType;

    // as entries in MagicRecord to use gfx resource
    // should provide default parameters to GfxEntry since it supports empty entries
    constexpr GfxEntry(
            const char *szName = u8"",

            int nGfxID      = -1,
            int nFrameCount = -1,

            int nLoop    = -1,
            int nDirType = -1)

        : Name(szName ? szName : u8"")
        , GfxID(nGfxID >= -1 ? nGfxID : -1)
        , FrameCount(nFrameCount >= -1 ? nFrameCount : -1)
        , Loop(ConstExprFunc::CheckIntParam(nLoop, -1, 0, 1) ? nLoop : -1)
        , DirType(ConstExprFunc::CheckIntParam(nDirType, -1, 1, 8, 16) ? nDirType : -1)
    {}

    constexpr operator bool () const
    {
        return !ConstExprFunc::CompareUTF8(Name, u8"");
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
