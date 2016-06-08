/*
 * =====================================================================================
 *
 *       Filename: regionmonitor.cpp
 *        Created: 04/22/2016 01:15:24
 *  Last Modified: 06/07/2016 18:05:24
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

// after we sent the neighbor cover check request, this function check whether we
// grant the permission or failed, when we decide to do neighbor cover check, we
// put marks for the 3 * 3 neighbor:
//      QUERY_NA        for those irrelavent RM
//      QUERY_PENDING   for those RM we need its permission
// then we waiting for response:
//      QUERY_OK        for those RM we granted permission
//      QUERY_ERROR     for those RM busy or anything can't get permission
//
// return:
//      QUERY_OK        if all needed cover check succeeds
//      QUERY_PENDING   if there is any pending RM
//      QUERY_ERROR     if there is no pending RM and any failed RM
//
// TODO
// currently I response only when I get all information, error or ok, but actually
// every time I'm doing the cover check, if I found any error, I can just do the
// cancel. however this make my logic complex, since I have no support to ``cancel
// those sent request".
int RegionMonitor::NeighborQueryCheck()
{
    bool bError   = false;
    bool bPending = false;

    for(int nY = 0; nY < 3; ++nY){
        for(int nX = 0; nX < 3; ++nX){
            bError   = (bError   || m_NeighborV2D[nY][nX].Query == QUERY_ERROR);
            bPending = (bPending || m_NeighborV2D[nY][nX].Query == QUERY_PENDING);
        }
    }

    // wait until all response have been got, even we already get errors, we
    // have to wait since we can't cancel those request already sent
    if(bPending){ return QUERY_PENDING; }

    // no pending now and we found errors
    if(bError){ return QUERY_ERROR; }

    // everything is ok
    return QUERY_OK;
}

void RegionMonitor::NeighborClearCheck()
{
    if(NeighborQueryCheck() == QUERY_PENDING){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "don't do neighbor clear when there is pending request");
        g_MonoServer->Restart();
    }

    for(int nY = 0; nY < 3; ++nY){
        for(int nX = 0; nX < 3; ++nX){
            // cancel all freezed neighbors
            if(m_NeighborV2D[nY][nX].Valid() && (m_NeighborV2D[nY][nX].Query == QUERY_OK)){
                m_ActorPod->Forward(MPK_ERROR, m_NeighborV2D[nY][nX].PodAddress, m_NeighborV2D[nY][nX].MPKID);
                m_NeighborV2D[nY][nX].Query = QUERY_NA;
            }
        }
    }
}

// when I need to put a circle to the position (nPosX, nPosY, nPosR) I need firstly check
// whether the RM's it covered are all qualified, otherwise I even don't need to send the
// cover check request
//
// when bMark is set, this function will check all 3 * 3 neighbors and set marks
// QUERY_NA     : not overlapped
// QUERY_OK     : overlapped and valid
// QUERY_ERROR  : overlapped and invalid
// and return false when there is any QUERY_ERROR.
//
// when bMakr is not set, this function just return true / false
bool RegionMonitor::NeighborValid(int nPosX, int nPosY, int nPosR, bool bMark)
{
    if(!(PointInRectangle(nPosX, nPosY, m_X, m_Y, m_W, m_H) && nPosR >= 0)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid arguments");
        g_MonoServer->Restart();
    }

    bool bAllValid = true;
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            if(nX == 1 && nY == 1){
                // we don't need to check this one
                if(bMark){ m_NeighborV2D[1][1].Query = QUERY_NA; }
            }else{
                int nNbrX = ((int)nX - 1) * m_W + m_X;
                int nNbrY = ((int)nY - 1) * m_H + m_Y;

                if(CircleRectangleOverlap(nPosX, nPosY, nPosR, nNbrX, nNbrY, m_W, m_H)){
                    if(m_NeighborV2D[nY][nX].PodAddress){
                        if(bMark){ m_NeighborV2D[nY][nX].Query = QUERY_OK; }
                    }else{
                        if(bMark){
                            // for bMark set, we need to do all 3 * 3
                            bAllValid = false;
                            m_NeighborV2D[nY][nX].Query = QUERY_ERROR;
                        }else{
                            // otherwise we can stop here
                            return false;
                        }
                    }
                }else{
                    if(bMark){ m_NeighborV2D[nY][nX].Query = QUERY_NA; }
                }
            }
        }
    }
    return bValid;
}

// send cover check to neighbors, if bByMark is set, use the cached Query, otherwise this
// function do overlap checking and then send cc request to proper RM.
//
// TODO: this function should be called after NeighborValid()
void RegionMonitor::NeighborSendCheck(uint32_t nUID, uint32_t nAddTime, int nPosX, int nPosY, int nPosR, bool bByMark)
{
    // 1. parepare the structure to send
    AMCheckCover stAMCC;

    stAMCC.UID = nUID;
    stAMCC.AddTime = nAddTime;

    stAMCC.X = nPosX;
    stAMCC.Y = nPosY;
    stAMCC.R = nPosR;

    // 2. do for each proper RM
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            if(nX == 1 && nY == 1){
                m_NeighborV2D[nY][nX].Query = QUERY_NA;
                continue;
            }

            // state change path:
            // QUERY_NA      -> QUERY_NA
            // QUERY_OK      -> QUERY_PENDING -> QUERY_OK / QUERY_ERROR
            // QUERY_ERROR   -> exit(0)
            // QUERY_PENDING -> exit(0)
            //
            auto fnROP = [this, nX, nY](const MessagePack &rstRMPK, const Theron::Address &){
                if(rstRMPK.Type() == MPK_OK){
                    m_NeighborV2D[nY][nX].Query = QUERY_OK;
                    m_NeighborV2D[nY][nX].MPKID = rstRMPK.ID(); // used when cancel the freeze
                }else{
                    m_NeighborV2D[nY][nX].Query = QUERY_ERROR;
                    m_NeighborV2D[nY][nX].MPKID = 0;
                }
            };

            if(bByMark){
                switch(m_NeighborV2D[nY][nX].Query){
                    case QUERY_NA:
                        {
                            // do nothing
                            break;
                        }
                    case QUERY_OK:
                        {
                            if(m_NeighborV2D[nY][nX].PodAddress){
                                m_ActorPod->Forward({MPK_CHECKCOVER, stAMCC}, m_NeighborV2D[nY][nX].PodAddress, fnROP);
                            }else{
                                extern MonoServer *g_MonoServer;
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "internal logic don't agree when using mark");
                                g_MonoServer->Restart();
                            }
                            break;
                        }
                    default:
                        {
                            extern MonoServer *g_MonoServer;
                            g_MonoServer->AddLog(LOGTYPE_WARNING, "internal logic don't agree when using mark");
                            g_MonoServer->Restart();
                            break;
                        }
                }
                continue;
            }

            // else we have no cached mark, we have to do it one by one
            int nNbrX = ((int)nX - 1) * m_W + m_X;
            int nNbrY = ((int)nY - 1) * m_H + m_Y;

            // no overlap, skip it
            if(!CircleRectangleOverlap(nPosX, nPosY, nPosR, nNbrX, nNbrY, m_W, m_H)){
                m_NeighborV2D[nY][nX].Query = QUERY_NA;
                continue;
            }

            if(m_NeighborV2D[nY][nX].PodAddress){
                m_NeighborV2D[nY][nX].Query = QUERY_PENDING;
                m_ActorPod->Forward({MPK_CHECKCOVER, stAMCC}, m_NeighborV2D[nY][nX].PodAddress, fnROP);
            }else{
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "should check neighbor validation before issue cover check");
                g_MonoServer->Restart();
            }
        }
    }
}
