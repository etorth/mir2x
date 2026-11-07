#include <climits>
#include <cstdlib>
#include "dbpod.hpp"
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "protocoldef.hpp"
#include "golddb.hpp"
#include "stalldb.hpp"
#include "deliverydb.hpp"
#include "inventorydb.hpp"

extern DBPod *g_dbPod;

static bool validDBID(uint32_t dbid)
{
    return dbid > 0 && dbid <= SYS_MAXDBID;
}

static std::vector<SDStallItem> dbLoadStallItemList(uint32_t dbid)
{
    std::vector<SDStallItem> itemList;
    auto query = g_dbPod->createQuery("select * from tbl_stallitem where fld_dbid = %llu order by rowid", to_llu(dbid));

    while(query.executeStep()){
        itemList.push_back(SDStallItem
        {
            .item = SDItem::fromQuery(query),
            .price = check_cast<size_t, int64_t>(query.getColumn("fld_price").getInt64()),
        });
    }
    return itemList;
}

std::optional<SDStall> dbLoadStall(uint32_t dbid)
{
    fflassert(validDBID(dbid), dbid);
    auto query = g_dbPod->createQuery("select fld_name, fld_selling from tbl_stall where fld_dbid = %llu", to_llu(dbid));

    if(!query.executeStep()){
        return std::nullopt;
    }

    return SDStall
    {
        .dbid = dbid,
        .selling = query.getColumn("fld_selling").getInt() != 0,
        .name = query.getColumn("fld_name").getString(),
        .itemList = dbLoadStallItemList(dbid),
    };
}

std::vector<SDOfflineStall> dbLoadOfflineStallList(uint32_t mapID)
{
    std::vector<SDOfflineStall> stallList;
    auto query = g_dbPod->createQuery("select fld_offline from tbl_stall where fld_map = %llu and fld_selling = 1 and fld_offline is not null", to_llu(mapID));

    while(query.executeStep()){
        stallList.push_back(cerealf::deserialize<SDOfflineStall>(query.getColumn("fld_offline").getString()));
    }
    return stallList;
}

std::expected<SDStallItem, int> dbAddStallItem(uint32_t dbid, const SDItem &item, size_t price)
{
    fflassert(validDBID(dbid), dbid);
    if(!item || item.isGold() || item.seqID == 0){
        return std::unexpected(STALLERR_BADITEM);
    }

    if(price == 0 || price > to_uz(INT_MAX)){
        return std::unexpected(STALLERR_BADPRICE);
    }

    auto transaction = g_dbPod->createTransaction();
    g_dbPod->exec("insert or ignore into tbl_stall(fld_dbid) values (%llu)", to_llu(dbid));

    const auto stall = dbLoadStall(dbid);
    fflassert(stall.has_value());

    if(stall->selling){
        return std::unexpected(STALLERR_SELLING);
    }

    if(stall->itemList.size() >= SYS_STALLMAXITEM){
        return std::unexpected(STALLERR_FULL);
    }

    if(!dbRemoveInventoryItem(dbid, item)){
        return std::unexpected(STALLERR_BADITEM);
    }

    SDStallItem stallItem
    {
        .item = item,
        .price = price,
    };

    stallItem.item.seqID = stall->allocSeqID(item.itemID);
    auto insertQuery = g_dbPod->createQuery(
            u8R"###( insert into tbl_stallitem(fld_dbid, fld_itemid, fld_seqid, fld_count, fld_duration, fld_maxduration, fld_extattrlist, fld_price) )###"
            u8R"###( values                                                                                                                         )###"
            u8R"###(     (%llu, %llu, %llu, %llu, %llu, %llu, ?, %llu)                                                                              )###",

            to_llu(dbid),
            to_llu(stallItem.item.itemID),
            to_llu(stallItem.item.seqID),
            to_llu(stallItem.item.count),
            to_llu(stallItem.item.duration[0]),
            to_llu(stallItem.item.duration[1]),
            to_llu(stallItem.price));

    insertQuery.bindBlob(1, cerealf::serialize(stallItem.item.extAttrList));
    insertQuery.exec();

    transaction.commit();
    return stallItem;
}

