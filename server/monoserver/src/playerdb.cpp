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
            .itemID      = check_cast<uint32_t, unsigned>         (query.getColumn("fld_itemid"     )),
            .seqID       = check_cast<uint32_t, unsigned>         (query.getColumn("fld_seqid"      )),
            .count       = check_cast<  size_t, unsigned>         (query.getColumn("fld_count"      )),
            .duration    = check_cast<  size_t, unsigned>         (query.getColumn("fld_count"      )),
            .extAttrList = cerealf::deserialize<SDItemExtAttrList>(query.getColumn("fld_extattrlist")),
        };

        item.checkEx();
        m_sdItemStorage.inventory.add(std::move(item), true);
    }
}

void Player::dbUpdateInventoryItem(const SDItem &item)
{
    item.checkEx();
    const auto attrBuf = cerealf::serialize(item.extAttrList);
    auto query = g_dbPod->createQuery(
            u8R"###( replace into tbl_inventory(fld_dbid, fld_itemid, fld_seqid, fld_count, fld_duration, fld_extattrlist) )###"
            u8R"###( values                                                                                                )###"
            u8R"###(     (%llu, %llu, %llu, %llu, %llu, ?)                                                                 )###",

            to_llu(dbid()),
            to_llu(item.itemID),
            to_llu(item.seqID),
            to_llu(item.count),
            to_llu(item.duration));

    query.bind(1, attrBuf.data(), attrBuf.length());
    query.exec();
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

void Player::dbRemoveInventoryItem(const SDItem &item)
{
    dbRemoveInventoryItem(item.itemID, item.seqID);
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

        item.checkEx();
        m_sdItemStorage.belt.list.at(index) = std::move(item);
    }
}

void Player::dbUpdateBeltItem(size_t index, const SDItem &item)
{
    if(index >= 6){
        throw fflerror("invalid belt slot index: %zu", index);
    }

    item.checkEx();
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
            to_llu(index),
            to_llu(item.itemID),
            to_llu(item.count));
}

void Player::dbRemoveBeltItem(size_t index)
{
    if(index >= 6){
        throw fflerror("invalid belt slot index: %zu", index);
    }
    g_dbPod->exec("delete from tbl_belt where fld_dbid = %llu and fld_belt = %zu", to_llu(dbid()), index);
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
            .seqID = 0,
            .count = check_cast<size_t, unsigned>(query.getColumn("fld_count")),
            .duration = check_cast<size_t, unsigned>(query.getColumn("fld_duration")),
            .extAttrList = cerealf::deserialize<SDItemExtAttrList>(query.getColumn("fld_extattrlist")),
        };

        if(to_u8sv(DBCOM_ITEMRECORD(item.itemID).type) != wlGridItemType(wltype)){
            throw fflerror("invalid item type to wear grid");
        }

        item.checkEx();
        m_sdItemStorage.wear.setWLItem(wltype, std::move(item));
    }
}

void Player::dbUpdateWearItem(int wltype, const SDItem &item)
{
    if(!(wltype >= WLG_BEGIN && wltype < WLG_END)){
        throw fflerror("bad wltype: %d", wltype);
    }

    item.checkEx();
    if(to_u8sv(DBCOM_ITEMRECORD(item.itemID).type) != wlGridItemType(wltype)){
        throw fflerror("invalid item type to wear grid");
    }

    // only save itemID and wltype
    // drop the seqID when saving to database

    const auto attrBuf = cerealf::serialize(item.extAttrList);
    auto query = g_dbPod->createQuery(
            u8R"###( replace into tbl_wear(fld_dbid, fld_wear, fld_itemid, fld_count, fld_duration, fld_extattrlist) )###"
            u8R"###( values                                                                                          )###"
            u8R"###(     (%llu, %llu, %llu, %llu, %llu, ?)                                                           )###",

            to_llu(dbid()),
            to_llu(wltype),
            to_llu(item.itemID),
            to_llu(item.count),
            to_llu(item.duration));

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
