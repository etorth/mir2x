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

void NPChar::On_MPK_ACTION(const MessagePack &mpk)
{
    const auto amA = mpk.conv<AMAction>();
    switch(uidf::getUIDType(amA.UID)){
        case UID_PLY:
        case UID_MON:
            {
                DispatchAction(amA.UID, ActionStand(X(), Y(), Direction()));
            }
        default:
            {
                break;
            }
    }
}

void NPChar::On_MPK_NPCEVENT(const MessagePack &mpk)
{
    if(!mpk.from()){
        throw fflerror("NPC event comes from zero uid");
    }

    const auto amNPCE = mpk.conv<AMNPCEvent>();

    // for NPC query we don't check location
    // it's sent by NPC itself

    if(std::string(amNPCE.event) == SYS_NPCQUERY || std::string(amNPCE.event) == SYS_NPCDONE){
        m_luaModule.setEvent(mpk.from(), amNPCE.event, amNPCE.value);
        return;
    }

    if(amNPCE.mapID != MapID() || mathf::LDistance2(amNPCE.x, amNPCE.y, X(), Y()) >= SYS_MAXNPCDISTANCE * SYS_MAXNPCDISTANCE){
        AMNPCError amNPCE;
        std::memset(&amNPCE, 0, sizeof(amNPCE));

        amNPCE.errorID = NPCE_TOOFAR;
        m_actorPod->forward(mpk.from(), {MPK_NPCERROR, amNPCE});

        m_luaModule.close(mpk.from());
        return;
    }
    m_luaModule.setEvent(mpk.from(), amNPCE.event, amNPCE.value);
}

void NPChar::On_MPK_NOTIFYNEWCO(const MessagePack &mpk)
{
    if(uidf::getUIDType(mpk.from()) == UID_PLY){
        DispatchAction(mpk.from(), ActionStand(X(), Y(), Direction()));
    }
}

void NPChar::On_MPK_QUERYCORECORD(const MessagePack &mpk)
{
    const auto fromUID = mpk.conv<AMQueryCORecord>().UID;
    if(uidf::getUIDType(fromUID) != UID_PLY){
        throw fflerror("NPC get AMQueryCORecord from %s", uidf::getUIDTypeString(fromUID));
    }
    DispatchAction(fromUID, ActionStand(X(), Y(), Direction()));
}

void NPChar::On_MPK_QUERYLOCATION(const MessagePack &mpk)
{
    AMLocation stAML;
    std::memset(&stAML, 0, sizeof(stAML));

    stAML.UID       = UID();
    stAML.MapID     = MapID();
    stAML.X         = X();
    stAML.Y         = Y();
    stAML.Direction = Direction();

    m_actorPod->forward(mpk.from(), {MPK_LOCATION, stAML}, mpk.ID());
}
