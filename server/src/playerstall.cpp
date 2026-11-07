#include <cstring>
#include <climits>
#include "uidsf.hpp"
#include "utf8f.hpp"
#include "player.hpp"
#include "dbcomid.hpp"
#include "actorpod.hpp"
#include "stalldb.hpp"

bool Player::stallBlockedCM(uint8_t cmType)
{
    // player can only chat, query, claim deliveries and manage the stall while stall is open
    switch(cmType){
        case CM_ACTION:
        case CM_BUY:
        case CM_CONSUMEITEM:
        case CM_DROPITEM:
        case CM_MAKEITEM:
        case CM_NPCEVENT:
        case CM_PICKUP:
        case CM_REQUESTEQUIPBELT:
        case CM_REQUESTEQUIPWEAR:
        case CM_REQUESTGRABBELT:
        case CM_REQUESTCONSUMEBELT:
        case CM_REQUESTGRABWEAR:
        case CM_REQUESTREVIVE:
        case CM_REQUESTRETRIEVESECUREDITEM:
        case CM_REQUESTSPACEMOVE:
        case CM_REQUESTMAGICDAMAGE:
        case CM_REGISTERAUCTIONITEM:
        case CM_REQUESTDIRECTTRADE:
        case CM_UPDATEDIRECTTRADE:
        case CM_COMMITDIRECTTRADE:
        case CM_ADDSTALLITEM:
        case CM_OPENSTALL:
            {
                return true;
            }
        default:
            {
                return false;
            }
    }
}

void Player::loadStall()
{
    // a stall left in setup state gets cancelled, items go back to inventory
    // an open stall restores, player spawns at the stall location since it can't move while the stall is open

    if(auto stall = dbLoadStall(dbid()); stall.has_value() && stall->selling && !stall->empty()){
        m_sdStall = std::move(stall.value());
    }
    else{
        if(stall.has_value()){
            dbCloseStall(dbid(), m_sdItemStorage.inventory);
        }
        m_sdStall.clear();
    }
    m_sdStall.dbid = dbid();
}

void Player::reloadStall()
{
    const bool wasSelling = stallSelling();

    m_sdStall = dbLoadStall(dbid()).value_or(SDStall{});
    m_sdStall.dbid = dbid();

    if(stallSelling() && m_sdStall.empty()){
        dbCloseStall(dbid(), m_sdItemStorage.inventory);
        m_sdStall.clear();
        m_sdStall.dbid = dbid();
    }

    postStall();
    if(wasSelling != stallSelling()){
        dispatchStallState();
    }
}

void Player::closeStall()
{
    auto inventory = m_sdItemStorage.inventory;
    const auto updatedItemList = dbCloseStall(dbid(), inventory);

    m_sdItemStorage.inventory = std::move(inventory);
    for(const auto &item: updatedItemList){
        reportUpdateItem(item);
    }
    reloadStall();
}

void Player::postStall()
{
    postNetMessage(SM_STALL, cerealf::serialize(m_sdStall));
}

void Player::postStallError(int error, uint64_t respID)
{
    postNetMessage(SM_STALLERROR, SMStallError
    {
        .error = to_u8(error),
    }, respID);
}

void Player::dispatchStallState()
{
    SMPlayerStall smPS;
    std::memset(&smPS, 0, sizeof(smPS));

    smPS.uid = UID();
    smPS.selling = stallSelling();
    dispatchNetPackage(true, SM_PLAYERSTALL, smPS);
}

void Player::dispatchOfflineStall()
{
    if(stallSelling()){
        if(hasActorPod()){
            m_actorPod->post(mapUID(), {AM_ADDOFFLINESTALL, cerealf::serialize(makeOfflineStall())});
        }
    }
    else if(!m_sdStall.empty()){
        closeStall();
    }
}

