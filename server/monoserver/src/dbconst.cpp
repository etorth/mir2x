/*
 * =====================================================================================
 *
 *       Filename: dbconst.cpp
 *        Created: 05/12/2017 17:58:02
 *  Last Modified: 07/30/2017 00:30:59
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

#include <map>
#include <cinttypes>

#include "dbcom.hpp"
#include "dbconst.hpp"
#include "monoserver.hpp"
#include "protocoldef.hpp"
#include "monsterrecord.hpp"
#include "dropitemconfig.hpp"

const DCRecord &DB_DCRECORD(uint32_t nDC)
{
    static std::map<uint32_t, DCRecord> stDCRecord
    {
        {           0, {           0, 0, 0, u8"", u8""}},
        {DC_PHY_PLAIN, {DC_PHY_PLAIN, 0, 0, u8"", u8""}},
    };
    return stDCRecord.at((stDCRecord.find(nDC) != stDCRecord.end()) ? nDC : 0);
}

const std::map<int, std::vector<DropItem>> &DB_MONSTERDROPITEM(uint32_t nMonsterID)
{
    using DropItemMap = std::map<int, std::vector<DropItem>>;
    if(nMonsterID){
        const static std::map<uint32_t, DropItemMap> s_DropItemMapList
        {
            []() -> std::map<uint32_t, DropItemMap>
            {
                std::vector<DropItemConfig> stvDropItemConfig
                {
                    #include "dropitemconfig.inc"
                };

                std::map<uint32_t, DropItemMap> stmDropItemMap;
                for(auto &rstEntry: stvDropItemConfig){
                    if(rstEntry){
                        // later we won't never call this two ID-retrieve functions again
                        // here we use DBCOM_MONSTERRID() and DBCOM_ITEMID() with string variable
                        // this is very slow but OK since we here to initialize the static hash table for global usage
                        stmDropItemMap[DBCOM_MONSTERID(rstEntry.MonsterName)][rstEntry.Group].emplace_back(DBCOM_ITEMID(rstEntry.ItemName), rstEntry.ProbRecip, rstEntry.Value);
                    }else{
                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_WARNING, "Skip invalid DropItemConfig::0X%0*" PRIXPTR, (int)(2 * sizeof(&rstEntry)), (uintptr_t)(&rstEntry));
                        rstEntry.Print();
                    }
                }
                return stmDropItemMap;
            }()
        };

        if(s_DropItemMapList.find(nMonsterID) != s_DropItemMapList.end()){
            return s_DropItemMapList.at(nMonsterID);
        }
    }

    static const DropItemMap stEmptyDropItemMap {};
    return stEmptyDropItemMap;
}
