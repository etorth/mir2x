#include <climits>
#include "dbpod.hpp"
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "auctiondb.hpp"
#include "deliverydb.hpp"
#include "inventorydb.hpp"
#include "raiitimer.hpp"

extern DBPod *g_dbPod;

SDAuctionItemList dbQueryAuctionItemList(int category)
{
    fflassert(category >= AUCTIONCAT_BEGIN, category);
    fflassert(category <  AUCTIONCAT_END  , category);

    auto now = hres_tstamp::epoch();
    auto query = g_dbPod->createQuery(
        u8R"###( select                                                                                                 )###"
        u8R"###(     tbl_auctionitem.*,                                                                                 )###"
        u8R"###(     tbl_char.fld_dbid   as fld_sellerdbid,                                                             )###"
        u8R"###(     tbl_char.fld_name   as fld_sellername,                                                             )###"
        u8R"###(     tbl_char.fld_gender as fld_sellergender,                                                           )###"
        u8R"###(     tbl_char.fld_job    as fld_sellerjob                                                               )###"
        u8R"###( from                                                                                                   )###"
        u8R"###(     tbl_auctionitem join tbl_char                                                                      )###"
        u8R"###(     on                                                                                                 )###"
        u8R"###(         tbl_char.fld_dbid = tbl_auctionitem.fld_seller                                                 )###"
        u8R"###( where                                                                                                  )###"
        u8R"###(     tbl_auctionitem.fld_expiretime > %lld and checkAuctionCategory(tbl_auctionitem.fld_itemid, %d) = 1 )###"
        u8R"###( order by                                                                                               )###"
        u8R"###(     tbl_auctionitem.fld_id                                                                             )###", to_lld(now), category);

    SDAuctionItemList result
    {
        .category = category,
    };

    while(query.executeStep()){
        const auto price      = query.getColumn("fld_price"     ).getInt64();
        const auto expireTime = query.getColumn("fld_expiretime").getInt64();

        auto item = SDItem::fromQuery(query);

        const auto &ir = DBCOM_ITEMRECORD(item.itemID);
        fflassert(ir, item.itemID);

        result.itemList.push_back(SDAuctionItem
        {
            .auctionID = check_cast<uint64_t>(query.getColumn("fld_id").getInt64()),
            .seller
            {
                .id   = check_cast<uint32_t, unsigned>(query.getColumn("fld_sellerdbid")),
                .name =                                query.getColumn("fld_sellername").getString(),

                .despvar = SDChatPeerPlayerVar
                {
                    .gender = query.getColumn("fld_sellergender").getUInt() > 0,
                    .job    = query.getColumn("fld_sellerjob"   ),
                },
            },

            .note = query.getColumn("fld_note").getString(),

            .timeLeft = check_cast<size_t, uint64_t>(expireTime - now),
            .price    = check_cast<size_t, uint64_t>(price),

            .item = std::move(item),
        });
    }
    return result;
}

bool dbRegisterAuctionItem(uint32_t sellerDBID, const SDItem &item, const std::string &note, uint64_t price)
{
    fflassert(sellerDBID > 0, sellerDBID);
    fflassert(sellerDBID <= SYS_MAXDBID, sellerDBID);

    fflassert(item);
    fflassert(!item.isGold());

    fflassert(note.size() <= SYS_AUCTIONNOTESIZE, note);

    fflassert(price > 0, price);
    fflassert(price <= INT_MAX, price);

    auto transaction = g_dbPod->createTransaction();
    if(!dbRemoveInventoryItem(sellerDBID, item)){
        return false;
    }

    auto insertQuery = g_dbPod->createQuery(
            u8R"###( insert into tbl_auctionitem(fld_seller, fld_note, fld_itemid, fld_seqid, fld_count, fld_duration, fld_maxduration, fld_extattrlist, fld_price, fld_expiretime) )###"
            u8R"###( values                                                                                                                                                         )###"
            u8R"###(     (%llu, ?, %llu, %llu, %llu, %llu, %llu, ?, %llu, %lld)                                                                                                     )###",

            to_llu(sellerDBID),
            to_llu(item.itemID),
            to_llu(item.seqID),
            to_llu(item.count),
            to_llu(item.duration[0]),
            to_llu(item.duration[1]),
            to_llu(price),
            to_lld(hres_tstamp::epoch() + SYS_AUCTIONLIFETIME));

    insertQuery.bind(1, note);
    insertQuery.bindBlob(2, cerealf::serialize(item.extAttrList));

    insertQuery.exec();
    transaction.commit();

    return true;
}

