#include <cstring>
#include "luaf.hpp"
#include "dbpod.hpp"
#include "player.hpp"
#include "monoserver.hpp"

extern DBPod *g_dbPod;
extern MonoServer *g_monoServer;

luaf::luaVar Player::dbGetVar(const std::string &var)
{
    fflassert(str_haschar(var));
    auto query = g_dbPod->createQuery("select fld_value from tbl_charvarlist where fld_dbid = %llu and fld_var = '%s' and fld_value is not null", to_llu(dbid()), var.c_str());

    if(query.executeStep()){
        return cerealf::deserialize<luaf::luaVar>(query.getColumn(0).getString());
    }
    else{
        return luaf::luaNil{}; // TODO throw or return nil ?
    }
}

void Player::dbSetVar(const std::string &var, luaf::luaVar value)
{
    fflassert(str_haschar(var));
    if(std::get_if<luaf::luaNil>(&value)){
        g_dbPod->exec("delete from tbl_charvarlist where fld_dbid = %llu and fld_var = '%s'", to_llu(dbid()), var.c_str());
    }
    else{
        auto query = g_dbPod->createQuery(
            u8R"###( insert into tbl_charvarlist(fld_dbid, fld_var, fld_value) )###"
            u8R"###( values                                                    )###"
            u8R"###(     (%llu, '%s', ?)                                       )###"
            u8R"###(                                                           )###"
            u8R"###( on conflict(fld_dbid, fld_var) do                         )###"
            u8R"###( update set                                                )###"
            u8R"###(                                                           )###"
            u8R"###(     fld_value=excluded.fld_value                          )###",

            to_llu(dbid()),
            var.c_str());

        query.bindBlob(1, cerealf::serialize(value));
        query.exec();
    }
}

std::pair<bool, luaf::luaVar> Player::dbHasVar(const std::string &var)
{
    fflassert(str_haschar(var));
    auto query = g_dbPod->createQuery("select fld_value from tbl_charvarlist where fld_dbid = %llu and fld_var = '%s' and fld_value is not null", to_llu(dbid()), var.c_str());

    if(query.executeStep()){
        return std::make_pair(true, cerealf::deserialize<luaf::luaVar>(query.getColumn(0).getString()));
    }
    else{
        return std::make_pair(false, luaf::luaNil{});
    }
}

void Player::dbRemoveVar(const std::string &var)
{
    fflassert(str_haschar(var));
    g_dbPod->exec("delete from tbl_charvarlist where fld_dbid = %llu and fld_var = '%s'", to_llu(dbid()), var.c_str());
}

void Player::dbUpdateExp()
{
    g_dbPod->exec(u8R"###( update tbl_char set fld_exp = %llu where fld_dbid = %llu )###", to_llu(exp()), to_llu(dbid()));
}

void Player::dbUpdateMapGLoc()
{
    g_dbPod->exec(u8R"###( update tbl_char set fld_map = %d, fld_mapx = %d, fld_mapy = %d where fld_dbid = %llu )###", to_d(mapID()), X(), Y(), to_llu(dbid()));
}

void Player::dbUpdateHealth()
{
    g_dbPod->exec(u8R"###( update tbl_char set fld_hp = %d, fld_mp = %d where fld_dbid = %llu )###", m_sdHealth.hp, m_sdHealth.mp, to_llu(dbid()));
}

void Player::dbLoadInventory()
{
    // tbl_inventory:
    // +----------+------------+-----------+-----------+--------------+-----------------+
    // | fld_dbid | fld_itemid | fld_seqid | fld_count | fld_duration | fld_extattrlist |
    // +----------+------------+-----------+-----------+--------------+-----------------+
    // |<----primary key---->|

    m_sdItemStorage.inventory.clear();
    auto query = g_dbPod->createQuery("select * from tbl_inventory where fld_dbid = %llu", to_llu(dbid()));

    while(query.executeStep()){
        SDItem item
        {
            .itemID = check_cast<uint32_t, unsigned>(query.getColumn("fld_itemid")),
            .seqID  = check_cast<uint32_t, unsigned>(query.getColumn("fld_seqid" )),
            .count  = check_cast<  size_t, unsigned>(query.getColumn("fld_count" )),
            .duration
            {
                check_cast<size_t, unsigned>(query.getColumn("fld_duration")),
                check_cast<size_t, unsigned>(query.getColumn("fld_maxduration")),
            },
            .extAttrList = cerealf::deserialize<std::unordered_map<int, std::string>>(query.getColumn("fld_extattrlist")),
        };

        fflassert(item);
        m_sdItemStorage.inventory.add(std::move(item), true);
    }
}

