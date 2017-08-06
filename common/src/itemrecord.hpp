/*
 * =====================================================================================
 *
 *       Filename: itemrecord.hpp
 *        Created: 07/28/2017 17:12:29
 *  Last Modified: 08/05/2017 12:19:01
 *
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
#include "constexprfunc.hpp"

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
        // point to the literal constant in itemrecord.inc
        // don't use std::string here since ItemRecord will be constexpr
        const char *Name;

        int Type;
        int Rarity;
        int Weight;

        int PkgGfxID;
        int UseGfxID;

        int NeedJob;
        int NeedLevel;
        int NeedDC;
        int NeedSPC;
        int NeedMDC;
        int NeedAC;
        int NeedMAC;

    public:
        constexpr ItemRecord(
                const char *szName,
                const char *szType,
                const char *szRarity,

                int nWeight,

                int nPkgGfxID,
                int nUseGfxID,

                const char *szNeedJob,
                int          nNeedLevel,
                int          nNeedDC,
                int          nNeedSPC,
                int          nNeedMDC,
                int          nNeedAC,
                int          nNeedMAC)
            : Name(szName ? szName : "")
            , Type(_Inn_ItemRecord_Type(szType))
            , Rarity(_Inn_ItemRecord_Rarity(szRarity))
            , Weight(nWeight)
            , PkgGfxID(nPkgGfxID)
            , UseGfxID(nUseGfxID)
            , NeedJob(_Inn_ItemRecord_NeedJob(szNeedJob))
            , NeedLevel(nNeedLevel) 
            , NeedDC(nNeedDC) 
            , NeedSPC(nNeedSPC) 
            , NeedMDC(nNeedMDC) 
            , NeedAC(nNeedAC) 
            , NeedMAC(nNeedMAC) 
        {
                // add check here
        }

    public:
        // check if item record is valid
        // will skip if invalid and *no* log supported
        operator bool() const;

    public:
        static constexpr int _Inn_ItemRecord_Type(const char *szType)
        {
            if(false){
            }else if(ConstExprFunc::CompareUTF8(szType, u8"金币"    )){ return IRTYPE_GOLD;
            }else if(ConstExprFunc::CompareUTF8(szType, u8"恢复药水")){ return IRTYPE_RESTORE;
            }else                                                       return IRTYPE_NONE;
        }

        static constexpr int _Inn_ItemRecord_Rarity(const char *szRarity)
        {
            if(false){
            }else if(ConstExprFunc::CompareUTF8(szRarity, u8"普通")){ return IRRARITY_COMMON;
            }else if(ConstExprFunc::CompareUTF8(szRarity, u8"高级")){ return IRRARITY_HIGHQ;
            }else if(ConstExprFunc::CompareUTF8(szRarity, u8"稀有")){ return IRRARITY_RARE;
            }else                                                     return IRRARITY_NONE;
        }

        static constexpr int _Inn_ItemRecord_NeedJob(const char *szNeedJob)
        {
            if(false){
            }else if(ConstExprFunc::CompareUTF8(szNeedJob, u8"武士"  )){ return IRNEEDJOB_WARRIOR;
            }else if(ConstExprFunc::CompareUTF8(szNeedJob, u8"战士"  )){ return IRNEEDJOB_WARRIOR;
            }else if(ConstExprFunc::CompareUTF8(szNeedJob, u8"道士"  )){ return IRNEEDJOB_TAOIST;
            }else if(ConstExprFunc::CompareUTF8(szNeedJob, u8"法师"  )){ return IRNEEDJOB_MAGICIAN;
            }else if(ConstExprFunc::CompareUTF8(szNeedJob, u8"魔法师")){ return IRNEEDJOB_MAGICIAN;
            }else if(ConstExprFunc::CompareUTF8(szNeedJob, u8"共用"  )){ return IRNEEDJOB_ALL;
            }else                                                        return IRNEEDJOB_NONE;
        }
};
