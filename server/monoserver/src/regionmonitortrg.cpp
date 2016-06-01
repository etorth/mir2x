/*
 * =====================================================================================
 *
 *       Filename: regionmonitortrg.cpp
 *        Created: 05/06/2016 17:39:36
 *  Last Modified: 05/31/2016 18:45:54
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
#include "player.hpp"
#include "monster.hpp"
#include "actorpod.hpp"
#include "monoserver.hpp"
#include "charobject.hpp"
#include "reactobject.hpp"
#include "regionmonitor.hpp"

void RegionMonitor::For_MoveRequest()
{
    // make sure that we do need the trigger functionality
    // we only need it when doing *cover check* for neighbors
    if(!m_MoveRequest.Freezed() || !m_MoveRequest.NeighborCheck){ return; }

    bool bCancel  = false;
    bool bPending = false;

    for(int nY = 0; nY < 3; ++nY){
        for(int nX = 0; nX < 3; ++nX){
            bCancel  = (bCancel  || m_NeighborV2D[nY][nX].Query == QUERY_ERROR);
            bPending = (bPending || m_NeighborV2D[nY][nX].Query == QUERY_PENDING);
        }
    }

    // wait until all response have been got, even we already get errors, we
    // have to wait since we can't cancel those request already sent
    if(bPending){ return; }

    // no pending now and we found errors
    if(bCancel){
        for(int nY = 0; nY < 3; ++nY){
            for(int nX = 0; nX < 3; ++nX){
                // cancel all freezed neighbors
                if(m_NeighborV2D[nY][nX].Valid() && (m_NeighborV2D[nY][nX].Query == QUERY_OK)){
                    m_ActorPod->Forward(MPK_ERROR,
                            m_NeighborV2D[nY][nX].PodAddress, m_NeighborV2D[nY][nX].MPKID);
                    m_NeighborV2D[nY][nX].Query = QUERY_NA;
                }
            }
        }

        // reject the object 
        m_ActorPod->Forward(MPK_ERROR, m_MoveRequest.PodAddress, m_MoveRequest.MPKID);
        m_MoveRequest.Clear();
        return;
    }

    // aha, we are now grant permission the object to move / add, finally
    // need to check this is a new obj or an existing obj?

    if(!(m_MoveRequest.UID && m_MoveRequest.AddTime)){
        // it's adding new object into current region
        CharObject *pCharObject = nullptr;
        switch(m_MoveRequest.Type){
            case OBJECT_MONSTER:
                {
                    pCharObject = new Monster(m_MoveRequest.Monster.MonsterID);
                    break;
                }
            case OBJECT_PLAYER:
                {
                    pCharObject = new Player(m_MoveRequest.Player.GUID, m_MoveRequest.Player.JobID);
                    break;
                }
            default:
                {
                    break;
                }
        }

        if(!pCharObject){
            m_ActorPod->Forward(MPK_ERROR, m_MoveRequest.PodAddress, m_MoveRequest.MPKID);
            return;
        }

        CORecord stCORecord;
        stCORecord.X = m_MoveRequest.X;
        stCORecord.Y = m_MoveRequest.Y;
        stCORecord.R = m_MoveRequest.R;

        pCharObject->ResetR(m_MoveRequest.R);
        pCharObject->Locate(m_MapID, m_MoveRequest.X, m_MoveRequest.Y);
        pCharObject->Locate(GetAddress());

        stCORecord.UID        = m_MoveRequest.UID;
        stCORecord.AddTime    = m_MoveRequest.AddTime;
        stCORecord.PodAddress = pCharObject->Activate();

        m_CORecordV.push_back(stCORecord);

        // we respond to ServerMap, but it won't respond again
        m_ActorPod->Forward(MPK_OK, m_MoveRequest.PodAddress, m_MoveRequest.MPKID);
        m_ActorPod->Forward(MPK_HI, stCORecord.PodAddress);
        m_MoveRequest.Clear();

        //also we need to clear the neighbor's cover check
        for(int nY = 0; nY < 3; ++nY){
            for(int nX = 0; nX < 3; ++nX){
                // cancel all freezed neighbors
                if(m_NeighborV2D[nY][nX].Query == QUERY_OK){
                    m_ActorPod->Forward(MPK_OK,
                            m_NeighborV2D[nY][nX].PodAddress, m_NeighborV2D[nY][nX].MPKID);
                    m_NeighborV2D[nY][nX].Query = QUERY_NA;
                }
            }
        }

        return;
    }

    // it's an object requesting to move into/inside current region
    // when we grant the permission, the object should respond to it

    auto fnROP = [this](const MessagePack &rstRMPK, const Theron::Address &rstAddr){
        if(rstRMPK.Type() == MPK_OK){
            // object picked this chance to move
            if(m_MoveRequest.CurrIn){
                // inside
                for(auto &rstRecord: m_CORecordV){
                    if(true
                            && rstRecord.UID == m_MoveRequest.UID
                            && rstRecord.AddTime == m_MoveRequest.AddTime){
                        rstRecord.X = m_MoveRequest.X;
                        rstRecord.Y = m_MoveRequest.Y;
                        break;
                    }
                }
            }else{
                // into
                // then we assume it has removed the old record in its last region
                CORecord stCORecord;
                stCORecord.X = m_MoveRequest.X;
                stCORecord.Y = m_MoveRequest.Y;
                stCORecord.R = m_MoveRequest.R;

                stCORecord.PodAddress = rstAddr;
                stCORecord.UID        = m_MoveRequest.UID;
                stCORecord.AddTime    = m_MoveRequest.AddTime;

                m_CORecordV.push_back(stCORecord);
            }
        }

        m_MoveRequest.Clear();

        // no matter the object decide to move or not, we need to free neighbors
        for(int nY = 0; nY < 3; ++nY){
            for(int nX = 0; nX < 3; ++nX){
                if(m_NeighborV2D[nY][nX].Query == QUERY_OK){
                    m_ActorPod->Forward(MPK_OK,
                            m_NeighborV2D[nY][nX].PodAddress, m_NeighborV2D[nY][nX].MPKID);
                    m_NeighborV2D[nY][nX].Query = QUERY_NA;
                }
            }
        }
    };
    // TODO
    // there was a bug here
    // when we notified the object, then neighbor check is done
    // however before the object responded, this RM should still be freezed
    //
    m_MoveRequest.NeighborCheck = false;
    m_ActorPod->Forward(MPK_OK, m_MoveRequest.PodAddress, m_MoveRequest.MPKID, fnROP);
}

void RegionMonitor::For_Update()
{
}
