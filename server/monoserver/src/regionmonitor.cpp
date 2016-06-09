/*
 * =====================================================================================
 *
 *       Filename: regionmonitor.cpp
 *        Created: 04/22/2016 01:15:24
 *  Last Modified: 06/08/2016 22:26:20
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

// asynchronous method to query rm address and then apply the trigger fnUseRMAddr(stRMAddr), the reason I design the function
// like this is because in RM there is no something like general RM address cache to store different RM address, everytime if
// a RM address is needed, current RM should query.
//
// why I don't want this general RM cache?
//  1. there are already 3 * 3 neighbor address cache, and the logic for this 3 * 3 matrix works pretty good, if now I add
//     some new cache, there maybe something inconsistant
//  2. more important, current RM has no idea of other map's metrics such as RM width or height in number of SYS_GRID, if
//     we need to keep a cache we should query for this info. otherwise we only have (MapID, MapX, MapY) cache instead of
//     cache (MapX, RMX, RMY), which is totally un-acceptable
//
// why I design this function as asynchronous?
//  1. I can make it synchronizd, just use the following logic:
//              while(true){
//                  SyncDriver().Forward(MPK_QUERYRMADDRESS, m_SCAddress, &stMPK);
//                  if(stMPK.Type() == MPK_ADDRESS){
//                      return Theron::Address(stMPK.Data());
//                  }
//              }
//     but there are issues, one is we hope to use asynchronous method if we can, and what's more when we decide to use this
//     method, we need m_SCAddress, however m_SCAddress maybe pending, so we have to put synchronizd query to get service
//     core address, but we have m_SCAddressQuery, which shows the sc address is NA, OK or PENDING, if it's PENDING, means
//     we already sent query for it and it's OTW, but now if use synchronizd, we wirte m_SCAddress, then the response which
//     we sent before, will overwrite this, which should be OK but I want to avoid it.
//
// arguments:       nMapID      :
//                  nMapX       : mentioned, we may have no info of map RM metrics of nMapID
//                  nMapY       :
//                  fnUseRMAddr : callback when we got this RM address
//                                TODO: currently I didn't check the invocability of it, if not callable then
//                                      just drive the corresponding RM to be ready
int RegionMonitor::QueryRMAddress(uint32_t nMapID, int nMapX, int nMapY, const std::function<void(const Theron::Address &)> &fnUseRMAddr)
{
    // 1. check arguments
    //    TODO: do I need to check whether  fnUseRMAddr is callable???
    if(!(nMapID > 0 && nMapX >= 0 && nMapY >= 0 /* && fnUseRMAddr */)){
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

    // for those RM's we already have address, short it, this NeighborAddress() will
    // return all corresponding address in the 3 * 3 matrix even for (1, 1)
    if(NeighborIn(nMapID, nMapX, nMapY)){ 
        if(fnUseRMAddr){
            fnUseRMAddr(NeighborAddress(nMapX, nMapY));
        }
        return QUERY_OK;
    }

    // 3. we need the service core address for rm address
    if(m_MapID != nMapID && !m_SCAddress){
        switch(m_SCAddressQuery){
            case QUERY_NA:
                {
                    // we haven't ask for the service core address yet
                    // prepare callback when we got the service core address
                    auto fnOnR = [this, nMapID, nMapX, nMapY, fnUseRMAddr](const MessagePack &rstRMPK, const Theron::Address &){
                        switch(rstRMPK.Type()){
                            case MPK_ADDRESS:
                                {
                                    m_SCAddressQuery = QUERY_OK;
                                    m_SCAddress = Theron::Address((char *)(rstRMPK.Data()));

                                    // since now service core address is ready, we call itself again
                                    QueryRMAddress(nMapID, nMapX, nMapY, fnUseRMAddr);
                                    break;
                                }
                            default:
                                {
                                    // TODO strict condition
                                    //      query service core address won't tolerate any error / failure
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

                    m_ActorPod->Forward({MPK_QUERYRMADDRESS, stAMQRMA}, m_MapAddress, fnOnR);
                    break;
                }
            case QUERY_PENDING:
                {
                    // we asked the service core address before and now it's OTW
                    // since we don't want to ask one more time, register an anyomous trigger
                    auto fnOnSCValid = [this, nMapID, nMapX, nMapY, fnUseRMAddr]() -> bool{
                        switch(m_SCAddressQuery){
                            case QUERY_OK:
                                {
                                    if(m_SCAddress){
                                        // service core address is ready, call it again
                                        QueryRMAddress(nMapID, nMapX, nMapY, fnUseRMAddr);
                                        // uninstall current trigger, but the work of fnUseRMAddr
                                        // may still be unfinished
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
                                    // couldn't be any other state, the m_SCAddressQuery can only be
                                    // NA, OK, PENDING, since we start from PENDING, it can only be
                                    // PENDING and OK
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
                    // m_SCAddressQuery can only be NA or PENDING when m_SCAddress is empty
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid state for m_SCAddressQuery");
                    g_MonoServer->Restart();
                    break;
                }
        }

        // current call of QueryRMAddress() ends because of invalidness of service core
        // but we installed trigger / callback to call this function again when service
        // core address is ready
        return QUERY_PENDING;
    }

    // ok for here service core address and map address are both ready

    // 4. set address to ask
    Theron::Address stAskAddr = ((nMapID == m_MapID) ? m_MapAddress : m_SCAddress);
    if(!stAskAddr){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "unexpected logic error");
        g_MonoServer->Restart();
    }

    // 5. prepare for trigger, (nQuery, stRMAddr) are the state of the lambda
    //    most tricky part of this function
    auto fnGetRMAddr = [this, fnUseRMAddr, stAskAddr, nQuery = QUERY_NA, stRMAddr = Theron::Address::Null()]() mutable -> bool {
        switch(nQuery){
            case QUERY_NA:
                {
                    // NA means we need to issue request
                    auto fnOnRR = [&nQuery, &stRMAddr](const MessagePack &rstRRMPK, const Theron::Address &){
                        switch(rstRRMPK.Type()){
                            case MPK_ADDRESS:
                                {
                                    nQuery   = QUERY_OK;
                                    stRMAddr = Theron::Address((char *)(rstRRMPK.Data()));
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
                    m_ActorPod->Forward(MPK_QUERYRMADDRESS, stAskAddr, fnOnRR);

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
                    if(fnUseRMAddr){
                        fnUseRMAddr(stRMAddr);
                    }
                    return true;
                }
            default:
                {
                    // we apply it to the null address and won't ask again
                    // can only be ERROR, this could happen since we may try to get an address not capable
                    if(fnUseRMAddr){
                        fnUseRMAddr(Theron::Address::Null());
                    }
                    return true;
                }
        }
    };

    // 6. prepare for the callback of the fisrst shoot, could be one shoot done
    auto fnOnRRR = [this, fnGetRMAddr, fnUseRMAddr](const MessagePack &rstRRRMPK, const Theron::Address &){
        switch(rstRRRMPK.Type()){
            case MPK_ADDRESS:
                {
                    if(fnUseRMAddr){
                        fnUseRMAddr(Theron::Address((char *)(rstRRRMPK.Data())));
                    }
                    break;
                }
            case MPK_PENDING:
                {
                    m_StateHook.Install(fnGetRMAddr);
                    break;
                }
            default:
                {
                    if(fnUseRMAddr){
                        fnUseRMAddr(Theron::Address::Null());
                    }
                    break;
                }
        }
    };

    m_ActorPod->Forward(MPK_QUERYRMADDRESS, stAskAddr, fnOnRRR);
    return QUERY_PENDING;
}
