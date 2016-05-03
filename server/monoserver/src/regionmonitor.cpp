/*
 * =====================================================================================
 *
 *       Filename: regionmonitor.cpp
 *        Created: 04/22/2016 01:15:24
 *  Last Modified: 05/03/2016 01:26:45
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

Theron::Address RegionMonitor::Activate()
{
    auto stAddr = Transponder::Activate();
    if(stAddr != Theron::Address::Null()){
        Send(MessagePack(MPK_ACTIVATE), m_MapAddress);
    }
    return stAddr;
}

void RegionMonitor::Operate(const MessagePack &rstMPK, const Theron::Address &) //rstFromAddr)
{
    switch(rstMPK.Type()){
        case MPK_NEWMONSTOR:
            {
                break;
            }
        case MPK_INITREGIONMONITOR:
            {
                AMRegion stAMRegion;
                std::memcpy(&stAMRegion, rstMPK.Data(), sizeof(stAMRegion));

                m_X = stAMRegion.X;
                m_Y = stAMRegion.Y;
                m_W = stAMRegion.W;
                m_H = stAMRegion.H;

                m_RegionDone = true;
                if(m_RegionDone && m_NeighborDone){
                    Send(MessagePack(MPK_READY), m_MapAddress);
                }
                break;
            }

        case MPK_NEIGHBOR:
            {
                char *pAddr = (char *)rstMPK.Data();
                for(size_t nY = 0; nY < 3; ++nY){
                    for(size_t nX = 0; nX < 3; ++nX){
                        size_t nLen = std::strlen(pAddr);
                        if(nLen == 0 || (nX == 1 && nY == 1)){
                            m_NeighborV2D[nY][nX] = Theron::Address::Null();
                        }else{
                            m_NeighborV2D[nY][nX] = Theron::Address(pAddr);
                        }
                        pAddr += (1 + nLen);
                    }
                }

                m_NeighborDone = true;
                if(m_RegionDone && m_NeighborDone){
                    Send(MessagePack(MPK_READY), m_MapAddress);
                }
                break;
            }
        default:
            {
                // when operating, MonoServer is ready for use
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "unsupported message type");
                break;
            }
    }
}
