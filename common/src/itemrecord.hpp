/*
 * =====================================================================================
 *
 *       Filename: itemrecord.hpp
 *        Created: 07/28/2017 17:12:29
 *  Last Modified: 07/30/2017 22:51:19
 *
 *    Description: 
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

        int GfxID;

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
                int nGfxID,

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
            , GfxID(nGfxID)
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

    private:
        // need some _Inn_Func_XXXX()
        // c++14 currently don't support constexpr lambda
        static constexpr bool _Inn_CompareUTF8(const char *szStr1, const char *szStr2)
        {
            if(szStr1 && szStr2){
                while(*szStr1 == *szStr2){
                    if(*szStr1){
                        ++szStr1;
                        ++szStr2;
                    }else{
                        return true;
                    }
                }
            }
            return false;
        }

        static constexpr int _Inn_ItemRecord_Type(const char *szType)
        {
            if(false){
            }else if(_Inn_CompareUTF8(szType, u8"金币"    )){ return IRTYPE_GOLD;
            }else if(_Inn_CompareUTF8(szType, u8"恢复药水")){ return IRTYPE_RESTORE;
            }else                                             return IRTYPE_NONE;
        }

        static constexpr int _Inn_ItemRecord_Rarity(const char *szRarity)
        {
            if(false){
            }else if(_Inn_CompareUTF8(szRarity, u8"普通")){ return IRRARITY_COMMON;
            }else if(_Inn_CompareUTF8(szRarity, u8"高级")){ return IRRARITY_HIGHQ;
            }else if(_Inn_CompareUTF8(szRarity, u8"稀有")){ return IRRARITY_RARE;
            }else                                           return IRRARITY_NONE;
        }

        static constexpr int _Inn_ItemRecord_NeedJob(const char *szNeedJob)
        {
            if(false){
            }else if(_Inn_CompareUTF8(szNeedJob, u8"武士"  )){ return IRNEEDJOB_WARRIOR;
            }else if(_Inn_CompareUTF8(szNeedJob, u8"战士"  )){ return IRNEEDJOB_WARRIOR;
            }else if(_Inn_CompareUTF8(szNeedJob, u8"道士"  )){ return IRNEEDJOB_TAOIST;
            }else if(_Inn_CompareUTF8(szNeedJob, u8"法师"  )){ return IRNEEDJOB_MAGICIAN;
            }else if(_Inn_CompareUTF8(szNeedJob, u8"魔法师")){ return IRNEEDJOB_MAGICIAN;
            }else if(_Inn_CompareUTF8(szNeedJob, u8"共用"  )){ return IRNEEDJOB_ALL;
            }else                                              return IRNEEDJOB_NONE;
        }
};
