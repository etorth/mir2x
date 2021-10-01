/*
 * =====================================================================================
 *
 *       Filename: playerdb.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include "dbpod.hpp"
#include "player.hpp"
#include "monoserver.hpp"
#include "dbcomrecord.hpp"

extern DBPod *g_dbPod;
extern MonoServer *g_monoServer;

void Player::dbUpdateExp()
{
    g_dbPod->exec(u8R"###( update tbl_dbid set fld_exp = %llu where fld_dbid = %llu )###", to_llu(exp()), to_llu(dbid()));
}

void Player::dbUpdateMapGLoc()
{
    g_dbPod->exec(u8R"###( update tbl_dbid set fld_map = %d, fld_mapx = %d, fld_mapy = %d where fld_dbid = %llu )###", to_d(mapID()), X(), Y(), to_llu(dbid()));
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
            .extAttrList = cerealf::deserialize<SDItemExtAttrList>(query.getColumn("fld_extattrlist")),
        };

        fflassert(item);
        m_sdItemStorage.inventory.add(std::move(item), true);
    }
}

void Player::dbUpdateInventoryItem(const SDItem &item)
{
    fflassert(item);
    const auto attrBuf = cerealf::serialize(item.extAttrList);
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

    query.bind(1, attrBuf.data(), attrBuf.length());
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

    const auto attrBuf = cerealf::serialize(item.extAttrList);
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

    query.bind(1, attrBuf.data(), attrBuf.length());
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
            .extAttrList = cerealf::deserialize<SDItemExtAttrList>(query.getColumn("fld_extattrlist")),
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
            .extAttrList = cerealf::deserialize<SDItemExtAttrList>(query.getColumn("fld_extattrlist")),
        };

        fflassert(item);
        itemList.push_back(std::move(item));
    }
    return itemList;
}

void Player::dbLoadRuntimeConfig()
{
    // tbl_runtimeconfig:
    // +----------+----------+--------------+
    // | fld_dbid | fld_mute | fld_magickey |
    // +----------+----------+--------------+

    m_sdRuntimeConfig.clear();
    auto query = g_dbPod->createQuery("select * from tbl_runtimeconfig where fld_dbid = %llu", to_llu(dbid()));

    if(query.executeStep()){
        m_sdRuntimeConfig.mute = query.getColumn("fld_mute");
        m_sdRuntimeConfig.magicKeyList = cerealf::deserialize<SDMagicKeyList>(query.getColumn("fld_magickeylist"));
    }
}

void Player::dbUpdateMagicKey(uint32_t magicID, char key)
{
    fflassert(m_sdLearnedMagicList.has(magicID));
    fflassert((key >= 'a' && key <= 'z') || (key >= '0' && key <= '9'));

    if(!m_sdRuntimeConfig.magicKeyList.setMagicKey(magicID, key)){
        return;
    }

    const auto keyBuf = cerealf::serialize(m_sdRuntimeConfig.magicKeyList);
    auto query = g_dbPod->createQuery(
            u8R"###( replace into tbl_runtimeconfig(fld_dbid, fld_magickeylist) )###"
            u8R"###( values                                                     )###"
            u8R"###(     (%llu, ?)                                              )###",

            to_llu(dbid()));

    query.bind(1, keyBuf.data(), keyBuf.length());
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
            .extAttrList = cerealf::deserialize<SDItemExtAttrList>(query.getColumn("fld_extattrlist")),
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

    const auto attrBuf = cerealf::serialize(item.extAttrList);
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

    query.bind(1, attrBuf.data(), attrBuf.length());
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