void Player::dbUpdateInventoryItem(const SDItem &item)
{
    fflassert(item);
    auto query = g_dbPod->createQuery(
            u8R"###( replace into tbl_inventory(fld_dbid, fld_itemid, fld_seqid, fld_count, fld_duration, fld_maxduration, fld_extattrlist) )###"
            u8R"###( values                                                                                                                 )###"
            u8R"###(     (%llu, %llu, %llu, %llu, %llu, %llu, ?)                                                                            )###",

            to_llu(dbid()),
            to_llu(item.itemID),
            to_llu(item.seqID),
            to_llu(item.count),
            to_llu(item.duration[0]),
            to_llu(item.duration[1]));

    query.bindBlob(1, cerealf::serialize(item.extAttrList));
    query.exec();
}

void Player::dbRemoveInventoryItem(const SDItem &item)
{
    dbRemoveInventoryItem(item.itemID, item.seqID);
}

void Player::dbRemoveInventoryItem(uint32_t itemID, uint32_t seqID)
{
    // requies seqID to be non-zero
    // means won't support remove more than 1 item in database per call

    if(!(DBCOM_ITEMRECORD(itemID) && seqID > 0)){
        throw fflerror("invalid arguments: itemID = %llu, seqID = %llu", to_llu(itemID), to_llu(seqID));
    }
    g_dbPod->exec("delete from tbl_inventory where fld_dbid = %llu and fld_itemid = %llu and fld_seqid = %llu", to_llu(dbid()), to_llu(itemID), to_llu(seqID));
}

void Player::dbSecureItem(uint32_t itemID, uint32_t seqID)
{
    const auto &item = findInventoryItem(itemID, seqID);
    fflassert(item);

    auto query = g_dbPod->createQuery(
            u8R"###( replace into tbl_secureditemlist(fld_dbid, fld_itemid, fld_seqid, fld_count, fld_duration, fld_maxduration, fld_extattrlist) )###"
            u8R"###( values                                                                                                                       )###"
            u8R"###(     (%llu, %llu, %llu, %llu, %llu, %llu, ?)                                                                                  )###",

            to_llu(dbid()),
            to_llu(item.itemID),
            to_llu(item.seqID),
            to_llu(item.count),
            to_llu(item.duration[0]),
            to_llu(item.duration[1]));

    query.bindBlob(1, cerealf::serialize(item.extAttrList));
    query.exec();
}

SDItem Player::dbRetrieveSecuredItem(uint32_t itemID, uint32_t seqID)
{
    fflassert(DBCOM_ITEMRECORD(itemID));
    fflassert(seqID > 0);

    auto query = g_dbPod->createQuery(
            u8R"###( delete from tbl_secureditemlist where fld_dbid = %llu and fld_itemid = %llu and fld_seqid = %llu returning * )###",

            to_llu(dbid()),
            to_llu(itemID),
            to_llu(seqID));

    while(query.executeStep()){
        SDItem item
        {
            .itemID = check_cast<uint32_t, unsigned>(query.getColumn("fld_itemid")),
            .seqID  = check_cast<uint32_t, unsigned>(query.getColumn("fld_seqid" )),
            .count  = check_cast<  size_t, unsigned>(query.getColumn("fld_count" )),
            .duration
            {
                check_cast<size_t, unsigned>(query.getColumn("fld_duration")),
                check_cast<size_t, unsigned>(query.getColumn("fld_maxduration")),
            },
            .extAttrList = cerealf::deserialize<std::unordered_map<int, std::string>>(query.getColumn("fld_extattrlist")),
        };

        fflassert(item);
        fflassert(!query.executeStep());
        return item;
    }
    throw fflerror("can't find item: itemID = %llu, seqID = %llu", to_llu(itemID), to_llu(seqID));
}

