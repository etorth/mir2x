#include <climits>
#include "dbpod.hpp"
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "golddb.hpp"

extern DBPod *g_dbPod;

std::optional<size_t> dbLoadGold(uint32_t dbid)
{
    fflassert(dbid > 0);
    fflassert(dbid <= SYS_MAXDBID, dbid);

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

void dbUpdateGold(uint32_t dbid, size_t gold)
{
    fflassert(dbid > 0);
    fflassert(dbid <= SYS_MAXDBID, dbid);
    fflassert(to_u64(gold) <= to_u64(INT64_MAX), gold);

    g_dbPod->exec(
            "update tbl_char set fld_gold = %llu where fld_dbid = %llu",
            to_llu(gold),
            to_llu(dbid));
}

std::optional<size_t> dbRemoveGold(uint32_t dbid, size_t gold)
{
    fflassert(dbid > 0);
    fflassert(dbid <= SYS_MAXDBID, dbid);
    fflassert(gold > 0);
    fflassert(to_u64(gold) <= to_u64(INT64_MAX), gold);

    auto query = g_dbPod->createQuery(
            u8R"###( update tbl_char                          )###"
            u8R"###( set                                      )###"
            u8R"###(     fld_gold = fld_gold - %llu           )###"
            u8R"###( where                                    )###"
            u8R"###(     fld_dbid = %llu and fld_gold >= %llu )###"
            u8R"###( returning fld_gold                       )###",

            to_llu(gold),
            to_llu(dbid),
            to_llu(gold));

    if(!query.executeStep()){
        return {};
    }

    const auto remainingGold = query.getColumn("fld_gold").getInt64();
    fflassert(remainingGold >= 0);
    fflassert(!query.executeStep());
    return check_cast<size_t, uint64_t>(to_u64(remainingGold));
}
