/*
 * =====================================================================================
 *
 *       Filename: regionmonitor.cpp
 *        Created: 04/22/2016 01:15:24
 *  Last Modified: 06/08/2016 19:55:25
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
#include "syncdriver.hpp"
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

bool RegionMonitor::GroundValid(int, int, int nR)
{
    // void state
    if(nR == 0){ return true; }
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
            bError   = (bError   || (m_NeighborV2D[nY][nX].Query == QUERY_ERROR));
            bPending = (bPending || (m_NeighborV2D[nY][nX].Query == QUERY_PENDING));
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
    if(!(In(m_MapID, nPosX, nPosY) && (nPosR >= 0))){
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
    return bAllValid;
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

// TODO: synchronizd method to get RMAddress, this function can return the address directly, if use asynchronous method
//       I have to put it in the trigger and can't have a stAddr = GetRMAddress(...), if we use RM address cache then
//       it's OK but I don't want to use it because:
//          1. there are already 3 * 3 neighbor address cache, if add another cache, then it doesn't make
//             sense to keep this 3 * 3 cache anymore
//          2. more important, current RM has no idea of other map's metrics of size, RM size etc. then if keep this
//             cache, we have to query for that, otherwise we have (MapID, MapX, MapY) cache instead of the cache
//             in (MapID, RMX, RMY), which is definitely un-acceptable
int RegionMonitor::GetRMAddress(uint32_t nMapID, int nRMX, int nRMY, Theron::Address *pRMAddr)
{
    // 1. check arguments
    if(!(nMapID > 0 && nRM >= 0 && nRMY >= 0)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid argument");
        g_MonoServer->Restart();
    }

    // 2. check RM validness
    if(!(m_MapID && m_MapAddress)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "activated rm address with invalid state");
        g_MonoServer->Restart();
    }

    // 3. get proper actor address to ask
    if(nMapID != m_MapID){
        MessagePack stMPK;
        if(SyncDriver().Forward(MPK_QUERYSCADDRESS, m_MapAddress, &stMPK) || (stMPK.Type() != MPK_ADDRESS)){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "query service core address failed");
            g_MonoServer->Restart();
        }

        // TODO
        // the service core address may be on the way
        // this will overwirte it, should be OK, this is the reason I write another asynchronous method
        m_SCAddress = Theron::Address((char *)(stMPK.Data()));
        m_SCAddressQuery = QUERY_OK;
    }

    Theron::Address stAskAddr = ((m_MapID == nMapID) ? m_MapAddress : m_SCAddress);
    if(!stAskAddr){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "logic error: no valid address to ask");
        g_MonoServer->Restart();
    }

    while(true){
        MessagePack stMPK;
        AMQueryRMAddress stAMQRMA;
        stAMQRMA.RMX = nRMX;
        stAMQRMA.RMY = nRMY;
        stAMQRMA.MapID = nMapID;

        // synchronizd query
        if(SyncDriver().Forward({MPK_QUERYRMADDRESS, stAMQRMA}, stAskAddr, &stMPK)){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "query rm address failed, actor system error");
            g_MonoServer->Restart();
        }

        switch(stMPK.Type()){
            case MPK_ADDRESS:
                {
                    if(pRMAddr){ *pRMAddr = Theron::Address((char *)stMPK.Data()); }
                    return QUERY_OK;
                }
            case MPK_PENDING:
                {
                    // do nothing, ask again
                    break;
                }
            default:
                {
                    // take all other possibilities as error
                    return QUERY_ERROR;
                }
        }
    }

    // to make the compiler happy
    return QUERY_ERROR;
}

// asynchronous method to query rm address and then apply the trigger fnUseRMAddr
int RegionMonitor::GetRMAddress(uint32_t nMapID, int nRMX, int nRMY, const std::function<void(const Theron::Address &)> &fnUseRMAddr)
{
    // 1. check arguments
    //    TODO: do I need to check whether  fnUseRMAddr is callable???
    if(!(nMapID > 0 && nRMX >= 0 && nRMY >= 0 /* && fnUseRMAddr */)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid argument");
        g_MonoServer->Restart();
    }

    // 2. check RM validness
    if(!(m_MapID && m_MapAddress)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "activated rm address with invalid state");
        g_MonoServer->Restart();
    }

    // for those RM's we already have address
    if(NeighborIn(nMapID, nMapX, nMapY)){ 
        fnUseRMAddr(NeighborAddress(nMapX, nMapY));
        return QUERY_OK;
    }

    // 3. we need the service core address for rm address
    if(m_MapID != nMapID && !m_SCAddress){
        switch(m_SCAddressQuery){
            case QUERY_NA:
                {
                    // we haven't ask for the sc address yet
                    auto fnOnGetSCAddr = [this, nMapID, nMapX, nMapY, fnUseRMAddr](const MessagePack &rstRMPK, const Theron::Address &){
                        switch(rstRMPK.Type()){
                            case MPK_ADDRESS:
                                {
                                    m_SCAddressQuery = QUERY_OK;
                                    m_SCAddress = Theron::Address((char *)(rstRMPK.Data()));
                                    GetRMAddress(nMapID, nMapX, nMapY, fnUseRMAddr);
                                    break;
                                }
                            default:
                                {
                                    // when quering sc address we won't bare any errors
                                    extern MonoServer *g_MonoServer;
                                    g_MonoServer->AddLog(LOGTYPE_WARNING, "query service core address failed");
                                    g_MonoServer->Restart();
                                    break;
                                }
                        }
                    };

                    AMQueryRMAddress stAMQRMA;
                    stAMQRMA.MapID = nMapID;
                    stAMQRMA.MapX  = nMapX;
                    stAMQRMA.MapY  = nMapY;

                    m_ActorPod->Forward({MPK_QUERYRMADDRESS, stAMQRMA}, m_MapAddress, fnOnGetSCAddr);
                    break;
                }
            case QUERY_PENDING:
                {
                    // thee is no service core address but it's OTW
                    auto fnOnSCValid = [this, nMapID, nMapX, nMapY, fnUseRMAddr]() -> bool{
                        switch(m_SCAddressQuery){
                            case QUERY_OK:
                                {
                                    if(m_SCAddress){
                                        GetRMAddress(nMapID, nMapX, nMapY, fnUseRMAddr);
                                        return true;
                                    }

                                    // you promise the sc address is valid but actually it's not
                                    extern MonoServer *g_MonoServer;
                                    g_MonoServer->AddLog(LOGTYPE_WARNING, "internal logic error");
                                    g_MonoServer->Restart();

                                    // to make the compiler happy
                                    break;
                                }
                            case QUERY_PENDING:
                                {
                                    // do nothing, just wait
                                    return false;
                                }
                            default:
                                {
                                    // couldn't be any other value
                                    extern MonoServer *g_MonoServer;
                                    g_MonoServer->AddLog(LOGTYPE_WARNING, "unexpected logic");
                                    g_MonoServer->Restart();
                                    break;
                                }
                        }

                        // make the compiler happy
                        return true;
                    };

                    m_StateHook.Install(fnOnSCValid);
                    break;
                }
            default:
                {
                    // m_SCAddressQuery can only be NA and PENDING when m_SCAddress is empty
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid state for m_SCAddressQuery");
                    g_MonoServer->Restart();
                    break;
                }
        }

        return QUERY_PENDING;
    }

    // 4. needed address for asking is ready, set address to ask
    Theron::Address stAskAddr = ((nMapID == m_MapID) ? m_MapAddress : m_SCAddress);

    // 5. prepare for trigger, (nQuery, stRMAddr) are the state of the lambda
    auto fnGetRMAddr = [this, fnUseRMAddr, stAskAddr, nQuery = QUERY_NA, stRMAddr = Theron::Address::Null()]() mutable -> bool {
        switch(nQuery){
            case QUERY_NA:
                {
                    // NA means we need to issue request
                    fnOnRR = [&nQuery, &stRMAddr](const MessagePack &rstRRMPK, const Theron::Address &){
                        switch(rstRRMPK.Type()){
                            case MPK_ADDRESS:
                                {
                                    stRMAddr = Theron::Address(rstRRMPK.Data());
                                    nQuery   = QUERY_OK;
                                    break;
                                }
                            case MPK_PENDING:
                                {
                                    // we have to ask again, so mark it as NA
                                    nQuery = QUERY_NA;
                                    break;
                                }
                            default:
                                {
                                    nQuery = QUERY_ERROR;
                                    break;
                                }
                        }
                    };

                    // after issued, we wait
                    nQuery = QUERY_PENDING;
                    m_ActorPod->Forward(MPK_QUERYRMADDRESS, stAddrToAsk, fnOnRR);

                    return false;
                }
            case QUERY_PENDING:
                {
                    // PENDING means it's OTW, do nothing
                    return false;
                }
            case QUERY_OK:
                {
                    // ready to use
                    fnUseRMAddr(stRMAddr);
                    return true;
                }
            default:
                {
                    // can only be ERROR, we apply it to the null address and won't ask again
                    fnUseRMAddr(Theron::Address::Null());
                    return true;
                }
        }
    };

    // 6. prepare for the callback of the fisrst shoot, could be one shoot done
    auto fnOnR = [this, fnGetRMAddr, fnUseRMAddr](const MessagePack &rstRMPK, const Theron::Address &){
        switch(rstRMPK.Type()){
            case MPK_ADDRESS:
                {
                    fnUseRMAddr(Theron::Address((char *)(rstRMPK.Data())));
                    break;
                }
            case MPK_PENDING:
                {
                    m_StateHook.Install(fnGetRMAddr);
                    break;
                }
            default:
                {
                    fnUseRMAddr(Theron::Address::Null());
                    break;
                }
        }
    };

    m_ActorPod->Forward(MPK_QUERYRMADDRESS, stAddrToAsk, fnOnR);
    return QUERY_PENDING;
}