std::vector<SDItem> Player::dbLoadSecuredItemList() const
{
    // tbl_secureditemlist:
    // +----------+------------+-----------+-----------+--------------+-----------------+
    // | fld_dbid | fld_itemid | fld_seqid | fld_count | fld_duration | fld_extattrlist |
    // +----------+------------+-----------+-----------+--------------+-----------------+
    // |<-----primary key----->|

    std::vector<SDItem> itemList;
    auto query = g_dbPod->createQuery("select * from tbl_secureditemlist where fld_dbid = %llu", to_llu(dbid()));

    while(query.executeStep()){
        SDItem item
        {
            .itemID = check_cast<uint32_t, unsigned>(query.getColumn("fld_itemid")),
            .seqID  = check_cast<uint32_t, unsigned>(query.getColumn("fld_seqid" )),
            .count  = check_cast<  size_t, unsigned>(query.getColumn("fld_count" )),
            .duration
            {
                check_cast<size_t, unsigned>(query.getColumn("fld_duration")),
                check_cast<size_t, unsigned>(query.getColumn("fld_maxduration")),
            },
            .extAttrList = cerealf::deserialize<std::unordered_map<int, std::string>>(query.getColumn("fld_extattrlist")),
        };

        fflassert(item);
        itemList.push_back(std::move(item));
    }
    return itemList;
}

void Player::dbLoadPlayerConfig()
{
    // tbl_playerconfig:
    // +----------+------------------+-------------------+
    // | fld_dbid | fld_magickeylist | fld_runtimeconfig |
    // +----------+------------------+-------------------+

    m_sdPlayerConfig.magicKeyList.clear();

    auto query = g_dbPod->createQuery("select * from tbl_playerconfig where fld_dbid = %llu", to_llu(dbid()));

    if(query.executeStep()){
        if(const std::string buf = query.getColumn("fld_magickeylist"); !buf.empty()){
            m_sdPlayerConfig.magicKeyList = cerealf::deserialize<SDMagicKeyList>(buf);
        }

        if(const std::string buf = query.getColumn("fld_runtimeconfig"); !buf.empty()){
            m_sdPlayerConfig.runtimeConfig = cerealf::deserialize<SDRuntimeConfig>(buf);
        }
    }
}

std::optional<SDChatPeer> Player::dbLoadChatPeer(bool argGroup, uint32_t argDBID)
{
    auto query = argGroup ? g_dbPod->createQuery("select * from tbl_chatgroup where fld_id   = %llu", to_llu(argDBID))
                          : g_dbPod->createQuery("select * from tbl_char      where fld_dbid = %llu", to_llu(argDBID));

    if(query.executeStep()){
        if(argGroup){
            return SDChatPeer
            {
                .id = query.getColumn(argGroup ? "fld_id" : "fld_dbid"),
                .name = query.getColumn("fld_name").getString(),
                .avatar = std::nullopt,

                .despvar = SDChatPeerGroupVar
                {
                    .creator = query.getColumn("fld_creator"),
                    .createtime = check_cast<uint64_t>(query.getColumn("fld_createtime").getInt64()),
                },
            };
        }
        else{
            return SDChatPeer
            {
                .id = query.getColumn(argGroup ? "fld_id" : "fld_dbid"),
                .name = query.getColumn("fld_name").getString(),
                .avatar = std::nullopt,

                .despvar = SDChatPeerPlayerVar
                {
                    .gender = query.getColumn("fld_gender").getUInt() > 0,
                    .job = query.getColumn("fld_job"),
                },
            };
        }
    }
    else{
        return {};
    }
}

std::vector<uint32_t> Player::dbLoadChatGroupMemberList(uint32_t chatGroup)
{
    std::vector<uint32_t> result;
    auto query = g_dbPod->createQuery("select * from tbl_chatgroupmember where fld_group = %llu", to_llu(chatGroup));

    while(query.executeStep()){
        result.push_back(to_u32(query.getColumn("fld_member").getInt64())); // self may not be in this group
    }
    return result;
}

SDChatPeerList Player::dbQueryChatPeerList(const std::string &query, bool includePlayer, bool includeGroup)
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

