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
#include "strf.hpp"
#include "mathf.hpp"
#include "sysconst.hpp"
#include "actorpod.hpp"
#include "servermap.hpp"
#include "monoserver.hpp"
#include "rotatecoord.hpp"
#include "dbcomrecord.hpp"
#include "serverargparser.hpp"

extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;

void ServerMap::On_MPK_METRONOME(const MessagePack &)
{
    if(m_luaModulePtr && !g_serverArgParser->DisableMapScript){
        m_luaModulePtr->resumeLoop();
    }
}

void ServerMap::On_MPK_BADACTORPOD(const MessagePack &)
{
}

void ServerMap::On_MPK_ACTION(const MessagePack &rstMPK)
{
    const auto amA = rstMPK.conv<AMAction>();
    if(!ValidC(amA.X, amA.Y)){
        return;
    }

    doCircle(amA.X, amA.Y, 10, [this, amA](int nX, int nY) -> bool
    {
        if(true || ValidC(nX, nY)){
            doUIDList(nX, nY, [this, amA](uint64_t nUID) -> bool
            {
                if(nUID != amA.UID){
                    switch(uidf::getUIDType(nUID)){
                        case UID_PLY:
                        case UID_MON:
                        case UID_NPC:
                            {
                                m_actorPod->forward(nUID, {MPK_ACTION, amA});
                                break;
                            }
                        default:
                            {
                                break;
                            }
                    }
                }
                return false;
            });
        }
        return false;
    });
}

