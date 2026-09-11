#include <climits>
#include "dbpod.hpp"
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "golddb.hpp"
#include "inventorydb.hpp"

extern DBPod *g_dbPod;
SDInventory dbLoadInventory(uint32_t dbid)
{
    fflassert(dbid > 0);
    fflassert(dbid <= SYS_MAXDBID, dbid);

    SDInventory inventory;
    auto query = g_dbPod->createQuery("select * from tbl_inventory where fld_dbid = %llu", to_llu(dbid));
    while(query.executeStep()){
        inventory.add(SDItem::fromQuery(query), true);
    }
    return inventory;
}

void dbStoreInventory(uint32_t dbid, const SDInventory &inventory)
{
    fflassert(dbid > 0);
    fflassert(dbid <= SYS_MAXDBID, dbid);

    g_dbPod->exec("delete from tbl_inventory where fld_dbid = %llu", to_llu(dbid));
    for(const auto &item: inventory.getItemList()){
        dbUpdateInventoryItem(dbid, item);
    }
}

void dbUpdateInventoryItem(uint32_t dbid, const SDItem &item)
{
    fflassert(dbid > 0);
    fflassert(dbid <= SYS_MAXDBID, dbid);
    fflassert(item);

    auto query = g_dbPod->createQuery(
            u8R"###( replace into tbl_inventory(fld_dbid, fld_itemid, fld_seqid, fld_count, fld_duration, fld_maxduration, fld_extattrlist) )###"
            u8R"###( values                                                                                                                 )###"
            u8R"###(     (%llu, %llu, %llu, %llu, %llu, %llu, ?)                                                                            )###",

            to_llu(dbid),
            to_llu(item.itemID),
            to_llu(item.seqID),
            to_llu(item.count),
            to_llu(item.duration[0]),
            to_llu(item.duration[1]));

    query.bindBlob(1, cerealf::serialize(item.extAttrList));
    query.exec();
}

bool dbRemoveInventoryItem(uint32_t dbid, const SDItem &item)
{
    fflassert(item);
    return dbRemoveInventoryItem(dbid, item.itemID, item.seqID);
}

bool dbRemoveInventoryItem(uint32_t dbid, uint32_t itemID, uint32_t seqID)
{
    fflassert(dbid > 0);
    fflassert(dbid <= SYS_MAXDBID, dbid);

    if(!(DBCOM_ITEMRECORD(itemID) && seqID > 0)){
        throw fflpanic("invalid arguments: itemID {}, seqID {}", itemID, seqID);
    }

    auto query = g_dbPod->createQuery(
            u8R"###( delete from tbl_inventory                                      )###"
            u8R"###( where                                                          )###"
            u8R"###(     fld_dbid = %llu and fld_itemid = %llu and fld_seqid = %llu )###"
            u8R"###( returning                                                      )###"
            u8R"###(     fld_itemid, fld_seqid                                      )###",

            to_llu(dbid),
            to_llu(itemID),
            to_llu(seqID));

    if(!query.executeStep()){
        return false;
    }

    const auto deletedItemID = check_cast<uint32_t, unsigned>(query.getColumn("fld_itemid"));
    const auto deletedSeqID  = check_cast<uint32_t, unsigned>(query.getColumn("fld_seqid" ));

    fflassert(deletedItemID == itemID, deletedItemID, itemID);
    fflassert(deletedSeqID  ==  seqID, deletedSeqID ,  seqID);

    fflassert(!query.executeStep());
    return true;
}

std::optional<SDItemGrant> dbGrantItemList(uint32_t dbid, const std::vector<SDItem> &itemList)
{
    fflassert(!itemList.empty());
    fflassert(std::ranges::all_of(itemList, [](const auto &item){ return item && item.seqID == 0; }));

    // stage the inventory separately
    // a rejected or failed batch changes neither player memory nor persisted items
    auto transaction = g_dbPod->createTransaction();
    const auto oldGold = dbLoadGold(dbid);
    if(!oldGold.has_value()){
        return std::nullopt;
    }

    SDItemGrant result
    {
        .gold = oldGold.value(),
        .inventory = dbLoadInventory(dbid),
    };

    for(const auto &item: itemList){
        if(item.isGold()){
            if(to_u64(item.count) > to_u64(INT64_MAX) - to_u64(result.gold)){
                return std::nullopt;
            }
            result.gold += item.count;
        }
        else{
            dbUpdateInventoryItem(dbid, result.inventory.add(item, false));
        }
    }

    if(result.gold != oldGold.value()){
        dbUpdateGold(dbid, result.gold);
    }

    transaction.commit();
    return result;
}