void Player::dbUpdateMagicKey(uint32_t magicID, char key)
{
    fflassert(m_sdLearnedMagicList.has(magicID));
    fflassert((key >= 'a' && key <= 'z') || (key >= '0' && key <= '9'));

    if(!m_sdPlayerConfig.magicKeyList.setMagicKey(magicID, key)){
        return;
    }

    const auto keyBuf = cerealf::serialize(m_sdPlayerConfig.magicKeyList);
    auto query = g_dbPod->createQuery(
            u8R"###( insert into tbl_playerconfig(fld_dbid, fld_magickeylist) )###"
            u8R"###( values                                                   )###"
            u8R"###(     (%llu, ?)                                            )###"
            u8R"###(                                                          )###"
            u8R"###( on conflict(fld_dbid) do update set                      )###"
            u8R"###(     fld_magickeylist = excluded.fld_magickeylist         )###",

            to_llu(dbid()));

    query.bindBlob(1, keyBuf.data(), keyBuf.length());
    query.exec();
}

void Player::dbUpdateRuntimeConfig()
{
    const auto configBuf = cerealf::serialize(m_sdPlayerConfig.runtimeConfig);
    auto query = g_dbPod->createQuery(
            u8R"###( insert into tbl_playerconfig(fld_dbid, fld_runtimeconfig) )###"
            u8R"###( values                                                    )###"
            u8R"###(     (%llu, ?)                                             )###"
            u8R"###(                                                           )###"
            u8R"###( on conflict(fld_dbid) do update set                       )###"
            u8R"###(     fld_runtimeconfig = excluded.fld_runtimeconfig        )###",

            to_llu(dbid()));

    query.bindBlob(1, configBuf.data(), configBuf.length());
    query.exec();
}

void Player::dbLoadBelt()
{
    // tbl_belt:
    // +----------+----------+------------+-----------+
    // | fld_dbid | fld_belt | fld_itemid | fld_count |
    // +----------+----------+------------+-----------+
    // |<----primary key---->|

    m_sdItemStorage.belt.clear();
    auto query = g_dbPod->createQuery("select * from tbl_belt where fld_dbid = %llu", to_llu(dbid()));

    while(query.executeStep()){
        const auto index = check_cast<size_t, unsigned>(query.getColumn("fld_belt"));
        if(index >= 6){
            throw fflerror("invalid belt slot: %zu", index);
        }

        const auto itemID = check_cast<uint32_t, unsigned>(query.getColumn("fld_itemid"));
        const auto typeStr = to_u8sv(DBCOM_ITEMRECORD(itemID).type);
        if(true
                && typeStr != u8"恢复药水"
                && typeStr != u8"传送卷轴"){
            throw fflerror("invalid item type to belt slot");
        }

        SDItem item
        {
            .itemID = itemID,
            .count  = check_cast<size_t, unsigned>(query.getColumn("fld_count")),
        };

        fflassert(item);
        m_sdItemStorage.belt.list.at(index) = std::move(item);
    }
}

void Player::dbUpdateBeltItem(size_t slot, const SDItem &item)
{
    if(slot >= 6){
        throw fflerror("invalid belt slot: %zu", slot);
    }

    fflassert(item);
    const auto typeStr = to_u8sv(DBCOM_ITEMRECORD(item.itemID).type);
    if(true
            && typeStr != u8"恢复药水"
            && typeStr != u8"传送卷轴"){
        throw fflerror("invalid item type to belt slot");
    }

    g_dbPod->exec(
            u8R"###( replace into tbl_belt(fld_dbid, fld_belt, fld_itemid, fld_count) )###"
            u8R"###( values                                                           )###"
            u8R"###(     (%llu, %llu, %llu, %llu)                                     )###",

            to_llu(dbid()),
            to_llu(slot),
            to_llu(item.itemID),
            to_llu(item.count));
}

void Player::dbRemoveBeltItem(size_t slot)
{
    if(slot >= 6){
        throw fflerror("invalid belt slot: %zu", slot);
    }
    g_dbPod->exec("delete from tbl_belt where fld_dbid = %llu and fld_belt = %zu", to_llu(dbid()), slot);
}

