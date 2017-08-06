/*
 * =====================================================================================
 *
 *       Filename: magicrecord.hpp
 *        Created: 08/04/2017 23:00:09
 *  Last Modified: 08/06/2017 12:50:52
 *
 *    Description: description of magic
 *                 1. prayer
 *                 2. magic
 *                 3. explosion
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
class MagicRecord
{
    public:
        const char *Name;

        // prayer doesn't have delay information
        // it gets updated with motion frame synchronized
        int EffectGfxID;
        int EffectDirection;
        int EffectFrameCount;
        int EffectLoop;

        int MagicGfxID;
        int MagicDirection;
        int MagicFrameCount;

        int StruckGfxID;
        int StruckDirection;
        int StruckFrameCount;
        int StruckLoop;

        int ExplosionGfxID;
        int ExplosionDirection;
        int ExplosionFrameCount;

    public:
        constexpr MagicRecord(
                const char *szName,

                int nEffectGfxID,
                int nEffectDirection,
                int nEffectFrameCount,
                int nEffectLoop,

                int nMagicGfxID,
                int nMagicDirection,
                int nMagicFrameCount,

                int nStruckGfxID,
                int nStruckDirection,
                int nStruckFrameCount,
                int nStruckLoop,

                int nExplosionGfxID,
                int nExplosionDirection,
                int nExplosionFrameCount)
            : Name(szName ? szName : "")

            , EffectGfxID(nEffectGfxID)
            , EffectDirection(nEffectDirection)
            , EffectFrameCount(nEffectFrameCount)
            , EffectLoop(nEffectLoop >= 1 ? 1 : 0)

            , MagicGfxID(nMagicGfxID)
            , MagicDirection(nMagicDirection)
            , MagicFrameCount(nMagicFrameCount)

            , StruckGfxID(nStruckGfxID)
            , StruckDirection(nStruckDirection)
            , StruckFrameCount(nStruckFrameCount)
            , StruckLoop(nStruckLoop >= 1 ? 1 : 0)

            , ExplosionGfxID(nExplosionGfxID)
            , ExplosionDirection(nExplosionDirection)
            , ExplosionFrameCount(nExplosionFrameCount)
        {}

    public:
        operator bool () const
        {
            return true;
        }

        void Print() const
        {
        }
};
