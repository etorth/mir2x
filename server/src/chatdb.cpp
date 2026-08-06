#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include "dbpod.hpp"
#include "strf.hpp"
#include "totype.hpp"
#include "sysconst.hpp"
#include "fflerror.hpp"
#include "raiitimer.hpp"
#include "chatdb.hpp"

extern DBPod *g_dbPod;

namespace
{
    SDChatMessage loadChatMessage(DBPod::Statement &query)
    {
        return SDChatMessage
        {
            .seq = SDChatMessageDBSeq
            {
                .id        = to_u64(query.getColumn("fld_id").getInt64()),
                .timestamp = to_u64(query.getColumn("fld_timestamp").getInt64()),
            },

            .refer = query.getColumn("fld_refer").isNull() ? std::nullopt : std::make_optional<uint64_t>(query.getColumn("fld_refer").getInt64()),

            .from = SDChatPeerID(to_u64(query.getColumn("fld_from").getInt64())),
            .to   = SDChatPeerID(to_u64(query.getColumn("fld_to"  ).getInt64())),

            .message = query.getColumn("fld_message").getString(),
        };
    }

    bool isChatGroupMember(uint32_t playerDBID, uint32_t groupID)
    {
        return g_dbPod->createQuery(
            u8R"###( select 1 from tbl_chatgroupmember where fld_group = %llu and fld_member = %llu )###",
            to_llu(groupID),
            to_llu(playerDBID)).executeStep();
    }
}

std::tuple<uint64_t, uint64_t> dbSaveChatMessage(const SDChatPeerID &fromCPID, const SDChatPeerID &toCPID, const std::string_view &message, std::optional<uint64_t> refID)
{
    const auto timestamp = hres_tstamp::localtime();
    auto query = g_dbPod->createQuery(
        u8R"###( insert into tbl_chatmessage(fld_timestamp, fld_refer, fld_from, fld_to, fld_message) )###"
        u8R"###( values                                                                               )###"
        u8R"###(     (%llu, %s, %llu, %llu, ?)                                                        )###"
        u8R"###( returning                                                                            )###"
        u8R"###(     fld_id                                                                           )###",

        to_llu(timestamp),
        refID.has_value() ? std::to_string(refID.value()).c_str() : "null",
        to_llu(fromCPID.asU64()),
        to_llu(toCPID.asU64()));

    query.bindBlob(1, message);
    if(!query.executeStep()){
        throw fflpanic("failed to insert chat message");
    }
    const auto messageID = to_u64(query.getColumn("fld_id").getInt64());
    fflassert(!query.executeStep());
    return {messageID, timestamp};
}

std::optional<SDChatPeer> dbLoadChatPeer(uint64_t argCPID)
{
    const SDChatPeerID sdCPID(argCPID);
    if(sdCPID.group()){
        if(auto query = g_dbPod->createQuery("select * from tbl_chatgroup where fld_id = %llu", to_llu(sdCPID.id())); query.executeStep()){
            return SDChatPeer
            {
                .id = query.getColumn("fld_id"),
                .name = query.getColumn("fld_name").getString(),
                .avatar = std::nullopt,

                .despvar = SDChatPeerGroupVar
                {
                    .creator = query.getColumn("fld_creator"),
                    .createtime = check_cast<uint64_t>(query.getColumn("fld_createtime").getInt64()),
                },
            };
        }
        return std::nullopt;
    }

    if(sdCPID.player()){
        if(auto query = g_dbPod->createQuery("select * from tbl_char where fld_dbid = %llu", to_llu(sdCPID.id())); query.executeStep()){
            return SDChatPeer
            {
                .id = query.getColumn("fld_dbid"),
                .name = query.getColumn("fld_name").getString(),
                .avatar = std::nullopt,

                .despvar = SDChatPeerPlayerVar
                {
                    .gender = query.getColumn("fld_gender").getUInt() > 0,
                    .job = query.getColumn("fld_job"),
                },
            };
        }
        return std::nullopt;
    }

    if(sdCPID.special()){
        switch(sdCPID.id()){
            case SYS_CHATDBID_SYSTEM:
                return SDChatPeer
                {
                    .id = SYS_CHATDBID_SYSTEM,
                    .name = SYS_NAME_CHATDBID_SYSTEM,
                };
            case SYS_CHATDBID_GROUP:
                return SDChatPeer
                {
                    .id = SYS_CHATDBID_GROUP,
                    .name = SYS_NAME_CHATDBID_GROUP,
                };
            case SYS_CHATDBID_AI:
                return SDChatPeer
                {
                    .id = SYS_CHATDBID_AI,
                    .name = SYS_NAME_CHATDBID_AI,
                };
            default:
                return std::nullopt;
        }
    }

    throw fflpanic("invalid cpid: 0x{:016x}", argCPID);
}

