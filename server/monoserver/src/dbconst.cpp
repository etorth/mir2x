/*
 * =====================================================================================
 *
 *       Filename: dbconst.cpp
 *        Created: 05/12/2017 17:58:02
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

#include "dbcomid.hpp"
#include "dbconst.hpp"
#include "monoserver.hpp"
#include "protocoldef.hpp"
#include "monsterrecord.hpp"
#include "dropitemconfig.hpp"

extern MonoServer *g_monoServer;

const DCRecord &DB_DCRECORD(uint32_t nDC)
{
    const static std::map<uint32_t, DCRecord> s_DCRecord
    {
        {           0, {           0, 0, 0, u8"", u8""}},
        {DC_PHY_PLAIN, {DC_PHY_PLAIN, 0, 0, u8"", u8""}},
    };
    return s_DCRecord.at((s_DCRecord.find(nDC) != s_DCRecord.end()) ? nDC : 0);
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
                for(auto &entry: stvDropItemConfig){
                    if(entry){
                        // later we won't never call this two ID-retrieve functions again
                        // here we use DBCOM_MONSTERRID() and DBCOM_ITEMID() with string variable
                        // this is very slow but OK since we here to initialize the static hash table for global usage
                        auto &rstCurrList = stmDropItemMap[DBCOM_MONSTERID(entry.monsterName)][entry.group];
                        rstCurrList.insert(rstCurrList.end(), entry.repeat, {DBCOM_ITEMID(entry.itemName), entry.probRecip, entry.value});
                    }else{
                        g_monoServer->addLog(LOGTYPE_WARNING, "Skip invalid DropItemConfig::0X%0*" PRIXPTR, to_d(2 * sizeof(&entry)), (uintptr_t)(&entry));
                        entry.print();
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
