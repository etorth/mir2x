/*
 * =====================================================================================
 *
 *       Filename: regionmonitorop.cpp
 *        Created: 05/03/2016 19:59:02
 *  Last Modified: 06/08/2016 01:26:09
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
    // 1. there is working in current RM
    if(m_MoveRequest.Freezed()){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // 2. ok let's do it
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
    // 1. parse the data
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

    // 2. init the center pod for NeighborAddress()
    m_NeighborV2D[1][1].PodAddress = m_ActorPod->GetAddress();

    // 3. report finished to server map
    m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
}

void RegionMonitor::On_MPK_METRONOME(const MessagePack &, const Theron::Address &)
{
    for(auto &rstRecord: m_CORecordV){
        m_ActorPod->Forward(MPK_METRONOME, rstRecord.PodAddress);
    }
}

// object send a try move request, if it's a local move, handle it otherwise respond with
// the proper remote address, we may need to get sc address first
void RegionMonitor::On_MPK_TRYMOVE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMTryMove stAMTM;
    std::memcpy(&stAMTM, rstMPK.Data(), sizeof(stAMTM));

    // 0-1. check whether the reqestor is in void state
    bool bVoidState = (stAMTM.R == 0);

    // 0-2. yes it's in void state
    if(bVoidState){
        // for void state, In() is good enough, we don't need neighbor's permission
        if(In(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
            for(auto &rstRecord: m_CORecordV){
                if(rstRecord.UID == stAMTM.UID && rstRecord.AddTime == stAMTM.AddTime){
                    rstRecord.X = stAMTM.X;
                    rstRecord.Y = stAMTM.Y;
                    rstRecord.R = 0;

                    // TODO: here we don't check automatically whether the co still
                    //       need the void state, the co should send void check
                    //       request sperately
                    return;
                }
            }

            // otherwise this should be an error
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "couldn't find this char object record");
            g_MonoServer->Restart();
        }

        if(NeighborIn(stAMTM.MapID, stAMTM.X, stAMTM.Y)){

            ///////////////////
            //dffffffffffffffffffffffffffffff
        }





        if(NeighborIn(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
            // return the RM address
            auto stAddress = NeighborAddress(stAMTM.X, stAMTM.Y);
            if(!stAddress){
                // this neighbor is not capable to holding the object
                m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
                return;
            }

            std::string szAddress = stAddress.AsString();
            m_ActorPod->Forward({MPK_ADDRESS, (uint8_t *)szAddress.c_str(), 1 + szAddress.size()}, rstFromAddr, rstMPK.ID());
            return;
        }
    }


    // 0. reject any move request
    if(m_MoveRequest.Freezed()){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    AMTryMove stAMTM;
    std::memcpy(&stAMTM, rstMPK.Data(), sizeof(stAMTM));

    if(!(stAMTM.MapID && stAMTM.X >= 0 && stAMTM.Y >= 0 && stAMTM.R >= 0)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog("invalid move request");
        g_MonoServer->Restart();
    }

    if(!(m_MapID && m_MapAddress)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog("region monitor activated with invalid state");
        g_MonoServer->Restart();
    }

    // ok now it's valid request and valid RM

    // 1. only in current RM, best situation
    if(OnlyIn(stAMTM.MapID, stAMTM.X, stAMTM.Y, stAMTM.R)){
        m_MoveRequest.Clear();
        m_MoveRequest.WaitCO = true;
        m_MoveRequest.Freeze();

        auto fnROP = [stAMTM, this](const MessagePack &rstRMPK, const Theron::Address &){
            // object moved, so we need to update the location
            if(rstRMPK.Type() == MPK_OK){
                for(auto &rstRecord: m_CORecordV){
                    if(rstRecord.UID == stAMTM.UID && rstRecord.AddTime == stAMTM.AddTime){
                        rstRecord.X = stAMTM.X;
                        rstRecord.Y = stAMTM.Y;
                        break;
                    }
                }
            }else{
                // object doesn't move actually
                // it gives up the chance to move, do nothing
            }

            // no matter moved or not, release the current RM
            m_MoveRequest.Clear();
        };

        m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID(), fnROP);
        return;
    }

    // 2. in current RM but overlap with neighbors
    if(In(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
        // some needed neighbors are not qualified
        if(!NeighborValid(stAMTM.X, stAMTM.Y, stAMTM.R, true)){
            m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
            return;
        }

        // ok now we need to prepare for the move request

        // 01. now we are to do neighbor check
        m_MoveRequest.Clear();
        m_MoveRequest.NeighborCheck = true;
        m_MoveRequest.Freeze();

        // 02. send cover check to neighbors
        NeighborSendCheck(stAMTM.UID, stAMTM.AddTime, stAMTM.X, stAMTM.Y, stAMTM.R, true);

        // 03. make sure there is only one move request working
        if(m_StateHook.Installed("MoveRequest")){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog("try to install move request trigger while there's still one unfinished");
            g_MonoServer->Restart();
        }

        // 03. install trigger to check the CC response
        auto fnMoveRequest = [stAMTM, this]() -> bool(){
            switch(NeighborQueryCheck()){
                case QUERY_OK:
                    {
                        // we grant permission and now it's time
                        auto fnROP = [stAMTM, this](const MessagePack &rstRMPK, const Theron::Address &rstAddr){
                            if(rstRMPK.Type() == MPK_OK){
                                // object picked this chance to move
                                for(auto &rstRecord: m_CORecordV){
                                    if(rstRecord.UID == stAMTM.UID && rstRecord.AddTime == stAMTM.AddTime){
                                        rstRecord.X = stAMTM.X;
                                        rstRecord.Y = stAMTM.Y;
                                        break;
                                    }
                                }
                            }

                            // no matter the object decide to move or not, we need to free neighbors
                            NeighborClearCheck();
                            m_MoveRequest.Clear();
                        };
                        // TODO
                        // there was a bug here
                        // when we notified the object, then neighbor check is done
                        // however before the object responded, this RM should still be freezed
                        //
                        m_MoveRequest.Clear();
                        m_MoveRequest.WaitCO = true;
                        m_MoveRequest.Freeze();

                        m_ActorPod->Forward(MPK_OK, m_MoveRequest.PodAddress, m_MoveRequest.MPKID, fnROP);
                        return true;
                    }
                case QUERY_ERROR:
                    {
                        // no matter the object decide to move or not, we need to free neighbors
                        NeighborClearCheck();
                        m_MoveRequest.Clear();

                        m_ActorPod->Forward(MPK_ERROR, m_MoveRequest.PodAddress, m_MoveRequest.MPKID, fnROP);
                        return true;
                    }
                default:
                    {
                        // didn't finish it yet
                        return false;
                    }
            }

            // to make the compiler happy
            return false;
        };

        m_StateHook.Install("MoveRequest", fnMoveRequest);
        return;
    }

    // trying to move to one of its neighbor
    // this is already a space move but we don't need to query the RM address since we have it

    if(NeighborIn(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
        // return the RM address
        auto stAddress = NeighborAddress(stAMTM.X, stAMTM.Y);
        if(!stAddress){
            // this neighbor is not capable to holding the object
            m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
            return;
        }

        std::string szAddress = stAddress.AsString();
        m_ActorPod->Forward({MPK_ADDRESS, (uint8_t *)szAddress.c_str(), 1 + szAddress.size()}, rstFromAddr, rstMPK.ID());
        return;
    }

    // now this move request has nothing to do with current RM, then it's a space move, we have 
    // to know the destination RM address, we design a logic which can take care of it

    // return       void
    // parameter    address of destination RM
    auto fnOnGetRMAddr = [this, rstFromAddr, nMPKID = rstMPK.ID()](const Theron::Address &rstRMAddr){
        if(rstRMAddr){
            std::string szAddr = rstRMAddr.AsString();
            m_ActorPod->Forward({MPK_ADDRESS, szAddr.c_str(), 1 + szAddr.size()}, rstFromAddr, nMPKID);
            return;
        }
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid RM address");
        g_MonoServer->Restart();
    };

    auto fnQueryRMAddr = [this](const Theron::Address &rstAskAddr){
        if(!rstAskAddr){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid address provided to query");
            g_MonoServer->Restart();
        }

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
                        // we ask (again) here
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
    };

    if(m_MapID == stAMTM.MapID || m_SCAddress){
        // I have checked the validness of (m_Map && m_MapAddress)
        // so here just use it
        fnQueryRMAddr((m_MapID == stAMTM.MapID) ? m_MapAddress :  m_SCAddress);
        return;
    }

    // ok the last annoying condition, current RM has no service address and the request is
    // for space move, have to get it from server map
    //
    // TODO: if I set sc address as global variable maybe I can get rid of this
    auto fnOnGetSCAddr = [this](const MessagePack &rstMPK, const Theron::Address &){
        switch(rstMPK.Type()){
            case MPK_ADDRESS:
                {
                    m_SCAddress = Theron::Address((const *)rstMPK.Data());
                    fnQueryRMAddr(m_SCAddress);
                    return;
                }
            case MPK_ERROR:
            case MPK_PENDING:
            default:
                {
                    // when we asking for sc address we should always get it
                    // any resp such as pending / error is un-acceptable

                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "query for service core address failed");
                    g_MonoServer->Restart();

                }
        }
    };

    m_ActorPod->Forward(MPK_QUERYSCADDRESS, m_MapAddress, fnOnGetSCAddr);

    // finally we are done here, no more possibilities
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

    // this is a space move
    // ask ServerMap for proper RM address, don't freeze current RM here
    if(!NeighborIn(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
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
    if(!In(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
        // return the RM address
        auto stAddress = NeighborAddress(stAMTM.X, stAMTM.Y);
        if(!stAddress){
            // this neighbor is not capable to holding the object
            m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
            return;
        }

        std::string szAddress = stAddress.AsString();
        m_ActorPod->Forward({MPK_ADDRESS,
                (uint8_t *)szAddress.c_str(), 1 + szAddress.size()}, rstFromAddr, rstMPK.ID());
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

    // some needed neighbors are not qualified
    if(!NeighborCheckQualified(stAMTM.X, stAMTM.Y, stAMTM.R)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // ok now we need to prepare for the move request

    // 1. make sure there is only one move request working

    // 1. prepare the MoveRequest
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
    NeighborSendCoverCheck(stAMTM.UID, stAMTM.AddTime, stAMTM.X, stAMTM.Y, stAMTM.R);

    // I have send MPK_CHECKCOVER to all qualified neighbors
    // now just wait for the response, install trigger here for it
    if(m_StateHook.Installed("MoveRequest")){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog("try to install move request trigger while there still be one unfinished");
        g_MonoServer->Restart();
    }

    auto fnMoveRequest = [this]() -> bool(){
        switch(NeighborCheck()){
            case QUERY_OK:
                {
                    // we grant permission and now it's time
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
                            }
                        }

                        m_MoveRequest.Clear();

                        // no matter the object decide to move or not, we need to free neighbors
                        NeighborClearQuery();
                    };
                    // TODO
                    // there was a bug here
                    // when we notified the object, then neighbor check is done
                    // however before the object responded, this RM should still be freezed
                    //
                    m_MoveRequest.NeighborCheck = false;
                    m_ActorPod->Forward(MPK_OK, m_MoveRequest.PodAddress, m_MoveRequest.MPKID, fnROP);
                    return true;
                }
            case QUERY_ERROR:
                {
                    m_MoveRequest.Clear();

                    // no matter the object decide to move or not, we need to free neighbors
                    NeighborClearQuery();
                    m_ActorPod->Forward(MPK_ERROR, m_MoveRequest.PodAddress, m_MoveRequest.MPKID, fnROP);
                    return true;
                }
            default:
                {
                    return false;
                }
        }
    };

    m_StateHook.Install("MoveRequest", fnMoveRequest);
}

void RegionMonitor::On_MPK_TRYSPACEMOVE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
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

void RegionMonitor::On_MPK_ADDCHAROBJECT(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
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
    if(!stAMACO.Common.AllowVoid && !CoverValid(0, 0, stAMACO.Common.MapX, stAMACO.Common.MapY, stAMACO.Common.R)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // ok now we can do something now

    // find conditions that we can finish the adding in this function
    // 1. allow void state, we even don't care about if current cover is freezed or not
    // 2. the new char object is only in current RM

    bool bAllowVoid = stAMACO.Common.AllowVoid;
    bool bOnlyIn    = OnlyIn(stAMACO.Common.MapX, stAMACO.Common.MapY, stAMACO.Common.R);

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

    // some needed neighbors are not qualified
    if(!NeighborCheckQualified(stAMACO.X, stAMACO.Y, stAMACO.R)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

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
        return;
    }

    // oooops we have no SC address, ask map for sc address
    if(m_MapAddress){
        auto fnOnR = [nMPKID = rstMPK.ID(), rstFromAddr, this](const MessagePack &rstRMPK, const Theron::Address &){
            switch(rstRMPK.Type()){
                case MPK_ADDRESS:
                    {
                        m_SCAddress = Theron::Address((const char *)rstRMPK.Data());
                        m_ActorPod->Forward({MPK_ADDRESS, rstRMPK.Data(), rstRMPK.DataLen()}, rstFromAddr, nMPKID);
                        break;
                    }
                default:
                    {
                        // sc address query failed, couldn't happen
                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_WARNING, "query service core address failed, couldn't happen");
                        g_MonoServer->Restart();
                        break;
                    }
            }
        };
        m_ActorPod->Forward(MPK_QUERYSCADDRESS, m_MapAddress, fnOnR);
    }

    // otherwise it's should be an error
    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "activated RM works without valid map address");
    g_MonoServer->Restart();
}

void RegionMonitor::On_MPK_QUERYMAPADDRESS(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    auto nMapID = *((uint32_t *)rstMPK.Data());
    if(nMapID == m_MapID){
        if(m_MapAddress){
            std::string szMapAddr = m_MapAddress.AsString();
            m_ActorPod->Forward({MPK_ADDRESS, (const uint8_t *)(szMapAddr.c_str(), 1 + szMapAddr.size())}, rstFromAddr, rstMPK.ID());
            return;
        }

        // otherwise it's should be an error
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "activated RM works without valid map address");
        g_MonoServer->Restart();
    }

    // you should ask sc directly
    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "not RM's map address, ask service core for this instead");
    g_MonoServer->Restart();
}