std::optional<SDChatMessage> dbQueryChatMessage(uint64_t argMsgID)
{
    if(auto query = g_dbPod->createQuery("select * from tbl_chatmessage where fld_id = %llu", to_llu(argMsgID)); query.executeStep()){
        return loadChatMessage(query);
    }
    return std::nullopt;
}

std::vector<uint32_t> dbLoadChatGroupMemberList(uint32_t chatGroup)
{
    std::vector<uint32_t> result;
    auto query = g_dbPod->createQuery("select * from tbl_chatgroupmember where fld_group = %llu", to_llu(chatGroup));

    while(query.executeStep()){
        result.push_back(to_u32(query.getColumn("fld_member").getInt64()));
    }
    return result;
}

SDChatPeerList dbLoadChatGroupList(uint32_t memberDBID)
{
    SDChatPeerList result;
    auto query = g_dbPod->createQuery("select * from tbl_chatgroup where fld_id in (select fld_group from tbl_chatgroupmember where fld_member = %llu)", to_llu(memberDBID));

    while(query.executeStep()){
        result.push_back(SDChatPeer
        {
            .id = query.getColumn("fld_id"),
            .name = query.getColumn("fld_name").getString(),

            .avatar = std::nullopt,
            .despvar = SDChatPeerGroupVar
            {
                .creator = query.getColumn("fld_creator"),
                .createtime = to_u64(query.getColumn("fld_createtime").getInt64()),
            },
        });
    }
    return result;
}

SDChatPeerList dbQueryChatPeerList(const std::string &query, bool includePlayer, bool includeGroup)
{
    fflassert(str_haschar(query));
    fflassert(includePlayer || includeGroup);

    SDChatPeerList sdPCL;
    const bool onlyDigits = (query.find_first_not_of("0123456789") == std::string_view::npos);

    if(includePlayer){
        auto queryChar = onlyDigits ? g_dbPod->createQuery("select * from tbl_char where fld_dbid = %s or instr(fld_name, '%s') > 0", query.c_str(), query.c_str())
                                    : g_dbPod->createQuery("select * from tbl_char where                  instr(fld_name, '%s') > 0", query.c_str());

        while(queryChar.executeStep()){
            sdPCL.push_back(SDChatPeer
            {
                .id = queryChar.getColumn("fld_dbid"),
                .name = queryChar.getColumn("fld_name").getString(),

                .avatar = std::nullopt,
                .despvar = SDChatPeerPlayerVar
                {
                    .gender = queryChar.getColumn("fld_gender").getUInt() > 0,
                    .job = queryChar.getColumn("fld_job"),
                },
            });
        }
    }

    if(includeGroup){
        auto queryGroup = onlyDigits ? g_dbPod->createQuery("select * from tbl_chatgroup where fld_id = %s or instr(fld_name, '%s') > 0", query.c_str(), query.c_str())
                                     : g_dbPod->createQuery("select * from tbl_chatgroup where                instr(fld_name, '%s') > 0", query.c_str());

        while(queryGroup.executeStep()){
            sdPCL.push_back(SDChatPeer
            {
                .id = queryGroup.getColumn("fld_id"),
                .name = queryGroup.getColumn("fld_name").getString(),

                .avatar = std::nullopt,
                .despvar = SDChatPeerGroupVar
                {
                    .creator = queryGroup.getColumn("fld_creator"),
                    .createtime = check_cast<uint64_t>(queryGroup.getColumn("fld_createtime").getInt64()),
                },
            });
        }
    }

    return sdPCL;
}

