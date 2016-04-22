/*
 * =====================================================================================
 *
 *       Filename: regionmonitor.cpp
 *        Created: 04/22/2016 01:15:24
 *  Last Modified: 04/22/2016 01:47:30
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

RegionMonitor::RegionMonitor(const Theron::Address &rstMapAddr)
    : MonitorBase()
    , m_MapAddress(rstMapAddr)
{}

void RegionMonitor::Operate(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    switch(rstMPK.Type()){
        case MPK_INITREGIONMONITOR:
            {
                AMInitRegionMonitor stAMIRM;



                AMRegion stAMRegion;
                std::memcpy(&stAMRegion, rstMPK.Data(), sizeof(stAMRegion));

                m_X = stAMRegion.X;
                m_Y = stAMRegion.Y;
                m_W = stAMRegion.W;
                m_H = stAMRegion.H;

                break;
            }

        case MPK_NEIGHBOR:
            {
                char szAddr[128];
                std::memcpy(szAddr, (const char *)rstMPK.Data(), rstMPK.DataLen());
                szAddr[rstMPK.DataLen()] = '\0';

                m_NeighborV[(size_t)szAddr[0]] = new Theron::Address(szAddr + 1);
                break;
            }
        default:
            {
                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_WARNING, "unsupported message type");
                break;
            }
    }
}

void RegionMonitor::Activate()
{
    MonitorBase::Activate();
    m_ObjectPod->Send(MessagePack(MPK_ACTIVATE));
}
