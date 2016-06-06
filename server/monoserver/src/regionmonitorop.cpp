/*
 * =====================================================================================
 *
 *       Filename: regionmonitorop.cpp
 *        Created: 05/03/2016 19:59:02
 *  Last Modified: 06/06/2016 00:43:25
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

#include "player.hpp"
#include "monster.hpp"
#include "mathfunc.hpp"
#include "actorpod.hpp"
#include "charobject.hpp"
#include "monoserver.hpp"
#include "reactobject.hpp"
#include "regionmonitor.hpp"

void RegionMonitor::On_MPK_LEAVE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMLeave stAML;
    std::memcpy(&stAML, rstMPK.Data(), rstMPK.DataLen());

    bool bFind = false;
    for(auto &rstRecord: m_CORecordV){
        if(rstRecord.UID == stAML.UID && rstRecord.AddTime == stAML.AddTime){
            std::swap(rstRecord, m_CORecordV.back());
            m_CORecordV.pop_back();
            bFind = true;
            break;
        }
    }

    if(!bFind){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "char object uid = %d, addtime = %d is not in current RM", stAML.UID, stAML.AddTime);
        g_MonoServer->Restart();
    }

    // commit the leave, then the object can move into another RM
    m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
}

void RegionMonitor::On_MPK_CHECKCOVER(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    // 1. there is checking in current RM
    if(m_MoveRequest.Freezed()){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // 2. ok do it
    AMCheckCover stAMCC;
    std::memcpy(&stAMCC, rstMPK.Data(), sizeof(stAMCC));

    // 3. checking failed, just repot and do nothing
    if(!CoverValid(stAMCC.UID, stAMCC.AddTime, stAMCC.X, stAMCC.Y, stAMCC.R)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // 3. checking succeeds
    //    we need to hold current RM till we receive notification
    //
    //    TODO: do I need to add a timeout functionality here??
    //          when time is up and we didn't get unlock notification, just report
    //          as error or automatically unlock it?
    m_MoveRequest.Clear();
    m_MoveRequest.CoverCheck = true;
    m_MoveRequest.Freeze();

    auto fnROP = [this](const MessagePack &, const Theron::Address &){
        // cover check requestor should response to clear the lock
        m_MoveRequest.Clear();
    };
    m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID(), fnROP);
}

void RegionMonitor::On_MPK_NEIGHBOR(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    char *pAddr = (char *)rstMPK.Data();
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            size_t nLen = std::strlen(pAddr);
            if(nLen == 0 || (nX == 1 && nY == 1)){
                m_NeighborV2D[nY][nX].PodAddress = Theron::Address::Null();
            }else{
                m_NeighborV2D[nY][nX].PodAddress = Theron::Address(pAddr);
            }
            pAddr += (1 + nLen);
        }
    }

    // response with OK
    m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
}

void RegionMonitor::On_MPK_METRONOME(const MessagePack &, const Theron::Address &)
{
    for(auto &rstRecord: m_CORecordV){
        m_ActorPod->Forward(MPK_METRONOME, rstRecord.PodAddress);
    }
}

// object send a try move request, if it's a local move, handle it
// otherwise respond with the proper remote address
void RegionMonitor::On_MPK_TRYMOVE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    if(m_MoveRequest.Freezed()){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    AMTryMove stAMTM;
    std::memcpy(&stAMTM, rstMPK.Data(), sizeof(stAMTM));

    if(false
            || stAMTM.MapID != m_MapID // no the same map ...
            || !PointInRectangle(stAMTM.X, stAMTM.Y, m_X - m_W, m_Y - m_H, 3 * m_W, 3 * m_H)){
        // this is a space move
        // ask ServerMap for proper RM address, don't freeze current RM here
        if(!m_MapAddress){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "activated RM is with empty map address");
            g_MonoServer->Restart();
        }

        AMQueryRMAddress stAMQRA;
        stAMQRA.RMX   = stAMTM.X / SYS_MAPGRIDXP;
        stAMQRA.RMY   = stAMTM.Y / SYS_MAPGRIDYP;
        stAMQRA.MapID = stAMTM.MapID;

        // trigger to query the RM address
        // TODO: don't use static or global variable for nQuery
        auto fnQueryRMAddr = [this, rstFromAddr, nQuery = QUERY_PENDING, stRMAddr = Theron::Address::Null(), nMPKID = rstMPK.ID()]() mutable -> bool {
            switch(nQuery){
                case QUERY_OK:
                    {
                        if(!stRMAddr){
                            extern MonoServer *g_MonoServer;
                            g_MonoServer->AddLog(LOGTYPE_WARNING, "inconsistant logic");
                            g_MonoServer->Restart();
                        }

                        std::string szRMAddr = stRMAddr.AsString();
                        m_ActorPod->Forward({MPK_ADDRESS, szRMAddr.c_str(), 1 + szRMAdd.size()}, rstFromAddr, nMPKID);
                        return true;
                    }
                case QUERY_PENDING:
                    {
                        // we ask again here
                        auto fnROP = [this, &stRMAddr, &nQuery](const MessagePack &rstRMPK, const Theron::Address &){
                            switch(rstRRMPK.Type()){
                                case MPK_ADDRESS:
                                    {
                                        nQuery = QUERY_OK;
                                        stRMAddr = Theron::Address((char *)rstRRMPK.Data());
                                        break;
                                    }
                                case MPK_PENDING:
                                    {
                                        break;
                                    }
                                default:
                                    {
                                        nQuery = QUERY_ERROR;
                                        break;
                                    }
                            }
                        };

                        // just return the requestor the proper address
                        // we assume the current map address is always valid!
                        m_ActorPod->Forward({MPK_QUERYRMADDRESS, stAMQRA}, m_MapAddress, fnROP);
                        return false;
                    }
                default:
                    {
                        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, nMPKID);
                        return true;
                    }
            }
        };

        auto fnROP = [this, fnQueryRMAddr, rstFromAddr, nMPKID = rstMPK.ID()](const MessagePack &rstRMPK, const Theron::Address &){
            switch(rstRMPK.Type()){
                case MPK_ADDRESS:
                    {
                        // best anwser we get, we can directly reply
                        m_ActorPod->Forward({MPK_ADDRESS, rstRMPK.Data(), rstRMPK.DataLen()}, rstFromAddr, nMPKID);
                        return;
                    }
                case MPK_PENDING:
                    {
                        // the RM address asked is not ready now, we need to ask again....
                        // ok install the handler
                        m_StateHook.Install(fnQueryRMAddr);
                        break;
                    }
                default:
                    {
                        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, nMPKID);
                        break;
                    }
            }
        };

        // just return the requestor the proper address
        // we assume the current map address is always valid!
        m_ActorPod->Forward({MPK_QUERYRMADDRESS, stAMQRA}, m_MapAddress, fnROP);
        return;
    }

    // ok it's a local move request

    // object is trying to move outside of the current region
    if(!PointInRectangle(stAMTM.X, stAMTM.Y, m_X, m_Y, m_W, m_H)){
        // return the RM address
        auto stAddress = NeighborAddress(stAMTM.X, stAMTM.Y);
        if(stAddress == Theron::Address::Null()){
            // this neighbor is not capable to holding the object
            m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
            return;
        }

        std::string szAddress = stAddress.AsString();
        m_ActorPod->Forward({MPK_ADDRESS,
                (uint8_t *)szAddress.c_str(), szAddress.size()}, rstFromAddr, rstMPK.ID());
        return;
    }

    // ok object is currently in this RM, and its requested distnation
    // is also in current RM

    // cover check failed in current RM, return directly
    if(!CoverValid(stAMTM.UID, stAMTM.AddTime, stAMTM.X, stAMTM.Y, stAMTM.R)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // check if the object is only in current region?
    int nObjX = stAMTM.X - stAMTM.R;
    int nObjY = stAMTM.Y - stAMTM.R;
    int nObjW = 2 * stAMTM.R;
    int nObjH = 2 * stAMTM.R;

    if(RectangleInside(m_X, m_Y, m_W, m_H, nObjX, nObjY, nObjW, nObjH)){
        // ok you are just only in current region
        // have to freeze this region since we need to wait object's response

        // 1. put the needed info for this OnlyIn
        m_MoveRequest.Clear();

        m_MoveRequest.UID     = stAMTM.UID;
        m_MoveRequest.AddTime = stAMTM.AddTime;
        m_MoveRequest.X       = stAMTM.X;
        m_MoveRequest.Y       = stAMTM.Y;
        m_MoveRequest.OnlyIn  = true;

        m_MoveRequest.Freeze();

        auto fnROP = [this](const MessagePack &rstRMPK, const Theron::Address &){
            // object moved, so we need to update the location
            if(rstRMPK.Type() == MPK_OK){
                for(auto &rstRecord: m_CORecordV){
                    if(true
                            && rstRecord.UID == m_MoveRequest.UID
                            && rstRecord.AddTime == m_MoveRequest.AddTime){

                        rstRecord.X = m_MoveRequest.X;
                        rstRecord.Y = m_MoveRequest.Y;
                        break;
                    }
                }
            }
            // no matter moved or not, release the current RM
            m_MoveRequest.Clear();
        };

        m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID(), fnROP);
        return;
    }

    // whether I need the cover check, this is a complex function
    // the avoid using goto
    bool bAllQualified = true;
    for(size_t nY = 0; nY < 3 && bAllQualified; ++nY){
        for(size_t nX = 0; nX < 3 && bAllQualified; ++nX){
            if(nX == 1 && nY == 1){ continue; }

            int nNbrX = ((int)nX - 1) * m_W + m_X;
            int nNbrY = ((int)nY - 1) * m_H + m_Y;

            // ok, no overlap, skip it
            if(CircleRectangleOverlap(stAMTM.X, stAMTM.Y, stAMTM.R, nNbrX, nNbrY, m_W, m_H)
                    && m_NeighborV2D[nY][nX].PodAddress == Theron::Address::Null()){
                bAllQualified = false;
                break;
            }
        }
    }

    // some needed neighbors are not qualified
    if(!bAllQualified){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // prepare the MoveRequest
    m_MoveRequest.Clear();

    m_MoveRequest.X             = stAMTM.X;
    m_MoveRequest.Y             = stAMTM.Y;
    m_MoveRequest.R             = stAMTM.R;
    m_MoveRequest.UID           = stAMTM.UID;
    m_MoveRequest.AddTime       = stAMTM.AddTime;
    m_MoveRequest.MPKID         = rstMPK.ID();
    m_MoveRequest.PodAddress    = rstFromAddr;
    m_MoveRequest.CurrIn        = true;
    m_MoveRequest.OnlyIn        = false;
    m_MoveRequest.NeighborCheck = true;

    m_MoveRequest.Freeze();

    // send cover check to neighbors
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            // skip this...
            if(nX == 1 && nY == 1){
                m_NeighborV2D[nY][nX].Query = QUERY_NA;
                continue;
            }

            int nNbrX = ((int)nX - 1) * m_W + m_X;
            int nNbrY = ((int)nY - 1) * m_H + m_Y;

            // no overlap, skip it
            if(!CircleRectangleOverlap(stAMTM.X, stAMTM.Y, stAMTM.R, nNbrX, nNbrY, m_W, m_H)){
                m_NeighborV2D[nY][nX].Query = QUERY_NA;
                continue;
            }

            // qualified, send the cover check request

            auto fnROP = [this, nX, nY](const MessagePack &rstRMPK, const Theron::Address &){
                if(rstRMPK.Type() == MPK_OK){
                    m_NeighborV2D[nY][nX].Query = QUERY_OK;
                    m_NeighborV2D[nY][nX].MPKID = rstRMPK.ID(); // used when cancel the freeze
                }else{
                    m_NeighborV2D[nY][nX].Query = QUERY_ERROR;
                }
            };

            AMCheckCover stAMCC;

            // since we know the object is in current region
            // we don't have to pass (UID, AddTime) to neighbors
            stAMCC.UID = stAMTM.UID;
            stAMCC.AddTime = stAMTM.AddTime;
            stAMCC.X = stAMTM.X;
            stAMCC.Y = stAMTM.Y;
            stAMCC.R = stAMTM.R;

            m_ActorPod->Forward({MPK_CHECKCOVER, stAMCC}, m_NeighborV2D[nY][nX].PodAddress, fnROP);

            // set current state to be pending
            m_NeighborV2D[nY][nX].Query = QUERY_PENDING;
        }
    }

    // I have send MPK_CHECKCOVER to all qualified neighbors
    // now just wait for the response
}

void RegionMonitor::On_MPK_TRYSPACEMOVE(
        const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    if(m_MoveRequest.Freezed()){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    AMTrySpaceMove stAMTSM;
    std::memcpy(&stAMTSM, rstMPK.Data(), sizeof(stAMTSM));

    // TODO
    // put some logic here to make sure we got into the correct RM

    // object is trying to move into current region
    if(!CoverValid(stAMTSM.UID, stAMTSM.AddTime, stAMTSM.X, stAMTSM.Y, stAMTSM.R)){
        // this request is sent from the object
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // cover is valid, check if I would only be in current region?
    int nObjX = stAMTSM.X - stAMTSM.R;
    int nObjY = stAMTSM.Y - stAMTSM.R;
    int nObjW = 2 * stAMTSM.R;
    int nObjH = 2 * stAMTSM.R;

    if(RectangleInside(m_X, m_Y, m_W, m_H, nObjX, nObjY, nObjW, nObjH)){
        // ok you would only be in this region
        // have to lock this region since we need to wait object's response

        // 1. put needed info here
        m_MoveRequest.Clear();

        m_MoveRequest.UID     = stAMTSM.UID;
        m_MoveRequest.AddTime = stAMTSM.AddTime;
        m_MoveRequest.X       = stAMTSM.X;
        m_MoveRequest.Y       = stAMTSM.Y;
        m_MoveRequest.R       = stAMTSM.R;
        m_MoveRequest.OnlyIn  = true;

        m_MoveRequest.Freeze();

        auto fnROP = [this](const MessagePack &rstRMPK, const Theron::Address &rstAddr){
            if(rstRMPK.ID() == MPK_OK){
                CORecord stCORecord;
                stCORecord.X = m_MoveRequest.X;
                stCORecord.Y = m_MoveRequest.Y;
                stCORecord.R = m_MoveRequest.R;

                stCORecord.UID        = m_MoveRequest.UID;
                stCORecord.AddTime    = m_MoveRequest.AddTime;
                stCORecord.PodAddress = rstAddr;

                // put the new record into current region
                m_CORecordV.push_back(stCORecord);
            }
            m_MoveRequest.Clear();

            // TODO
            // broadcast this new object
        };
        m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID(), fnROP);
        return;
    }

    // not only in the single region, then need cover check
    // all needed RM are qualified?
    bool bAllQualified = true;
    for(size_t nY = 0; nY < 3 && bAllQualified; ++nY){
        for(size_t nX = 0; nX < 3 && bAllQualified; ++nX){
            if(nX == 1 && nY == 1){ continue; }

            int nNbrX = ((int)nX - 1) * m_W + m_X;
            int nNbrY = ((int)nY - 1) * m_H + m_Y;

            // ok, no overlap, skip it
            if(CircleRectangleOverlap(stAMTSM.X, stAMTSM.Y, stAMTSM.R, nNbrX, nNbrY, m_W, m_H)
                    && m_NeighborV2D[nY][nX].PodAddress == Theron::Address::Null()){
                bAllQualified = false;
                break;
            }
        }
    }

    // some needed neighbors are not qualified
    if(!bAllQualified){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // prepare the MoveRequest
    m_MoveRequest.Clear();

    m_MoveRequest.X             = stAMTSM.X;
    m_MoveRequest.Y             = stAMTSM.Y;
    m_MoveRequest.R             = stAMTSM.R;
    m_MoveRequest.UID           = stAMTSM.UID;
    m_MoveRequest.AddTime       = stAMTSM.AddTime;
    m_MoveRequest.MPKID         = rstMPK.ID();
    m_MoveRequest.PodAddress    = rstFromAddr;
    m_MoveRequest.CurrIn        = false;
    m_MoveRequest.OnlyIn        = false;
    m_MoveRequest.NeighborCheck = true;

    m_MoveRequest.Freeze();

    // send cover check to neighbors
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            // skip this...
            if(nX == 1 && nY == 1){
                m_NeighborV2D[nY][nX].Query = QUERY_NA;
                continue;
            }

            int nNbrX = ((int)nX - 1) * m_W + m_X;
            int nNbrY = ((int)nY - 1) * m_H + m_Y;

            // no overlap, skip it
            if(!CircleRectangleOverlap(stAMTSM.X, stAMTSM.Y, stAMTSM.R, nNbrX, nNbrY, m_W, m_H)){
                m_NeighborV2D[nY][nX].Query = QUERY_NA;
                continue;
            }

            // qualified, send the cover check request

            auto fnROP = [this, nX, nY](const MessagePack &rstRMPK, const Theron::Address &){
                if(rstRMPK.Type() == MPK_OK){
                    m_NeighborV2D[nY][nX].Query = QUERY_OK;
                    m_NeighborV2D[nY][nX].MPKID = rstRMPK.ID(); // used when cancel the freeze
                }else{
                    m_NeighborV2D[nY][nX].Query = QUERY_ERROR;
                }
            };

            AMCheckCover stAMCC;

            // since we know the object is in current region
            // we don't have to pass (UID, AddTime) to neighbors
            stAMCC.UID = stAMTSM.UID;
            stAMCC.AddTime = stAMTSM.AddTime;
            stAMCC.X = stAMTSM.X;
            stAMCC.Y = stAMTSM.Y;
            stAMCC.R = stAMTSM.R;

            m_ActorPod->Forward({MPK_CHECKCOVER, stAMCC}, m_NeighborV2D[nY][nX].PodAddress, fnROP);

            // set current state to be pending
            m_NeighborV2D[nY][nX].Query = QUERY_PENDING;
        }
    }
    // I have send MPK_CHECKCOVER to all qualified neighbors
    // now just wait for the response
}

// add a completely new object into current region
void RegionMonitor::On_MPK_ADDCHAROBJECT(
        const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMAddCharObject stAMACO;
    std::memcpy(&stAMACO, rstMPK.Data(), sizeof(stAMACO));

    // routing error
    if(!In(stAMACO.Common.MapID, stAMACO.Common.MapX, stAMACO.Common.MapY)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // try to be in an impossible place
    if(!GroundValid(stAMACO.Common.MapX, stAMACO.Common.MapY, stAMACO.Common.R)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // try to be active in a locked RM
    if(!stAMACO.Common.AllowVoid && m_MoveRequest.Freezed()){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // try to be in a place taken by others
    if(!stAMACO.Common.AllowVoid &&
            !CoverValid(0, 0, stAMACO.Common.MapX, stAMACO.Common.MapY, stAMACO.Common.R)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // ok now we can do something now

    // find conditions that we can finish the adding in this function
    // 1. allow void state, we even don't care about if current cover is freezed or not
    // 2. the new char object is only in current RM

    int nObjX = stAMACO.Common.MapX - stAMACO.Common.R;
    int nObjY = stAMACO.Common.MapY - stAMACO.Common.R;
    int nObjW = 2 * stAMACO.Common.R;
    int nObjH = 2 * stAMACO.Common.R;

    bool bAllowVoid = stAMACO.Common.AllowVoid;
    bool bOnlyIn    = RectangleInside(m_X, m_Y, m_W, m_H, nObjX, nObjY, nObjW, nObjH);

    if(bAllowVoid || bOnlyIn){
        CharObject *pCharObject = nullptr;
        switch(stAMACO.Type){
            case OBJECT_MONSTER:
                {
                    pCharObject = new Monster(stAMACO.Monster.MonsterID);
                    break;
                }
            case OBJECT_PLAYER:
                {
                    pCharObject = new Player(stAMACO.Player.GUID, stAMACO.Player.JobID);
                    break;
                }
            default:
                {
                    break;
                }
        }

        if(!pCharObject){
            m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
            return;
        }

        // if it's only in then we can set it as active immediately
        pCharObject->ResetState(bOnlyIn ? STATE_ACTIVE : STATE_INCARNATED, true);
        pCharObject->ResetR(stAMACO.Common.R);
        pCharObject->Locate(m_MapID, stAMACO.Common.MapX, stAMACO.Common.MapY);
        pCharObject->Locate(GetAddress());

        CORecord stCORecord;
        stCORecord.X = pCharObject->X();
        stCORecord.Y = pCharObject->Y();
        stCORecord.R = pCharObject->R();

        stCORecord.UID        = pCharObject->UID();
        stCORecord.AddTime    = pCharObject->AddTime();
        stCORecord.PodAddress = pCharObject->Activate();

        m_CORecordV.push_back(stCORecord);

        // we respond to ServiceCore, but it won't respond again
        // TODO & TBD
        // here maybe protential bug:
        // 1. message to creater did arrival yet
        // 2. but the actor has already been working
        //
        m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());

        // actually here we don't need to create the RM address and send it
        // since for the receiving object, it can take the address of the Operate()
        m_ActorPod->Forward(MPK_HI, stCORecord.PodAddress);
        if(pCharObject->Type(OBJECT_PLAYER)){
            m_ActorPod->Forward({MPK_BINDSESSION, stAMACO.Player.SessionID}, stCORecord.PodAddress);
        }
        return;
    }

    // don't allow void state, and the object is not only in one RM
    // thing be more complicated we have to ask neighbor's opinion for cover

    // all needed RM are qualified?
    bool bAllQualified = true;
    for(size_t nY = 0; nY < 3 && bAllQualified; ++nY){
        for(size_t nX = 0; nX < 3 && bAllQualified; ++nX){
            if(nX == 1 && nY == 1){
                m_NeighborV2D[nY][nX].Query = QUERY_NA;
                continue;
            }

            int nNbrX = ((int)nX - 1) * m_W + m_X;
            int nNbrY = ((int)nY - 1) * m_H + m_Y;

            // check overlap, skip it
            int nMapX = stAMACO.Common.MapX;
            int nMapY = stAMACO.Common.MapY;
            int nR    = stAMACO.Common.R;

            if(CircleRectangleOverlap(nMapX, nMapY, nR, nNbrX, nNbrY, m_W, m_H)){
                if(m_NeighborV2D[nY][nX].PodAddress != Theron::Address::Null()){
                    // overlap, and qualified
                    m_NeighborV2D[nY][nX].Query = QUERY_OK;
                }else{
                    // overlap and not qualified
                    m_NeighborV2D[nY][nX].Query = QUERY_NA;
                    bAllQualified = false;
                    break;
                }
            }else{
                m_NeighborV2D[nY][nX].Query = QUERY_NA;
            }
        }
    }

    // some needed neighbors are not qualified
    // then we need to inform the ServerMap who creates this monster
    if(!bAllQualified){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // all 3 x 3 are marked as QUERY_NA or QUERY_OK
    // prepare the MoveRequest
    m_MoveRequest.Clear();

    m_MoveRequest.Type          = stAMACO.Type;
    m_MoveRequest.X             = stAMACO.Common.MapX;
    m_MoveRequest.Y             = stAMACO.Common.MapY;
    m_MoveRequest.R             = stAMACO.Common.R;
    m_MoveRequest.UID           = 0; // pending
    m_MoveRequest.AddTime       = 0; // pending
    m_MoveRequest.MPKID         = rstMPK.ID();
    m_MoveRequest.PodAddress    = rstFromAddr;
    m_MoveRequest.CurrIn        = false;
    m_MoveRequest.OnlyIn        = false;
    m_MoveRequest.NeighborCheck = true;

    // for adding new object, we have to keep these info
    // since for just moving, we can ask the actor, here we keep it as arguments
    // when creating the new object
    switch(m_MoveRequest.Type){
        case OBJECT_PLAYER:
            {
                m_MoveRequest.Player.GUID  = stAMACO.Player.GUID;
                m_MoveRequest.Player.JobID = stAMACO.Player.JobID;
                break;
            }
        case OBJECT_MONSTER:
            {
                m_MoveRequest.Monster.MonsterID = stAMACO.Monster.MonsterID;
                break;
            }
        default:
            {
                // unsupported object type
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "unsupported object type");
                g_MonoServer->Restart();

                // make the compiler happy
                break;
            }
    }

    m_MoveRequest.Freeze();

    // send cover check to neighbors
    for(size_t nY = 0; nY < 3; ++nY){
        for(size_t nX = 0; nX < 3; ++nX){
            // skip those un-needed RM's
            if(m_NeighborV2D[nY][nX].Query == QUERY_NA){ continue; }

            // qualified, send the cover check request

            auto fnROP = [this, nX, nY](const MessagePack &rstRMPK, const Theron::Address &){
                if(rstRMPK.Type() == MPK_OK){
                    m_NeighborV2D[nY][nX].Query = QUERY_OK;
                    m_NeighborV2D[nY][nX].MPKID = rstRMPK.ID(); // used when cancel the freeze
                }else{
                    m_NeighborV2D[nY][nX].Query = QUERY_ERROR;
                }
            };

            AMCheckCover stAMCC;

            // since we know the object is in current region
            // we don't have to pass (UID, AddTime) to neighbors
            stAMCC.X       = stAMACO.Common.MapX;
            stAMCC.Y       = stAMACO.Common.MapY;
            stAMCC.R       = stAMACO.Common.R;
            stAMCC.UID     = 0;
            stAMCC.AddTime = 0;

            m_ActorPod->Forward({MPK_CHECKCOVER, stAMCC}, m_NeighborV2D[nY][nX].PodAddress, fnROP);

            // set current state to be pending
            m_NeighborV2D[nY][nX].Query = QUERY_PENDING;
        }
    }
    // I have send MPK_CHECKCOVER to all qualified neighbors
    // now just wait for the response
    return;
}

void RegionMonitor::On_MPK_MOTIONSTATE(const MessagePack &rstMPK, const Theron::Address &)
{
    for(auto &rstCORecord: m_CORecordV){
        if(rstCORecord.Type == OBJECT_PLAYER && rstCORecord.PodAddress){
            m_ActorPod->Forward({MPK_MOTIONSTATE, rstMPK.Data(), rstMPK.DataLen()}, rstCORecord.PodAddress);
        }
    }
}

void RegionMonitor::On_MPK_QUERYSCADDRESS(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    if(m_SCAddress){
        std::string szAddr = m_SCAddress.AsString();
        m_ActorPod->Forward({MPK_ADDRESS, (const uint8_t *)(szAddr.c_str()), 1 + szAddr.size()}, rstFromAddr, rstMPK.ID());
        return;
    }

    // sc address is not valid, but it's on the way
    if(m_SCAddressQuery == QUERY_PENDING){
        auto fnResp = [this, rstFromAddr, rstMPK](){
            if(m_SCAddress){
                std::string szAddr = m_SCAddress.AsString();
                m_ActorPod->Forward({MPK_ADDRESS, (const uint8_t *)(szAddr.c_str()), 1 + szAddr.size()}, rstFromAddr, rstMPK.ID());
                return true;
            }

            return false;
        };

        m_StateHook.Install(fnResp);
    }

    // oooops we have no SC address, ask map for sc address
    if(!m_MapAddress){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "activated RM works without valid map address");
        g_MonoServer->Restart();
    }

    auto fnOnR = [nMPKID = rstMPK.ID(), rstFromAddr, this](const MessagePack &rstRMPK, const Theron::Address &){
        if(rstRMPK.Type() == MPK_ADDRESS){
            m_SCAddress = Theron::Address((const char *)rstRMPK.Data());
            m_ActorPod->Forward({MPK_ADDRESS, rstRMPK.Data(), rstRMPK.DataLen()}, rstFromAddr, nMPKID);
            return;
        }

        // failed
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, nMPKID);
    };

    m_ActorPod->Forward(MPK_QUERYSCADDRESS, m_MapAddress, fnOnR);
}

void RegionMonitor::On_MPK_QUERYMAPADDRESS(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    uint32_t nMapID = *((uint32_t *)rstMPK.Data());
    if(nMapID == m_MapID){
        if(m_MapAddress){
            std::string szMapAddr = m_MapAddress.AsString();
            m_ActorPod->Forward({MPK_ADDRESS, (const uint8_t *)(szMapAddr.c_str(), szMapAddr.size() + 1)}, rstFromAddr, rstMPK.ID());
            return;
        }

        // otherwise it's should be an error
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "activated RM works without valid map address");
        g_MonoServer->Restart();
    }

    // TODO
    // add logic here for other map id
}