void Player::dbLoadWear()
{
    // tbl_wear:
    // +----------+----------+------------+-----------+--------------+-----------------+
    // | fld_dbid | fld_wear | fld_itemid | fld_count | fld_duration | fld_extattrlist |
    // +----------+----------+------------+-----------+--------------+-----------------+
    // |<----primary key---->|

    m_sdItemStorage.wear.clear();
    auto query = g_dbPod->createQuery("select * from tbl_wear where fld_dbid = %llu", to_llu(dbid()));

    while(query.executeStep()){
        const auto wltype = check_cast<int, unsigned>(query.getColumn("fld_wear"));
        SDItem item
        {
            .itemID = check_cast<uint32_t, unsigned>(query.getColumn("fld_itemid")),
            .seqID  = 0,
            .count  = check_cast<size_t, unsigned>(query.getColumn("fld_count")),
            .duration
            {
                check_cast<size_t, unsigned>(query.getColumn("fld_duration")),
                check_cast<size_t, unsigned>(query.getColumn("fld_maxduration")),
            },
            .extAttrList = cerealf::deserialize<std::unordered_map<int, std::string>>(query.getColumn("fld_extattrlist")),
        };

        if(!DBCOM_ITEMRECORD(item.itemID).wearable(wltype)){
            throw fflerror("invalid item type to wear grid");
        }

        fflassert(item);
        m_sdItemStorage.wear.setWLItem(wltype, std::move(item));
    }
}

void Player::dbUpdateWearItem(int wltype, const SDItem &item)
{
    if(!(wltype >= WLG_BEGIN && wltype < WLG_END)){
        throw fflerror("bad wltype: %d", wltype);
    }

    fflassert(item);
    if(!DBCOM_ITEMRECORD(item.itemID).wearable(wltype)){
        throw fflerror("invalid item type to wear grid");
    }

    // only save itemID and wltype
    // drop the seqID when saving to database

    auto query = g_dbPod->createQuery(
            u8R"###( replace into tbl_wear(fld_dbid, fld_wear, fld_itemid, fld_count, fld_duration, fld_maxduration, fld_extattrlist) )###"
            u8R"###( values                                                                                                           )###"
            u8R"###(     (%llu, %llu, %llu, %llu, %llu, %llu, ?)                                                                      )###",

            to_llu(dbid()),
            to_llu(wltype),
            to_llu(item.itemID),
            to_llu(item.count),
            to_llu(item.duration[0]),
            to_llu(item.duration[1]));

    query.bindBlob(1, cerealf::serialize(item.extAttrList));
    query.exec();
}

void Player::dbRemoveWearItem(int wltype)
{
    if(!(wltype >= WLG_BEGIN && wltype < WLG_END)){
        throw fflerror("bad wltype: %d", wltype);
    }
    g_dbPod->exec("delete from tbl_wear where fld_dbid = %llu and fld_wear = %llu", to_llu(dbid()), to_llu(wltype));
}

void Player::dbLearnMagic(uint32_t magicID)
{
    // tbl_learnedmagiclist:
    // +----------+-------------+---------+
    // | fld_dbid | fld_magicid | fld_exp |
    // +----------+-------------+---------+
    // |<-----primary key------>|

    fflassert(DBCOM_MAGICRECORD(magicID));
    g_dbPod->exec(
            u8R"###( insert into tbl_learnedmagiclist(fld_dbid, fld_magicid) )###"
            u8R"###( values                                                  )###"
            u8R"###(     (%llu, %llu)                                        )###",

            to_llu(dbid()),
            to_llu(magicID));
}

void Player::dbAddMagicExp(uint32_t magicID, size_t exp)
{
    fflassert(DBCOM_MAGICRECORD(magicID));
    if(exp > 0){
        g_dbPod->exec("update tbl_learnedmagiclist set fld_exp = fld_exp + %llu where fld_dbid = %llu and fld_magic = %zu", to_llu(dbid()), to_llu(magicID), exp);
    }
}

void Player::dbLoadLearnedMagic()
{
    // tbl_learnedmagiclist:
    // +----------+-----------+---------+
    // | fld_dbid | fld_magic | fld_exp |
    // +----------+-----------+---------+
    // |<----primary key----->|

    m_sdLearnedMagicList.clear();
    auto query = g_dbPod->createQuery("select * from tbl_learnedmagiclist where fld_dbid = %llu", to_llu(dbid()));

    while(query.executeStep()){
        m_sdLearnedMagicList.magicList.push_back(SDLearnedMagic
        {
            .magicID = check_cast<uint32_t, unsigned>(query.getColumn("fld_magicid")),
            .exp = query.getColumn("fld_exp"),
        });
    }
}