SDChatMessageList dbRetrieveLatestChatMessage(uint32_t playerDBID, const std::span<const uint64_t> &cpidList, size_t limitPerID, bool includeSend, bool includeRecv)
{
    if(cpidList.empty() || !(includeSend || includeRecv)){
        return {};
    }

    const auto playerCPID = SDChatPeerID(CPR_PLAYER, playerDBID);

    std::vector<std::string> queries;
    for(const auto other: cpidList){
        const SDChatPeerID otherCPID(other);
        if(otherCPID.group() && !isChatGroupMember(playerDBID, otherCPID.id())){
            continue;
        }

        queries.push_back("select * from ( select * from tbl_chatmessage where ");
        if(includeSend){
            queries.back().append(str_printf("(fld_from = %llu and fld_to = %llu) ", to_llu(playerCPID.asU64()), to_llu(other)));
        }

        if(includeRecv){
            if(includeSend){
                queries.back().append("or ");
            }

            if(otherCPID.group()){
                queries.back().append(str_printf("(fld_to = %llu) ", to_llu(other)));
            }
            else{
                queries.back().append(str_printf("(fld_from = %llu and fld_to = %llu) ", to_llu(other), to_llu(playerCPID.asU64())));
            }
        }

        queries.back().append("order by fld_timestamp desc ");

        if(limitPerID > 0){
            queries.back().append(str_printf("limit %zu ", limitPerID));
        }

        queries.back().append(" )");

        if(includeRecv && other == SDChatPeerID(CPR_SPECIAL, SYS_CHATDBID_SYSTEM).asU64()){
            queries.push_back(str_printf(
                R"###( select                                                             )###"
                R"###(     tbl_chatmessage.*                                              )###"
                R"###( from                                                               )###"
                R"###(     tbl_delivery                                                   )###"
                R"###(     inner join tbl_chatmessage                                     )###"
                R"###(         on tbl_chatmessage.fld_id = tbl_delivery.fld_messageid     )###"
                R"###( where                                                              )###"
                R"###(     tbl_delivery.fld_dbid = %llu and tbl_delivery.fld_claimed = 0; )###", to_llu(playerDBID)));
        }
    }

    if(queries.empty()){
        return {};
    }

    SDChatMessageList result;
    auto query = g_dbPod->createQuery(str_join(queries, " union ").c_str());

    while(query.executeStep()){
        result.push_back(loadChatMessage(query));
    }
    return result;
}

SDChatPeer dbCreateChatGroup(uint32_t creatorDBID, const char *name, const std::span<const uint32_t> &dbidList)
{
    fflassert(str_haschar(name));
    fflassert(!dbidList.empty());

    const auto tstamp = hres_tstamp::localtime();
    auto query = g_dbPod->createQuery(
            u8R"###( insert into tbl_chatgroup(fld_creator, fld_createtime, fld_name) )###"
            u8R"###( values                                                           )###"
            u8R"###(     (%llu, %llu, '%s')                                           )###"
            u8R"###( returning                                                        )###"
            u8R"###(     fld_id;                                                      )###",

            to_llu(creatorDBID),
            to_llu(tstamp),
            name);

    if(!query.executeStep()){
        throw fflpanic("failed to create a group");
    }

    SDChatPeer groupCP
    {
        .id = query.getColumn("fld_id"),
        .name = name,
        .despvar = SDChatPeerGroupVar
        {
            .creator = creatorDBID,
            .createtime = tstamp,
        },
    };

    bool foundCreator = false;
    std::string valStr;

    const auto fnAppendStr = [&valStr, &groupCP, tstamp](uint32_t memberDBID)
    {
        if(!valStr.empty()){
            valStr.append(",");
        }

        valStr.append(str_printf("(%llu, %llu, %llu, %llu)",
            to_llu(groupCP.id),
            to_llu(memberDBID),
            to_llu(0),
            to_llu(tstamp)));
    };

    for(const auto memberDBID: dbidList){
        if(memberDBID == creatorDBID){
            foundCreator = true;
        }
        fnAppendStr(memberDBID);
    }

    if(!foundCreator){
        fnAppendStr(creatorDBID);
    }

    auto addMemberQuery = g_dbPod->createQuery(
            u8R"###( insert into tbl_chatgroupmember(fld_group, fld_member, fld_permission, fld_jointime) )###"
            u8R"###( values                                                                               )###"
            u8R"###(     %s;                                                                              )###",

            valStr.c_str());

    addMemberQuery.executeStep();
    return groupCP;
}
