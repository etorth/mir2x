#include <chrono>
#include "dbpod.hpp"
#include "dbcomid.hpp"
#include "fflerror.hpp"
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
            case ACUTIONCAT_POTION  : return ir.isPotion() || ir.isDope() || to_u8sv(ir.type) == u8"药粉";
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
}

SDAcutionItemList dbQueryAcutionItemList(int category)
{
    fflassert(category >= ACUTIONCAT_BEGIN && category < ACUTIONCAT_END, category);

    const auto now = epochSeconds();
    auto query = g_dbPod->createQuery(
        u8R"###(
            select
                ai.*,
                ch.fld_name as fld_sellername
            from tbl_acutionitem as ai
            join tbl_char as ch on ch.fld_dbid = ai.fld_seller
            where ai.fld_expiretime > %lld
            order by ai.fld_id
        )###",
        static_cast<long long>(now));

    SDAcutionItemList result
    {
        .category = category,
    };

    while(query.executeStep()){
        const auto expireTime = query.getColumn("fld_expiretime").getInt64();
        const auto price = query.getColumn("fld_price").getInt64();
        fflassert(expireTime > now, expireTime, now);
        fflassert(price >= 0, price);

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
            .seller = query.getColumn("fld_sellername").getString(),
            .timeLeft = check_cast<size_t, uint64_t>(expireTime - now),
            .price = check_cast<size_t, uint64_t>(price),
            .item = std::move(item),
        });
    }
    return result;
}
