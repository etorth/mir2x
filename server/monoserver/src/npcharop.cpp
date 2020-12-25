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
#include "messagepack.hpp"

void NPChar::on_MPK_ACTION(const MessagePack &mpk)
{
    const auto amA = mpk.conv<AMAction>();
    switch(uidf::getUIDType(amA.UID)){
        case UID_PLY:
        case UID_MON:
            {
                dispatchAction(amA.UID, ActionStand
                {
                    .x = X(),
                    .y = Y(),
                    .direction = Direction(),
                });
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

    // for NPC query we don't check location
    // it's sent by NPC itself

    if(std::string(amNPCE.event) == SYS_NPCQUERY || std::string(amNPCE.event) == SYS_NPCDONE){
        m_luaModulePtr->setEvent(mpk.from(), amNPCE.event, amNPCE.value);
        return;
    }

    if(amNPCE.mapID != MapID() || mathf::LDistance2(amNPCE.x, amNPCE.y, X(), Y()) >= SYS_MAXNPCDISTANCE * SYS_MAXNPCDISTANCE){
        AMNPCError amNPCE;
        std::memset(&amNPCE, 0, sizeof(amNPCE));

        amNPCE.errorID = NPCE_TOOFAR;
        m_actorPod->forward(mpk.from(), {MPK_NPCERROR, amNPCE});

        m_luaModulePtr->close(mpk.from());
        return;
    }
    m_luaModulePtr->setEvent(mpk.from(), amNPCE.event, amNPCE.value);
}

void NPChar::on_MPK_NOTIFYNEWCO(const MessagePack &mpk)
{
    if(uidf::getUIDType(mpk.from()) == UID_PLY){
        dispatchAction(mpk.from(), ActionStand
        {
            .x = X(),
            .y = Y(),
            .direction = Direction(),
        });
    }
}

void NPChar::on_MPK_QUERYCORECORD(const MessagePack &mpk)
{
    const auto fromUID = mpk.conv<AMQueryCORecord>().UID;
    if(uidf::getUIDType(fromUID) != UID_PLY){
        throw fflerror("NPC get AMQueryCORecord from %s", uidf::getUIDTypeString(fromUID));
    }
    dispatchAction(fromUID, ActionStand
    {
        .x = X(),
        .y = Y(),
        .direction = Direction(),
    });
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

void NPChar::on_MPK_BADACTORPOD(const MessagePack &mpk)
{
    const auto amBAP = mpk.conv<AMBadActorPod>();
    m_luaModulePtr->close(amBAP.UID);
}