void Player::dbLoadFriendList()
{
    // tbl_friend:
    // +----------+------------+
    // | fld_dbid | fld_friend |
    // +----------+------------+
    // |          +------+
    // |                 |
    // |<--primary key-->|

    m_sdFriendList.clear();
    auto queryPlayer = g_dbPod->createQuery("select * from tbl_char where fld_dbid in (select fld_friend from tbl_friend where fld_dbid = %llu)", to_llu(dbid()));

    while(queryPlayer.executeStep()){
        m_sdFriendList.push_back(SDChatPeer
        {
            .id = queryPlayer.getColumn("fld_dbid"),
            .name = queryPlayer.getColumn("fld_name").getString(),

            .avatar = std::nullopt,
            .despvar = SDChatPeerPlayerVar
            {
                .gender = queryPlayer.getColumn("fld_gender").getUInt() > 0,
                .job = queryPlayer.getColumn("fld_job"),
            },
        });
    }

    auto queryChatGroup = g_dbPod->createQuery("select * from tbl_chatgroup where fld_id in (select fld_group from tbl_chatgroupmember where fld_member = %llu)", to_llu(dbid()));

    while(queryChatGroup.executeStep()){
        m_sdFriendList.push_back(SDChatPeer
        {
            .id = queryChatGroup.getColumn("fld_id"),
            .name = queryChatGroup.getColumn("fld_name").getString(),

            .avatar = std::nullopt,
            .despvar = SDChatPeerGroupVar
            {
                .creator = queryChatGroup.getColumn("fld_creator"),
                .createtime = to_u64(queryChatGroup.getColumn("fld_createtime").getInt64()),
            },
        });
    }
}

std::tuple<uint64_t, uint64_t> Player::dbSaveChatMessage(const SDChatPeerID &toCPID, const std::string_view &sv)
{
    auto tstamp= hres_tstamp::localtime();
    auto query = g_dbPod->createQuery(
        u8R"###( insert into tbl_chatmessage(fld_timestamp, fld_from, fld_to, fld_message) )###"
        u8R"###( values                                                                    )###"
        u8R"###(     (%llu, %llu, %llu, ?)                                                 )###"
        u8R"###( returning                                                                 )###"
        u8R"###(     fld_id;                                                               )###",

        to_llu(tstamp),
        to_llu(cpid().asU64()),
        to_llu(toCPID.asU64()));

    query.bindBlob(1, sv.data(), sv.size());
    if(query.executeStep()){
        return {to_u64(query.getColumn("fld_id").getInt64()), tstamp};
    }
    else{
        throw fflerror("failed to insert chat message to database");
    }
}

SDChatMessageList Player::dbRetrieveLatestChatMessage(const std::span<const uint64_t> &cpidList, size_t limitPerID, bool includeSend, bool includeRecv)
{
    if(cpidList.empty() || !(includeSend || includeRecv)){
        return {};
    }

    std::vector<std::string> queries;
    for(const auto other: cpidList){
        if(SDChatPeerID(other).group() && !findFriendChatPeer(SDChatPeerID(other))){
            continue;
        }

        queries.push_back("select * from ( select * from tbl_chatmessage where ");
        if(includeSend){
            queries.back().append(str_printf("(fld_from = %llu and fld_to = %llu) ", to_llu(cpid().asU64()), to_llu(other)));
        }

        if(includeRecv){
            if(includeSend){
                queries.back().append("or ");
            }

            if(SDChatPeerID(other).group()){
                queries.back().append(str_printf("(fld_to = %llu) ", to_llu(other)));
            }
            else{
                queries.back().append(str_printf("(fld_from = %llu and fld_to = %llu) ", to_llu(other), to_llu(cpid().asU64())));
            }
        }

        queries.back().append("order by fld_timestamp desc ");

        if(limitPerID > 0){
            queries.back().append(str_printf("limit %zu ", limitPerID));
        }

        queries.back().append(" )");
    }

    if(queries.empty()){
        return {};
    }

    SDChatMessageList result;
    auto query = g_dbPod->createQuery(str_join(queries, " union ").c_str());

    while(query.executeStep()){
        result.push_back(SDChatMessage
        {
            .seq = SDChatMessageDBSeq
            {
                .id        = to_u64(query.getColumn("fld_id").getInt64()),
                .timestamp = to_u64(query.getColumn("fld_timestamp").getInt64()),
            },

            .from = SDChatPeerID(to_u64(query.getColumn("fld_from").getInt64())),
            .to   = SDChatPeerID(to_u64(query.getColumn("fld_to"  ).getInt64())),

            .message = query.getColumn("fld_message").getString(),
        });
    }
    return result;
}

