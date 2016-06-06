/*
 * =====================================================================================
 *
 *       Filename: regionmonitor.cpp
 *        Created: 04/22/2016 01:15:24
 *  Last Modified: 06/05/2016 22:53:01
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

#include "mathfunc.hpp"
#include "actorpod.hpp"
#include "monoserver.hpp"
#include "regionmonitor.hpp"

void RegionMonitor::Operate(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    switch(rstMPK.Type()){
        case MPK_CHECKCOVER:
            {
                On_MPK_CHECKCOVER(rstMPK, rstFromAddr);
                break;
            }
        case MPK_LEAVE:
            {
                On_MPK_LEAVE(rstMPK, rstFromAddr);
                break;
            }
        case MPK_TRYMOVE:
            {
                On_MPK_TRYMOVE(rstMPK, rstFromAddr);
                break;
            }
        case MPK_TRYSPACEMOVE:
            {
                On_MPK_TRYSPACEMOVE(rstMPK, rstFromAddr);
                break;
            }
        case MPK_METRONOME:
            {
                On_MPK_METRONOME(rstMPK, rstFromAddr);
                break;
            }
        case MPK_NEIGHBOR:
            {
                On_MPK_NEIGHBOR(rstMPK, rstFromAddr);
                break;
            }
        case MPK_ADDCHAROBJECT:
            {
                On_MPK_ADDCHAROBJECT(rstMPK, rstFromAddr);
                break;
            }
        case MPK_QUERYSCADDRESS:
            {
                On_MPK_QUERYSCADDRESS(rstMPK, rstFromAddr);
                break;
            }
        case MPK_QUERYMAPADDRESS:
            {
                On_MPK_QUERYMAPADDRESS(rstMPK, rstFromAddr);
                break;
            }
        default:
            {
                // when operating, MonoServer is ready for use
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING,
                        "unsupported message: type = %d, name = %s, id = %d, resp = %d",
                        rstMPK.Type(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
                break;
            }
    }
}

bool RegionMonitor::GroundValid(int, int, int)
{
    return true;
}

// check the cover is valid for current region
bool RegionMonitor::CoverValid(uint32_t nUID, uint32_t nAddTime, int nX, int nY, int nR)
{
    // 1. valid ground here?
    if(!GroundValid(nX, nY, nR)){ return false; }

    // 2. will I collide with any one in current region?
    for(auto &rstRecord: m_CORecordV){
        if(rstRecord.UID == nUID && rstRecord.AddTime == nAddTime){ continue; }
        if(CircleOverlap(rstRecord.X, rstRecord.Y, rstRecord.R, nX, nY, nR)){ return false; }
    }

    // 3. OK
    return true;
}

const Theron::Address &RegionMonitor::NeighborAddress(int nX, int nY)
{
    int nDX = (nX - (m_X - m_W)) / m_W;
    int nDY = (nY - (m_Y - m_H)) / m_H;

    if(nDX >= 0 && nDX < 3 && nDY >= 0 && nDY < 3){
        return m_NeighborV2D[nDY][nDX].PodAddress;
    }

    return m_EmptyAddress;
}
