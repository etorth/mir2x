#include <algorithm>
#include "dbpod.hpp"
#include "totype.hpp"
#include "cerealf.hpp"
#include "fflerror.hpp"
#include "raiitimer.hpp"
#include "sysconst.hpp"
#include "questdb.hpp"

extern DBPod *g_dbPod;

namespace
{
    // field names go into the sql text, only names of the value columns pass
    void checkQuestFieldName(const std::string &fieldName)
    {
        fflassert(fieldName.starts_with("fld_"), fieldName);
        fflassert(std::ranges::all_of(fieldName, [](char ch){ return (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || (ch == '_'); }), fieldName);
        fflassert(fieldName != "fld_dbid" && fieldName != "fld_timestamp", fieldName);
    }
}

void dbCreateQuestTable(const std::string &dbName)
{
    if(g_dbPod->createQuery(u8R"###(select name from sqlite_master where type='table' and name='%s')###", dbName.c_str()).executeStep()){
        return;
    }

    g_dbPod->exec(
        u8R"###( create table %s(                                                            )###"
        u8R"###(     fld_dbid         int unsigned not null,                                 )###"
        u8R"###(     fld_timestamp    int unsigned not null,                                 )###"
        u8R"###(     fld_states       blob             null,                                 )###"
        u8R"###(     fld_flags        blob             null,                                 )###"
        u8R"###(     fld_team         blob             null,                                 )###"
        u8R"###(     fld_vars         blob             null,                                 )###"
        u8R"###(     fld_desp         blob             null,                                 )###"
        u8R"###(     fld_context      blob             null,                                 )###"
        u8R"###(                                                                             )###"
        u8R"###(     foreign key (fld_dbid) references tbl_char(fld_dbid) on delete cascade, )###"
        u8R"###(     primary key (fld_dbid)                                                  )###"
        u8R"###( );                                                                          )###", dbName.c_str());
}

std::optional<luaf::luaVar> dbLoadQuestField(const std::string &dbName, uint32_t dbid, const std::string &fieldName)
{
    checkQuestFieldName(fieldName);

    auto query = g_dbPod->createQuery(u8R"###(select %s from %s where fld_dbid=%llu and %s is not null)###", fieldName.c_str(), dbName.c_str(), to_llu(dbid), fieldName.c_str());
    if(!query.executeStep()){
        return std::nullopt;
    }
    return cerealf::deserialize<luaf::luaVar>(query.getColumn(0).getString());
}

void dbUpdateQuestFields(const std::string &dbName, uint32_t dbid, const std::vector<std::pair<std::string, std::optional<luaf::luaVar>>> &fieldList, bool replace)
{
    fflassert(!fieldList.empty());

    std::string columns;
    std::string values;
    std::string updates;

    for(const auto &[fieldName, value]: fieldList){
        checkQuestFieldName(fieldName);
        fflassert(std::ranges::count_if(fieldList, [&fieldName](const auto &field){ return field.first == fieldName; }) == 1, fieldName);

        columns += ", " + fieldName;
        values  += ", ?";
        updates += ", " + fieldName + "=excluded." + fieldName;
    }

    const auto timestamp = hres_tstamp().to_nsec();
    auto query = replace
        ? g_dbPod->createQuery(u8R"###(replace into %s(fld_dbid, fld_timestamp%s) values (%llu, %llu%s))###", dbName.c_str(), columns.c_str(), to_llu(dbid), to_llu(timestamp), values.c_str())
        : g_dbPod->createQuery(u8R"###(insert into %s(fld_dbid, fld_timestamp%s) values (%llu, %llu%s) on conflict(fld_dbid) do update set fld_timestamp=excluded.fld_timestamp%s)###", dbName.c_str(), columns.c_str(), to_llu(dbid), to_llu(timestamp), values.c_str(), updates.c_str());

    for(int index = 1; const auto &[fieldName, value]: fieldList){
        if(value.has_value()){
            query.bindBlob(index++, cerealf::serialize(value.value()));
        }
        else{
            query.bind(index++);
        }
    }
    query.exec();
}

std::vector<std::pair<std::string, std::optional<luaf::luaVar>>> buildQuestFieldList(const sol::table &fieldTable)
{
    std::vector<std::pair<std::string, std::optional<luaf::luaVar>>> fieldList;
    for(const auto &[key, value]: fieldTable){
        fflassert(key.is<std::string>(), luaf::luaObjTypeString(key));
        if(value.is<std::string>() && (value.as<std::string>() == SYS_LUANIL)){
            fieldList.emplace_back(key.as<std::string>(), std::nullopt);
        }
        else{
            fieldList.emplace_back(key.as<std::string>(), luaf::buildLuaVar(value));
        }
    }
    return fieldList;
}