std::expected<DBAuctionBuyResult, int> dbBuyAuctionItem(uint32_t buyerDBID, uint64_t auctionID)
{
    if(true
            && buyerDBID > 0
            && buyerDBID <= SYS_MAXDBID
            && auctionID > 0
            && auctionID <= to_u64(INT64_MAX)){

        auto transaction = g_dbPod->createTransaction();
        const auto now = hres_tstamp::epoch();

        auto deleteQuery = g_dbPod->createQuery(
            u8R"###( delete from tbl_auctionitem                                                                      )###"
            u8R"###( where                                                                                            )###"
            u8R"###(     fld_id = %llu and fld_expiretime > %llu                                                      )###"
            u8R"###( returning                                                                                        )###"
            u8R"###(     fld_seller, fld_itemid, fld_seqid, fld_count, fld_duration, fld_maxduration, fld_extattrlist, fld_price )###",

            to_llu(auctionID),
            to_llu(now));

        if(!deleteQuery.executeStep()){
            return std::unexpected(AUCTIONBUYERR_UNAVAILABLE);
        }

        const auto sellerDBID = check_cast<uint32_t, unsigned>(deleteQuery.getColumn("fld_seller"));
        if(sellerDBID == buyerDBID){
            return std::unexpected(AUCTIONBUYERR_OWNITEM);
        }

        auto item = SDItem::fromQuery(deleteQuery);
        item.seqID = 0;
        fflassert(!item.isGold());

        const auto price = check_cast<uint64_t>(deleteQuery.getColumn("fld_price").getInt64());
        fflassert(price > 0);

        fflassert(!deleteQuery.executeStep());

        auto goldQuery = g_dbPod->createQuery(
            u8R"###( update tbl_char                          )###"
            u8R"###( set                                      )###"
            u8R"###(     fld_gold = fld_gold - %llu           )###"
            u8R"###( where                                    )###"
            u8R"###(     fld_dbid = %llu and fld_gold >= %llu )###"
            u8R"###( returning fld_gold                       )###",

            to_llu(price),
            to_llu(buyerDBID),
            to_llu(price));

        if(!goldQuery.executeStep()){
            return std::unexpected(AUCTIONBUYERR_INSUFFICIENT);
        }

        const auto buyerGold = check_cast<size_t, uint64_t>(goldQuery.getColumn("fld_gold").getInt64());
        fflassert(!goldQuery.executeStep());

        auto buyerDelivery = dbCreateDeliveryInTransaction(buyerDBID, {item}, to_cstr(u8"你购买的寄售物品已送达："));

        const auto sellerGold = price - price * SYS_AUCTIONTAXRATE / 100;
        fflassert(sellerGold > 0);

        const auto &ir = DBCOM_ITEMRECORD(item.itemID);
        fflassert(ir);

        auto sellerDelivery = dbCreateDeliveryInTransaction(sellerDBID, SDItem::buildGoldItem(check_cast<size_t, uint64_t>(sellerGold)), to_cstr(str_printf(u8"你寄售的%s已售出，扣除%zu%%交易税后获得：", to_cstr(ir.name), SYS_AUCTIONTAXRATE)));
        transaction.commit();

        return DBAuctionBuyResult
        {
            .buyerGold  = buyerGold,
            .sellerDBID = sellerDBID,

            .buyerMessage  = std::move( buyerDelivery.message),
            .sellerMessage = std::move(sellerDelivery.message),
        };
    }
    return std::unexpected(AUCTIONBUYERR_BADITEM);
}

std::expected<DBAuctionUnregisterResult, int> dbUnregisterAuctionItem(uint32_t sellerDBID, uint64_t auctionID)
{
    if(true
            && sellerDBID > 0
            && sellerDBID <= SYS_MAXDBID
            && auctionID > 0
            && auctionID <= to_u64(INT64_MAX)){

        auto transaction = g_dbPod->createTransaction();
        auto deleteQuery = g_dbPod->createQuery(
            u8R"###( delete from tbl_auctionitem                                                     )###"
            u8R"###( where                                                                           )###"
            u8R"###(     fld_id = %llu and fld_seller = %llu                                         )###"
            u8R"###( returning fld_itemid, fld_seqid, fld_count, fld_duration, fld_maxduration, fld_extattrlist )###",

            to_llu(auctionID),
            to_llu(sellerDBID));

        if(!deleteQuery.executeStep()){
            return std::unexpected(AUCTIONUNREGERR_UNAVAILABLE);
        }

        auto item = SDItem::fromQuery(deleteQuery);
        item.seqID = 0;
        fflassert(!item.isGold());
        fflassert(!deleteQuery.executeStep());

        auto delivery = dbCreateDeliveryInTransaction(sellerDBID, {item}, to_cstr(u8"你下架的寄售物品已退回："));
        transaction.commit();

        return DBAuctionUnregisterResult
        {
            .sellerMessage = std::move(delivery.message),
        };
    }
    return std::unexpected(AUCTIONUNREGERR_BADITEM);
}