SDAddFriendNotif Player::dbAddFriend(uint32_t argDBID)
{
    g_dbPod->exec("delete from tbl_blacklist where fld_dbid = %llu and fld_blocked = %llu", to_llu(dbid()), to_llu(argDBID));

    auto query = g_dbPod->createQuery(
        u8R"###( insert or ignore into tbl_friend(fld_dbid, fld_friend) )###"
        u8R"###( values                                                 )###"
        u8R"###(     (%llu, %llu)                                       )###"
        u8R"###( returning                                              )###"
        u8R"###(     fld_dbid;                                          )###",

        to_llu(dbid()),
        to_llu(argDBID));

    if(query.executeStep()){
        return SDAddFriendNotif
        {
            .notif = AF_ACCEPTED,
        };
    }
    else{
        return SDAddFriendNotif
        {
            .notif = AF_EXIST,
        };
    }
}

SDAddBlockedNotif Player::dbAddBlocked(uint32_t argDBID)
{
    g_dbPod->exec("delete from tbl_friend where fld_dbid = %llu and fld_friend = %llu", to_llu(dbid()), to_llu(argDBID));

    auto query = g_dbPod->createQuery(
        u8R"###( insert or ignore into tbl_blacklist(fld_dbid, fld_blocked) )###"
        u8R"###( values                                                     )###"
        u8R"###(     (%llu, %llu)                                           )###"
        u8R"###( returning                                                  )###"
        u8R"###(     fld_dbid;                                              )###",

        to_llu(dbid()),
        to_llu(argDBID));

    if(query.executeStep()){
        return SDAddBlockedNotif
        {
            .notif = AB_DONE,
        };
    }
    else{
        return SDAddBlockedNotif
        {
            .notif = AB_EXIST,
        };
    }
}

SDChatPeer Player::dbCreateChatGroup(const char *name, const std::span<const uint32_t> &dbidList)
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


            to_llu(dbid()), // not cpid, for cross-table-reference
            to_llu(tstamp),
            name);

    if(query.executeStep()){
        SDChatPeer groupCP
        {
            .id = query.getColumn("fld_id"),
            .name = name,
            .despvar = SDChatPeerGroupVar
            {
                .creator = dbid(),
                .createtime = tstamp,
            },
        };

        bool foundSelf = false;
        std::string valStr;

        const auto fnAppendStr = [&valStr, &groupCP, tstamp, this](uint32_t memberDBID)
        {
            if(!valStr.empty()){
                valStr.append(",");
            }

            valStr.append(str_printf("(%llu, %llu, %llu, %llu)",
                to_llu(groupCP.id),
                to_llu(memberDBID),
                to_llu(0), // TODO - define group permission
                to_llu(tstamp)));
        };

        for(const auto memberDBID: dbidList){
            if(memberDBID == dbid()){
                foundSelf = true;
            }
            fnAppendStr(memberDBID);
        }

        if(!foundSelf){
            fnAppendStr(dbid());
        }

        auto addMemberQuery = g_dbPod->createQuery(
                u8R"###( insert into tbl_chatgroupmember(fld_group, fld_member, fld_permission, fld_jointime) )###"
                u8R"###( values                                                                               )###"
                u8R"###(     %s;                                                                              )###",

                valStr.c_str());

        addMemberQuery.executeStep();
        return groupCP;
    }
    else{
        throw fflerror("failed to create a group");
    }
}
