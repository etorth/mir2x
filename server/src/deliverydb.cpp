#include <algorithm>
#include <map>
#include <string_view>
#include <tuple>
#include "dbpod.hpp"
#include "mathf.hpp"
#include "xmlf.hpp"
#include "dbcomid.hpp"
#include "sysconst.hpp"
#include "raiitimer.hpp"
#include "deliverydb.hpp"

extern DBPod *g_dbPod;

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

DBDelivery dbCreateDeliveryInTransaction(uint32_t recipientDBID, std::vector<SDItem> itemList, std::string title)
{
    fflassert(recipientDBID > 0 && recipientDBID <= SYS_MAXDBID, recipientDBID);
    fflassert(!itemList.empty());
    fflassert(!title.empty());
    fflassert(std::ranges::all_of(itemList, [](const auto &item){ return item && item.seqID == 0; }));

    const auto payload = cerealf::serialize(itemList);

    std::string record;
    for(int i = 0; i < 16; ++i){
        record = mathf::randstr(SYS_DELIVERYRECORDSIZE);

        auto query = g_dbPod->createQuery(
            u8R"###( insert into tbl_delivery(fld_record, fld_dbid, fld_timestamp, fld_payload) )###"
            u8R"###( values                                                                     )###"
            u8R"###(     (?, %llu, %llu, ?)                                                     )###"
            u8R"###( on conflict(fld_record) do nothing                                         )###"
            u8R"###( returning fld_record                                                       )###",

            to_llu(recipientDBID),
            to_llu(hres_tstamp::localtime()));

        query.bind(1, record);
        query.bindBlob(2, payload);
        if(query.executeStep()){
            break;
        }
        record.clear();
    }

    if(record.empty()){
        throw fflpanic("failed to generate delivery record");
    }

    std::map<uint32_t, size_t> itemSummary;
    for(const auto &item: itemList){
        itemSummary[item.itemID] += item.count;
    }

    std::string xmlString = "<layout>";
    xmlString += xmlf::toParString("%s", title.c_str());
    for(const auto &[itemID, count]: itemSummary){
        const auto &ir = DBCOM_ITEMRECORD(itemID);
        fflassert(ir);
        xmlString += xmlf::toParString("%s：%zu", ir.isGold() ? to_cstr(u8"金币") : to_cstr(ir.name), count);
    }
    xmlString += str_printf(R"###(<par><event id="%s" record="%s">收取</event></par>)###", SYS_DELIVERY, record.c_str());
    xmlString += "</layout>";

    const auto fromCPID = SDChatPeerID(CPR_SPECIAL, SYS_CHATDBID_SYSTEM);
    const auto toCPID = SDChatPeerID(CPR_PLAYER, recipientDBID);
    const auto messageBuf = cerealf::serialize(xmlString);
    const auto [messageID, messageTimestamp] = dbSaveChatMessage(fromCPID, toCPID, messageBuf, std::nullopt);
    {
        auto query = g_dbPod->createQuery(
            u8R"###( update tbl_delivery set fld_messageid = %llu where fld_record = ? returning fld_record )###",
            to_llu(messageID));

        query.bind(1, record);
        if(!query.executeStep()){
            throw fflpanic("failed to bind delivery record to chat message");
        }
        fflassert(!query.executeStep());
    }

    return DBDelivery
    {
        .record = std::move(record),
        .message
        {
            .seq = SDChatMessageDBSeq
            {
                .id = messageID,
                .timestamp = messageTimestamp,
            },

            .from = fromCPID,
            .to = toCPID,
            .message = messageBuf,
        },
    };
}

DBDelivery dbCreateDelivery(uint32_t recipientDBID, std::vector<SDItem> itemList, std::string title)
{
    auto transaction = g_dbPod->createTransaction();
    auto delivery = dbCreateDeliveryInTransaction(recipientDBID, std::move(itemList), std::move(title));
    transaction.commit();
    return delivery;
}
