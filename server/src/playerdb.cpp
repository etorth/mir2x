#include <cstring>
#include <cctype>
#include <climits>
#include <algorithm>
#include <unordered_set>
#include "luaf.hpp"
#include "xmlf.hpp"
#include "mathf.hpp"
#include "dbpod.hpp"
#include "player.hpp"
#include "server.hpp"
#include "chatdb.hpp"
#include "dbcomid.hpp"
#include "sysconst.hpp"

extern DBPod *g_dbPod;
extern Server *g_server;

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

void Player::dbUpdatePKPoint()
{
    g_dbPod->exec(u8R"###( update tbl_char set fld_pkpoint = %llu where fld_dbid = %llu )###", to_llu(pkPoint()), to_llu(dbid()));
}

void Player::dbUpdateMapGLoc()
{
    if(uidf::isBaseMap(mapUID())){
        g_dbPod->exec(u8R"###( update tbl_char set fld_map = %d, fld_mapx = %d, fld_mapy = %d where fld_dbid = %llu )###", to_d(mapID()), X(), Y(), to_llu(dbid()));
    }
}

void Player::dbUpdateHealth()
{
    g_dbPod->exec(u8R"###( update tbl_char set fld_hp = %d, fld_mp = %d where fld_dbid = %llu )###", m_sdHealth.hp, m_sdHealth.mp, to_llu(dbid()));
}

void Player::dbSecureItem(uint32_t itemID, uint32_t seqID)
{
    const auto item = findInventoryItem(itemID, seqID);
    fflassert(item);

    auto query = g_dbPod->createQuery(
            u8R"###( replace into tbl_secureditemlist(fld_dbid, fld_itemid, fld_seqid, fld_count, fld_duration, fld_maxduration, fld_extattrlist) )###"
            u8R"###( values                                                                                                                       )###"
            u8R"###(     (%llu, %llu, %llu, %llu, %llu, %llu, ?)                                                                                  )###",

            to_llu(dbid()),
            to_llu(item->itemID),
            to_llu(item->seqID),
            to_llu(item->count),
            to_llu(item->duration[0]),
            to_llu(item->duration[1]));

    query.bindBlob(1, cerealf::serialize(item->extAttrList));
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
        auto item = SDItem::fromQuery(query);
        fflassert(!query.executeStep());
        return item;
    }
    throw fflpanic("can't find item: itemID = {}, seqID = {}", itemID, seqID);
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
        itemList.push_back(SDItem::fromQuery(query));
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
            throw fflpanic("invalid belt slot: {}", index);
        }

        const auto itemID = check_cast<uint32_t, unsigned>(query.getColumn("fld_itemid"));
        const auto typeStr = to_u8sv(DBCOM_ITEMRECORD(itemID).type);
        if(true
                && typeStr != u8"恢复药水"
                && typeStr != u8"传送卷轴"){
            throw fflpanic("invalid item type to belt slot");
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
        throw fflpanic("invalid belt slot: {}", slot);
    }

    fflassert(item);
    const auto typeStr = to_u8sv(DBCOM_ITEMRECORD(item.itemID).type);
    if(true
            && typeStr != u8"恢复药水"
            && typeStr != u8"传送卷轴"){
        throw fflpanic("invalid item type to belt slot");
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
        throw fflpanic("invalid belt slot: {}", slot);
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
    auto query = g_dbPod->createQuery("select *, 0 as fld_seqid from tbl_wear where fld_dbid = %llu", to_llu(dbid()));

    while(query.executeStep()){
        const auto wltype = check_cast<int, unsigned>(query.getColumn("fld_wear"));
        auto item = SDItem::fromQuery(query);

        if(!DBCOM_ITEMRECORD(item.itemID).wearable(wltype)){
            throw fflpanic("invalid item type to wear grid");
        }

        m_sdItemStorage.wear.setWLItem(wltype, std::move(item));
    }
}

void Player::dbUpdateWearItem(int wltype, const SDItem &item)
{
    if(!(wltype >= WLG_BEGIN && wltype < WLG_END)){
        throw fflpanic("bad wltype: {}", wltype);
    }

    fflassert(item);
    if(!DBCOM_ITEMRECORD(item.itemID).wearable(wltype)){
        throw fflpanic("invalid item type to wear grid");
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
        throw fflpanic("bad wltype: {}", wltype);
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
        g_dbPod->exec("update tbl_learnedmagiclist set fld_exp = fld_exp + %llu where fld_dbid = %llu and fld_magicid = %llu", to_llu(exp), to_llu(dbid()), to_llu(magicID));
    }
}

void Player::dbLoadLearnedMagic()
{
    // tbl_learnedmagiclist:
    // +----------+-------------+---------+
    // | fld_dbid | fld_magicid | fld_exp |
    // +----------+-------------+---------+
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

    for(auto &&chatGroup: dbLoadChatGroupList(dbid())){
        m_sdFriendList.push_back(std::move(chatGroup));
    }
}

