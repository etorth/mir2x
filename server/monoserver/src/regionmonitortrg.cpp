/*
 * =====================================================================================
 *
 *       Filename: regionmonitortrg.cpp
 *        Created: 05/06/2016 17:39:36
 *  Last Modified: 05/07/2016 00:47:35
 *
 *    Description: The first place I am thinking of using trigger or not.
 *                 
 *                 GOOD:
 *                 trigger can simplify logic of handling messages, we only need to
 *                 make flags of the object when we get new message.
 *
 *                 DRAWBACK:
 *                 we split the logic of message handling and operation caused by
 *                 message. and currently I haven't a rule to apply trigger
 *
 *                 current rule:
 *                 1. if an operation only depends on current actor, don't do it in
 *                    the trigger system
 *                 2. if an operation depends on current actor and information from
 *                    another actor, do it with message callback. i.e.
 *
 *                      // 1. get message rstMPK
 *                      // 2. define message callback
 *                      auto fnROP = [rstMPK](MessagePack &rstRMPK, Theron::Address &){
 *                          // do something based on rstRMPK
 *                      };
 *
 *                      m_ActorPod->Forward(..., fnROP);
 *
 *                 3. depends current actors and more than one other actor, ok we have
 *                    to use trigger, one example is one try to move
 *
 *                    1. tell the RM for move request
 *                    2. RM query its 8 neighbors
 *                    3. here we need 8 neighbors information
 *
 *                 4. depends on two or more message, use trigger
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
#include "actorpod.hpp"
#include "monoserver.hpp"
#include "reactobject.hpp"
#include "regionmonitor.hpp"

// trigger function for move request, this logic only handles
// the ``get-in" move since it takes the affect of 8 neighbors
void RegionMonitor::For_MoveRequest()
{
    if(!m_MoveRequest.Valid()){ return; }

    // only handle the ``get-in" move
    if(!m_MoveRequest.In){ return; }

    bool bCancel = false;
    for(int nY = 0; nY < 3; ++nY){
        for(int nX = 0; nX < 3; ++nX){
            if(m_NeighborV2D[nY][nX].Query == QUERY_ERROR){
                bCancel = true;
                goto __REGIONMONITOR_FOR_MOVEREQUEST_DONE_1;
            }
        }
    }

__REGIONMONITOR_FOR_MOVEREQUEST_DONE_1:
    bool bPending = false;
    for(int nY = 0; nY < 3; ++nY){
        for(int nX = 0; nX < 3; ++nX){
            switch(m_NeighborV2D[nY][nX].Query){
                case QUERY_PENDING:
                    {
                        bPending = true;
                        break;
                    }
                case QUERY_ERROR:
                    {
                        // do nothing since it's already an error
                        break;
                    }
                case QUERY_OK:
                    {
                        if(bCancel){
                            m_ActorPod->Forward(MPK_ERROR,
                                    m_NeighborV2D[nY][nX].PodAddress, m_NeighborV2D[nY][nX].MPKID);
                            m_NeighborV2D[nY][nX].Query = QUERY_ERROR;
                        }
                        break;
                    }
                case QUERY_NA:
                default:
                    {
                        break;
                    }
            }
        }
    }

    // done our job for current status
    if(bPending){ return; }

    // no pending state, then
    if(bCancel){
        // if this is an ADDMONSTER then the address is from the ServerMap
        // otherwise it's from the requesting object
        m_ActorPod->Forward(MPK_ERROR, m_MoveRequest.PodAddress, m_MoveRequest.MPKID);
        m_MoveRequest.Clear();
        return;
    }

    // aha, we are now grant permission the object to move, finally
    if(m_MoveRequest.Data){
        // it's adding new object into current region
        CharObjectRecord stCORecord;
        stCORecord.X = m_MoveRequest.X;
        stCORecord.Y = m_MoveRequest.Y;
        stCORecord.R = m_MoveRequest.R;

        extern MonoServer *g_MonoServer;

        stCORecord.UID = g_MonoServer->GetUID();
        stCORecord.AddTime = g_MonoServer->GetTickCount();
        stCORecord.PodAddress = ((ReactObject *)m_MoveRequest.Data)->Activate();

        m_CharObjectRecordV.push_back(stCORecord);

        // we respond to ServerMap, but it won't respond again
        m_ActorPod->Forward(MPK_OK, m_MoveRequest.PodAddress, m_MoveRequest.MPKID);
        m_MoveRequest.Clear();
    }else{
        // it's an object requesting to move into current region
        // when we grant the permission, the object should respond to it
        auto fnROP = [this](const MessagePack &rstRMPK, const Theron::Address &){
            if(rstRMPK.Type() == MPK_OK){
                // object picked this chance to move
                for(auto &rstRecord: m_CharObjectRecordV){
                    if(true
                            && rstRecord.UID == m_MoveRequest.UID
                            && rstRecord.AddTime == m_MoveRequest.AddTime){
                        rstRecord.X = m_MoveRequest.X;
                        rstRecord.Y = m_MoveRequest.Y;
                        break;
                    }
                }
            }
            m_MoveRequest.Clear();
        };
        m_ActorPod->Forward(MPK_OK, m_MoveRequest.PodAddress, m_MoveRequest.MPKID, fnROP);
    }
}

void RegionMonitor::For_Update()
{
}
