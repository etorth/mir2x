/*
 * =====================================================================================
 *
 *       Filename: regionmonitortrg.cpp
 *        Created: 05/04/2016 17:18:43
 *  Last Modified: 05/05/2016 11:40:05
 *
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
#include "actorpod.hpp"
#include "regionmonitor.hpp"

// for broadcast move request, it won't be handled here
// this is for the ``main" region waiting for response from its neighbors
void RegionMonitor::For_MoveRequest()
{
    if(!m_MoveRequest.Valid()){ return; }
    if(!m_MoveRequest.In){ return; }

    bool bMoveOK = true;
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            switch(m_NeighborV2D[nY][nX].Query){
                case -1:
                    {
                        // we are not finished, waiting for respond
                        return;
                    }
                case 0:
                    {
                        // ooops one neighbor reject this request
                        bMoveOK = false;
                        goto __REGIONMONITOR_FOR_MOVE_REQUEST_DONE_1;
                    }
                default:
                    {
                        // check next
                        break;
                    }
            }
        }
    }

__REGIONMONITOR_FOR_MOVE_REQUEST_DONE_1:
    auto funROP = [this](const MessagePack &, const Theron::Address &){
        m_MoveRequest.Clear();
    };
    m_ActorPod->Forward((bMoveOK ? MPK_OK : MPK_ERROR),
            m_MoveRequest.PodAddress, m_MoveRequest.MPKID, fnROP);
}