std::expected<std::vector<SDItem>, int> Player::dbClaimDelivery(const std::string &record)
{
    fflassert(record.size() == SYS_DELIVERYRECORDSIZE, record);
    fflassert(std::ranges::all_of(record, [](unsigned char ch) -> bool { return std::isalnum(ch); }), record);

    std::string payload;
    {
        auto query = g_dbPod->createQuery(
            u8R"###( update tbl_delivery                                         )###"
            u8R"###(     set fld_claimed = 1, fld_claimtime = %llu               )###"
            u8R"###( where                                                       )###"
            u8R"###(     fld_record = ? and fld_dbid = %llu and fld_claimed = 0  )###"
            u8R"###( returning fld_payload;                                      )###",

            to_llu(hres_tstamp::localtime()),
            to_llu(dbid()));

        query.bind(1, record);
        if(query.executeStep()){
            payload = query.getColumn("fld_payload").getString();
        }
    }

    if(payload.empty()){
        auto query = g_dbPod->createQuery(u8R"###( select fld_claimed from tbl_delivery where fld_record = ? and fld_dbid = %llu )###", to_llu(dbid()));
        query.bind(1, record);

        if(query.executeStep() && query.getColumn("fld_claimed").getInt() != 0){
            return std::unexpected(CDERR_INVALID_CLAIM);
        }
        return std::unexpected(CDERR_INVALID_RECORD);
    }

    return cerealf::deserialize<std::vector<SDItem>>(payload);
}

bool Player::dbHasPlayer(uint32_t argDBID)
{
    return g_dbPod->createQuery("select fld_dbid from tbl_char where fld_dbid = %llu", to_llu(argDBID)).executeStep();
}

SDRuntimeConfig Player::dbGetRuntimeConfig(uint32_t argDBID)
{
    auto query = g_dbPod->createQuery("select * from tbl_playerconfig where fld_dbid = %llu", to_llu(argDBID));
    if(query.executeStep()){
        if(const std::string buf = query.getColumn("fld_runtimeconfig"); !buf.empty()){
            return cerealf::deserialize<SDRuntimeConfig>(buf);
        }
    }
    return SDRuntimeConfig {};
}

std::string Player::dbGetPlayerName(uint32_t argDBID)
{
    auto query = g_dbPod->createQuery("select fld_name from tbl_char where fld_dbid = %llu", to_llu(argDBID));
    if(query.executeStep()){
        return query.getColumn("fld_name");
    }
    throw fflpanic("invalid dbid: {}", argDBID);
}

bool Player::dbIsFriend(uint32_t argDBID, uint32_t argFriendDBID)
{
    return g_dbPod->createQuery("select fld_dbid from tbl_friend where fld_dbid = %llu and fld_friend = %llu", to_llu(argDBID), to_llu(argFriendDBID)).executeStep();
}

bool Player::dbIsBlocked(uint32_t argDBID, uint32_t argBlockedDBID)
{
    return g_dbPod->createQuery("select fld_dbid from tbl_blacklist where fld_dbid = %llu and fld_blocked = %llu", to_llu(argDBID), to_llu(argBlockedDBID)).executeStep();
}

int Player::dbAddFriend(uint32_t argDBID, uint32_t argFriendDBID)
{
    auto query = g_dbPod->createQuery(
        u8R"###( insert or ignore into tbl_friend(fld_dbid, fld_friend) )###"
        u8R"###( values                                                 )###"
        u8R"###(     (%llu, %llu)                                       )###"
        u8R"###( returning                                              )###"
        u8R"###(     fld_dbid;                                          )###",

        to_llu(argDBID),
        to_llu(argFriendDBID));

    if(query.executeStep()){
        return AF_ACCEPTED;
    }
    else{
        return AF_EXIST;
    }
}

int Player::dbBlockPlayer(uint32_t argDBID, uint32_t argPlayerDBID)
{
    int result = BP_NONE;
    auto dbTrans = g_dbPod->createTransaction();

    g_dbPod->exec("delete from tbl_friend where fld_dbid = %llu and fld_friend = %llu", to_llu(argDBID), to_llu(argPlayerDBID));
    {
        auto query = g_dbPod->createQuery(
            u8R"###( insert or ignore into tbl_blacklist(fld_dbid, fld_blocked) )###"
            u8R"###( values                                                     )###"
            u8R"###(     (%llu, %llu)                                           )###"
            u8R"###( returning                                                  )###"
            u8R"###(     fld_dbid;                                              )###",

            to_llu(argDBID),
            to_llu(argPlayerDBID));

        if(query.executeStep()){
            result = BP_DONE;
        }
        else{
            result = BP_EXIST;
        }
    }

    dbTrans.commit();
    return result;
}