SDOfflineStall Player::makeOfflineStall() const
{
    return SDOfflineStall
    {
        .dbid = dbid(),

        .x = X(),
        .y = Y(),
        .direction = Direction(),

        .gender = gender(),
        .job = job(),

        .name = name(),
        .nameColor = nameColor(),

        .stallName = m_sdStall.name,
        .desp
        {
            .wear = m_sdItemStorage.wear,
            .hair = m_hair,
            .hairColor = m_hairColor,
        },
    };
}

corof::awaitable<> Player::on_AM_STALLSOLD(const ActorMsgPack &mpk)
{
    if(mpk.conv<AMStallSold>().sellerDBID == dbid()){
        reloadStall();
    }
    return {};
}

corof::awaitable<> Player::net_CM_ADDSTALLITEM(uint8_t, const uint8_t *buf, size_t, uint64_t)
{
    const auto cmASI = ClientMsg::conv<CMAddStallItem>(buf);
    const auto error = [&cmASI, this]() -> int
    {
        if(stallSelling()){
            return STALLERR_OPENED;
        }

        if(m_sdHealth.dead()){
            return STALLERR_DEAD;
        }

        if(directTradeBusy()){
            return STALLERR_BUSY;
        }

        if(cmASI.price == 0 || cmASI.price > to_u64(INT_MAX)){
            return STALLERR_BADPRICE;
        }

        if(!(DBCOM_ITEMRECORD(cmASI.itemID) && cmASI.seqID > 0)){
            return STALLERR_BADITEM;
        }

        const auto itemPtr = findInventoryItem(cmASI.itemID, cmASI.seqID);
        if(!itemPtr || itemPtr->isGold()){
            return STALLERR_BADITEM;
        }

        const auto item = *itemPtr; // keep a copy, remove() invalidates itemPtr
        if(const auto result = dbAddStallItem(dbid(), item, cmASI.price); !result.has_value()){
            return result.error();
        }

        const auto [removedCount, removedSeqID, removedItemPtr] = m_sdItemStorage.inventory.remove(item.itemID, item.seqID, item.count, true);
        fflassert(removedCount == item.count, removedCount, item.count);
        fflassert(removedSeqID == item.seqID, removedSeqID, item.seqID);
        fflassert(!removedItemPtr);

        reportRemoveItem(item.itemID, item.seqID, item.count);
        return STALLERR_NONE;
    }();

    if(error != STALLERR_NONE){
        postStallError(error);
    }

    reloadStall();
    return {};
}

corof::awaitable<> Player::net_CM_REMOVESTALLITEM(uint8_t, const uint8_t *buf, size_t, uint64_t)
{
    const auto cmRSI = ClientMsg::conv<CMRemoveStallItem>(buf);

    auto inventory = m_sdItemStorage.inventory;
    if(const auto result = dbRemoveStallItem(dbid(), inventory, cmRSI.itemID, cmRSI.seqID); result.has_value()){
        m_sdItemStorage.inventory = std::move(inventory);
        reportUpdateItem(result.value());
    }
    else{
        postStallError(result.error());
    }

    reloadStall();
    return {};
}

corof::awaitable<> Player::net_CM_OPENSTALL(uint8_t, const uint8_t *buf, size_t, uint64_t)
{
    const auto cmOS = ClientMsg::conv<CMOpenStall>(buf);
    const auto error = [&cmOS, this]() -> int
    {
        if(stallSelling()){
            return STALLERR_OPENED;
        }

        if(m_sdHealth.dead()){
            return STALLERR_DEAD;
        }

        if(directTradeBusy()){
            return STALLERR_BUSY;
        }

        // TODO stall should be allowed in safe zone only, but no safe zone is configurable yet
        // only base map supported, offline stall gets restored by mapID
        if(!uidf::isBaseMap(mapUID())){
            return STALLERR_NOTALLOWED;
        }

        if(m_sdStall.empty()){
            return STALLERR_EMPTY;
        }

        if(cmOS.name.size == 0 || cmOS.name.size >= cmOS.name.capacity()){
            return STALLERR_BADNAME;
        }

        const auto name = cmOS.name.to_str();
        if(!utf8f::valid(name)){
            return STALLERR_BADNAME;
        }

        m_sdStall.name = name;
        if(!dbOpenStall(dbid(), name, mapID(), X(), Y(), makeOfflineStall())){
            return STALLERR_DBFAILED;
        }

        dbUpdateMapGLoc();
        return STALLERR_NONE;
    }();

    if(error != STALLERR_NONE){
        postStallError(error);
    }

    reloadStall();

    if(stallSelling()){
        const auto stand = makeActionStand();
        dispatchAction(stand);
        reportAction(UID(), mapUID(), stand);
    }
    return {};
}