std::expected<SDItem, int> dbRemoveStallItem(uint32_t dbid, SDInventory &inventory, uint32_t itemID, uint32_t seqID)
{
    fflassert(validDBID(dbid), dbid);
    if(!(DBCOM_ITEMRECORD(itemID) && seqID > 0)){
        return std::unexpected(STALLERR_BADITEM);
    }

    auto transaction = g_dbPod->createTransaction();
    auto deleteQuery = g_dbPod->createQuery(
            u8R"###( delete from tbl_stallitem                                                                   )###"
            u8R"###( where                                                                                       )###"
            u8R"###(     fld_dbid = %llu and fld_itemid = %llu and fld_seqid = %llu                              )###"
            u8R"###( returning                                                                                   )###"
            u8R"###(     fld_itemid, fld_seqid, fld_count, fld_duration, fld_maxduration, fld_extattrlist        )###",

            to_llu(dbid),
            to_llu(itemID),
            to_llu(seqID));

    if(!deleteQuery.executeStep()){
        return std::unexpected(STALLERR_BADITEM);
    }

    auto item = SDItem::fromQuery(deleteQuery);
    fflassert(!deleteQuery.executeStep());

    item.seqID = 0;
    const auto addedItem = inventory.add(item, false);

    dbUpdateInventoryItem(dbid, addedItem);
    transaction.commit();

    return addedItem;
}

bool dbStartStall(uint32_t dbid, const std::string &name, uint32_t mapID, int x, int y, const SDOfflineStall &offlineStall)
{
    fflassert(validDBID(dbid), dbid);
    fflassert(!name.empty());
    fflassert(name.size() < SYS_STALLNAMESIZE, name);

    auto query = g_dbPod->createQuery(
            u8R"###( update tbl_stall                                                                             )###"
            u8R"###( set                                                                                          )###"
            u8R"###(     fld_name = ?, fld_selling = 1, fld_map = %llu, fld_mapx = %d, fld_mapy = %d, fld_offline = ? )###"
            u8R"###( where                                                                                        )###"
            u8R"###(     fld_dbid = %llu and fld_selling = 0 and exists                                              )###"
            u8R"###(     (                                                                                        )###"
            u8R"###(         select 1 from tbl_stallitem where tbl_stallitem.fld_dbid = %llu                      )###"
            u8R"###(     )                                                                                        )###",

            to_llu(mapID),
            x,
            y,
            to_llu(dbid),
            to_llu(dbid));

    query.bind(1, name);
    query.bindBlob(2, cerealf::serialize(offlineStall));
    return query.exec() > 0;
}

std::vector<SDItem> dbCloseStall(uint32_t dbid, SDInventory &inventory)
{
    fflassert(validDBID(dbid), dbid);

    std::vector<SDItem> updatedItemList;
    auto transaction = g_dbPod->createTransaction();

    for(auto stallItem: dbLoadStallItemList(dbid)){
        stallItem.item.seqID = 0;
        updatedItemList.push_back(inventory.add(stallItem.item, false));
        dbUpdateInventoryItem(dbid, updatedItemList.back());
    }

    g_dbPod->exec("delete from tbl_stall where fld_dbid = %llu", to_llu(dbid));
    transaction.commit();

    return updatedItemList;
}

