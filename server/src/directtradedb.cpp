#include <climits>
#include <utility>
#include "dbpod.hpp"
#include "fflerror.hpp"
#include "protocoldef.hpp"
#include "sysconst.hpp"
#include "uidf.hpp"
#include "directtradedb.hpp"

extern DBPod *g_dbPod;

namespace
{
    struct DirectTradeParticipant final
    {
        uint64_t uid = 0;
        uint32_t dbid = 0;
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

    std::optional<size_t> loadGold(uint32_t dbid)
    {
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

    SDInventory loadInventory(uint32_t dbid)
    {
        SDInventory inventory;
        auto query = g_dbPod->createQuery("select * from tbl_inventory where fld_dbid = %llu", to_llu(dbid));
        while(query.executeStep()){
            inventory.add(SDItem
            {
                .itemID = check_cast<uint32_t, unsigned>(query.getColumn("fld_itemid")),
                .seqID = check_cast<uint32_t, unsigned>(query.getColumn("fld_seqid")),
                .count = check_cast<size_t, unsigned>(query.getColumn("fld_count")),
                .duration
                {
                    check_cast<size_t, unsigned>(query.getColumn("fld_duration")),
                    check_cast<size_t, unsigned>(query.getColumn("fld_maxduration")),
                },
                .extAttrList = cerealf::deserialize<std::unordered_map<int, std::string>>(query.getColumn("fld_extattrlist")),
            }, true);
        }
        return inventory;
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

    void storeInventory(const DirectTradeParticipant &participant)
    {
        g_dbPod->exec("delete from tbl_inventory where fld_dbid = %llu", to_llu(participant.dbid));

        for(const auto &item: participant.inventory.getItemList()){
            auto query = g_dbPod->createQuery(
                    u8R"###( insert into tbl_inventory(fld_dbid, fld_itemid, fld_seqid, fld_count, fld_duration, fld_maxduration, fld_extattrlist) )###"
                    u8R"###( values                                                                                                                )###"
                    u8R"###(     (%llu, %llu, %llu, %llu, %llu, %llu, ?)                                                                           )###",

                    to_llu(participant.dbid),
                    to_llu(item.itemID),
                    to_llu(item.seqID),
                    to_llu(item.count),
                    to_llu(item.duration[0]),
                    to_llu(item.duration[1]));

            query.bindBlob(1, cerealf::serialize(item.extAttrList));
            query.exec();
        }
    }
}

std::expected<std::array<SDDirectTradeResult, 2>, int> dbCommitDirectTrade(const SDDirectTradeCommit &commit)
{
    const auto &offer0 = commit.offerList.at(0);
    const auto &offer1 = commit.offerList.at(1);
    if(!uidf::isPlayer(offer0.uid)
            || !uidf::isPlayer(offer1.uid)
            || offer0.uid == offer1.uid
            || !to_bool(offer0.locked)
            || !to_bool(offer1.locked)
            || !to_bool(offer0.confirmed)
            || !to_bool(offer1.confirmed)
            || offer0.itemList.size() > SYS_DIRECTTRADEMAXITEM
            || offer1.itemList.size() > SYS_DIRECTTRADEMAXITEM){
        return std::unexpected(DTRADEERR_COMMITFAILED);
    }

    DirectTradeParticipant participant0
    {
        .uid = offer0.uid,
        .dbid = uidf::getPlayerDBID(offer0.uid),
    };
    DirectTradeParticipant participant1
    {
        .uid = offer1.uid,
        .dbid = uidf::getPlayerDBID(offer1.uid),
    };

    // Validation and both rewrites share one transaction. Early error returns
    // rely on transaction rollback, so gold and items move together or not at all.
    auto transaction = g_dbPod->createTransaction();
    const auto gold0 = loadGold(participant0.dbid);
    const auto gold1 = loadGold(participant1.dbid);
    if(!gold0 || !gold1){
        return std::unexpected(DTRADEERR_COMMITFAILED);
    }

    participant0.gold = gold0.value();
    participant1.gold = gold1.value();
    participant0.inventory = loadInventory(participant0.dbid);
    participant1.inventory = loadInventory(participant1.dbid);

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

    storeInventory(participant0);
    storeInventory(participant1);
    g_dbPod->exec("update tbl_char set fld_gold = %llu where fld_dbid = %llu", to_llu(participant0.gold), to_llu(participant0.dbid));
    g_dbPod->exec("update tbl_char set fld_gold = %llu where fld_dbid = %llu", to_llu(participant1.gold), to_llu(participant1.dbid));
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
