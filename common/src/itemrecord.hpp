/*
 * =====================================================================================
 *
 *       Filename: itemrecord.hpp
 *        Created: 07/28/2017 17:12:29
 *    Description: for each item I have two GfxID:
 *                      PkgGfxID : when on ground and in inventory
 *                      UseGfxID : when drawing with player body
 *
 *                 why should I introduce all two different IDs?
 *
 *                 Best practice should be one unique GfxID for all items and retrieve
 *                 different set of animations from different archieves using the unique
 *                 GfxID.
 *
 *                 But in ghe graphical libraries, one item may share one GfxID for one
 *                 kind of usage but own its own resource for another usage. i.e. for the
 *                 诅咒屠龙 and 幸运屠龙, when drawing it with human body they are sharing
 *                 one set of animation, but when staying in inventory they have their own
 *                 different animations.
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
#include <string>
#include <cstdint>
#include <string_view>

enum IRType: int
{
    IRTYPE_NONE = 0,
    IRTYPE_GOLD,        // u8"金币"
    IRTYPE_RESTORE,     // u8"恢复药水"
};

enum IRRarity: int
{
    IRRARITY_NONE = 0,
    IRRARITY_COMMON,    // u8"普通"
    IRRARITY_HIGHQ,     // u8"高级"
    IRRARITY_RARE,      // u8"稀有"
};

enum IRNeedJob: int
{
    IRNEEDJOB_NONE = 0,
    IRNEEDJOB_ALL,          // u8"共用"
    IRNEEDJOB_WARRIOR,      // u8"战士"
    IRNEEDJOB_TAOIST,       // u8"道士"
    IRNEEDJOB_MAGICIAN,     // u8"法师"
};

class ItemRecord
{
    public:
        const char8_t *name;

        int type;
        int rarity;
        int weight;

        int pkgGfxID;
        int useGfxID;

        int needJob;
        int needLevel;
        int needDC;
        int needSPC;
        int needMDC;
        int needAC;
        int needMAC;

    public:
        constexpr ItemRecord(
                const char8_t *argName,
                const char8_t *argType,
                const char8_t *argRarity,

                int argWeight,

                int argPkgGfxID,
                int argUseGfxID,

                const char8_t *argNeedJob,
                int            argNeedLevel,
                int            argNeedDC,
                int            argNeedSPC,
                int            argNeedMDC,
                int            argNeedAC,
                int            argNeedMAC)
            : name(argName ? argName : u8"")
            , type(_inn_ItemRecord_Type(argType))
            , rarity(_inn_ItemRecord_Rarity(argRarity))
            , weight(argWeight)
            , pkgGfxID(argPkgGfxID)
            , useGfxID(argUseGfxID)
            , needJob(_inn_ItemRecord_NeedJob(argNeedJob))
            , needLevel(argNeedLevel) 
            , needDC(argNeedDC) 
            , needSPC(argNeedSPC) 
            , needMDC(argNeedMDC) 
            , needAC(argNeedAC) 
            , needMAC(argNeedMAC) 
        {
                // add check here
        }

    public:
        operator bool() const
        {
            return true;
        }

    private:
        static constexpr int _inn_ItemRecord_Type(const char8_t *type)
        {
            using namespace std::literals;
            if     (u8"金币"sv     == type) return IRTYPE_GOLD;
            else if(u8"恢复药水"sv == type) return IRTYPE_RESTORE;
            else                            return IRTYPE_NONE;
        }

        static constexpr int _inn_ItemRecord_Rarity(const char8_t *rarity)
        {
            using namespace std::literals;
            if     (u8"普通"sv == rarity) return IRRARITY_COMMON;
            else if(u8"高级"sv == rarity) return IRRARITY_HIGHQ;
            else if(u8"稀有"sv == rarity) return IRRARITY_RARE;
            else                          return IRRARITY_NONE;
        }

        static constexpr int _inn_ItemRecord_NeedJob(const char8_t *needJob)
        {
            using namespace std::literals;
            if     (u8"战士"sv   == needJob) return IRNEEDJOB_WARRIOR;
            else if(u8"道士"sv   == needJob) return IRNEEDJOB_TAOIST;
            else if(u8"法师"sv   == needJob) return IRNEEDJOB_MAGICIAN;
            else if(u8"魔法师"sv == needJob) return IRNEEDJOB_MAGICIAN;
            else if(u8"共用"sv   == needJob) return IRNEEDJOB_ALL;
            else                            return IRNEEDJOB_NONE;
        }
};