std::expected<DBStallBuyResult, int> dbBuyStallItem(uint32_t buyerDBID, SDInventory &buyerInventory, uint32_t sellerDBID, uint32_t mapID, int buyerX, int buyerY, uint32_t itemID, uint32_t seqID, size_t count, size_t price)
{
    if(!(validDBID(buyerDBID) && validDBID(sellerDBID))){
        return std::unexpected(STALLERR_NOTALLOWED);
    }

    if(buyerDBID == sellerDBID){
        return std::unexpected(STALLERR_SELFBUY);
    }

    if(!(DBCOM_ITEMRECORD(itemID) && seqID > 0)){
        return std::unexpected(STALLERR_BADITEM);
    }

    if(count == 0){
        return std::unexpected(STALLERR_BADCOUNT);
    }

    auto transaction = g_dbPod->createTransaction();
    {
        auto query = g_dbPod->createQuery("select fld_map, fld_mapx, fld_mapy from tbl_stall where fld_dbid = %llu and fld_selling = 1", to_llu(sellerDBID));
        if(!query.executeStep()){
            return std::unexpected(STALLERR_NOTSELLING);
        }

        if(check_cast<uint32_t, int64_t>(query.getColumn("fld_map").getInt64()) != mapID){
            return std::unexpected(STALLERR_NOTALLOWED);
        }

        const auto stallX = query.getColumn("fld_mapx").getInt64();
        const auto stallY = query.getColumn("fld_mapy").getInt64();

        if(std::abs(stallX - buyerX) > SYS_MAX_VIEWR || std::abs(stallY - buyerY) > SYS_MAX_VIEWR){
            return std::unexpected(STALLERR_NOTALLOWED);
        }
    }

    auto itemQuery = g_dbPod->createQuery("select * from tbl_stallitem where fld_dbid = %llu and fld_itemid = %llu and fld_seqid = %llu", to_llu(sellerDBID), to_llu(itemID), to_llu(seqID));
    if(!itemQuery.executeStep()){
        return std::unexpected(STALLERR_STALE);
    }

    auto item = SDItem::fromQuery(itemQuery);
    const auto itemPrice = check_cast<size_t, int64_t>(itemQuery.getColumn("fld_price").getInt64());

    // client price is only for staleness check
    // seller can't change price while stall is selling, check anyway
    if(itemPrice != price){
        return std::unexpected(STALLERR_STALE);
    }

    if(count > item.count){
        return std::unexpected(STALLERR_BADCOUNT);
    }

    if(itemPrice > to_uz(INT64_MAX) / count){
        return std::unexpected(STALLERR_BADCOUNT);
    }

    const auto totalPrice = itemPrice * count;
    const auto buyerGold = dbRemoveGold(buyerDBID, totalPrice);

    if(!buyerGold.has_value()){
        return std::unexpected(STALLERR_NOGOLD);
    }

    if(count == item.count){
        g_dbPod->exec("delete from tbl_stallitem where fld_dbid = %llu and fld_itemid = %llu and fld_seqid = %llu", to_llu(sellerDBID), to_llu(itemID), to_llu(seqID));
    }
    else{
        g_dbPod->exec("update tbl_stallitem set fld_count = %llu where fld_dbid = %llu and fld_itemid = %llu and fld_seqid = %llu", to_llu(item.count - count), to_llu(sellerDBID), to_llu(itemID), to_llu(seqID));
    }

    item.seqID = 0;
    item.count = count;

    DBStallBuyResult result
    {
        .buyerGold = buyerGold.value(),
        .buyerItem = buyerInventory.add(item, false),
    };

    dbUpdateInventoryItem(buyerDBID, result.buyerItem);

    const auto &ir = DBCOM_ITEMRECORD(itemID);
    fflassert(ir);

    result.sellerMessage = dbCreateDeliveryInTransaction(sellerDBID, SDItem::buildGoldItem(totalPrice), to_cstr(str_printf(u8"你摆摊出售的%s（%zu个）已售出，获得：", to_cstr(ir.name), count))).message;

    const auto leftCount = [sellerDBID]
    {
        auto countQuery = g_dbPod->createQuery("select count(*) from tbl_stallitem where fld_dbid = %llu", to_llu(sellerDBID));
        if(!countQuery.executeStep()){
            throw fflpanic("failed to count stall items");
        }
        return countQuery.getColumn(0).getInt64();
    }();

    if(leftCount == 0){
        g_dbPod->exec("delete from tbl_stall where fld_dbid = %llu", to_llu(sellerDBID));
        result.empty = true;
    }

    transaction.commit();
    return result;
}
