#include <climits>
#include <utility>
#include "dbpod.hpp"
#include "fflerror.hpp"
#include "protocoldef.hpp"
#include "sysconst.hpp"
#include "uidf.hpp"
#include "golddb.hpp"
#include "inventorydb.hpp"
#include "directtradedb.hpp"

extern DBPod *g_dbPod;
std::expected<std::array<SDDirectTradeResult, 2>, int> dbCommitDirectTrade(const SDDirectTradeOffer &offer0, const SDDirectTradeOffer &offer1)
{
    if(false
            || !uidf::isPlayer(offer0.uid)
            || !uidf::isPlayer(offer1.uid)

            || offer0.uid == offer1.uid

            || !offer0.locked
            || !offer1.locked

            || !offer0.confirmed
            || !offer1.confirmed){
        return std::unexpected(DTRADEERR_COMMITFAILED);
    }

    SDDirectTradeResult res0 {.uid = offer0.uid};
    SDDirectTradeResult res1 {.uid = offer1.uid};

    auto transaction = g_dbPod->createTransaction();
    {
        const auto goldOpt0 = dbLoadGold(uidf::getPlayerDBID(offer0.uid));
        const auto goldOpt1 = dbLoadGold(uidf::getPlayerDBID(offer1.uid));

        if(!goldOpt0.has_value() || !goldOpt1.has_value()){
            return std::unexpected(DTRADEERR_COMMITFAILED);
        }

        const auto gold0 = goldOpt0.value();
        const auto gold1 = goldOpt1.value();

        if(gold0 < offer0.gold || gold1 < offer1.gold){
            return std::unexpected(DTRADEERR_COMMITFAILED);
        }

        const uint64_t finalGold0 = gold0 - offer0.gold + offer1.gold;
        const uint64_t finalGold1 = gold1 - offer1.gold + offer0.gold;

        if(finalGold0 > to_uz(INT_MAX) || finalGold1 > to_uz(INT_MAX)){
            return std::unexpected(DTRADEERR_COMMITFAILED);
        }

        dbUpdateGold(uidf::getPlayerDBID(offer0.uid), finalGold0);
        dbUpdateGold(uidf::getPlayerDBID(offer1.uid), finalGold1);

        res0.gold = finalGold0;
        res1.gold = finalGold1;
    }

    const auto fnRemoveItemList = [](SDInventory &inventory, const std::vector<SDItem> &itemList)
    {
        for(const auto &item: itemList){
            fflassert(item);
            fflassert(!item.isGold());

            if(const auto [deletedCount, _, _] = inventory.remove(item.itemID, item.seqID, item.count, true); deletedCount <= 0){
                return false;
            }

        }
        return true;
    };

    {
        auto inv0 = dbLoadInventory(uidf::getPlayerDBID(offer0.uid));
        auto inv1 = dbLoadInventory(uidf::getPlayerDBID(offer1.uid));

        if(!fnRemoveItemList(inv0, offer0.itemList)) return std::unexpected(DTRADEERR_COMMITFAILED);
        if(!fnRemoveItemList(inv1, offer1.itemList)) return std::unexpected(DTRADEERR_COMMITFAILED);

        for(const auto &item: offer0.itemList) inv1.add(item, false);
        for(const auto &item: offer1.itemList) inv0.add(item, false);

        dbStoreInventory(uidf::getPlayerDBID(offer0.uid), inv0);
        dbStoreInventory(uidf::getPlayerDBID(offer1.uid), inv1);

        res0.inventory = std::move(inv0);
        res1.inventory = std::move(inv1);
    }

    transaction.commit();

    return std::array<SDDirectTradeResult, 2>
    {
        std::move(res0),
        std::move(res1),
    };
}
