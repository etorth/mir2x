/*
 * =====================================================================================
 *
 *       Filename: servermapop.cpp
 *        Created: 05/03/2016 20:21:32
 *  Last Modified: 09/13/2017 21:29:13
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
#include <cinttypes>
#include "dbcomid.hpp"
#include "player.hpp"
#include "monster.hpp"
#include "mathfunc.hpp"
#include "sysconst.hpp"
#include "actorpod.hpp"
#include "metronome.hpp"
#include "servermap.hpp"
#include "monoserver.hpp"
#include "rotatecoord.hpp"
#include "dbcomrecord.hpp"

void ServerMap::On_MPK_METRONOME(const MessagePack &, const Theron::Address &)
{
    for(auto &rstRecordLine: m_UIDRecordV2D){
        for(auto &rstRecordV: rstRecordLine){

            // this part check all recorded UID and remove those invalid ones
            // do it periodically in 1s, then for all rest logic we can skip the clean job

            for(size_t nIndex = 0; nIndex < rstRecordV.size();){
                extern MonoServer *g_MonoServer;
                if(auto stUIDRecord = g_MonoServer->GetUIDRecord(rstRecordV[nIndex])){
                    if(stUIDRecord.ClassFrom<ActiveObject>()){
                        if(m_ActorPod->Forward(MPK_METRONOME, stUIDRecord.Address)){
                            nIndex++;
                            continue;
                        }else{
                            std::swap(rstRecordV[nIndex], rstRecordV.back());
                            rstRecordV.pop_back();
                            continue;
                        }
                    }else{
                        nIndex++;
                        continue;
                    }
                }else{
                    std::swap(rstRecordV[nIndex], rstRecordV.back());
                    rstRecordV.pop_back();
                    continue;
                }
            }
        }
    }
}

void ServerMap::On_MPK_BADACTORPOD(const MessagePack &, const Theron::Address &)
{
}

void ServerMap::On_MPK_ACTION(const MessagePack &rstMPK, const Theron::Address &)
{
    AMAction stAMA;
    std::memcpy(&stAMA, rstMPK.Data(), sizeof(stAMA));

    if(ValidC(stAMA.X, stAMA.Y)){
        auto nX0 = std::max<int>(0,   (stAMA.X - SYS_MAPVISIBLEW));
        auto nY0 = std::max<int>(0,   (stAMA.Y - SYS_MAPVISIBLEH));
        auto nX1 = std::min<int>(W(), (stAMA.X + SYS_MAPVISIBLEW));
        auto nY1 = std::min<int>(H(), (stAMA.Y + SYS_MAPVISIBLEH));

        for(int nX = nX0; nX < nX1; ++nX){
            for(int nY = nY0; nY < nY1; ++nY){
                if(ValidC(nX, nY)){
                    for(auto nUID: m_UIDRecordV2D[nX][nY]){
                        if(nUID != stAMA.UID){
                            extern MonoServer *g_MonoServer;
                            if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
                                if(stUIDRecord.ClassFrom<CharObject>()){
                                    m_ActorPod->Forward({MPK_ACTION, stAMA}, stUIDRecord.Address);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void ServerMap::On_MPK_ADDCHAROBJECT(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMAddCharObject stAMACO;
    std::memcpy(&stAMACO, rstMPK.Data(), sizeof(stAMACO));

    bool bValidLoc = true;
    if(false
            || !In(stAMACO.Common.MapID, stAMACO.Common.X, stAMACO.Common.Y)
            || !CanMove(true, true, stAMACO.Common.X, stAMACO.Common.Y)){

        // the location field provides an invalid location
        // check if we can do random pick
        bValidLoc = false;

        if(stAMACO.Common.Random){
            if(In(stAMACO.Common.MapID, stAMACO.Common.X, stAMACO.Common.Y)){
                // OK we failed to add monster at the specified location
                // but still to try to add near it
            }else{
                // an invalid location provided
                // randomly pick one
                stAMACO.Common.X = std::rand() % W();
                stAMACO.Common.Y = std::rand() % H();
            }

            RotateCoord stRC;
            if(stRC.Reset(stAMACO.Common.X, stAMACO.Common.Y, 0, 0, W(), H())){
                do{
                    if(true
                            && In(stAMACO.Common.MapID, stRC.X(), stRC.Y())
                            && CanMove(true, true, stRC.X(), stRC.Y())){

                        // find a valid location
                        // use it to add new charobject
                        bValidLoc = true;

                        stAMACO.Common.X = stRC.X();
                        stAMACO.Common.Y = stRC.Y();
                        break;
                    }
                }while(stRC.Forward());
            }
        }
    }

    if(bValidLoc){
        switch(stAMACO.Type){
            case TYPE_MONSTER:
                {
                    auto pCO = new Monster(stAMACO.Monster.MonsterID,
                            m_ServiceCore,
                            this,
                            stAMACO.Common.X,
                            stAMACO.Common.Y,
                            DIR_UP,
                            STATE_INCARNATED,
                            stAMACO.Monster.MasterUID);

                    auto nUID = pCO->UID();
                    auto nX   = stAMACO.Common.X;
                    auto nY   = stAMACO.Common.Y;

                    pCO->Activate();
                    m_UIDRecordV2D[nX][nY].push_back(nUID);
                    m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
                    return;
                }
            case TYPE_PLAYER:
                {
                    auto pCO = new Player(stAMACO.Player.DBID,
                            m_ServiceCore,
                            this,
                            stAMACO.Common.X,
                            stAMACO.Common.Y,
                            stAMACO.Player.Direction,
                            STATE_INCARNATED);

                    auto nUID = pCO->UID();
                    auto nX   = stAMACO.Common.X;
                    auto nY   = stAMACO.Common.Y;

                    pCO->Activate();
                    m_UIDRecordV2D[nX][nY].push_back(nUID);
                    m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
                    m_ActorPod->Forward({MPK_BINDSESSION, stAMACO.Player.SessionID}, pCO->GetAddress());
                    return;
                }
            default:
                {
                    break;
                }
        }
    }

    // anything incorrect happened
    // report MPK_ERROR to service core that we failed
    m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
}

void ServerMap::On_MPK_TRYSPACEMOVE(const MessagePack &rstMPK, const Theron::Address &rstAddress)
{
    AMTrySpaceMove stAMTSM;
    std::memcpy(&stAMTSM, rstMPK.Data(), sizeof(stAMTSM));

    int nDstX = stAMTSM.X;
    int nDstY = stAMTSM.Y;

    if(true
            && !stAMTSM.StrictMove
            && !ValidC(stAMTSM.X, stAMTSM.Y)){

        nDstX = std::rand() % W();
        nDstY = std::rand() % H();
    }

    bool bDstOK = false;
    if(CanMove(false, false, nDstX, nDstY)){
        bDstOK = true;
    }else{
        if(!stAMTSM.StrictMove){
            RotateCoord stRC;
            if(stRC.Reset(nDstX, nDstY, 0, 0, W(), H())){
                int nDoneCheck = 0;
                do{
                    if(CanMove(false, false, stRC.X(), stRC.Y())){
                        nDstX  = stRC.X();
                        nDstY  = stRC.Y();
                        bDstOK = true;

                        break;
                    }
                    nDoneCheck++;
                }while(stRC.Forward() && nDoneCheck < 100);
            }
        }
    }

    if(!bDstOK){
        m_ActorPod->Forward(MPK_ERROR, rstAddress, rstMPK.ID());
        return;
    }

    // get valid location
    // respond with MPK_SPACEMOVEOK and wait for response

    auto fnOP = [this, nUID = stAMTSM.UID, nDstX, nDstY](const MessagePack &rstRMPK, const Theron::Address &)
    {
        switch(rstRMPK.Type()){
            case MPK_OK:
                {
                    // the object comfirm to move
                    // and it's internal state has changed

                    // for space move
                    // 1. we won't take care of map switch
                    // 2. we won't take care of where it comes from
                    // 3. we don't take reservation of the dstination cell

                    AddGridUID(nUID, nDstX, nDstY);
                    break;
                }
            default:
                {
                    break;
                }
        }
    };

    AMSpaceMoveOK stAMSMOK;
    stAMSMOK.Data  = this;

    m_ActorPod->Forward({MPK_SPACEMOVEOK, stAMSMOK}, rstAddress, rstMPK.ID(), fnOP);
}

void ServerMap::On_MPK_TRYMOVE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMTryMove stAMTM;
    std::memcpy(&stAMTM, rstMPK.Data(), sizeof(stAMTM));

    if(!In(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid location: (MapID = %d, X = %d, Y = %d)", stAMTM.MapID, stAMTM.X, stAMTM.Y);
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // we never allow server to handle motion to invalid grid
    // for client every motion request need to be prepared to avoid this

    if(!In(stAMTM.MapID, stAMTM.EndX, stAMTM.EndY)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid location: (MapID = %d, X = %d, Y = %d)", stAMTM.MapID, stAMTM.EndX, stAMTM.EndY);
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    bool bFindCO = false;
    for(auto &nUID: m_UIDRecordV2D[stAMTM.X][stAMTM.Y]){
        if(nUID == stAMTM.UID){
            bFindCO = true;
            break;
        }
    }

    if(!bFindCO){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "CO is not at current location: UID = %" PRIu32 , stAMTM.UID);
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // we add new concept here: allow-half-move
    // means if we request StepSize = 2 or 3 failed
    // we try StepSize = 1 move move as much as possible

    int nStepSize = -1;
    switch(LDistance2(stAMTM.X, stAMTM.Y, stAMTM.EndX, stAMTM.EndY)){
        case 0:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "Empty move request from UID = %" PRIu32 , stAMTM.UID);
                m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
                return;
            }
        case 1:
        case 2:
            {
                nStepSize = 1;
                break;
            }
        case 4:
        case 8:
            {
                nStepSize = 2;
                break;
            }
        case  9:
        case 18:
            {
                nStepSize = 3;
                break;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid move request from UID = %" PRIu32 , stAMTM.UID);
                m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
                return;
            }
    }

    int nDX = (stAMTM.EndX > stAMTM.X) - (stAMTM.EndX < stAMTM.X);
    int nDY = (stAMTM.EndY > stAMTM.Y) - (stAMTM.EndY < stAMTM.Y);

    int nMostDistance = 0;
    for(int nDistance = 1; nDistance <= nStepSize; ++nDistance){
        if(true
                && CanMove(false, true,  true,  stAMTM.X,       stAMTM.Y,       stAMTM.X + nDX * nDistance, stAMTM.Y + nDY * nDistance)
                && CanMove(true,  false, false, stAMTM.X + nDX, stAMTM.Y + nDY, stAMTM.X + nDX * nDistance, stAMTM.Y + nDY * nDistance)){

            // 1. check locks for all grids except two end points
            // 2. check co's  for all grids except the starting point

            nMostDistance = nDistance;
        }else{ break; }
    }

    if(false
            || ((nMostDistance == 0))
            || ((nMostDistance != nStepSize) && !(stAMTM.AllowHalfMove))){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    int nMostX = stAMTM.X + nDX * nMostDistance;
    int nMostY = stAMTM.Y + nDY * nMostDistance;

    auto fnOnR = [this, stAMTM, nMostX, nMostY](const MessagePack &rstRMPK, const Theron::Address &) -> void
    {
        switch(rstRMPK.Type()){
            case MPK_OK:
                {
                    // the object comfirm to move
                    // and it's internal state has changed

                    // 1. leave last cell
                    {
                        bool bFind = false;
                        auto &rstRecordV = m_UIDRecordV2D[stAMTM.X][stAMTM.Y];

                        for(auto &nUID: rstRecordV){
                            if(nUID == stAMTM.UID){
                                // 1. mark as find
                                bFind = true;

                                // 2. remove from the object list
                                std::swap(nUID, rstRecordV.back());
                                rstRecordV.pop_back();

                                break;
                            }
                        }

                        if(!bFind){
                            extern MonoServer *g_MonoServer;
                            g_MonoServer->AddLog(LOGTYPE_FATAL, "CO is not at current location: UID = %" PRIu32 , stAMTM.UID);
                            g_MonoServer->Restart();
                        }
                    }

                    // 2. push it to the new cell
                    //    check if it should switch the map
                    extern MonoServer *g_MonoServer;
                    if(auto stRecord = g_MonoServer->GetUIDRecord(stAMTM.UID)){
                        m_UIDRecordV2D[nMostX][nMostY].push_back(stRecord.UID);
                        if(true
                                && stRecord.ClassFrom<Player>()
                                && m_CellRecordV2D[nMostX][nMostY].MapID){
                            if(m_CellRecordV2D[nMostX][nMostY].UID){
                                AMMapSwitch stAMMS;
                                stAMMS.UID   = m_CellRecordV2D[nMostX][nMostY].UID;
                                stAMMS.MapID = m_CellRecordV2D[nMostX][nMostY].MapID;
                                stAMMS.X     = m_CellRecordV2D[nMostX][nMostY].SwitchX;
                                stAMMS.Y     = m_CellRecordV2D[nMostX][nMostY].SwitchY;
                                m_ActorPod->Forward({MPK_MAPSWITCH, stAMMS}, stRecord.Address);
                            }else{
                                switch(m_CellRecordV2D[nMostX][nMostY].Query){
                                    case QUERY_NA:
                                        {
                                            auto fnOnResp = [this, nMostX, nMostY, stRecord](const MessagePack &rstMPK, const Theron::Address &){
                                                switch(rstMPK.Type()){
                                                    case MPK_UID:
                                                        {
                                                            AMUID stAMUID;
                                                            std::memcpy(&stAMUID, rstMPK.Data(), sizeof(stAMUID));

                                                            if(stAMUID.UID){
                                                                m_CellRecordV2D[nMostX][nMostY].UID   = stAMUID.UID;
                                                                m_CellRecordV2D[nMostX][nMostY].Query = QUERY_OK;

                                                                // then we do the map switch notification
                                                                AMMapSwitch stAMMS;
                                                                stAMMS.UID   = m_CellRecordV2D[nMostX][nMostY].UID;
                                                                stAMMS.MapID = m_CellRecordV2D[nMostX][nMostY].MapID;
                                                                stAMMS.X     = m_CellRecordV2D[nMostX][nMostY].SwitchX;
                                                                stAMMS.Y     = m_CellRecordV2D[nMostX][nMostY].SwitchY;
                                                                m_ActorPod->Forward({MPK_MAPSWITCH, stAMMS}, stRecord.Address);
                                                            }

                                                            break;
                                                        }
                                                    default:
                                                        {
                                                            m_CellRecordV2D[nMostX][nMostY].UID   = 0;
                                                            m_CellRecordV2D[nMostX][nMostY].Query = QUERY_ERROR;
                                                            break;
                                                        }
                                                }
                                            };

                                            AMQueryMapUID stAMQMUID;
                                            stAMQMUID.MapID = m_CellRecordV2D[nMostX][nMostY].MapID;
                                            m_ActorPod->Forward({MPK_QUERYMAPUID, stAMQMUID}, m_ServiceCore->GetAddress(), fnOnResp);
                                            m_CellRecordV2D[nMostX][nMostY].Query = QUERY_PENDING;
                                        }
                                    case QUERY_PENDING:
                                    case QUERY_OK:
                                    case QUERY_ERROR:
                                    default:
                                        {
                                            break;
                                        }
                                }
                            }
                        }
                    }
                    break;
                }
            default:
                {
                    break;
                }
        }
        m_CellRecordV2D[nMostX][nMostY].Lock = false;
    };

    AMMoveOK stAMMOK;
    stAMMOK.UID   = UID();
    stAMMOK.MapID = ID();
    stAMMOK.X     = stAMTM.X;
    stAMMOK.Y     = stAMTM.Y;
    stAMMOK.EndX  = nMostX;
    stAMMOK.EndY  = nMostY;

    m_CellRecordV2D[nMostX][nMostY].Lock = true;
    m_ActorPod->Forward({MPK_MOVEOK, stAMMOK}, rstFromAddr, rstMPK.ID(), fnOnR);
}

void ServerMap::On_MPK_TRYLEAVE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMTryLeave stAMTL;
    std::memcpy(&stAMTL, rstMPK.Data(), rstMPK.DataLen());

    if(true
            && stAMTL.UID
            && stAMTL.MapID
            && ValidC(stAMTL.X, stAMTL.Y)){
        auto &rstRecordV = m_UIDRecordV2D[stAMTL.X][stAMTL.Y];
        for(auto &nUID: rstRecordV){
            if(nUID == stAMTL.UID){
                std::swap(rstRecordV.back(), nUID);
                rstRecordV.pop_back();
                m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
                return;
            }
        }
    }

    // otherwise try leave failed
    // can't leave means it's illegal then we won't report MPK_ERROR
    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "Leave request failed: UID = " PRIu32 ", X = %d, Y = %d", stAMTL.UID, stAMTL.X, stAMTL.Y);
}

void ServerMap::On_MPK_PULLCOINFO(const MessagePack &rstMPK, const Theron::Address &)
{
    AMPullCOInfo stAMPCOI;
    std::memcpy(&stAMPCOI, rstMPK.Data(), sizeof(stAMPCOI));

    for(auto &rstRecordLine: m_UIDRecordV2D){
        for(auto &rstRecordV: rstRecordLine){
            for(auto nUID: rstRecordV){
                extern MonoServer *g_MonoServer;
                if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
                    m_ActorPod->Forward({MPK_PULLCOINFO, stAMPCOI.SessionID}, stUIDRecord.Address);
                }
            }
        }
    }
}

void ServerMap::On_MPK_TRYMAPSWITCH(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMTryMapSwitch stAMTMS;
    std::memcpy(&stAMTMS, rstMPK.Data(), sizeof(stAMTMS));

    int nX = stAMTMS.EndX;
    int nY = stAMTMS.EndY;

    if(CanMove(false, false, nX, nY)){
        AMMapSwitchOK stAMMSOK;
        stAMMSOK.Data = this;
        stAMMSOK.X    = nX;
        stAMMSOK.Y    = nY;

        auto fnOnResp = [this, stAMTMS, stAMMSOK](const MessagePack &rstRMPK, const Theron::Address &){
            switch(rstRMPK.Type()){
                case MPK_OK:
                    {
                        extern MonoServer *g_MonoServer;
                        if(auto stRecord = g_MonoServer->GetUIDRecord(stAMTMS.UID)){
                            m_UIDRecordV2D[stAMMSOK.X][stAMMSOK.Y].push_back(stRecord.UID);
                        }
                        // won't check map switch here
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
            m_CellRecordV2D[stAMMSOK.X][stAMMSOK.Y].Lock = false;
        };
        m_CellRecordV2D[nX][nY].Lock = true;
        m_ActorPod->Forward({MPK_MAPSWITCHOK, stAMMSOK}, rstFromAddr, rstMPK.ID(), fnOnResp);
    }else{
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Requested map switch location is invalid: MapName = %s, X = %d, Y = %d", DBCOM_MAPRECORD(ID()).Name, nX, nY);
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
    }
}

void ServerMap::On_MPK_PATHFIND(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMPathFind stAMPF;
    std::memcpy(&stAMPF, rstMPK.Data(), sizeof(stAMPF));

    int nX0 = stAMPF.X;
    int nY0 = stAMPF.Y;
    int nX1 = stAMPF.EndX;
    int nY1 = stAMPF.EndY;

    AMPathFindOK stAMPFOK;
    stAMPFOK.UID   = stAMPF.UID;
    stAMPFOK.MapID = ID();

    // we fill all slots with -1 for initialization
    // won't keep a record of ``how many path nodes are valid"
    auto nPathCount = (int)(sizeof(stAMPFOK.Point) / sizeof(stAMPFOK.Point[0]));
    for(int nIndex = 0; nIndex < nPathCount; ++nIndex){
        stAMPFOK.Point[nIndex].X = -1;
        stAMPFOK.Point[nIndex].Y = -1;
    }

    // should make sure MaxStep is OK
    if(true
            && stAMPF.MaxStep != 1
            && stAMPF.MaxStep != 2
            && stAMPF.MaxStep != 3){

        // we get a dangerous parameter from actormessage
        // correct here and put an warning in the log system
        stAMPF.MaxStep = 1;

        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid MaxStep: %d, should be (1, 2, 3)", stAMPF.MaxStep);
    }

    ServerPathFinder stPathFinder(this, stAMPF.MaxStep, stAMPF.CheckCO);
    if(true
            && stPathFinder.Search(nX0, nY0, nX1, nY1)
            && stPathFinder.GetSolutionStart()){

        int nCurrN = 0;
        int nCurrX = nX0;
        int nCurrY = nY0;

        while(auto pNode1 = stPathFinder.GetSolutionNext()){
            if(nCurrN >= nPathCount){ break; }
            int nEndX = pNode1->X();
            int nEndY = pNode1->Y();
            switch(LDistance2(nCurrX, nCurrY, nEndX, nEndY)){
                case 1:
                case 2:
                    {
                        stAMPFOK.Point[nCurrN].X = nCurrX;
                        stAMPFOK.Point[nCurrN].Y = nCurrY;

                        nCurrN++;

                        nCurrX = nEndX;
                        nCurrY = nEndY;
                        break;
                    }
                case 0:
                default:
                    {
                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid path node");
                        break;
                    }
            }
        }

        // we filled all possible nodes to the message
        m_ActorPod->Forward({MPK_PATHFINDOK, stAMPFOK}, rstFromAddr, rstMPK.ID());
        return;
    }

    // failed to find a path
    m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
}

void ServerMap::On_MPK_UPDATEHP(const MessagePack &rstMPK, const Theron::Address &)
{
    AMUpdateHP stAMUHP;
    std::memcpy(&stAMUHP, rstMPK.Data(), sizeof(stAMUHP));

    if(ValidC(stAMUHP.X, stAMUHP.Y)){
        auto nX0 = std::max<int>(0,   (stAMUHP.X - SYS_MAPVISIBLEW));
        auto nY0 = std::max<int>(0,   (stAMUHP.Y - SYS_MAPVISIBLEH));
        auto nX1 = std::min<int>(W(), (stAMUHP.X + SYS_MAPVISIBLEW));
        auto nY1 = std::min<int>(H(), (stAMUHP.Y + SYS_MAPVISIBLEH));

        for(int nX = nX0; nX <= nX1; ++nX){
            for(int nY = nY0; nY <= nY1; ++nY){
                if(ValidC(nX, nY)){
                    for(auto nUID: m_UIDRecordV2D[nX][nY]){
                        if(nUID != stAMUHP.UID){
                            extern MonoServer *g_MonoServer;
                            if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
                                if(stUIDRecord.ClassFrom<CharObject>()){
                                    m_ActorPod->Forward({MPK_UPDATEHP, stAMUHP}, stUIDRecord.Address);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void ServerMap::On_MPK_DEADFADEOUT(const MessagePack &rstMPK, const Theron::Address &)
{
    AMDeadFadeOut stAMDFO;
    std::memcpy(&stAMDFO, rstMPK.Data(), sizeof(stAMDFO));

    if(ValidC(stAMDFO.X, stAMDFO.Y)){
        auto nX0 = std::max<int>(0,   (stAMDFO.X - SYS_MAPVISIBLEW));
        auto nY0 = std::max<int>(0,   (stAMDFO.Y - SYS_MAPVISIBLEH));
        auto nX1 = std::min<int>(W(), (stAMDFO.X + SYS_MAPVISIBLEW));
        auto nY1 = std::min<int>(H(), (stAMDFO.Y + SYS_MAPVISIBLEH));

        for(int nX = nX0; nX <= nX1; ++nX){
            for(int nY = nY0; nY <= nY1; ++nY){
                if(ValidC(nX, nY)){
                    for(auto nUID: m_UIDRecordV2D[nX][nY]){
                        if(nUID != stAMDFO.UID){
                            extern MonoServer *g_MonoServer;
                            if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
                                if(stUIDRecord.ClassFrom<Player>()){
                                    m_ActorPod->Forward({MPK_DEADFADEOUT, stAMDFO}, stUIDRecord.Address);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void ServerMap::On_MPK_QUERYCORECORD(const MessagePack &rstMPK, const Theron::Address &)
{
    AMQueryCORecord stAMQCOR;
    std::memcpy(&stAMQCOR, rstMPK.Data(), sizeof(stAMQCOR));

    if(true
            && stAMQCOR.UID
            && stAMQCOR.MapID == ID()
            && ValidC(stAMQCOR.X, stAMQCOR.Y)){

        for(auto nUID: m_UIDRecordV2D[stAMQCOR.X][stAMQCOR.Y]){
            if(nUID == stAMQCOR.UID){
                extern MonoServer *g_MonoServer;
                if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
                    if(stUIDRecord.ClassFrom<CharObject>()){
                        m_ActorPod->Forward({MPK_PULLCOINFO, stAMQCOR.SessionID}, stUIDRecord.Address);
                        return;
                    }
                }
            }
        }

        auto nX0 = std::max<int>(0,   (stAMQCOR.X - SYS_MAPVISIBLEW));
        auto nY0 = std::max<int>(0,   (stAMQCOR.Y - SYS_MAPVISIBLEH));
        auto nX1 = std::min<int>(W(), (stAMQCOR.X + SYS_MAPVISIBLEW));
        auto nY1 = std::min<int>(H(), (stAMQCOR.Y + SYS_MAPVISIBLEH));

        for(int nX = nX0; nX <= nX1; ++nX){
            for(int nY = nY0; nY <= nY1; ++nY){
                if(ValidC(nX, nY)){
                    for(auto nUID: m_UIDRecordV2D[nX][nY]){
                        if(nUID == stAMQCOR.UID){
                            extern MonoServer *g_MonoServer;
                            if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
                                if(stUIDRecord.ClassFrom<CharObject>()){
                                    m_ActorPod->Forward({MPK_PULLCOINFO, stAMQCOR.SessionID}, stUIDRecord.Address);
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void ServerMap::On_MPK_QUERYCOCOUNT(const MessagePack &rstMPK, const Theron::Address &rstAddress)
{
    AMQueryCOCount stAMQCOC;
    std::memcpy(&stAMQCOC, rstMPK.Data(), sizeof(stAMQCOC));

    if(false
            || (stAMQCOC.MapID == 0)
            || (stAMQCOC.MapID == ID())){
        int nCOCount = 0;
        for(size_t nX = 0; nX < m_UIDRecordV2D.size(); ++nX){
            for(size_t nY = 0; nY < m_UIDRecordV2D[0].size(); ++nY){
                std::for_each(m_UIDRecordV2D[nX][nY].begin(), m_UIDRecordV2D[nX][nY].end(), [stAMQCOC, &nCOCount](uint32_t nUID){
                    extern MonoServer *g_MonoServer;
                    if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
                        if(stUIDRecord.ClassFrom<CharObject>()){
                            if(stAMQCOC.Check.NPC    ){ nCOCount++; return; }
                            if(stAMQCOC.Check.Player ){ nCOCount++; return; }
                            if(stAMQCOC.Check.Monster){ nCOCount++; return; }
                        }
                    }
                });
            }
        }

        // done the count and return it
        AMCOCount stAMCOC;
        stAMCOC.Count = nCOCount;
        m_ActorPod->Forward({MPK_COCOUNT, stAMCOC}, rstAddress, rstMPK.ID());
        return;
    }

    // failed to count and report error
    m_ActorPod->Forward(MPK_ERROR, rstAddress, rstMPK.ID());
}

void ServerMap::On_MPK_QUERYRECTUIDV(const MessagePack &rstMPK, const Theron::Address &rstAddress)
{
    AMQueryRectUIDV stAMQRUIDV;
    std::memcpy(&stAMQRUIDV, rstMPK.Data(), sizeof(stAMQRUIDV));

    AMUIDV stAMUIDV;
    std::memset(&stAMUIDV, 0, sizeof(stAMUIDV));

    size_t nIndex = 0;
    for(int nY = stAMQRUIDV.Y; nY < stAMQRUIDV.Y + stAMQRUIDV.H; ++nY){
        for(int nX = stAMQRUIDV.X; nX < stAMQRUIDV.X + stAMQRUIDV.W; ++nX){
            if(In(stAMQRUIDV.MapID, nX, nY)){
                for(auto nUID: m_UIDRecordV2D[nX][nY]){
                    stAMUIDV.UIDV[nIndex++] = nUID;
                }
            }
        }
    }

    m_ActorPod->Forward({MPK_UIDV, stAMUIDV}, rstAddress, rstMPK.ID());
}

void ServerMap::On_MPK_NEWDROPITEM(const MessagePack &rstMPK, const Theron::Address &)
{
    AMNewDropItem stAMNDI;
    std::memcpy(&stAMNDI, rstMPK.Data(), sizeof(stAMNDI));

    if(true
            && stAMNDI.ID
            && stAMNDI.Value > 0
            && ValidC(stAMNDI.X, stAMNDI.Y)){

        bool bHoldInOne = false;
        switch(stAMNDI.ID){
            case DBCOM_ITEMID(u8"金币"):
                {
                    bHoldInOne = true;
                    break;
                }
            default:
                {
                    break;
                }
        }

        auto nLoop = bHoldInOne ? 1 : stAMNDI.Value;
        for(int nIndex = 0; nIndex < nLoop; ++nIndex){

            // we check SYS_MAXDROPITEMGRID grids to find a best place to hold current item
            // ``best" means there are not too many drop item already
            int nCheckGrid = 0;

            int nBestX    = -1;
            int nBestY    = -1;
            int nMinCount = SYS_MAXDROPITEM + 1;

            RotateCoord stRC;
            if(stRC.Reset(stAMNDI.X, stAMNDI.Y, 0, 0, W(), H())){
                do{
                    if(true
                            && ValidC(stRC.X(), stRC.Y())
                            && GroundValid(stRC.X(), stRC.Y())){

                        // valid grid
                        // check if gird good to hold
                        auto nCurrCount = DropItemListCount(stRC.X(), stRC.Y());
                        if(nCurrCount >= 0){
                            if(nCurrCount < nMinCount){
                                nMinCount = nCurrCount;
                                nBestX    = stRC.X();
                                nBestY    = stRC.Y();

                                // short it if it's an empty slot
                                // directly use it and won't compare more
                                if(nMinCount == 0){ break; }
                            }
                        }
                    }
                }while(stRC.Forward() && (nCheckGrid <= SYS_MAXDROPITEMGRID));
            }

            if(GroundValid(nBestX, nBestY)){
                AddGroundItem(nBestX, nBestY, {stAMNDI.ID, 0});
            }else{

                // we scanned the square but find we can't find a valid place
                // abort current operation since following check should also fail
                return;
            }
        }
    }
}
