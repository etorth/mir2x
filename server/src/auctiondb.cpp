#include <chrono>
#include <climits>
#include "dbpod.hpp"
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "auctiondb.hpp"
#include "raiitimer.hpp"

extern DBPod *g_dbPod;

namespace
{
    bool categoryMatch(const ItemRecord &ir, int category)
    {
        switch(category){
            case AUCTIONCAT_DRESS   : return ir.isDress();
            case AUCTIONCAT_WEAPON  : return ir.isWeapon();
            case AUCTIONCAT_NECKLACE: return ir.wearable(WLG_NECKLACE);
            case AUCTIONCAT_HELMET  : return ir.isHelmet();
            case AUCTIONCAT_RING    : return ir.isRing();
            case AUCTIONCAT_ARMRING : return ir.wearable(WLG_ARMRING0);
            case AUCTIONCAT_SHOES   : return ir.wearable(WLG_SHOES);
            case AUCTIONCAT_POTION  : return ir.isPotion() || ir.isDope() || ir.isPowder();
            case AUCTIONCAT_BOOK    : return ir.isBook();
            default                 : return false;
        }
    }

    bool categoryMatchIncludingOther(const ItemRecord &ir, int category)
    {
        if(category == AUCTIONCAT_ALL){
            return true;
        }

        if(category == AUCTIONCAT_OTHER){
            for(int knownCategory = AUCTIONCAT_DRESS; knownCategory < AUCTIONCAT_OTHER; ++knownCategory){
                if(categoryMatch(ir, knownCategory)){
                    return false;
                }
            }
            return true;
        }
        return categoryMatch(ir, category);
    }

}

SDAuctionItemList dbQueryAuctionItemList(int category)
{
    fflassert(category >= AUCTIONCAT_BEGIN, category);
    fflassert(category <  AUCTIONCAT_END  , category);

    auto now = hres_tstamp::epoch();
    auto query = g_dbPod->createQuery(
        u8R"###( select                                                 )###"
        u8R"###(     tbl_auctionitem.*,                                 )###"
        u8R"###(     tbl_char.fld_dbid   as fld_sellerdbid,             )###"
        u8R"###(     tbl_char.fld_name   as fld_sellername,             )###"
        u8R"###(     tbl_char.fld_gender as fld_sellergender,           )###"
        u8R"###(     tbl_char.fld_job    as fld_sellerjob               )###"
        u8R"###( from                                                   )###"
        u8R"###(     tbl_auctionitem join tbl_char                      )###"
        u8R"###(     on                                                 )###"
        u8R"###(         tbl_char.fld_dbid = tbl_auctionitem.fld_seller )###"
        u8R"###( where                                                  )###"
        u8R"###(     tbl_auctionitem.fld_expiretime > %lld              )###"
        u8R"###( order by                                               )###"
        u8R"###(     tbl_auctionitem.fld_id                             )###", to_lld(now));

    SDAuctionItemList result
    {
        .category = category,
    };

    while(query.executeStep()){
        const auto price      = query.getColumn("fld_price"     ).getInt64();
        const auto expireTime = query.getColumn("fld_expiretime").getInt64();

        SDItem item
        {
            .itemID = check_cast<uint32_t, unsigned>(query.getColumn("fld_itemid")),
            .seqID  = check_cast<uint32_t, unsigned>(query.getColumn("fld_seqid" )),
            .count  = check_cast<  size_t, unsigned>(query.getColumn("fld_count" )),
            .duration
            {
                check_cast<size_t, unsigned>(query.getColumn("fld_duration"   )),
                check_cast<size_t, unsigned>(query.getColumn("fld_maxduration")),
            },
            .extAttrList = cerealf::deserialize<std::unordered_map<int, std::string>>(query.getColumn("fld_extattrlist")),
        };

        fflassert(item);

        const auto &ir = DBCOM_ITEMRECORD(item.itemID);
        fflassert(ir, item.itemID);

        if(!categoryMatchIncludingOther(ir, category)){
            continue;
        }

        result.itemList.push_back(SDAuctionItem
        {
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
    {
        auto deleteQuery = g_dbPod->createQuery(
            u8R"###( delete from tbl_inventory                                      )###"
            u8R"###( where                                                          )###"
            u8R"###(     fld_dbid = %llu and fld_itemid = %llu and fld_seqid = %llu )###",

            to_llu(sellerDBID),
            to_llu(item.itemID),
            to_llu(item.seqID));

        if(!deleteQuery.executeStep()){
            return false;
        }

        // we can reconstruct SDItem by returned fields
        // and validate if item from database and item from argument matches, but not very necessary
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
