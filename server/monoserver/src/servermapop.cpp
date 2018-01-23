/*
 * =====================================================================================
 *
 *       Filename: servermapop.cpp
 *        Created: 05/03/2016 20:21:32
 *  Last Modified: 01/23/2018 12:45:45
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
#include "serverenv.hpp"
#include "metronome.hpp"
#include "servermap.hpp"
#include "monoserver.hpp"
#include "rotatecoord.hpp"
#include "dbcomrecord.hpp"

void ServerMap::On_MPK_METRONOME(const MessagePack &, const Theron::Address &)
{
    extern ServerEnv *g_ServerEnv;
    if(m_LuaModule && !g_ServerEnv->DisableMapScript){

        // could this slow down the server
        // if so I have to move it to a seperated thread

        m_LuaModule->LoopOne();
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
        auto fnNotifyAction = [this, stAMA](int nX, int nY) -> bool
        {
            if(true || ValidC(nX, nY)){
                auto fnDoList = [this, stAMA](const UIDRecord &rstUIDRecord) -> bool
                {
                    if(rstUIDRecord.UID() != stAMA.UID){
                        if(false
                                || rstUIDRecord.ClassFrom<Player >()
                                || rstUIDRecord.ClassFrom<Monster>()){
                            m_ActorPod->Forward({MPK_ACTION, stAMA}, rstUIDRecord.GetAddress());
                        }
                    }
                    return false;
                };
                DoUIDList(nX, nY, fnDoList);
            }
            return false;
        };
        DoCircle(stAMA.X, stAMA.Y, 10, fnNotifyAction);
    }
}

void ServerMap::On_MPK_ADDCHAROBJECT(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMAddCharObject stAMACO;
    std::memcpy(&stAMACO, rstMPK.Data(), sizeof(stAMACO));

    auto nX = stAMACO.Common.X;
    auto nY = stAMACO.Common.Y;

    auto bRandom = stAMACO.Common.Random;

    switch(stAMACO.Type){
        case TYPE_MONSTER:
            {
                auto nMonsterID = stAMACO.Monster.MonsterID;
                auto nMasterUID = stAMACO.Monster.MasterUID;

                if(AddMonster(nMonsterID, nMasterUID, nX, nY, bRandom)){
                    m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
                    return;
                }
                break;
            }
        case TYPE_PLAYER:
            {
                auto nDBID      = stAMACO.Player.DBID;
                auto nSessionID = stAMACO.Player.SessionID;
                auto nDirection = stAMACO.Player.Direction;

                if(auto pPlayer = AddPlayer(nDBID, nX, nY, nDirection, bRandom)){
                    m_ActorPod->Forward(MPK_OK, rstFromAddr, rstMPK.ID());
                    m_ActorPod->Forward({MPK_BINDSESSION, nSessionID}, pPlayer->GetAddress());

                    auto fnReportGroundItem = [this, nSessionID](int nX, int nY) -> bool
                    {
                        if(true || ValidC(nX, nY)){
                            // ReportGroundItem(nSessionID, nX, nY);
                        }
                        return false;
                    };

                    DoCircle(nX, nY, 20, fnReportGroundItem);
                    return;
                }
                break;
            }
        default:
            {
                break;
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

    auto fnPrintMoveError = [&stAMTM]()
    {
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "[%p]::UID           = %" PRIu32 , &stAMTM, stAMTM.UID);
        g_MonoServer->AddLog(LOGTYPE_WARNING, "[%p]::MapID         = %" PRIu32 , &stAMTM, stAMTM.MapID);
        g_MonoServer->AddLog(LOGTYPE_WARNING, "[%p]::X             = %d"       , &stAMTM, stAMTM.X);
        g_MonoServer->AddLog(LOGTYPE_WARNING, "[%p]::Y             = %d"       , &stAMTM, stAMTM.Y);
        g_MonoServer->AddLog(LOGTYPE_WARNING, "[%p]::EndX          = %d"       , &stAMTM, stAMTM.EndX);
        g_MonoServer->AddLog(LOGTYPE_WARNING, "[%p]::EndY          = %d"       , &stAMTM, stAMTM.EndY);
        g_MonoServer->AddLog(LOGTYPE_WARNING, "[%p]::AllowHalfMove = %s"       , &stAMTM, stAMTM.AllowHalfMove ? "true" : "false");
    };

    if(!In(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid location: (X, Y)");
        fnPrintMoveError();
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // we never allow server to handle motion to invalid grid
    // for client every motion request need to be prepared to avoid this

    if(!In(stAMTM.MapID, stAMTM.EndX, stAMTM.EndY)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid location: (EndX, EndY)");
        fnPrintMoveError();
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    bool bFindCO = false;
    for(auto nUID: m_CellRecordV2D[stAMTM.X][stAMTM.Y].UIDList){
        if(nUID == stAMTM.UID){
            bFindCO = true;
            break;
        }
    }

    if(!bFindCO){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Can't find CO at current location: (UID, X, Y)");
        fnPrintMoveError();
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    int nStepSize = -1;
    switch(LDistance2(stAMTM.X, stAMTM.Y, stAMTM.EndX, stAMTM.EndY)){
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
                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid move request: (X, Y) -> (EndX, EndY)");
                fnPrintMoveError();
                m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
                return;
            }
    }

    bool bCheckMove = false;
    auto nMostX = stAMTM.EndX;
    auto nMostY = stAMTM.EndY;

    switch(nStepSize){
        case 1:
            {
                bCheckMove = false;
                if(CanMove(true, true, stAMTM.EndX, stAMTM.EndY)){
                    bCheckMove = true;
                }
                break;
            }
        default:
            {
                // for step size > 1
                // we check the end grids for CO and Lock
                // but for middle grids we only check the CO, no Lock

                condcheck(nStepSize == 2 || nStepSize == 3);

                int nDX = (stAMTM.EndX > stAMTM.X) - (stAMTM.EndX < stAMTM.X);
                int nDY = (stAMTM.EndY > stAMTM.Y) - (stAMTM.EndY < stAMTM.Y);

                bCheckMove = true;
                if(CanMove(true, true, stAMTM.EndX, stAMTM.EndY)){
                    for(int nIndex = 1; nIndex < nStepSize; ++nIndex){
                        if(!CanMove(true, false, stAMTM.X + nDX * nIndex, stAMTM.Y + nDY * nIndex)){
                            bCheckMove = false;
                            break;
                        }
                    }
                }

                if(!bCheckMove){
                    if(true
                            && stAMTM.AllowHalfMove
                            && CanMove(true, true, stAMTM.X + nDX, stAMTM.Y + nDY)){
                        bCheckMove = true;
                        nMostX = stAMTM.X + nDX;
                        nMostY = stAMTM.Y + nDY;
                    }
                }
                break;
            }
    }

    if(!bCheckMove){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    auto fnOnR = [this, stAMTM, nMostX, nMostY](const MessagePack &rstRMPK, const Theron::Address &)
    {
        switch(rstRMPK.Type()){
            case MPK_OK:
                {
                    // the object comfirm to move
                    // and it's internal state has changed

                    // 1. leave last cell
                    {
                        bool bFindCO = false;
                        auto &rstRecordV = m_CellRecordV2D[stAMTM.X][stAMTM.Y].UIDList;

                        for(auto &nUID: rstRecordV){
                            if(nUID == stAMTM.UID){
                                // 1. mark as find
                                bFindCO = true;

                                // 2. remove from the object list
                                std::swap(nUID, rstRecordV.back());
                                rstRecordV.pop_back();

                                break;
                            }
                        }

                        if(!bFindCO){
                            extern MonoServer *g_MonoServer;
                            g_MonoServer->AddLog(LOGTYPE_FATAL, "CO location error: (UD = %" PRIu32 ", X = %d, Y = %d)", stAMTM.UID, stAMTM.X, stAMTM.Y);
                            g_MonoServer->Restart();
                        }
                    }

                    // 2. push it to the new cell
                    //    check if it should switch the map
                    extern MonoServer *g_MonoServer;
                    if(auto stRecord = g_MonoServer->GetUIDRecord(stAMTM.UID)){
                        AddGridUID(stAMTM.UID, nMostX, nMostY);
                        if(true
                                && stRecord.ClassFrom<Player>()
                                && m_CellRecordV2D[nMostX][nMostY].MapID){
                            if(m_CellRecordV2D[nMostX][nMostY].UID){
                                AMMapSwitch stAMMS;
                                stAMMS.UID   = m_CellRecordV2D[nMostX][nMostY].UID;
                                stAMMS.MapID = m_CellRecordV2D[nMostX][nMostY].MapID;
                                stAMMS.X     = m_CellRecordV2D[nMostX][nMostY].SwitchX;
                                stAMMS.Y     = m_CellRecordV2D[nMostX][nMostY].SwitchY;
                                m_ActorPod->Forward({MPK_MAPSWITCH, stAMMS}, stRecord.GetAddress());
                            }else{
                                switch(m_CellRecordV2D[nMostX][nMostY].Query){
                                    case QUERY_NONE:
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
                                                                m_ActorPod->Forward({MPK_MAPSWITCH, stAMMS}, stRecord.GetAddress());
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
        auto &rstRecordV = m_CellRecordV2D[stAMTL.X][stAMTL.Y].UIDList;
        for(auto &nUID: rstRecordV){
            if(nUID == stAMTL.UID){
                RemoveGridUID(nUID, stAMTL.X, stAMTL.Y);
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

    auto fnPullCOInfo = [this, stAMPCOI](int nX, int nY) -> bool
    {
        if(true || ValidC(nX, nY)){
            auto fnDoList = [this, stAMPCOI](const UIDRecord &rstUIDRecord) -> bool
            {
                if(rstUIDRecord.UID() != stAMPCOI.UID){
                    if(false
                            || rstUIDRecord.ClassFrom<Player >()
                            || rstUIDRecord.ClassFrom<Monster>()){
                        m_ActorPod->Forward({MPK_PULLCOINFO, stAMPCOI}, rstUIDRecord.GetAddress());
                    }
                }
                return false;
            };
            DoUIDList(nX, nY, fnDoList);
        }
        return false;
    };
    DoCircle(stAMPCOI.X, stAMPCOI.Y, 10, fnPullCOInfo);
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

        auto fnOnResp = [this, stAMTMS, stAMMSOK](const MessagePack &rstRMPK, const Theron::Address &)
        {
            switch(rstRMPK.Type()){
                case MPK_OK:
                    {
                        extern MonoServer *g_MonoServer;
                        if(auto stRecord = g_MonoServer->GetUIDRecord(stAMTMS.UID)){
                            AddGridUID(stRecord.UID(), stAMMSOK.X, stAMMSOK.Y);
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
    constexpr auto nPathCount = std::extent<decltype(stAMPFOK.Point)>::value;
    for(int nIndex = 0; nIndex < (int)(nPathCount); ++nIndex){
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
            if(nCurrN >= (int)(nPathCount)){ break; }
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
        auto fnUpdateHP = [this, stAMUHP](int nX, int nY) -> bool
        {
            if(true || ValidC(nX, nY)){
                for(auto nUID: m_CellRecordV2D[nX][nY].UIDList){
                    if(nUID != stAMUHP.UID){
                        extern MonoServer *g_MonoServer;
                        if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
                            if(false
                                    || stUIDRecord.ClassFrom<Player >()
                                    || stUIDRecord.ClassFrom<Monster>()){
                                m_ActorPod->Forward({MPK_UPDATEHP, stAMUHP}, stUIDRecord.GetAddress());
                            }
                        }
                    }
                }
            }
            return false;
        };
        DoCircle(stAMUHP.X, stAMUHP.Y, 20, fnUpdateHP);
    }
}

void ServerMap::On_MPK_DEADFADEOUT(const MessagePack &rstMPK, const Theron::Address &)
{
    AMDeadFadeOut stAMDFO;
    std::memcpy(&stAMDFO, rstMPK.Data(), sizeof(stAMDFO));

    if(ValidC(stAMDFO.X, stAMDFO.Y)){
        auto fnDeadFadeOut = [this, stAMDFO](int nX, int nY) -> bool
        {
            if(true || ValidC(nX, nY)){
                for(auto nUID: m_CellRecordV2D[nX][nY].UIDList){
                    if(nUID != stAMDFO.UID){
                        extern MonoServer *g_MonoServer;
                        if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
                            if(stUIDRecord.ClassFrom<Player>()){
                                m_ActorPod->Forward({MPK_DEADFADEOUT, stAMDFO}, stUIDRecord.GetAddress());
                            }
                        }
                    }
                }
            }
            return false;
        };
        DoCircle(stAMDFO.X, stAMDFO.Y, 20, fnDeadFadeOut);
    }
}

void ServerMap::On_MPK_QUERYCORECORD(const MessagePack &rstMPK, const Theron::Address &)
{
    AMQueryCORecord stAMQCOR;
    std::memcpy(&stAMQCOR, rstMPK.Data(), sizeof(stAMQCOR));

    if(true
            && stAMQCOR.UID
            && stAMQCOR.AimUID
            && stAMQCOR.MapID == ID()
            && ValidC(stAMQCOR.X, stAMQCOR.Y)){

        auto fnQueryCORecord = [this, stAMQCOR](int nX, int nY) -> bool
        {
            if(true || ValidC(nX, nY)){
                for(auto nUID: m_CellRecordV2D[nX][nY].UIDList){
                    if(nUID == stAMQCOR.AimUID){
                        extern MonoServer *g_MonoServer;
                        if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
                            if(false
                                    || stUIDRecord.ClassFrom<Player >()
                                    || stUIDRecord.ClassFrom<Monster>()){

                                // FIXME
                                // here I didn't set other fields of PULLCOINFO
                                // PULLCOINFO and QUERYCORECORD need to be merged

                                AMPullCOInfo stAMPCOI;
                                std::memset(&stAMPCOI, 0, sizeof(stAMPCOI));

                                stAMPCOI.UID = stAMQCOR.UID;

                                m_ActorPod->Forward({MPK_PULLCOINFO, stAMPCOI}, stUIDRecord.GetAddress());
                                return true;
                            }
                        }
                    }
                }
            }
            return false;
        };
        DoCenterCircle(stAMQCOR.X, stAMQCOR.Y, 10, true, fnQueryCORecord);
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
        for(size_t nX = 0; nX < m_CellRecordV2D.size(); ++nX){
            for(size_t nY = 0; nY < m_CellRecordV2D[0].size(); ++nY){
                std::for_each(m_CellRecordV2D[nX][nY].UIDList.begin(), m_CellRecordV2D[nX][nY].UIDList.end(), [stAMQCOC, &nCOCount](uint32_t nUID){
                    extern MonoServer *g_MonoServer;
                    if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
                        if(false
                                || stUIDRecord.ClassFrom<Player >()
                                || stUIDRecord.ClassFrom<Monster>()){
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

void ServerMap::On_MPK_QUERYRECTUIDLIST(const MessagePack &rstMPK, const Theron::Address &rstAddress)
{
    AMQueryRectUIDList stAMQRUIDL;
    std::memcpy(&stAMQRUIDL, rstMPK.Data(), sizeof(stAMQRUIDL));

    AMUIDList stAMUIDL;
    std::memset(&stAMUIDL, 0, sizeof(stAMUIDL));

    size_t nIndex = 0;
    for(int nY = stAMQRUIDL.Y; nY < stAMQRUIDL.Y + stAMQRUIDL.H; ++nY){
        for(int nX = stAMQRUIDL.X; nX < stAMQRUIDL.X + stAMQRUIDL.W; ++nX){
            if(In(stAMQRUIDL.MapID, nX, nY)){
                for(auto nUID: m_CellRecordV2D[nX][nY].UIDList){
                    stAMUIDL.UIDList[nIndex++] = nUID;
                }
            }
        }
    }

    m_ActorPod->Forward({MPK_UIDLIST, stAMUIDL}, rstAddress, rstMPK.ID());
}

void ServerMap::On_MPK_NEWDROPITEM(const MessagePack &rstMPK, const Theron::Address &)
{
    AMNewDropItem stAMNDI;
    std::memcpy(&stAMNDI, rstMPK.Data(), sizeof(stAMNDI));

    if(true
            && stAMNDI.ID
            && stAMNDI.Value > 0
            && GroundValid(stAMNDI.X, stAMNDI.Y)){

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
                    if(GroundValid(stRC.X(), stRC.Y())){

                        // valid grid
                        // check if gird good to hold

                        auto nCurrCount = GetGroundItemList(stRC.X(), stRC.Y()).Length();
                        if((int)(nCurrCount) < nMinCount){
                            nMinCount = nCurrCount;
                            nBestX    = stRC.X();
                            nBestY    = stRC.Y();

                            // short it if it's an empty slot
                            // directly use it and won't compare more

                            if(nMinCount == 0){
                                break;
                            }
                        }
                    }
                }while(stRC.Forward() && (nCheckGrid++ <= SYS_MAXDROPITEMGRID));
            }

            if(GroundValid(nBestX, nBestY)){
                AddGroundItem(CommonItem(stAMNDI.ID, 0), nBestX, nBestY);
            }else{

                // we scanned the square but find we can't find a valid place
                // abort current operation since following check should also fail
                return;
            }
        }
    }
}

void ServerMap::On_MPK_OFFLINE(const MessagePack &rstMPK, const Theron::Address &)
{
    AMOffline stAMO;
    std::memcpy(&stAMO, rstMPK.Data(), sizeof(stAMO));
       
    auto fnNotifyOffline = [stAMO, this](int nX, int nY) -> bool
    {
        if(true || ValidC(nX, nY)){
            for(auto nUID: m_CellRecordV2D[nX][nY].UIDList){
                if(nUID != stAMO.UID){
                    extern MonoServer *g_MonoServer;
                    if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
                        m_ActorPod->Forward({MPK_OFFLINE, stAMO}, stUIDRecord.GetAddress());
                    }
                }
            }
        }
        return false;
    };

    DoCircle(stAMO.X, stAMO.Y, 10, fnNotifyOffline);
}

void ServerMap::On_MPK_PICKUP(const MessagePack &rstMPK, const Theron::Address &)
{
    AMPickUp stAMPU;
    std::memcpy(&stAMPU, rstMPK.Data(), sizeof(stAMPU));

    if(ValidC(stAMPU.X, stAMPU.Y) && stAMPU.ItemID){
        extern MonoServer *g_MonoServer;
        if(auto stUIDRecord = g_MonoServer->GetUIDRecord(stAMPU.UID)){
            auto nIndex = FindGroundItem(CommonItem(stAMPU.ItemID, 0), stAMPU.X, stAMPU.Y);
            if(nIndex >= 0){
                RemoveGroundItem(CommonItem(stAMPU.ItemID, 0), stAMPU.X, stAMPU.Y);
                auto fnRemoveGroundItem = [this, stAMPU](int nX, int nY) -> bool
                {
                    if(true || ValidC(nX, nY)){
                        for(auto nUID: m_CellRecordV2D[nX][nY].UIDList){
                            extern MonoServer *g_MonoServer;
                            if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
                                if(stUIDRecord.ClassFrom<Player>()){
                                    AMRemoveGroundItem stAMRGI;
                                    stAMRGI.X      = nX;
                                    stAMRGI.Y      = nY;
                                    stAMRGI.DBID   = stAMPU.DBID;
                                    stAMRGI.ItemID = stAMPU.ItemID;
                                    m_ActorPod->Forward({MPK_REMOVEGROUNDITEM, stAMRGI}, stUIDRecord.GetAddress());
                                }
                            }
                        }
                    }
                    return false;
                };
                DoCircle(stAMPU.X, stAMPU.Y, 10, fnRemoveGroundItem);

                // notify the picker
                AMPickUpOK stAMPUOK;
                stAMPUOK.X      = stAMPU.X;
                stAMPUOK.Y      = stAMPU.Y;
                stAMPUOK.UID    = stAMPU.UID;
                stAMPUOK.DBID   = 0;
                stAMPUOK.ItemID = stAMPU.ItemID;
                m_ActorPod->Forward({MPK_PICKUPOK, stAMPUOK}, stUIDRecord.GetAddress());
            }else{
                // no such item
                // likely the client need re-sync for the gound items
            }
        }
    }
}
