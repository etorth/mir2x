/*
 * =====================================================================================
 *
 *       Filename: npcop.cpp
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

#include "npc.hpp"
#include "mathf.hpp"
#include "messagepack.hpp"

void NPC::On_MPK_NPCEVENT(const MessagePack &msg)
{
    const auto event = msg.conv<AMNPCEvent>();
    if(event.mapID != mapID() || mathf::LDistance2(event.x, event.y, X(), Y()) >= SYS_MAXNPCDISTANCE){
        AMNPCError amNPCE;
        std::memset(&amNPCE, 0, sizeof(amNPCE));

        amNPCE.errorID = NPCE_TOOFAR;
        m_actorPod->Forward(msg.From(), {MPK_NPCERROR, amNPCE});
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
    m_actorPod->Forward(msg.From(), {MPK_NPCERROR, amNPCE});
}
