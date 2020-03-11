/*
 * =====================================================================================
 *
 *       Filename: servermapop.cpp
 *        Created: 05/03/2016 20:21:32
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
#include "dbcomid.hpp"
#include "monster.hpp"
#include "strfunc.hpp"
#include "mathfunc.hpp"
#include "sysconst.hpp"
#include "actorpod.hpp"
#include "servermap.hpp"
#include "monoserver.hpp"
#include "rotatecoord.hpp"
#include "dbcomrecord.hpp"
#include "serverargparser.hpp"

extern MonoServer *g_MonoServer;
extern ServerArgParser *g_ServerArgParser;

void ServerMap::On_MPK_METRONOME(const MessagePack &)
{
    if(m_LuaModule && !g_ServerArgParser->DisableMapScript){
        m_LuaModule->LoopOne();
    }
}

void ServerMap::On_MPK_BADACTORPOD(const MessagePack &)
{
}

void ServerMap::On_MPK_ACTION(const MessagePack &rstMPK)
{
    AMAction stAMA;
    std::memcpy(&stAMA, rstMPK.Data(), sizeof(stAMA));

    if(ValidC(stAMA.X, stAMA.Y)){
        DoCircle(stAMA.X, stAMA.Y, 10, [this, stAMA](int nX, int nY) -> bool
        {
            if(true || ValidC(nX, nY)){
                DoUIDList(nX, nY, [this, stAMA](uint64_t nUID) -> bool
                {
                    if(nUID != stAMA.UID){
                        if(auto nType = UIDFunc::GetUIDType(nUID); nType == UID_PLY || nType == UID_MON){
                            m_ActorPod->Forward(nUID, {MPK_ACTION, stAMA});
                        }
                    }
                    return false;
                });
            }
            return false;
        });
    }
}

void ServerMap::On_MPK_ADDCHAROBJECT(const MessagePack &rstMPK)
{
    AMAddCharObject stAMACO;
    std::memcpy(&stAMACO, rstMPK.Data(), sizeof(stAMACO));

    auto nX = stAMACO.Common.X;
    auto nY = stAMACO.Common.Y;
    auto bStrictLoc = stAMACO.Common.StrictLoc;

    switch(stAMACO.Type){
        case TYPE_MONSTER:
            {
                auto nMonsterID = stAMACO.Monster.MonsterID;
                auto nMasterUID = stAMACO.Monster.MasterUID;

                if(AddMonster(nMonsterID, nMasterUID, nX, nY, bStrictLoc)){
                    m_ActorPod->Forward(rstMPK.From(), MPK_OK, rstMPK.ID());
                    return;
                }

                m_ActorPod->Forward(rstMPK.From(), MPK_ERROR, rstMPK.ID());
                return;
            }
        case TYPE_PLAYER:
            {
                auto nDBID      = stAMACO.Player.DBID;
                auto nChannID   = stAMACO.Player.ChannID;
                auto nDirection = stAMACO.Player.Direction;

                if(auto pPlayer = AddPlayer(nDBID, nX, nY, nDirection, bStrictLoc)){
                    m_ActorPod->Forward(rstMPK.From(), MPK_OK, rstMPK.ID());
                    m_ActorPod->Forward(pPlayer->UID(), {MPK_BINDCHANNEL, nChannID});

                    DoCircle(nX, nY, 20, [this, nChannID](int nX, int nY) -> bool
                    {
                        if(true || ValidC(nX, nY)){
                            // ReportGroundItem(nChannID, nX, nY);
                        }
                        return false;
                    });
                    return;
                }

                m_ActorPod->Forward(rstMPK.From(), MPK_ERROR, rstMPK.ID());
                return;
            }
        default:
            {
                m_ActorPod->Forward(rstMPK.From(), MPK_ERROR, rstMPK.ID());
                return;
            }
    }
}

void ServerMap::On_MPK_TRYSPACEMOVE(const MessagePack &rstMPK)
{
    AMTrySpaceMove stAMTSM;
    std::memcpy(&stAMTSM, rstMPK.Data(), sizeof(stAMTSM));

    int nDstX = stAMTSM.X;
    int nDstY = stAMTSM.Y;

    if(!ValidC(stAMTSM.X, stAMTSM.Y)){
        if(stAMTSM.StrictMove){
            m_ActorPod->Forward(rstMPK.From(), MPK_ERROR, rstMPK.ID());
            return;
        }

        nDstX = std::rand() % W();
        nDstY = std::rand() % H();
    }

    bool bDstOK = false;
    std::tie(bDstOK, nDstX, nDstY) = GetValidGrid(false, false, stAMTSM.StrictMove ? 1 : 100, nDstX, nDstY);

    if(!bDstOK){
        m_ActorPod->Forward(rstMPK.From(), MPK_ERROR, rstMPK.ID());
        return;
    }

    // get valid location
    // respond with MPK_SPACEMOVEOK and wait for response

    AMSpaceMoveOK stAMSMOK;
    std::memset(&stAMSMOK, 0, sizeof(stAMSMOK));

    stAMSMOK.Ptr = this;
    stAMSMOK.X   = nDstX;
    stAMSMOK.Y   = nDstY;

    m_ActorPod->Forward(rstMPK.From(), {MPK_SPACEMOVEOK, stAMSMOK}, rstMPK.ID(), [this, nUID = stAMTSM.UID, nDstX, nDstY](const MessagePack &rstRMPK)
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

                    AddGridUID(nUID, nDstX, nDstY, true);
                    break;
                }
            default:
                {
                    break;
                }
        }
    });
}

void ServerMap::On_MPK_TRYMOVE(const MessagePack &rstMPK)
{
    AMTryMove stAMTM;
    std::memcpy(&stAMTM, rstMPK.Data(), sizeof(stAMTM));

    auto fnPrintMoveError = [&stAMTM]()
    {
        g_MonoServer->AddLog(LOGTYPE_WARNING, "TRYMOVE[%p]::UID           = %" PRIu32 , &stAMTM, stAMTM.UID);
        g_MonoServer->AddLog(LOGTYPE_WARNING, "TRYMOVE[%p]::MapID         = %" PRIu32 , &stAMTM, stAMTM.MapID);
        g_MonoServer->AddLog(LOGTYPE_WARNING, "TRYMOVE[%p]::X             = %d"       , &stAMTM, stAMTM.X);
        g_MonoServer->AddLog(LOGTYPE_WARNING, "TRYMOVE[%p]::Y             = %d"       , &stAMTM, stAMTM.Y);
        g_MonoServer->AddLog(LOGTYPE_WARNING, "TRYMOVE[%p]::EndX          = %d"       , &stAMTM, stAMTM.EndX);
        g_MonoServer->AddLog(LOGTYPE_WARNING, "TRYMOVE[%p]::EndY          = %d"       , &stAMTM, stAMTM.EndY);
        g_MonoServer->AddLog(LOGTYPE_WARNING, "TRYMOVE[%p]::AllowHalfMove = %s"       , &stAMTM, stAMTM.AllowHalfMove ? "true" : "false");
    };

    if(!In(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid location: (X, Y)");
        fnPrintMoveError();
        m_ActorPod->Forward(rstMPK.From(), MPK_ERROR, rstMPK.ID());
        return;
    }

    // we never allow server to handle motion to invalid grid
    // for client every motion request need to be prepared to avoid this

    if(!In(stAMTM.MapID, stAMTM.EndX, stAMTM.EndY)){
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid location: (EndX, EndY)");
        fnPrintMoveError();
        m_ActorPod->Forward(rstMPK.From(), MPK_ERROR, rstMPK.ID());
        return;
    }

    bool bFindCO = false;
    for(auto nUID: GetUIDListRef(stAMTM.X, stAMTM.Y)){
        if(nUID == stAMTM.UID){
            bFindCO = true;
            break;
        }
    }

    if(!bFindCO){
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Can't find CO at current location: (UID, X, Y)");
        fnPrintMoveError();
        m_ActorPod->Forward(rstMPK.From(), MPK_ERROR, rstMPK.ID());
        return;
    }

    int nStepSize = -1;
    switch(MathFunc::LDistance2(stAMTM.X, stAMTM.Y, stAMTM.EndX, stAMTM.EndY)){
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
                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid move request: (X, Y) -> (EndX, EndY)");
                fnPrintMoveError();
                m_ActorPod->Forward(rstMPK.From(), MPK_ERROR, rstMPK.ID());
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
        m_ActorPod->Forward(rstMPK.From(), MPK_ERROR, rstMPK.ID());
        return;
    }

    AMMoveOK stAMMOK;
    std::memset(&stAMMOK, 0, sizeof(stAMMOK));

    stAMMOK.UID   = UID();
    stAMMOK.MapID = ID();
    stAMMOK.X     = stAMTM.X;
    stAMMOK.Y     = stAMTM.Y;
    stAMMOK.EndX  = nMostX;
    stAMMOK.EndY  = nMostY;

    GetCell(nMostX, nMostY).Locked = true;
    m_ActorPod->Forward(rstMPK.From(), {MPK_MOVEOK, stAMMOK}, rstMPK.ID(), [this, stAMTM, nMostX, nMostY](const MessagePack &rstRMPK)
    {
        if(!GetCell(nMostX, nMostY).Locked){
            throw std::runtime_error(str_fflprintf(": Cell lock released before MOVEOK get responsed: MapUID = %" PRIu64, UID()));
        }
        GetCell(nMostX, nMostY).Locked = false;

        switch(rstRMPK.Type()){
            case MPK_OK:
                {
                    // the object comfirm to move
                    // and it's internal state has changed

                    // 1. leave last cell
                    bool bFindCO = false;
                    auto &rstUIDList = GetUIDListRef(stAMTM.X, stAMTM.Y);

                    for(auto &nUID: rstUIDList){
                        if(nUID == stAMTM.UID){
                            bFindCO = true;
                            std::swap(nUID, rstUIDList.back());
                            rstUIDList.pop_back();
                            break;
                        }
                    }

                    if(!bFindCO){
                        throw std::runtime_error(str_fflprintf("CO location error: (UID = %" PRIu32 ", X = %d, Y = %d)", stAMTM.UID, stAMTM.X, stAMTM.Y));
                    }

                    // 2. push to the new cell
                    //    check if it should switch the map
                    AddGridUID(stAMTM.UID, nMostX, nMostY, true);
                    if(UIDFunc::GetUIDType(stAMTM.UID) == UID_PLY && GetCell(nMostX, nMostY).MapID){
                        AMMapSwitch stAMMS;
                        std::memset(&stAMMS, 0, sizeof(stAMMS));

                        stAMMS.UID   = UIDFunc::GetMapUID(GetCell(nMostX, nMostY).MapID);
                        stAMMS.MapID = GetCell(nMostX, nMostY).MapID;
                        stAMMS.X     = GetCell(nMostX, nMostY).SwitchX;
                        stAMMS.Y     = GetCell(nMostX, nMostY).SwitchY;
                        m_ActorPod->Forward(stAMTM.UID, {MPK_MAPSWITCH, stAMMS});
                    }
                    break;
                }
            default:
                {
                    break;
                }
        }
    });
}

void ServerMap::On_MPK_TRYLEAVE(const MessagePack &rstMPK)
{
    AMTryLeave stAMTL;
    std::memcpy(&stAMTL, rstMPK.Data(), rstMPK.DataLen());

    if(stAMTL.UID && In(stAMTL.MapID, stAMTL.X, stAMTL.Y)){
        for(auto nUID: GetUIDListRef(stAMTL.X, stAMTL.Y)){
            if(nUID == stAMTL.UID){
                RemoveGridUID(nUID, stAMTL.X, stAMTL.Y);
                m_ActorPod->Forward(rstMPK.From(), MPK_OK, rstMPK.ID());
                return;
            }
        }
    }

    // otherwise try leave failed
    // we reply MPK_ERROR but this is already something wrong, map never prevert leave on purpose
    m_ActorPod->Forward(rstMPK.From(), MPK_ERROR, rstMPK.ID());
    g_MonoServer->AddLog(LOGTYPE_WARNING, "Leave request failed: UID = %" PRIu64 ", X = %d, Y = %d", stAMTL.UID, stAMTL.X, stAMTL.Y);
}

void ServerMap::On_MPK_PULLCOINFO(const MessagePack &rstMPK)
{
    AMPullCOInfo stAMPCOI;
    std::memcpy(&stAMPCOI, rstMPK.Data(), sizeof(stAMPCOI));

    DoCenterSquare(stAMPCOI.X, stAMPCOI.Y, stAMPCOI.W, stAMPCOI.H, false, [this, stAMPCOI](int nX, int nY) -> bool
    {
        if(true || ValidC(nX, nY)){
            DoUIDList(nX, nY, [this, stAMPCOI](uint64_t nUID) -> bool
            {
                if(nUID != stAMPCOI.UID){
                    if(UIDFunc::GetUIDType(nUID) == UID_PLY || UIDFunc::GetUIDType(nUID) == UID_MON){
                        AMQueryCORecord stAMQCOR;
                        std::memset(&stAMQCOR, 0, sizeof(stAMQCOR));

                        stAMQCOR.UID = stAMPCOI.UID;
                        m_ActorPod->Forward(nUID, {MPK_QUERYCORECORD, stAMQCOR});
                    }
                }
                return false;
            });
        }
        return false;
    });
}

void ServerMap::On_MPK_TRYMAPSWITCH(const MessagePack &rstMPK)
{
    AMTryMapSwitch stAMTMS;
    std::memcpy(&stAMTMS, rstMPK.Data(), sizeof(stAMTMS));

    int nX = stAMTMS.EndX;
    int nY = stAMTMS.EndY;

    if(!CanMove(false, false, nX, nY)){
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Requested map switch location is invalid: MapName = %s, X = %d, Y = %d", DBCOM_MAPRECORD(ID()).Name, nX, nY);
        m_ActorPod->Forward(rstMPK.From(), MPK_ERROR, rstMPK.ID());
        return;
    }

    AMMapSwitchOK stAMMSOK;
    std::memset(&stAMMSOK, 0, sizeof(stAMMSOK));

    stAMMSOK.Ptr = this;
    stAMMSOK.X   = nX;
    stAMMSOK.Y   = nY;

    GetCell(nX, nY).Locked = true;
    m_ActorPod->Forward(rstMPK.From(), {MPK_MAPSWITCHOK, stAMMSOK}, rstMPK.ID(), [this, stAMTMS, stAMMSOK](const MessagePack &rstRMPK)
    {
        if(!GetCell(stAMMSOK.X, stAMMSOK.Y).Locked){
            throw std::runtime_error(str_fflprintf("Cell lock released before MAPSWITCHOK get responsed: MapUID = %" PRIu64, UID()));
        }

        GetCell(stAMMSOK.X, stAMMSOK.Y).Locked = false;
        switch(rstRMPK.Type()){
            case MPK_OK:
                {
                    // didn't check map switch here
                    // map switch should be triggered by move request
                    AddGridUID(stAMTMS.UID, stAMMSOK.X, stAMMSOK.Y, true);
                    break;
                }
            default:
                {
                    break;
                }
        }
    });
}

void ServerMap::On_MPK_PATHFIND(const MessagePack &rstMPK)
{
    AMPathFind stAMPF;
    std::memcpy(&stAMPF, rstMPK.Data(), sizeof(stAMPF));

    int nX0 = stAMPF.X;
    int nY0 = stAMPF.Y;
    int nX1 = stAMPF.EndX;
    int nY1 = stAMPF.EndY;

    AMPathFindOK stAMPFOK;
    std::memset(&stAMPFOK, 0, sizeof(stAMPFOK));

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
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid MaxStep: %d, should be (1, 2, 3)", stAMPF.MaxStep);
    }

    ServerPathFinder stPathFinder(this, stAMPF.MaxStep, stAMPF.CheckCO);
    if(!stPathFinder.Search(nX0, nY0, nX1, nY1)){
        m_ActorPod->Forward(rstMPK.From(), MPK_ERROR, rstMPK.ID());
        return;
    }

    // drop the first node
    // it's should be the provided start point
    if(!stPathFinder.GetSolutionStart()){
        m_ActorPod->Forward(rstMPK.From(), MPK_ERROR, rstMPK.ID());
        return;
    }

    int nCurrN = 0;
    int nCurrX = nX0;
    int nCurrY = nY0;

    while(auto pNode1 = stPathFinder.GetSolutionNext()){
        if(nCurrN >= (int)(nPathCount)){
            break;
        }
        int nEndX = pNode1->X();
        int nEndY = pNode1->Y();
        switch(MathFunc::LDistance2(nCurrX, nCurrY, nEndX, nEndY)){
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
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid path node found");
                    break;
                }
        }
    }
    m_ActorPod->Forward(rstMPK.From(), {MPK_PATHFINDOK, stAMPFOK}, rstMPK.ID());
}

void ServerMap::On_MPK_UPDATEHP(const MessagePack &rstMPK)
{
    AMUpdateHP stAMUHP;
    std::memcpy(&stAMUHP, rstMPK.Data(), sizeof(stAMUHP));

    if(ValidC(stAMUHP.X, stAMUHP.Y)){
        DoCircle(stAMUHP.X, stAMUHP.Y, 20, [this, stAMUHP](int nX, int nY) -> bool
        {
            if(true || ValidC(nX, nY)){
                for(auto nUID: GetUIDListRef(nX, nY)){
                    if(nUID != stAMUHP.UID){
                        if(UIDFunc::GetUIDType(nUID) == UID_PLY || UIDFunc::GetUIDType(nUID) == UID_MON){
                            m_ActorPod->Forward(nUID, {MPK_UPDATEHP, stAMUHP});
                        }
                    }
                }
            }
            return false;
        });
    }
}

void ServerMap::On_MPK_DEADFADEOUT(const MessagePack &rstMPK)
{
    AMDeadFadeOut stAMDFO;
    std::memcpy(&stAMDFO, rstMPK.Data(), sizeof(stAMDFO));

    if(ValidC(stAMDFO.X, stAMDFO.Y)){
        RemoveGridUID(stAMDFO.UID, stAMDFO.X, stAMDFO.Y);
        DoCircle(stAMDFO.X, stAMDFO.Y, 20, [this, stAMDFO](int nX, int nY) -> bool
        {
            if(true || ValidC(nX, nY)){
                for(auto nUID: GetUIDListRef(nX, nY)){
                    if(nUID != stAMDFO.UID){
                        if(UIDFunc::GetUIDType(nUID) == UID_PLY || UIDFunc::GetUIDType(nUID) == UID_MON){
                            m_ActorPod->Forward(nUID, {MPK_DEADFADEOUT, stAMDFO});
                        }
                    }
                }
            }
            return false;
        });
    }
}

void ServerMap::On_MPK_QUERYCOCOUNT(const MessagePack &rstMPK)
{
    AMQueryCOCount stAMQCOC;
    std::memcpy(&stAMQCOC, rstMPK.Data(), sizeof(stAMQCOC));

    if(stAMQCOC.MapID && (stAMQCOC.MapID != ID())){
        m_ActorPod->Forward(rstMPK.From(), MPK_ERROR, rstMPK.ID());
        return;
    }

    int nCOCount = 0;
    for(int nX = 0; nX < W(); ++nX){
        for(int nY = 0; nY < H(); ++nY){
            std::for_each(GetUIDListRef(nX, nY).begin(), GetUIDListRef(nX, nY).end(), [stAMQCOC, &nCOCount](uint64_t nUID){
                if(UIDFunc::GetUIDType(nUID) == UID_PLY || UIDFunc::GetUIDType(nUID) == UID_MON){
                    if(stAMQCOC.Check.NPC    ){ nCOCount++; return; }
                    if(stAMQCOC.Check.Player ){ nCOCount++; return; }
                    if(stAMQCOC.Check.Monster){ nCOCount++; return; }
                }
            });
        }
    }

    AMCOCount stAMCOC;
    std::memset(&stAMCOC, 0, sizeof(stAMCOC));

    stAMCOC.Count = nCOCount;
    m_ActorPod->Forward(rstMPK.From(), {MPK_COCOUNT, stAMCOC}, rstMPK.ID());
}

void ServerMap::On_MPK_QUERYRECTUIDLIST(const MessagePack &rstMPK)
{
    AMQueryRectUIDList stAMQRUIDL;
    std::memcpy(&stAMQRUIDL, rstMPK.Data(), sizeof(stAMQRUIDL));

    AMUIDList stAMUIDL;
    std::memset(&stAMUIDL, 0, sizeof(stAMUIDL));

    size_t nIndex = 0;
    for(int nY = stAMQRUIDL.Y; nY < stAMQRUIDL.Y + stAMQRUIDL.H; ++nY){
        for(int nX = stAMQRUIDL.X; nX < stAMQRUIDL.X + stAMQRUIDL.W; ++nX){
            if(In(stAMQRUIDL.MapID, nX, nY)){
                for(auto nUID: GetUIDListRef(nX, nY)){
                    stAMUIDL.UIDList[nIndex++] = nUID;
                }
            }
        }
    }

    m_ActorPod->Forward(rstMPK.From(), {MPK_UIDLIST, stAMUIDL}, rstMPK.ID());
}

void ServerMap::On_MPK_NEWDROPITEM(const MessagePack &rstMPK)
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

            RotateCoord stRC(stAMNDI.X, stAMNDI.Y, 0, 0, W(), H());
            do{
                if(GroundValid(stRC.X(), stRC.Y())){

                    // valid grid
                    // check if grid good to hold

                    if(auto nCurrCount = GetGroundItemList(stRC.X(), stRC.Y()).Length(); (int)(nCurrCount) < nMinCount){
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

void ServerMap::On_MPK_OFFLINE(const MessagePack &rstMPK)
{
    AMOffline stAMO;
    std::memcpy(&stAMO, rstMPK.Data(), sizeof(stAMO));

    // this may fail
    // because player may get offline at try move
    RemoveGridUID(stAMO.UID, stAMO.X, stAMO.Y);

    DoCircle(stAMO.X, stAMO.Y, 10, [stAMO, this](int nX, int nY) -> bool
    {
        if(true || ValidC(nX, nY)){
            for(auto nUID: GetUIDListRef(nX, nY)){
                if(nUID != stAMO.UID){
                    m_ActorPod->Forward(nUID, {MPK_OFFLINE, stAMO});
                }
            }
        }
        return false;
    });
}

void ServerMap::On_MPK_PICKUP(const MessagePack &rstMPK)
{
    AMPickUp stAMPU;
    std::memcpy(&stAMPU, rstMPK.Data(), sizeof(stAMPU));

    if(!ValidC(stAMPU.X, stAMPU.Y) || !stAMPU.ID){
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid pickup request: X = %d, Y = %d, ID = %" PRIu32, stAMPU.X, stAMPU.Y, stAMPU.ID);
        return;
    }

    if(auto nIndex = FindGroundItem(CommonItem(stAMPU.ID, 0), stAMPU.X, stAMPU.Y); nIndex >= 0){
        RemoveGroundItem(CommonItem(stAMPU.ID, 0), stAMPU.X, stAMPU.Y);
        DoCircle(stAMPU.X, stAMPU.Y, 10, [this, stAMPU](int nX, int nY) -> bool
        {
            if(true || ValidC(nX, nY)){
                AMRemoveGroundItem stAMRGI;
                std::memset(&stAMRGI, 0, sizeof(stAMRGI));

                stAMRGI.X      = nX;
                stAMRGI.Y      = nY;
                stAMRGI.DBID   = stAMPU.DBID;
                stAMRGI.ID = stAMPU.ID;

                DoUIDList(nX, nY, [this, &stAMRGI](uint64_t nUID) -> bool
                {
                    if(UIDFunc::GetUIDType(nUID) == UID_PLY){
                        m_ActorPod->Forward(nUID, {MPK_REMOVEGROUNDITEM, stAMRGI});
                    }
                    return false;
                });
            }
            return false;
        });

        AMPickUpOK stAMPUOK;
        std::memset(&stAMPUOK, 0, sizeof(stAMPUOK));

        stAMPUOK.X    = stAMPU.X;
        stAMPUOK.Y    = stAMPU.Y;
        stAMPUOK.UID  = stAMPU.UID;
        stAMPUOK.DBID = 0;
        stAMPUOK.ID   = stAMPU.ID;
        m_ActorPod->Forward(stAMPU.UID, {MPK_PICKUPOK, stAMPUOK});
    }else{
        // no such item
        // likely the client need re-sync for the gound items
    }
}