void ServerMap::On_MPK_ADDCHAROBJECT(const MessagePack &rstMPK)
{
    const auto stAMACO = rstMPK.conv<AMAddCharObject>();
    const auto nX = stAMACO.x;
    const auto nY = stAMACO.y;
    const bool bStrictLoc = stAMACO.strictLoc;

    switch(stAMACO.type){
        case UID_MON:
            {
                const auto nMonsterID = stAMACO.monster.monsterID;
                const auto nMasterUID = stAMACO.monster.masterUID;

                if(addMonster(nMonsterID, nMasterUID, nX, nY, bStrictLoc)){
                    m_actorPod->forward(rstMPK.from(), MPK_OK, rstMPK.ID());
                    return;
                }

                m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
                return;
            }
        case UID_PLY:
            {
                const auto nDBID      = stAMACO.player.DBID;
                const auto nChannID   = stAMACO.player.channID;
                const auto nDirection = stAMACO.player.direction;

                if(auto pPlayer = addPlayer(nDBID, nX, nY, nDirection, bStrictLoc)){
                    m_actorPod->forward(rstMPK.from(), MPK_OK, rstMPK.ID());
                    m_actorPod->forward(pPlayer->UID(), {MPK_BINDCHANNEL, nChannID});

                    doCircle(nX, nY, 20, [this, nChannID](int nX, int nY) -> bool
                    {
                        if(true || ValidC(nX, nY)){
                            // ReportGroundItem(nChannID, nX, nY);
                        }
                        return false;
                    });
                    return;
                }

                m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
                return;
            }
        case UID_NPC:
            {
                const auto npcID = stAMACO.NPC.NPCID;
                const auto x     = stAMACO.x;
                const auto y     = stAMACO.y;
                const auto strictLoc = stAMACO.strictLoc;
                const auto direction = stAMACO.NPC.direction;

                if(addNPChar(npcID, x, y, direction, strictLoc)){
                    m_actorPod->forward(rstMPK.from(), MPK_OK, rstMPK.ID());
                    doCircle(nX, nY, 20, [this](int nX, int nY) -> bool
                    {
                        if(true || ValidC(nX, nY)){
                            // ReportGroundItem(nChannID, nX, nY);
                        }
                        return false;
                    });
                    return;
                }

                m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
                return;
            }
        default:
            {
                m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
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
            m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
            return;
        }

        nDstX = std::rand() % W();
        nDstY = std::rand() % H();
    }

    bool bDstOK = false;
    std::tie(bDstOK, nDstX, nDstY) = GetValidGrid(false, false, stAMTSM.StrictMove ? 1 : 100, nDstX, nDstY);

    if(!bDstOK){
        m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
        return;
    }

    // get valid location
    // respond with MPK_SPACEMOVEOK and wait for response

    AMSpaceMoveOK stAMSMOK;
    std::memset(&stAMSMOK, 0, sizeof(stAMSMOK));

    stAMSMOK.Ptr = this;
    stAMSMOK.X   = nDstX;
    stAMSMOK.Y   = nDstY;

    m_actorPod->forward(rstMPK.from(), {MPK_SPACEMOVEOK, stAMSMOK}, rstMPK.ID(), [this, nUID = stAMTSM.UID, nDstX, nDstY](const MessagePack &rstRMPK)
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

                    addGridUID(nUID, nDstX, nDstY, true);
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
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::UID           = %" PRIu32 , &stAMTM, stAMTM.UID);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::MapID         = %" PRIu32 , &stAMTM, stAMTM.MapID);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::X             = %d"       , &stAMTM, stAMTM.X);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::Y             = %d"       , &stAMTM, stAMTM.Y);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::EndX          = %d"       , &stAMTM, stAMTM.EndX);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::EndY          = %d"       , &stAMTM, stAMTM.EndY);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::AllowHalfMove = %s"       , &stAMTM, stAMTM.AllowHalfMove ? "true" : "false");
    };

    if(!In(stAMTM.MapID, stAMTM.X, stAMTM.Y)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid location: (X, Y)");
        fnPrintMoveError();
        m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
        return;
    }

    // we never allow server to handle motion to invalid grid
    // for client every motion request need to be prepared to avoid this

    if(!In(stAMTM.MapID, stAMTM.EndX, stAMTM.EndY)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid location: (EndX, EndY)");
        fnPrintMoveError();
        m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
        return;
    }

    bool bFindCO = false;
    for(auto nUID: getUIDList(stAMTM.X, stAMTM.Y)){
        if(nUID == stAMTM.UID){
            bFindCO = true;
            break;
        }
    }

    if(!bFindCO){
        g_monoServer->addLog(LOGTYPE_WARNING, "Can't find CO at current location: (UID, X, Y)");
        fnPrintMoveError();
        m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
        return;
    }

    int nStepSize = -1;
    switch(mathf::LDistance2(stAMTM.X, stAMTM.Y, stAMTM.EndX, stAMTM.EndY)){
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
                g_monoServer->addLog(LOGTYPE_WARNING, "Invalid move request: (X, Y) -> (EndX, EndY)");
                fnPrintMoveError();
                m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
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
                if(canMove(true, true, stAMTM.EndX, stAMTM.EndY)){
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
                if(canMove(true, true, stAMTM.EndX, stAMTM.EndY)){
                    for(int nIndex = 1; nIndex < nStepSize; ++nIndex){
                        if(!canMove(true, false, stAMTM.X + nDX * nIndex, stAMTM.Y + nDY * nIndex)){
                            bCheckMove = false;
                            break;
                        }
                    }
                }

                if(!bCheckMove){
                    if(true
                            && stAMTM.AllowHalfMove
                            && canMove(true, true, stAMTM.X + nDX, stAMTM.Y + nDY)){
                        bCheckMove = true;
                        nMostX = stAMTM.X + nDX;
                        nMostY = stAMTM.Y + nDY;
                    }
                }
                break;
            }
    }

    if(!bCheckMove){
        m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
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

    getCell(nMostX, nMostY).Locked = true;
    m_actorPod->forward(rstMPK.from(), {MPK_MOVEOK, stAMMOK}, rstMPK.ID(), [this, stAMTM, nMostX, nMostY](const MessagePack &rstRMPK)
    {
        if(!getCell(nMostX, nMostY).Locked){
            throw fflerror("cell lock released before MOVEOK get responsed: MapUID = %" PRIu64, UID());
        }
        getCell(nMostX, nMostY).Locked = false;

        switch(rstRMPK.Type()){
            case MPK_OK:
                {
                    // the object comfirm to move
                    // and it's internal state has changed

                    // 1. leave last cell
                    bool bFindCO = false;
                    auto &rstUIDList = getUIDList(stAMTM.X, stAMTM.Y);

                    for(auto &nUID: rstUIDList){
                        if(nUID == stAMTM.UID){
                            bFindCO = true;
                            std::swap(nUID, rstUIDList.back());
                            rstUIDList.pop_back();
                            break;
                        }
                    }

                    if(!bFindCO){
                        throw fflerror("CO location error: (UID = %llu, X = %d, Y = %d)", to_llu(stAMTM.UID), stAMTM.X, stAMTM.Y);
                    }

                    // 2. push to the new cell
                    //    check if it should switch the map
                    addGridUID(stAMTM.UID, nMostX, nMostY, true);
                    if(uidf::getUIDType(stAMTM.UID) == UID_PLY && getCell(nMostX, nMostY).mapID){
                        AMMapSwitch stAMMS;
                        std::memset(&stAMMS, 0, sizeof(stAMMS));

                        stAMMS.UID   = uidf::buildMapUID(getCell(nMostX, nMostY).mapID); // TODO
                        stAMMS.MapID = getCell(nMostX, nMostY).mapID;
                        stAMMS.X     = getCell(nMostX, nMostY).switchX;
                        stAMMS.Y     = getCell(nMostX, nMostY).switchY;
                        m_actorPod->forward(stAMTM.UID, {MPK_MAPSWITCH, stAMMS});
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

void ServerMap::On_MPK_TRYLEAVE(const MessagePack &mpk)
{
    const auto amTL = mpk.conv<AMTryLeave>();
    if(In(ID(), amTL.X, amTL.Y) && hasGridUID(mpk.from(), amTL.X, amTL.Y)){
        removeGridUID(mpk.from(), amTL.X, amTL.Y);
        m_actorPod->forward(mpk.from(), MPK_OK, mpk.ID());
        return;
    }

    // otherwise try leave failed
    // we reply MPK_ERROR but this is already something wrong, map never prevert leave on purpose

    m_actorPod->forward(mpk.from(), MPK_ERROR, mpk.ID());
    g_monoServer->addLog(LOGTYPE_WARNING, "Leave request failed: UID = %llu, X = %d, Y = %d", to_llu(mpk.from()), amTL.X, amTL.Y);
}

void ServerMap::On_MPK_PULLCOINFO(const MessagePack &rstMPK)
{
    AMPullCOInfo stAMPCOI;
    std::memcpy(&stAMPCOI, rstMPK.Data(), sizeof(stAMPCOI));

    DoCenterSquare(stAMPCOI.X, stAMPCOI.Y, stAMPCOI.W, stAMPCOI.H, false, [this, stAMPCOI](int nX, int nY) -> bool
    {
        if(true || ValidC(nX, nY)){
            doUIDList(nX, nY, [this, stAMPCOI](uint64_t nUID) -> bool
            {
                if(nUID != stAMPCOI.UID){
                    if(uidf::getUIDType(nUID) == UID_PLY || uidf::getUIDType(nUID) == UID_MON){
                        AMQueryCORecord stAMQCOR;
                        std::memset(&stAMQCOR, 0, sizeof(stAMQCOR));

                        stAMQCOR.UID = stAMPCOI.UID;
                        m_actorPod->forward(nUID, {MPK_QUERYCORECORD, stAMQCOR});
                    }
                }
                return false;
            });
        }
        return false;
    });
}

void ServerMap::On_MPK_TRYMAPSWITCH(const MessagePack &mpk)
{
    const auto reqUID = mpk.from();
    const auto amTMS  = mpk.conv<AMTryMapSwitch>();

    if(!canMove(false, false, amTMS.X, amTMS.Y)){
        m_actorPod->forward(mpk.from(), MPK_ERROR, mpk.ID());
        return;
    }

    AMMapSwitchOK amMSOK;
    std::memset(&amMSOK, 0, sizeof(amMSOK));

    amMSOK.Ptr = this;
    amMSOK.X   = amTMS.X;
    amMSOK.Y   = amTMS.Y;

    getCell(amTMS.X, amTMS.Y).Locked = true;
    m_actorPod->forward(mpk.from(), {MPK_MAPSWITCHOK, amMSOK}, mpk.ID(), [this, reqUID, amMSOK](const MessagePack &rmpk)
    {
        if(!getCell(amMSOK.X, amMSOK.Y).Locked){
            throw fflerror("cell lock released before MAPSWITCHOK get responsed: MapUID = %lld", to_llu(UID()));
        }

        getCell(amMSOK.X, amMSOK.Y).Locked = false;
        switch(rmpk.Type()){
            case MPK_OK:
                {
                    // didn't check map switch here
                    // map switch should be triggered by move request

                    addGridUID(reqUID, amMSOK.X, amMSOK.Y, true);
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
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid MaxStep: %d, should be (1, 2, 3)", stAMPF.MaxStep);
    }

    ServerPathFinder stPathFinder(this, stAMPF.MaxStep, stAMPF.CheckCO);
    if(!stPathFinder.Search(nX0, nY0, nX1, nY1)){
        m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
        return;
    }

    // drop the first node
    // it's should be the provided start point
    if(!stPathFinder.GetSolutionStart()){
        m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
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
        switch(mathf::LDistance2(nCurrX, nCurrY, nEndX, nEndY)){
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
                    g_monoServer->addLog(LOGTYPE_WARNING, "Invalid path node found");
                    break;
                }
        }
    }
    m_actorPod->forward(rstMPK.from(), {MPK_PATHFINDOK, stAMPFOK}, rstMPK.ID());
}

void ServerMap::On_MPK_UPDATEHP(const MessagePack &rstMPK)
{
    AMUpdateHP stAMUHP;
    std::memcpy(&stAMUHP, rstMPK.Data(), sizeof(stAMUHP));

    if(ValidC(stAMUHP.X, stAMUHP.Y)){
        doCircle(stAMUHP.X, stAMUHP.Y, 20, [this, stAMUHP](int nX, int nY) -> bool
        {
            if(true || ValidC(nX, nY)){
                for(auto nUID: getUIDList(nX, nY)){
                    if(nUID != stAMUHP.UID){
                        if(uidf::getUIDType(nUID) == UID_PLY || uidf::getUIDType(nUID) == UID_MON){
                            m_actorPod->forward(nUID, {MPK_UPDATEHP, stAMUHP});
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
        removeGridUID(stAMDFO.UID, stAMDFO.X, stAMDFO.Y);
        doCircle(stAMDFO.X, stAMDFO.Y, 20, [this, stAMDFO](int nX, int nY) -> bool
        {
            if(true || ValidC(nX, nY)){
                for(auto nUID: getUIDList(nX, nY)){
                    if(nUID != stAMDFO.UID){
                        if(uidf::getUIDType(nUID) == UID_PLY || uidf::getUIDType(nUID) == UID_MON){
                            m_actorPod->forward(nUID, {MPK_DEADFADEOUT, stAMDFO});
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
        m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
        return;
    }

    int nCOCount = 0;
    for(int nX = 0; nX < W(); ++nX){
        for(int nY = 0; nY < H(); ++nY){
            std::for_each(getUIDList(nX, nY).begin(), getUIDList(nX, nY).end(), [stAMQCOC, &nCOCount](uint64_t nUID){
                if(uidf::getUIDType(nUID) == UID_PLY || uidf::getUIDType(nUID) == UID_MON){
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
    m_actorPod->forward(rstMPK.from(), {MPK_COCOUNT, stAMCOC}, rstMPK.ID());
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
                for(auto nUID: getUIDList(nX, nY)){
                    stAMUIDL.UIDList[nIndex++] = nUID;
                }
            }
        }
    }

    m_actorPod->forward(rstMPK.from(), {MPK_UIDLIST, stAMUIDL}, rstMPK.ID());
}

void ServerMap::On_MPK_NEWDROPITEM(const MessagePack &rstMPK)
{
    AMNewDropItem stAMNDI;
    std::memcpy(&stAMNDI, rstMPK.Data(), sizeof(stAMNDI));

    if(true
            && stAMNDI.ID
            && stAMNDI.Value > 0
            && groundValid(stAMNDI.X, stAMNDI.Y)){

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
                if(groundValid(stRC.X(), stRC.Y())){

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
            }while(stRC.forward() && (nCheckGrid++ <= SYS_MAXDROPITEMGRID));

            if(groundValid(nBestX, nBestY)){
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
    removeGridUID(stAMO.UID, stAMO.X, stAMO.Y);

    doCircle(stAMO.X, stAMO.Y, 10, [stAMO, this](int nX, int nY) -> bool
    {
        if(true || ValidC(nX, nY)){
            for(auto nUID: getUIDList(nX, nY)){
                if(nUID != stAMO.UID){
                    m_actorPod->forward(nUID, {MPK_OFFLINE, stAMO});
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
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid pickup request: X = %d, Y = %d, ID = %" PRIu32, stAMPU.X, stAMPU.Y, stAMPU.ID);
        return;
    }

    if(auto nIndex = FindGroundItem(CommonItem(stAMPU.ID, 0), stAMPU.X, stAMPU.Y); nIndex >= 0){
        RemoveGroundItem(CommonItem(stAMPU.ID, 0), stAMPU.X, stAMPU.Y);
        doCircle(stAMPU.X, stAMPU.Y, 10, [this, stAMPU](int nX, int nY) -> bool
        {
            if(true || ValidC(nX, nY)){
                AMRemoveGroundItem stAMRGI;
                std::memset(&stAMRGI, 0, sizeof(stAMRGI));

                stAMRGI.X      = nX;
                stAMRGI.Y      = nY;
                stAMRGI.DBID   = stAMPU.DBID;
                stAMRGI.ID = stAMPU.ID;

                doUIDList(nX, nY, [this, &stAMRGI](uint64_t nUID) -> bool
                {
                    if(uidf::getUIDType(nUID) == UID_PLY){
                        m_actorPod->forward(nUID, {MPK_REMOVEGROUNDITEM, stAMRGI});
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
        m_actorPod->forward(stAMPU.UID, {MPK_PICKUPOK, stAMPUOK});
    }else{
        // no such item
        // likely the client need re-sync for the gound items
    }
}
