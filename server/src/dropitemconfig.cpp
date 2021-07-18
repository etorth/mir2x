/*
 * =====================================================================================
 *
 *       Filename: dropitemconfig.cpp
 *        Created: 07/30/2017 00:12:33
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

#include <cstdint>
#include <cstdlib>
#include <cinttypes>

#include "mathf.hpp"
#include "dbcomid.hpp"
#include "monoserver.hpp"
#include "dbcomrecord.hpp"
#include "dropitemconfig.hpp"
#include "serverconfigurewindow.hpp"

extern ServerConfigureWindow *g_serverConfigureWindow;

struct InnDropItemConfig final
{
    const char8_t * const monsterName = nullptr;
    const char8_t * const    itemName = nullptr;

    const int group;        // can only drop at most one item in the group when group is not zero
    const int probRecip;    // zero means disabled, 1 / p

    const int repeat;       // zero means disabled, how many times to try to drop this item
    const int count;        // zero means disabled, how many items to drop if tried succeefully, need to decompose if itemID is not packable

    operator bool() const
    {
        return true
            && DBCOM_MONSTERID(monsterName)
            && DBCOM_ITEMID(itemName)

            && group     >= 0
            && probRecip >= 1

            && repeat >= 1
            && count  >= 1;
    }
};

const std::map<int, std::vector<DropItemConfig>> &getMonsterDropItemConfigList(uint32_t monsterID)
{
    const static auto s_monsterDropitemConfigList = []()
    {
        const std::vector<InnDropItemConfig> dropItemConfigNodeList
        {
            #include "dropitemconfig.inc"
        };

        std::unordered_map<uint32_t, std::map<int, std::vector<DropItemConfig>>> monsterDropItemList;
        for(const auto &node: dropItemConfigNodeList){
            if(!node){
                continue;
            }

            const auto monsterID = DBCOM_MONSTERID(node.monsterName);
            if(!monsterID){
                continue;
            }

            const auto itemID = DBCOM_ITEMID(node.itemName);
            if(!itemID){
                continue;
            }

            for(int i = 0; i < node.repeat; ++i){
                monsterDropItemList[monsterID][node.group].push_back(DropItemConfig
                {
                    .itemID    = itemID,
                    .probRecip = node.probRecip,
                    .count     = node.count,
                });
            }
        }
        return monsterDropItemList;
    }();

    const static std::map<int, std::vector<DropItemConfig>> s_emptyConfigList;
    if(!DBCOM_MONSTERRECORD(monsterID)){
        return s_emptyConfigList;
    }

    if(const auto p = s_monsterDropitemConfigList.find(monsterID); p != s_monsterDropitemConfigList.end()){
        return p->second;
    }
    return s_emptyConfigList;
}

std::vector<SDItem> getMonsterDropItemList(uint32_t monsterID)
{
    std::vector<SDItem> itemList;
    const auto serverConfig = g_serverConfigureWindow->getConfig();

    for(const auto &[group, dropItemList]: getMonsterDropItemConfigList(monsterID)){
        for(const auto &dropItem: dropItemList){
            const auto adjProbRecip = [&dropItem, &serverConfig]() -> int
            {
                if(dropItem.probRecip <= 0){
                    return 0;
                }

                fflassert(serverConfig.dropRate >= 0.0);
                return std::max<int>(1, std::lround(dropItem.probRecip / serverConfig.dropRate));
            }();

            if((dropItem.probRecip > 0) && ((std::rand() % adjProbRecip) == 0)){
                const auto &ir = DBCOM_ITEMRECORD(dropItem.itemID);
                fflassert(ir);

                if(ir.isGold()){
                    fflassert(serverConfig.goldRate >= 0.0);
                    const auto adjGold = std::max<size_t>(1, std::lround(dropItem.count * serverConfig.goldRate));
                    for(const auto &goldItem: SDItem::buildGoldItem(adjGold)){
                        itemList.push_back(SDItem
                        {
                            .itemID = goldItem.itemID,
                            .seqID  = 1,
                            .count  = std::max<size_t>(1, std::lround(goldItem.count * mathf::rand<float>(0.9, 1.1))),
                        });
                    }
                }
                else if(ir.packable()){
                    itemList.push_back(SDItem
                    {
                        .itemID = dropItem.itemID,
                        .seqID  = 1,
                        .count  = to_uz(dropItem.count),
                    });
                }
                else{
                    for(int i = 0; i < dropItem.count; ++i){
                        itemList.push_back(SDItem
                        {
                            .itemID = dropItem.itemID,
                            .seqID  = 1,
                            .count  = 1,
                        });
                    }
                }

                if(group > 0){
                    break;  // can only drop one item if not in zero group
                }
            }
        }
    }
    return itemList;
}
