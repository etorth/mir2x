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

#include "npchar.hpp"
#include "mathf.hpp"
#include "serdesmsg.hpp"
#include "messagepack.hpp"
#include "dbcomrecord.hpp"

void NPChar::on_MPK_ACTION(const MessagePack &mpk)
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

void NPChar::on_MPK_NPCEVENT(const MessagePack &mpk)
{
    if(!mpk.from()){
        throw fflerror("NPC event comes from zero uid");
    }

    const auto amNPCE = mpk.conv<AMNPCEvent>();

    // when CO initiatively sends a message to NPC, we assume it's UID is the sessionUID
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

    if(amNPCE.mapID != MapID() || mathf::LDistance2(amNPCE.x, amNPCE.y, X(), Y()) >= SYS_MAXNPCDISTANCE * SYS_MAXNPCDISTANCE){
        AMNPCError amNPCE;
        std::memset(&amNPCE, 0, sizeof(amNPCE));

        amNPCE.errorID = NPCE_TOOFAR;
        m_actorPod->forward(mpk.from(), {MPK_NPCERROR, amNPCE});

        m_luaModulePtr->close(mpk.from());
        return;
    }
    m_luaModulePtr->setEvent(mpk.from(), mpk.from(), amNPCE.event, amNPCE.value);
}

void NPChar::on_MPK_NOTIFYNEWCO(const MessagePack &mpk)
{
    if(uidf::getUIDType(mpk.from()) == UID_PLY){
        dispatchAction(mpk.from(), makeActionStand());
    }
}

void NPChar::on_MPK_QUERYCORECORD(const MessagePack &mpk)
{
    const auto fromUID = mpk.conv<AMQueryCORecord>().UID;
    if(uidf::getUIDType(fromUID) != UID_PLY){
        throw fflerror("NPC get AMQueryCORecord from %s", uidf::getUIDTypeString(fromUID));
    }
    dispatchAction(fromUID, makeActionStand());
}

void NPChar::on_MPK_QUERYLOCATION(const MessagePack &mpk)
{
    AMLocation amL;
    std::memset(&amL, 0, sizeof(amL));

    amL.UID       = UID();
    amL.MapID     = MapID();
    amL.X         = X();
    amL.Y         = Y();
    amL.Direction = Direction();

    m_actorPod->forward(mpk.from(), {MPK_LOCATION, amL}, mpk.ID());
}

void NPChar::on_MPK_QUERYSELLITEM(const MessagePack &mpk)
{
    const auto amQSI = mpk.conv<AMQuerySellItem>();
    SDSellItem sdSI;

    sdSI.itemID = amQSI.itemID;
    if(DBCOM_ITEMRECORD(sdSI.itemID).hasDBID()){
        for(size_t i = 0, count = 20 + std::rand() % 100; i < count; ++i){
            sdSI.list.data.push_back(
            {
                .price = (uint32_t)(20 + std::rand() % 100),
            });
        }
    }
    else{
        sdSI.single.price = 100 + std::rand() % 20;
    }
    sendNetPackage(mpk.from(), SM_SELLITEM, cerealf::serialize<SDSellItem>(sdSI, true));
}

void NPChar::on_MPK_BADACTORPOD(const MessagePack &mpk)
{
    const auto amBAP = mpk.conv<AMBadActorPod>();
    m_luaModulePtr->close(amBAP.UID);
}
