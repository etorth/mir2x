/*
 * =====================================================================================
 *
 *       Filename: npcharop.cpp
 *        Created: 04/12/2020 16:27:40
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

#include "mathf.hpp"
#include "npchar.hpp"
#include "dbcomid.hpp"
#include "serdesmsg.hpp"
#include "dbcomrecord.hpp"
#include "actormsgpack.hpp"

void NPChar::on_AM_ACTION(const ActorMsgPack &mpk)
{
    const auto amA = mpk.conv<AMAction>();
    switch(uidf::getUIDType(amA.UID)){
        case UID_PLY:
        case UID_MON:
            {
                dispatchAction(amA.UID, makeActionStand());
                break;
            }
        default:
            {
                break;
            }
    }
}

void NPChar::on_AM_NPCEVENT(const ActorMsgPack &mpk)
{
    if(!mpk.from()){
        throw fflerror("NPC event comes from zero uid");
    }

    const auto amNPCE = mpk.conv<AMNPCEvent>();

    // when CO initiatively sends a message to NPC, we assume it's UID is the callStackUID
    // when NPC querys CO attributes the response should be handled in actor response handler, not here

    if(false
            || std::string(amNPCE.event) == SYS_NPCDONE
            || std::string(amNPCE.event) == SYS_NPCERROR){
        m_luaModulePtr->close(mpk.from());
        return;
    }

    if(std::string(amNPCE.event) == SYS_NPCQUERY){
        throw fflerror("unexcepted NPC event: event = %s, value = %s", to_cstr(amNPCE.event), to_cstr(amNPCE.value));
    }

    // can be SYS_NPCINIT or scritp event
    // script event defines like text button pressed etc

    if(amNPCE.mapID != mapID() || mathf::LDistance2(amNPCE.x, amNPCE.y, X(), Y()) >= SYS_MAXNPCDISTANCE * SYS_MAXNPCDISTANCE){
        AMNPCError amNPCE;
        std::memset(&amNPCE, 0, sizeof(amNPCE));

        amNPCE.errorID = NPCE_TOOFAR;
        m_actorPod->forward(mpk.from(), {AM_NPCERROR, amNPCE});

        m_luaModulePtr->close(mpk.from());
        return;
    }

    // last call stack has not been done yet
    // but player initializes new call stack, have to abandon last call stack and start a new one

    if(m_luaModulePtr->getCallStackSeqID(mpk.from())){
        m_luaModulePtr->close(mpk.from());
    }

    m_luaModulePtr->setEvent(mpk.from(), mpk.from(), amNPCE.event, amNPCE.value);
}

void NPChar::on_AM_NOTIFYNEWCO(const ActorMsgPack &mpk)
{
    if(uidf::getUIDType(mpk.from()) == UID_PLY){
        dispatchAction(mpk.from(), makeActionStand());
    }
}

void NPChar::on_AM_QUERYCORECORD(const ActorMsgPack &mpk)
{
    const auto fromUID = mpk.conv<AMQueryCORecord>().UID;
    if(uidf::getUIDType(fromUID) != UID_PLY){
        throw fflerror("NPC get AMQueryCORecord from %s", uidf::getUIDTypeCStr(fromUID));
    }
    dispatchAction(fromUID, makeActionStand());
}

void NPChar::on_AM_QUERYLOCATION(const ActorMsgPack &mpk)
{
    AMLocation amL;
    std::memset(&amL, 0, sizeof(amL));

    amL.UID       = UID();
    amL.mapID     = mapID();
    amL.X         = X();
    amL.Y         = Y();
    amL.Direction = Direction();

    m_actorPod->forward(mpk.from(), {AM_LOCATION, amL}, mpk.seqID());
}

void NPChar::on_AM_QUERYSELLITEMLIST(const ActorMsgPack &mpk)
{
    // 
    // query all selling items for one specified itemID

    const auto amQSIL = mpk.conv<AMQuerySellItemList>();
    SDSellItemList sdSIL;
    sdSIL.npcUID = UID();
    if(DBCOM_ITEMRECORD(amQSIL.itemID).packable()){
        sdSIL.list.push_back(SDSellItem
        {
            .item     = m_sellItemList.at(amQSIL.itemID).at(0).item,
            .costList = m_sellItemList.at(amQSIL.itemID).at(0).costList,
        });
    }
    else{
        if(auto p = m_sellItemList.find(amQSIL.itemID); p != m_sellItemList.end()){
            for(const auto &[seqID, sellItem]: p->second){
                sdSIL.list.push_back(SDSellItem
                {
                    .item = sellItem.item,
                    .costList = sellItem.costList,
                });
            }
        }
    }
    forwardNetPackage(mpk.from(), SM_SELLITEMLIST, cerealf::serialize(sdSIL, true));
}

void NPChar::on_AM_BADACTORPOD(const ActorMsgPack &mpk)
{
    const auto amBAP = mpk.conv<AMBadActorPod>();
    m_luaModulePtr->close(amBAP.UID);
}

void NPChar::on_AM_BUY(const ActorMsgPack &mpk)
{
    const auto fnSendBuyError = [&mpk, this](int buyError)
    {
        AMBuyError amBE;
        std::memset(&amBE, 0, sizeof(amBE));
        amBE.error = buyError;
        m_actorPod->forward(mpk.from(), {AM_BUYERROR, amBE});
    };

    const auto amB = mpk.conv<AMBuy>();
    const auto &ir = DBCOM_ITEMRECORD(amB.itemID);
    if(!ir){
        throw fflerror("invalid itemID = %llu", to_llu(amB.itemID));
    }

    if(!ir.packable() && amB.count > 1){
        throw fflerror("buying multiple unpackable items");
    }

    auto p = m_sellItemList.find(amB.itemID);
    if(p == m_sellItemList.end()){
        fnSendBuyError(BUYERR_BADITEM);
        return;
    }

    auto q = p->second.find(ir.packable() ? 0 : amB.seqID);
    if(q == p->second.end()){
        fnSendBuyError(BUYERR_SOLDOUT);
        return;
    }

    if(q->second.locked){
        fnSendBuyError(BUYERR_LOCKED);
        return;
    }

    SDBuyCost sdBC;
    sdBC.item = q->second.item;
    sdBC.item.count = amB.count;
    sdBC.costList = q->second.costList; // TODO: for packable cost = itemCost x count

    if(!ir.packable()){
        q->second.locked = true;
    }

    m_actorPod->forward(mpk.from(), {AM_BUYCOST, cerealf::serialize(sdBC)}, mpk.seqID(), [amB, this](const ActorMsgPack &rmpk)
    {
        const auto &ir = DBCOM_ITEMRECORD(amB.itemID);
        if(!ir){
            throw fflerror("invalid itemID = %llu", to_llu(amB.itemID));
        }

        auto p = m_sellItemList.find(amB.itemID);
        if(p == m_sellItemList.end()){
            throw fflerror("no item selling: itemID = %llu", to_llu(amB.itemID));
        }

        auto q = p->second.find(ir.packable() ? 0 : amB.seqID);
        if(q == p->second.end()){
            throw fflerror("no item selling: itemID = %llu, seqID = %llu", to_llu(amB.itemID), to_llu(amB.seqID));
        }

        if(!ir.packable()){
            if(!q->second.locked){
                throw fflerror("item lock released before get response");
            }
            q->second.locked = false;
        }

        switch(rmpk.type()){
            case AM_OK:
                {
                    if(!ir.packable()){
                        p->second.erase(q);
                    }
                    return;
                }
            case AM_ERROR:
                {
                    return;
                }
            default:
                {
                    return;
                }
        }
    });
}
