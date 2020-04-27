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
    const auto stAMA = mpk.conv<AMAction>();
    if(uidf::getUIDType(stAMA.UID) == UID_PLY){
        DispatchAction(stAMA.UID, ActionStand(X(), Y(), Direction()));
    }
}

void NPChar::On_MPK_NPCEVENT(const MessagePack &msg)
{
    const auto event = msg.conv<AMNPCEvent>();
    if(event.mapID != mapID() || mathf::LDistance2(event.x, event.y, X(), Y()) >= SYS_MAXNPCDISTANCE){
        AMNPCError amNPCE;
        std::memset(&amNPCE, 0, sizeof(amNPCE));

        amNPCE.errorID = NPCE_TOOFAR;
        m_actorPod->forward(msg.From(), {MPK_NPCERROR, amNPCE});
        return;
    }

    if(auto p = m_onEventID.find(event.eventID); p != m_onEventID.end()){
        if(p->second){
            p->second(msg.From(), event);
            return;
        }
        throw fflerror("invalid handler registered for eventID: %lld", toLLD(event.eventID));
    }

    AMNPCError amNPCE;
    std::memset(&amNPCE, 0, sizeof(amNPCE));

    amNPCE.errorID = NPCE_BADEVENTID;
    m_actorPod->forward(msg.From(), {MPK_NPCERROR, amNPCE});
}

void NPChar::On_MPK_NOTIFYNEWCO(const MessagePack &mpk)
{
    if(uidf::getUIDType(mpk.From()) == UID_PLY){
        DispatchAction(mpk.From(), ActionStand(X(), Y(), Direction()));
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

    m_actorPod->forward(mpk.From(), {MPK_LOCATION, stAML}, mpk.ID());
}
