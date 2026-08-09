#include <climits>
#include <utility>
#include "dbpod.hpp"
#include "fflerror.hpp"
#include "protocoldef.hpp"
#include "sysconst.hpp"
#include "uidf.hpp"
#include "inventorydb.hpp"
#include "directtradedb.hpp"

extern DBPod *g_dbPod;

namespace
{
    struct DirectTradeParticipant final
    {
        uint64_t uid = 0;
        size_t gold = 0;
        SDInventory inventory;
    };

    bool sameItemState(const SDItem &inventoryItem, const SDItem &offeredItem)
    {
        return inventoryItem.itemID == offeredItem.itemID
            && inventoryItem.seqID == offeredItem.seqID
            && inventoryItem.count >= offeredItem.count
            && inventoryItem.duration[0] == offeredItem.duration[0]
            && inventoryItem.duration[1] == offeredItem.duration[1]
            && inventoryItem.extAttrList == offeredItem.extAttrList;
    }

    std::optional<size_t> loadGold(uint64_t uid)
    {
        const auto dbid = uidf::getPlayerDBID(uid);
        auto query = g_dbPod->createQuery("select fld_gold from tbl_char where fld_dbid = %llu", to_llu(dbid));
        if(!query.executeStep()){
            return {};
        }

        const auto gold = query.getColumn("fld_gold").getInt64();
        if(gold < 0){
            return {};
        }

        fflassert(!query.executeStep());
        return check_cast<size_t, uint64_t>(to_u64(gold));
    }

    bool removeOffer(DirectTradeParticipant &participant, const SDDirectTradeOffer &offer)
    {
        if(offer.gold > participant.gold){
            return false;
        }

        for(const auto &offeredItem: offer.itemList){
            const auto &ir = DBCOM_ITEMRECORD(offeredItem.itemID);
            if(!ir
                    || ir.isGold()
                    || offeredItem.seqID == 0
                    || offeredItem.count == 0){
                return false;
            }

            const auto &inventoryItem = participant.inventory.find(offeredItem.itemID, offeredItem.seqID);
            if(!inventoryItem || !sameItemState(inventoryItem, offeredItem)){
                return false;
            }

            const auto removeResult = participant.inventory.remove(
                    offeredItem.itemID,
                    offeredItem.seqID,
                    offeredItem.count);
            if(std::get<0>(removeResult) != offeredItem.count
                    || std::get<1>(removeResult) != offeredItem.seqID){
                return false;
            }
        }
        return true;
    }

    void addOffer(DirectTradeParticipant &participant, const SDDirectTradeOffer &offer)
    {
        for(const auto &item: offer.itemList){
            participant.inventory.add(item, false);
        }
    }

}

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

    DirectTradeParticipant participant0
    {
        .uid = offer0.uid,
    };

    DirectTradeParticipant participant1
    {
        .uid = offer1.uid,
    };

    auto transaction = g_dbPod->createTransaction();

    const auto gold0 = loadGold(participant0.uid);
    const auto gold1 = loadGold(participant1.uid);

    if(!gold0 || !gold1){
        return std::unexpected(DTRADEERR_COMMITFAILED);
    }

    participant0.gold = gold0.value();
    participant1.gold = gold1.value();

    participant0.inventory = dbLoadInventory(uidf::getPlayerDBID(participant0.uid));
    participant1.inventory = dbLoadInventory(uidf::getPlayerDBID(participant1.uid));

    if(!removeOffer(participant0, offer0) || !removeOffer(participant1, offer1)){
        return std::unexpected(DTRADEERR_COMMITFAILED);
    }

    const uint64_t finalGold0 = to_u64(participant0.gold) - offer0.gold + offer1.gold;
    const uint64_t finalGold1 = to_u64(participant1.gold) - offer1.gold + offer0.gold;

    if(finalGold0 > to_u64(INT64_MAX) || finalGold1 > to_u64(INT64_MAX)){
        return std::unexpected(DTRADEERR_COMMITFAILED);
    }

    participant0.gold = check_cast<size_t, uint64_t>(finalGold0);
    participant1.gold = check_cast<size_t, uint64_t>(finalGold1);

    addOffer(participant0, offer1);
    addOffer(participant1, offer0);

    dbStoreInventory(uidf::getPlayerDBID(participant0.uid), participant0.inventory);
    dbStoreInventory(uidf::getPlayerDBID(participant1.uid), participant1.inventory);

    g_dbPod->exec(
            "update tbl_char set fld_gold = %llu where fld_dbid = %llu",
            to_llu(participant0.gold),
            to_llu(uidf::getPlayerDBID(participant0.uid)));

    g_dbPod->exec(
            "update tbl_char set fld_gold = %llu where fld_dbid = %llu",
            to_llu(participant1.gold),
            to_llu(uidf::getPlayerDBID(participant1.uid)));

    transaction.commit();

    return std::array<SDDirectTradeResult, 2>
    {
        SDDirectTradeResult
        {
            .uid = participant0.uid,
            .gold = participant0.gold,
            .inventory = std::move(participant0.inventory),
        },

        SDDirectTradeResult
        {
            .uid = participant1.uid,
            .gold = participant1.gold,
            .inventory = std::move(participant1.inventory),
        },
    };
}