corof::awaitable<> Player::net_CM_CLOSESTALL(uint8_t, const uint8_t *, size_t, uint64_t)
{
    closeStall();
    return {};
}

corof::awaitable<> Player::net_CM_QUERYSTALL(uint8_t, const uint8_t *buf, size_t, uint64_t)
{
    const auto cmQS = ClientMsg::conv<CMQueryStall>(buf);
    if(cmQS.dbid == 0 || cmQS.dbid > SYS_MAXDBID){
        co_return;
    }

    // buyer can only see an open stall
    // a stall not selling is reported as selling = false, without its items
    auto stall = dbLoadStall(cmQS.dbid).value_or(SDStall{});
    stall.dbid = cmQS.dbid;

    if(!stall.selling){
        stall.clear();
        stall.dbid = cmQS.dbid;
    }
    postNetMessage(SM_QUERYSTALLOK, cerealf::serialize(stall));
}

corof::awaitable<> Player::net_CM_BUYSTALLITEM(uint8_t, const uint8_t *buf, size_t, uint64_t)
{
    const auto cmBSI = ClientMsg::conv<CMBuyStallItem>(buf);
    const auto error = [&cmBSI, this]() -> int
    {
        if(m_sdHealth.dead()){
            return STALLERR_DEAD;
        }

        if(!uidf::isBaseMap(mapUID())){
            return STALLERR_NOTALLOWED;
        }

        auto inventory = m_sdItemStorage.inventory;
        auto result = dbBuyStallItem(dbid(), inventory, cmBSI.sellerDBID, mapID(), X(), Y(), cmBSI.itemID, cmBSI.seqID, cmBSI.count, cmBSI.price);

        if(!result.has_value()){
            return result.error();
        }

        m_sdItemStorage.inventory = std::move(inventory);
        setGold(result->buyerGold);
        reportUpdateItem(result->buyerItem);

        const auto sellerUID = uidf::getPlayerUID(cmBSI.sellerDBID);
        forwardNetPackage(sellerUID, SM_CHATMESSAGELIST, cerealf::serialize(SDChatMessageList
        {
            std::move(result->sellerMessage),
        }));

        // seller reloads its stall if online
        // map drops the offline stall if sold out, it's no-op if seller is online
        AMStallSold amSS;
        std::memset(&amSS, 0, sizeof(amSS));

        amSS.sellerDBID = cmBSI.sellerDBID;
        m_actorPod->post(sellerUID, {AM_STALLSOLD, amSS});

        if(result->empty){
            AMRemoveOfflineStall amROS;
            std::memset(&amROS, 0, sizeof(amROS));

            amROS.dbid = cmBSI.sellerDBID;
            m_actorPod->post(mapUID(), {AM_REMOVEOFFLINESTALL, amROS});
        }
        return STALLERR_NONE;
    }();

    if(error != STALLERR_NONE){
        postStallError(error);
    }

    // always refresh buyer's view of the stall
    CMQueryStall cmQS;
    std::memset(&cmQS, 0, sizeof(cmQS));

    cmQS.dbid = cmBSI.sellerDBID;
    co_await net_CM_QUERYSTALL(CM_QUERYSTALL, reinterpret_cast<const uint8_t *>(&cmQS), sizeof(cmQS), 0);
}
