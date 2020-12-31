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

void ServerMap::on_MPK_METRONOME(const MessagePack &)
{
    if(m_luaModulePtr && !g_serverArgParser->DisableMapScript){
        m_luaModulePtr->resumeLoop();
    }
}

void ServerMap::on_MPK_BADACTORPOD(const MessagePack &)
{
}

void ServerMap::on_MPK_ACTION(const MessagePack &rstMPK)
{
    const auto amA = rstMPK.conv<AMAction>();
    if(!ValidC(amA.action.x, amA.action.y)){
        return;
    }

    // spawn message is sent by startup trigger
    // before CO send ACTION_SPAWN servermap won't let other know it's ready

    if(amA.action.type == ACTION_SPAWN){
        addGridUID(amA.UID, amA.action.x, amA.action.y, true);
    }

    doCircle(amA.action.x, amA.action.y, 10, [this, amA](int nX, int nY) -> bool
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

void ServerMap::on_MPK_ADDCHAROBJECT(const MessagePack &rstMPK)
{
    const auto amACO = rstMPK.conv<AMAddCharObject>();
    const auto nX = amACO.x;
    const auto nY = amACO.y;
    const bool bStrictLoc = amACO.strictLoc;

    switch(amACO.type){
        case UID_MON:
            {
                const auto nMonsterID = amACO.monster.monsterID;
                const auto nMasterUID = amACO.monster.masterUID;

                if(addMonster(nMonsterID, nMasterUID, nX, nY, bStrictLoc)){
                    m_actorPod->forward(rstMPK.from(), MPK_OK, rstMPK.ID());
                    return;
                }

                m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
                return;
            }
        case UID_PLY:
            {
                const auto nDBID      = amACO.player.DBID;
                const auto nChannID   = amACO.player.channID;
                const auto nDirection = amACO.player.direction;

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
                const auto npcID = amACO.NPC.NPCID;
                const auto x     = amACO.x;
                const auto y     = amACO.y;
                const auto strictLoc = amACO.strictLoc;

                if(addNPChar(npcID, x, y, strictLoc)){
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

void ServerMap::on_MPK_TRYSPACEMOVE(const MessagePack &rstMPK)
{
    AMTrySpaceMove amTSM;
    std::memcpy(&amTSM, rstMPK.Data(), sizeof(amTSM));

    int nDstX = amTSM.X;
    int nDstY = amTSM.Y;

    if(!ValidC(amTSM.X, amTSM.Y)){
        if(amTSM.StrictMove){
            m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
            return;
        }

        nDstX = std::rand() % W();
        nDstY = std::rand() % H();
    }

    bool bDstOK = false;
    std::tie(bDstOK, nDstX, nDstY) = GetValidGrid(false, false, amTSM.StrictMove ? 1 : 100, nDstX, nDstY);

    if(!bDstOK){
        m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
        return;
    }

    // get valid location
    // respond with MPK_SPACEMOVEOK and wait for response

    AMSpaceMoveOK amSMOK;
    std::memset(&amSMOK, 0, sizeof(amSMOK));

    amSMOK.X = nDstX;
    amSMOK.Y = nDstY;

    m_actorPod->forward(rstMPK.from(), {MPK_SPACEMOVEOK, amSMOK}, rstMPK.ID(), [this, nUID = amTSM.UID, nDstX, nDstY](const MessagePack &rstRMPK)
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

void ServerMap::on_MPK_TRYMOVE(const MessagePack &rstMPK)
{
    AMTryMove amTM;
    std::memcpy(&amTM, rstMPK.Data(), sizeof(amTM));

    auto fnPrintMoveError = [&amTM]()
    {
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::UID           = %" PRIu32 , &amTM, amTM.UID);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::MapID         = %" PRIu32 , &amTM, amTM.MapID);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::X             = %d"       , &amTM, amTM.X);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::Y             = %d"       , &amTM, amTM.Y);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::EndX          = %d"       , &amTM, amTM.EndX);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::EndY          = %d"       , &amTM, amTM.EndY);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::AllowHalfMove = %s"       , &amTM, amTM.AllowHalfMove ? "true" : "false");
    };

    if(!In(amTM.MapID, amTM.X, amTM.Y)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid location: (X, Y)");
        fnPrintMoveError();
        m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
        return;
    }

    // we never allow server to handle motion to invalid grid
    // for client every motion request need to be prepared to avoid this

    if(!In(amTM.MapID, amTM.EndX, amTM.EndY)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid location: (EndX, EndY)");
        fnPrintMoveError();
        m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
        return;
    }

    bool bFindCO = false;
    for(auto nUID: getUIDList(amTM.X, amTM.Y)){
        if(nUID == amTM.UID){
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
    switch(mathf::LDistance2(amTM.X, amTM.Y, amTM.EndX, amTM.EndY)){
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
    auto nMostX = amTM.EndX;
    auto nMostY = amTM.EndY;

    switch(nStepSize){
        case 1:
            {
                bCheckMove = false;
                if(canMove(true, true, amTM.EndX, amTM.EndY)){
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

                int nDX = (amTM.EndX > amTM.X) - (amTM.EndX < amTM.X);
                int nDY = (amTM.EndY > amTM.Y) - (amTM.EndY < amTM.Y);

                bCheckMove = true;
                if(canMove(true, true, amTM.EndX, amTM.EndY)){
                    for(int nIndex = 1; nIndex < nStepSize; ++nIndex){
                        if(!canMove(true, false, amTM.X + nDX * nIndex, amTM.Y + nDY * nIndex)){
                            bCheckMove = false;
                            break;
                        }
                    }
                }

                if(!bCheckMove){
                    if(true
                            && amTM.AllowHalfMove
                            && canMove(true, true, amTM.X + nDX, amTM.Y + nDY)){
                        bCheckMove = true;
                        nMostX = amTM.X + nDX;
                        nMostY = amTM.Y + nDY;
                    }
                }
                break;
            }
    }

    if(!bCheckMove){
        m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
        return;
    }

    if(getCell(nMostX, nMostY).Locked){
        m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
        return;
    }

    AMMoveOK amMOK;
    std::memset(&amMOK, 0, sizeof(amMOK));

    amMOK.UID   = UID();
    amMOK.MapID = ID();
    amMOK.X     = amTM.X;
    amMOK.Y     = amTM.Y;
    amMOK.EndX  = nMostX;
    amMOK.EndY  = nMostY;

    getCell(nMostX, nMostY).Locked = true;
    m_actorPod->forward(rstMPK.from(), {MPK_MOVEOK, amMOK}, rstMPK.ID(), [this, amTM, nMostX, nMostY](const MessagePack &rstRMPK)
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
                    auto &rstUIDList = getUIDList(amTM.X, amTM.Y);

                    for(auto &nUID: rstUIDList){
                        if(nUID == amTM.UID){
                            bFindCO = true;
                            std::swap(nUID, rstUIDList.back());
                            rstUIDList.pop_back();
                            break;
                        }
                    }

                    if(!bFindCO){
                        throw fflerror("CO location error: (UID = %llu, X = %d, Y = %d)", to_llu(amTM.UID), amTM.X, amTM.Y);
                    }

                    // 2. push to the new cell
                    //    check if it should switch the map
                    addGridUID(amTM.UID, nMostX, nMostY, true);
                    if(uidf::getUIDType(amTM.UID) == UID_PLY && getCell(nMostX, nMostY).mapID){
                        AMMapSwitch amMS;
                        std::memset(&amMS, 0, sizeof(amMS));

                        amMS.UID   = uidf::buildMapUID(getCell(nMostX, nMostY).mapID); // TODO
                        amMS.MapID = getCell(nMostX, nMostY).mapID;
                        amMS.X     = getCell(nMostX, nMostY).switchX;
                        amMS.Y     = getCell(nMostX, nMostY).switchY;
                        m_actorPod->forward(amTM.UID, {MPK_MAPSWITCH, amMS});
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

void ServerMap::on_MPK_TRYLEAVE(const MessagePack &mpk)
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

void ServerMap::on_MPK_PULLCOINFO(const MessagePack &rstMPK)
{
    AMPullCOInfo amPCOI;
    std::memcpy(&amPCOI, rstMPK.Data(), sizeof(amPCOI));

    DoCenterSquare(amPCOI.X, amPCOI.Y, amPCOI.W, amPCOI.H, false, [this, amPCOI](int nX, int nY) -> bool
    {
        if(true || ValidC(nX, nY)){
            doUIDList(nX, nY, [this, amPCOI](uint64_t nUID) -> bool
            {
                if(nUID != amPCOI.UID){
                    if(uidf::getUIDType(nUID) == UID_PLY || uidf::getUIDType(nUID) == UID_MON){
                        AMQueryCORecord amQCOR;
                        std::memset(&amQCOR, 0, sizeof(amQCOR));

                        amQCOR.UID = amPCOI.UID;
                        m_actorPod->forward(nUID, {MPK_QUERYCORECORD, amQCOR});
                    }
                }
                return false;
            });
        }
        return false;
    });
}

void ServerMap::on_MPK_TRYMAPSWITCH(const MessagePack &mpk)
{
    const auto reqUID = mpk.from();
    const auto amTMS  = mpk.conv<AMTryMapSwitch>();

    if(!canMove(false, true, amTMS.X, amTMS.Y)){
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

void ServerMap::on_MPK_PATHFIND(const MessagePack &rstMPK)
{
    AMPathFind amPF;
    std::memcpy(&amPF, rstMPK.Data(), sizeof(amPF));

    int nX0 = amPF.X;
    int nY0 = amPF.Y;
    int nX1 = amPF.EndX;
    int nY1 = amPF.EndY;

    AMPathFindOK amPFOK;
    std::memset(&amPFOK, 0, sizeof(amPFOK));

    amPFOK.UID   = amPF.UID;
    amPFOK.MapID = ID();

    // we fill all slots with -1 for initialization
    // won't keep a record of ``how many path nodes are valid"
    constexpr auto nPathCount = std::extent<decltype(amPFOK.Point)>::value;
    for(int nIndex = 0; nIndex < (int)(nPathCount); ++nIndex){
        amPFOK.Point[nIndex].X = -1;
        amPFOK.Point[nIndex].Y = -1;
    }

    // should make sure MaxStep is OK
    if(true
            && amPF.MaxStep != 1
            && amPF.MaxStep != 2
            && amPF.MaxStep != 3){

        // we get a dangerous parameter from actormsg
        // correct here and put an warning in the log system
        amPF.MaxStep = 1;
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid MaxStep: %d, should be (1, 2, 3)", amPF.MaxStep);
    }

    ServerPathFinder stPathFinder(this, amPF.MaxStep, amPF.CheckCO);
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
                    amPFOK.Point[nCurrN].X = nCurrX;
                    amPFOK.Point[nCurrN].Y = nCurrY;

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
    m_actorPod->forward(rstMPK.from(), {MPK_PATHFINDOK, amPFOK}, rstMPK.ID());
}

void ServerMap::on_MPK_UPDATEHP(const MessagePack &rstMPK)
{
    AMUpdateHP amUHP;
    std::memcpy(&amUHP, rstMPK.Data(), sizeof(amUHP));

    if(ValidC(amUHP.X, amUHP.Y)){
        doCircle(amUHP.X, amUHP.Y, 20, [this, amUHP](int nX, int nY) -> bool
        {
            if(true || ValidC(nX, nY)){
                for(auto nUID: getUIDList(nX, nY)){
                    if(nUID != amUHP.UID){
                        if(uidf::getUIDType(nUID) == UID_PLY || uidf::getUIDType(nUID) == UID_MON){
                            m_actorPod->forward(nUID, {MPK_UPDATEHP, amUHP});
                        }
                    }
                }
            }
            return false;
        });
    }
}

void ServerMap::on_MPK_DEADFADEOUT(const MessagePack &rstMPK)
{
    AMDeadFadeOut amDFO;
    std::memcpy(&amDFO, rstMPK.Data(), sizeof(amDFO));

    if(ValidC(amDFO.X, amDFO.Y)){
        removeGridUID(amDFO.UID, amDFO.X, amDFO.Y);
        doCircle(amDFO.X, amDFO.Y, 20, [this, amDFO](int nX, int nY) -> bool
        {
            if(true || ValidC(nX, nY)){
                for(auto nUID: getUIDList(nX, nY)){
                    if(nUID != amDFO.UID){
                        if(uidf::getUIDType(nUID) == UID_PLY){
                            m_actorPod->forward(nUID, {MPK_DEADFADEOUT, amDFO});
                        }
                        else if(uidf::getUIDType(nUID) == UID_MON && DBCOM_MONSTERRECORD(uidf::getMonsterID(nUID)).deadFadeOut){
                            m_actorPod->forward(nUID, {MPK_DEADFADEOUT, amDFO});
                        }
                    }
                }
            }
            return false;
        });
    }
}

void ServerMap::on_MPK_QUERYCOCOUNT(const MessagePack &rstMPK)
{
    AMQueryCOCount amQCOC;
    std::memcpy(&amQCOC, rstMPK.Data(), sizeof(amQCOC));

    if(amQCOC.MapID && (amQCOC.MapID != ID())){
        m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
        return;
    }

    int nCOCount = 0;
    for(int nX = 0; nX < W(); ++nX){
        for(int nY = 0; nY < H(); ++nY){
            std::for_each(getUIDList(nX, nY).begin(), getUIDList(nX, nY).end(), [amQCOC, &nCOCount](uint64_t nUID){
                if(uidf::getUIDType(nUID) == UID_PLY || uidf::getUIDType(nUID) == UID_MON){
                    if(amQCOC.Check.NPC    ){ nCOCount++; return; }
                    if(amQCOC.Check.Player ){ nCOCount++; return; }
                    if(amQCOC.Check.Monster){ nCOCount++; return; }
                }
            });
        }
    }

    AMCOCount amCOC;
    std::memset(&amCOC, 0, sizeof(amCOC));

    amCOC.Count = nCOCount;
    m_actorPod->forward(rstMPK.from(), {MPK_COCOUNT, amCOC}, rstMPK.ID());
}

void ServerMap::on_MPK_NEWDROPITEM(const MessagePack &rstMPK)
{
    AMNewDropItem amNDI;
    std::memcpy(&amNDI, rstMPK.Data(), sizeof(amNDI));

    if(true
            && amNDI.ID
            && amNDI.Value > 0
            && groundValid(amNDI.X, amNDI.Y)){

        bool bHoldInOne = false;
        switch(amNDI.ID){
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

        auto nLoop = bHoldInOne ? 1 : amNDI.Value;
        for(int nIndex = 0; nIndex < nLoop; ++nIndex){

            // we check SYS_MAXDROPITEMGRID grids to find a best place to hold current item
            // ``best" means there are not too many drop item already
            int nCheckGrid = 0;

            int nBestX    = -1;
            int nBestY    = -1;
            int nMinCount = SYS_MAXDROPITEM + 1;

            RotateCoord stRC(amNDI.X, amNDI.Y, 0, 0, W(), H());
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
                AddGroundItem(CommonItem(amNDI.ID, 0), nBestX, nBestY);
            }else{

                // we scanned the square but find we can't find a valid place
                // abort current operation since following check should also fail
                return;
            }
        }
    }
}

void ServerMap::on_MPK_OFFLINE(const MessagePack &rstMPK)
{
    AMOffline amO;
    std::memcpy(&amO, rstMPK.Data(), sizeof(amO));

    // this may fail
    // because player may get offline at try move
    removeGridUID(amO.UID, amO.X, amO.Y);

    doCircle(amO.X, amO.Y, 10, [amO, this](int nX, int nY) -> bool
    {
        if(true || ValidC(nX, nY)){
            for(auto nUID: getUIDList(nX, nY)){
                if(nUID != amO.UID){
                    m_actorPod->forward(nUID, {MPK_OFFLINE, amO});
                }
            }
        }
        return false;
    });
}

void ServerMap::on_MPK_PICKUP(const MessagePack &rstMPK)
{
    AMPickUp amPU;
    std::memcpy(&amPU, rstMPK.Data(), sizeof(amPU));

    if(!ValidC(amPU.X, amPU.Y) || !amPU.ID){
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid pickup request: X = %d, Y = %d, ID = %" PRIu32, amPU.X, amPU.Y, amPU.ID);
        return;
    }

    if(auto nIndex = FindGroundItem(CommonItem(amPU.ID, 0), amPU.X, amPU.Y); nIndex >= 0){
        RemoveGroundItem(CommonItem(amPU.ID, 0), amPU.X, amPU.Y);
        doCircle(amPU.X, amPU.Y, 10, [this, amPU](int nX, int nY) -> bool
        {
            if(true || ValidC(nX, nY)){
                AMRemoveGroundItem amRGI;
                std::memset(&amRGI, 0, sizeof(amRGI));

                amRGI.X      = nX;
                amRGI.Y      = nY;
                amRGI.DBID   = amPU.DBID;
                amRGI.ID = amPU.ID;

                doUIDList(nX, nY, [this, &amRGI](uint64_t nUID) -> bool
                {
                    if(uidf::getUIDType(nUID) == UID_PLY){
                        m_actorPod->forward(nUID, {MPK_REMOVEGROUNDITEM, amRGI});
                    }
                    return false;
                });
            }
            return false;
        });

        AMPickUpOK amPUOK;
        std::memset(&amPUOK, 0, sizeof(amPUOK));

        amPUOK.X    = amPU.X;
        amPUOK.Y    = amPU.Y;
        amPUOK.UID  = amPU.UID;
        amPUOK.DBID = 0;
        amPUOK.ID   = amPU.ID;
        m_actorPod->forward(amPU.UID, {MPK_PICKUPOK, amPUOK});
    }else{
        // no such item
        // likely the client need re-sync for the gound items
    }
}
