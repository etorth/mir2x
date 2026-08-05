#include <chrono>
#include <limits>
#include "dbpod.hpp"
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "acutiondb.hpp"

extern DBPod *g_dbPod;

namespace
{
    bool categoryMatch(const ItemRecord &ir, int category)
    {
        switch(category){
            case ACUTIONCAT_DRESS   : return ir.isDress();
            case ACUTIONCAT_WEAPON  : return ir.isWeapon();
            case ACUTIONCAT_NECKLACE: return ir.wearable(WLG_NECKLACE);
            case ACUTIONCAT_HELMET  : return ir.isHelmet();
            case ACUTIONCAT_RING    : return ir.isRing();
            case ACUTIONCAT_ARMRING : return ir.wearable(WLG_ARMRING0);
            case ACUTIONCAT_SHOES   : return ir.wearable(WLG_SHOES);
            case ACUTIONCAT_POTION  : return ir.isPotion() || ir.isDope() || ir.isPowder();
            case ACUTIONCAT_BOOK    : return ir.isBook();
            default                 : return false;
        }
    }

    bool categoryMatchIncludingOther(const ItemRecord &ir, int category)
    {
        if(category == ACUTIONCAT_ALL){
            return true;
        }

        if(category == ACUTIONCAT_OTHER){
            for(int knownCategory = ACUTIONCAT_DRESS; knownCategory < ACUTIONCAT_OTHER; ++knownCategory){
                if(categoryMatch(ir, knownCategory)){
                    return false;
                }
            }
            return true;
        }
        return categoryMatch(ir, category);
    }

    int64_t epochSeconds()
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
    }

    constexpr int64_t ACUTION_LISTING_LIFETIME = 7 * 24 * 60 * 60;
}

SDAcutionItemList dbQueryAcutionItemList(int category)
{
    fflassert(category >= ACUTIONCAT_BEGIN, category);
    fflassert(category <  ACUTIONCAT_END  , category);

    const auto now = epochSeconds();
    auto query = g_dbPod->createQuery(
        u8R"###( select                                                 )###"
        u8R"###(     tbl_acutionitem.*,                                 )###"
        u8R"###(     tbl_char.fld_dbid   as fld_sellerdbid,             )###"
        u8R"###(     tbl_char.fld_name   as fld_sellername,             )###"
        u8R"###(     tbl_char.fld_gender as fld_sellergender,           )###"
        u8R"###(     tbl_char.fld_job    as fld_sellerjob               )###"
        u8R"###( from                                                   )###"
        u8R"###(     tbl_acutionitem join tbl_char                      )###"
        u8R"###(     on                                                 )###"
        u8R"###(         tbl_char.fld_dbid = tbl_acutionitem.fld_seller )###"
        u8R"###( where                                                  )###"
        u8R"###(     tbl_acutionitem.fld_expiretime > %lld              )###"
        u8R"###( order by                                               )###"
        u8R"###(     tbl_acutionitem.fld_id                             )###", to_lld(now));

    SDAcutionItemList result
    {
        .category = category,
    };

    while(query.executeStep()){
        const auto expireTime = query.getColumn("fld_expiretime").getInt64();
        const auto price      = query.getColumn("fld_price"     ).getInt64();

        fflassert(expireTime > now, expireTime, now);
        fflassert(price > 0, price);

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

        result.itemList.push_back(SDAcutionItem
        {
            .seller
            {
                .id   = check_cast<uint32_t, unsigned>(query.getColumn("fld_sellerdbid")),
                .name =                                query.getColumn("fld_sellername").getString(),

                .despvar = SDChatPeerPlayerVar
                {
                    .gender = query.getColumn("fld_sellergender").getUInt() > 0,
                    .job    = query.getColumn("fld_sellerjob"),
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

bool dbRegisterAcutionItem(uint32_t sellerDBID, const SDItem &item, const std::string &note, uint64_t price)
{
    fflassert(sellerDBID > 0 && sellerDBID <= SYS_MAXDBID, sellerDBID);
    fflassert(item && !item.isGold());
    fflassert(note.size() <= SYS_ACUTIONNOTESIZE, note.size());
    fflassert(price > 0 && price <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()), price);

    auto transaction = g_dbPod->createTransaction();
    {
        auto deleteQuery = g_dbPod->createQuery(
            u8R"###(
                delete from tbl_inventory
                where fld_dbid = %llu and fld_itemid = %llu and fld_seqid = %llu
                returning fld_itemid, fld_seqid, fld_count, fld_duration, fld_maxduration, fld_extattrlist
            )###",

            to_llu(sellerDBID),
            to_llu(item.itemID),
            to_llu(item.seqID));

        if(!deleteQuery.executeStep()){
            return false;
        }

        const SDItem dbItem
        {
            .itemID = check_cast<uint32_t, unsigned>(deleteQuery.getColumn("fld_itemid")),
            .seqID  = check_cast<uint32_t, unsigned>(deleteQuery.getColumn("fld_seqid" )),
            .count  = check_cast<  size_t, unsigned>(deleteQuery.getColumn("fld_count" )),
            .duration
            {
                check_cast<size_t, unsigned>(deleteQuery.getColumn("fld_duration"   )),
                check_cast<size_t, unsigned>(deleteQuery.getColumn("fld_maxduration")),
            },
            .extAttrList = cerealf::deserialize<std::unordered_map<int, std::string>>(deleteQuery.getColumn("fld_extattrlist")),
        };

        if(true
                && dbItem.itemID       == item.itemID
                && dbItem.seqID        == item.seqID
                && dbItem.count        == item.count
                && dbItem.duration[0]  == item.duration[0]
                && dbItem.duration[1]  == item.duration[1]
                && dbItem.extAttrList  == item.extAttrList){
            fflassert(!deleteQuery.executeStep());
        }
        else{
            throw fflpanic("inventory item differs from its database row: dbid = {}, itemid = {}, seqid = {}", sellerDBID, item.itemID, item.seqID);
        }
    }

    const auto expireTime = epochSeconds() + ACUTION_LISTING_LIFETIME;
    auto insertQuery = g_dbPod->createQuery(
        u8R"###(
            insert into tbl_acutionitem(
                fld_seller,
                fld_note,
                fld_itemid,
                fld_seqid,
                fld_count,
                fld_duration,
                fld_maxduration,
                fld_extattrlist,
                fld_price,
                fld_expiretime
            )
            values(%llu, ?, %llu, %llu, %llu, %llu, %llu, ?, %llu, %lld)
        )###",

        to_llu(sellerDBID),
        to_llu(item.itemID),
        to_llu(item.seqID),
        to_llu(item.count),
        to_llu(item.duration[0]),
        to_llu(item.duration[1]),
        to_llu(price),
        static_cast<long long>(expireTime));

    insertQuery.bind(1, note);
    insertQuery.bindBlob(2, cerealf::serialize(item.extAttrList));
    insertQuery.exec();
    transaction.commit();
    return true;
}
