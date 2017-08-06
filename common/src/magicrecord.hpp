/*
 * =====================================================================================
 *
 *       Filename: magicrecord.hpp
 *        Created: 08/04/2017 23:00:09
 *  Last Modified: 08/05/2017 12:25:22
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
        int PrayerGfxID;
        int PrayerDirection;
        int PrayerFrameCount;

        int MagicGfxID;
        int MagicDirection;
        int MagicFrameCount;

        int ExplosionGfxID;
        int ExplosionDirection;
        int ExplosionFrameCount;

    public:
        constexpr MagicRecord(
                const char *szName,

                int nPrayerGfxID,
                int nPrayerDirection,
                int nPrayerFrameCount,

                int nMagicGfxID,
                int nMagicDirection,
                int nMagicFrameCount,

                int nExplosionGfxID,
                int nExplosionDirection,
                int nExplosionFrameCount)
            : Name(szName ? szName : "")

            , PrayerGfxID(nPrayerGfxID)
            , PrayerDirection(nPrayerDirection)
            , PrayerFrameCount(nPrayerFrameCount)

            , MagicGfxID(nMagicGfxID)
            , MagicDirection(nMagicDirection)
            , MagicFrameCount(nMagicFrameCount)

            , ExplosionGfxID(nExplosionGfxID)
            , ExplosionDirection(nExplosionDirection)
            , ExplosionFrameCount(nExplosionFrameCount)
        {}

    public:
        operator bool () const
        {
            return true;
        }
};
