/*
 * =====================================================================================
 *
 *       Filename: servermapop.cpp
 *        Created: 05/03/2016 20:21:32
 *  Last Modified: 05/26/2017 01:26:11
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
#include "player.hpp"
#include "monster.hpp"
#include "mathfunc.hpp"
#include "sysconst.hpp"
#include "actorpod.hpp"
#include "metronome.hpp"
#include "servermap.hpp"
#include "monoserver.hpp"

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

        for(int nX = nX0; nX <= nX1; ++nX){
            for(int nY = nY0; nY <= nY1; ++nY){
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

    if(!In(stAMACO.Common.MapID, stAMACO.Common.X, stAMACO.Common.Y)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid argument in adding char object package");
        g_MonoServer->Restart();
    }

    if(!CanMove(stAMACO.Common.X, stAMACO.Common.Y)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    if(m_CellRecordV2D[stAMACO.Common.X][stAMACO.Common.Y].Freezed){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    switch(stAMACO.Type){
        case TYPE_MONSTER:
            {
                auto pCO = new Monster(stAMACO.Monster.MonsterID,
                        m_ServiceCore,
                        this,
                        stAMACO.Common.X,
                        stAMACO.Common.Y,
                        DIR_UP,
                        STATE_INCARNATED);

                auto nUID = pCO->UID();
                auto nX   = stAMACO.Common.X;
                auto nY   = stAMACO.Common.Y;

                pCO->Activate();
                m_UIDRecordV2D[nX][nY].push_back(nUID);
                m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
                break;
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
                break;
            }
        default:
            {
                m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
                break;
            }
    }
}

void ServerMap::On_MPK_TRYSPACEMOVE(const MessagePack &rstMPK, const Theron::Address &)
{
    AMTrySpaceMove stAMTSM;
    std::memcpy(&stAMTSM, rstMPK.Data(), sizeof(stAMTSM));

    if(!In(stAMTSM.MapID, stAMTSM.X, stAMTSM.Y)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "destination is not in current map, routing error");
        g_MonoServer->Restart();
    }
}

void ServerMap::On_MPK_TRYMOVE(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMTryMove stAMTM;
    std::memcpy(&stAMTM, rstMPK.Data(), sizeof(stAMTM));

    if(!In(stAMTM.MapID, stAMTM.EndX, stAMTM.EndY)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Destination is not in current map, routing error");
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    if(!CanMove(stAMTM.EndX, stAMTM.EndY)){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    if(m_CellRecordV2D[stAMTM.EndX][stAMTM.EndY].Freezed){
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
        g_MonoServer->AddLog(LOGTYPE_FATAL, "CharObject is not in current map: UID = %" PRIu32 , stAMTM.UID);
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    int nDX = std::abs<int>(stAMTM.EndX - stAMTM.X);
    int nDY = std::abs<int>(stAMTM.EndY - stAMTM.Y);
    if(nDX == 0 && nDY == 0){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_FATAL, "Empty move request from UID = %" PRIu32 , stAMTM.UID);
        m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
        return;
    }

    if(true
            && ((std::min<int>(nDX, nDY) == 0) || (nDX == nDY))
            && ((std::max<int>(nDX, nDY) <= 3))){
    }else{
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_FATAL, "Invalid move request DX = %d, DY = %d", nDX, nDY);
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    int nStepX = (stAMTM.EndX > stAMTM.X) - (stAMTM.EndX < stAMTM.X);
    int nStepY = (stAMTM.EndY > stAMTM.Y) - (stAMTM.EndY < stAMTM.Y);

    int nMostDistance = 0;
    for(int nDistance = 1; nDistance <= std::max<int>(nDX, nDY); ++nDistance){
        if(CanMove(stAMTM.X + nStepX * nDistance, stAMTM.Y + nStepY * nDistance)){
            nMostDistance = nDistance;
        }else{ break; }
    }

    if(false
            || ((nMostDistance == 0))
            || ((nMostDistance != std::max<int>(nDX, nDY)) && !(stAMTM.AllowHalfMove))){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    int nMostX = stAMTM.X + nStepX * nMostDistance;
    int nMostY = stAMTM.Y + nStepY * nMostDistance;

    auto fnOnR = [this, stAMTM, nMostX, nMostY](const MessagePack &rstRMPK, const Theron::Address &){
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
                            g_MonoServer->AddLog(LOGTYPE_FATAL, "CharObject is not in current map: UID = %" PRIu32 , stAMTM.UID);
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
        m_CellRecordV2D[nMostX][nMostY].Freezed = false;
    };

    AMMoveOK stAMMOK;
    stAMMOK.UID   = UID();
    stAMMOK.MapID = ID();
    stAMMOK.X     = stAMTM.X;
    stAMMOK.Y     = stAMTM.Y;
    stAMMOK.EndX  = nMostX;
    stAMMOK.EndY  = nMostY;

    m_CellRecordV2D[nMostX][nMostY].Freezed = true;
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

    int nX = -1;
    int nY = -1;
    if(RandomLocation(&nX, &nY)){
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
            m_CellRecordV2D[stAMMSOK.X][stAMMSOK.Y].Freezed = false;
        };
        m_CellRecordV2D[nX][nY].Freezed = true;
        m_ActorPod->Forward({MPK_MAPSWITCHOK, stAMMSOK}, rstFromAddr, rstMPK.ID(), fnOnResp);
    }else{
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

    ServerPathFinder stPathFinder(this, stAMPF.CheckCO);
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

