/*
 * =====================================================================================
 *
 *       Filename: regionmonitortrg.cpp
 *        Created: 05/04/2016 17:18:43
 *  Last Modified: 05/04/2016 17:22:19
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

#include "regionmonitor.hpp"

void RegionMonitor::For_MoveRequest()
{
    if(!m_MoveRequest.Valid()){ return; }

    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            if(m_NeighborV2D[nY][nX].Query == false){
                return;
            }
        }
    }

    m_ActorPod->Forward(MPK_ERROR, m_MoveRequest.PodAddress, m_MoveRequest.ID);
}
