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

void ServerMap::on_AM_METRONOME(const ActorMsgPack &)
{
    updateFireWall();
    if(m_luaModulePtr && !g_serverArgParser->disableMapScript){
        m_luaModulePtr->resumeLoop();
    }
}

void ServerMap::on_AM_BADACTORPOD(const ActorMsgPack &)
{
}

void ServerMap::on_AM_ACTION(const ActorMsgPack &rstMPK)
{
    const auto amA = rstMPK.conv<AMAction>();
    if(!validC(amA.action.x, amA.action.y)){
        return;
    }

    // spawn message is sent by startup trigger
    // before CO send ACTION_SPAWN servermap won't let other know it's ready

    if(amA.action.type == ACTION_SPAWN){
        addGridUID(amA.UID, amA.action.x, amA.action.y, true);
        if(uidf::getUIDType(amA.UID) == UID_PLY){
            postGroundItemIDList(amA.UID, amA.action.x, amA.action.y);
            postGroundFireWallList(amA.UID, amA.action.x, amA.action.y);
        }
    }

    doCircle(amA.action.x, amA.action.y, 10, [this, amA](int nX, int nY) -> bool
    {
        if(true || validC(nX, nY)){
            doUIDList(nX, nY, [this, amA](uint64_t nUID) -> bool
            {
                if(nUID != amA.UID){
                    switch(uidf::getUIDType(nUID)){
                        case UID_PLY:
                        case UID_MON:
                        case UID_NPC:
                            {
                                m_actorPod->forward(nUID, {AM_ACTION, amA});
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

void ServerMap::on_AM_ADDCHAROBJECT(const ActorMsgPack &rstMPK)
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
                    m_actorPod->forward(rstMPK.from(), AM_OK, rstMPK.seqID());
                    return;
                }

                m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
                return;
            }
        case UID_PLY:
            {
                const auto initParam = cerealf::deserialize<SDInitPlayer>(amACO.buf.data, amACO.buf.size);
                if(auto playerPtr = addPlayer(initParam)){
                    AMBindChannel amBC;
                    std::memset(&amBC, 0, sizeof(amBC));
                    amBC.channID = initParam.channID;

                    m_actorPod->forward(rstMPK.from(), AM_OK, rstMPK.seqID());
                    m_actorPod->forward(playerPtr->UID(), {AM_BINDCHANNEL, amBC});
                    doCircle(nX, nY, 20, [this](int nX, int nY) -> bool
                    {
                        if(true || validC(nX, nY)){
                            // ReportGroundItem(nChannID, nX, nY);
                        }
                        return false;
                    });
                    return;
                }

                m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
                return;
            }
        case UID_NPC:
            {
                if(addNPChar(cerealf::deserialize<SDInitNPChar>(amACO.buf.data, amACO.buf.size))){
                    m_actorPod->forward(rstMPK.from(), AM_OK, rstMPK.seqID());
                    doCircle(nX, nY, 20, [this](int nX, int nY) -> bool
                    {
                        if(true || validC(nX, nY)){
                            // ReportGroundItem(nChannID, nX, nY);
                        }
                        return false;
                    });
                    return;
                }

                m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
                return;
            }
        default:
            {
                m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
                return;
            }
    }
}

void ServerMap::on_AM_TRYSPACEMOVE(const ActorMsgPack &mpk)
{
    const auto amTSM = mpk.conv<AMTrySpaceMove>();
    const auto fnPrintMoveError = [&amTSM]()
    {
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYSPACEMOVE[%p]::X          = %d", &amTSM, amTSM.X);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYSPACEMOVE[%p]::Y          = %d", &amTSM, amTSM.Y);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYSPACEMOVE[%p]::EndX       = %d", &amTSM, amTSM.EndX);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYSPACEMOVE[%p]::EndY       = %d", &amTSM, amTSM.EndY);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYSPACEMOVE[%p]::StrictMove = %s", &amTSM, to_boolcstr(amTSM.StrictMove));
    };

    if(!In(ID(), amTSM.X, amTSM.Y)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid location: (X, Y)");
        fnPrintMoveError();
        m_actorPod->forward(mpk.from(), AM_ERROR, mpk.seqID());
        return;
    }

    if(!hasGridUID(mpk.from(), amTSM.X, amTSM.Y)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Can't find CO at current location: (UID, X, Y)");
        fnPrintMoveError();
        m_actorPod->forward(mpk.from(), AM_ERROR, mpk.seqID());
        return;
    }

    int nDstX = amTSM.EndX;
    int nDstY = amTSM.EndY;

    if(!validC(amTSM.X, amTSM.Y)){
        if(amTSM.StrictMove){
            m_actorPod->forward(mpk.from(), AM_ERROR, mpk.seqID());
            return;
        }

        nDstX = std::rand() % W();
        nDstY = std::rand() % H();
    }

    bool bDstOK = false;
    std::tie(bDstOK, nDstX, nDstY) = GetValidGrid(false, false, amTSM.StrictMove ? 1 : 100, nDstX, nDstY);

    if(!bDstOK){
        m_actorPod->forward(mpk.from(), AM_ERROR, mpk.seqID());
        return;
    }

    AMSpaceMoveOK amSMOK;
    std::memset(&amSMOK, 0, sizeof(amSMOK));

    amSMOK.X = amTSM.X;
    amSMOK.Y = amTSM.Y;
    amSMOK.EndX = nDstX;
    amSMOK.EndY = nDstY;

    m_actorPod->forward(mpk.from(), {AM_SPACEMOVEOK, amSMOK}, mpk.seqID(), [this, fromUID = mpk.from(), amTSM, nDstX, nDstY](const ActorMsgPack &rmpk)
    {
        switch(rmpk.type()){
            case AM_OK:
                {
                    // the object comfirm to move
                    // and it's internal state has changed

                    // for space move
                    // 1. we won't take care of map switch
                    // 2. we don't take reservation of the dstination cell

                    if(!removeGridUID(fromUID, amTSM.X, amTSM.Y)){
                        throw fflerror("CO location error: (UID = %llu, X = %d, Y = %d)", to_llu(fromUID), amTSM.X, amTSM.Y);
                    }

                    addGridUID(fromUID, nDstX, nDstY, true);
                    if(uidf::getUIDType(fromUID) == UID_PLY){
                        postGroundItemIDList(fromUID, nDstX, nDstY); // doesn't help if switched map
                        postGroundFireWallList(fromUID, nDstX, nDstY);
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

void ServerMap::on_AM_TRYJUMP(const ActorMsgPack &mpk)
{
    const auto amTJ = mpk.conv<AMTryJump>();
    const auto fnPrintJumpError = [&amTJ]()
    {
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYJUMP[%p]::X    = %d", &amTJ, amTJ.X);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYJUMP[%p]::Y    = %d", &amTJ, amTJ.Y);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYJUMP[%p]::EndX = %d", &amTJ, amTJ.EndX);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYJUMP[%p]::EndY = %d", &amTJ, amTJ.EndY);
    };

    if(!In(ID(), amTJ.X, amTJ.Y)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid location: (X, Y)");
        fnPrintJumpError();
        m_actorPod->forward(mpk.from(), AM_ERROR, mpk.seqID());
        return;
    }

    // we never allow server to handle motion to invalid grid
    // for client every motion request need to be prepared to avoid this

    if(!In(ID(), amTJ.EndX, amTJ.EndY)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid location: (EndX, EndY)");
        fnPrintJumpError();
        m_actorPod->forward(mpk.from(), AM_ERROR, mpk.seqID());
        return;
    }

    if(!hasGridUID(mpk.from(), amTJ.X, amTJ.Y)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Can't find CO at current location: (UID, X, Y)");
        fnPrintJumpError();
        m_actorPod->forward(mpk.from(), AM_ERROR, mpk.seqID());
        return;
    }

    AMJumpOK amJOK;
    std::memset(&amJOK, 0, sizeof(amJOK));

    amJOK.EndX = amTJ.EndX;
    amJOK.EndY = amTJ.EndY;

    m_actorPod->forward(mpk.from(), {AM_JUMPOK, amJOK}, mpk.seqID(), [this, amTJ, fromUID = mpk.from()](const ActorMsgPack &rmpk)
    {
        switch(rmpk.type()){
            case AM_OK:
                {
                    // the object comfirm to move
                    // and it's internal state has changed

                    if(!hasGridUID(fromUID, amTJ.X, amTJ.Y)){
                        throw fflerror("CO location error: (UID = %llu, X = %d, Y = %d)", to_llu(fromUID), amTJ.X, amTJ.Y);
                    }

                    removeGridUID(fromUID, amTJ.X, amTJ.Y);

                    // 2. push to the new cell
                    //    check if it should switch the map
                    addGridUID(fromUID, amTJ.EndX, amTJ.EndY, true);
                    if(uidf::getUIDType(fromUID) == UID_PLY){
                        postGroundItemIDList(fromUID, amTJ.EndX, amTJ.EndY); // doesn't help if switched map
                        postGroundFireWallList(fromUID, amTJ.EndX, amTJ.EndY);
                    }

                    if(uidf::getUIDType(fromUID) == UID_PLY && getGrid(amTJ.EndX, amTJ.EndY).mapID){
                        AMMapSwitch amMS;
                        std::memset(&amMS, 0, sizeof(amMS));

                        amMS.UID   = uidf::getMapUID(getGrid(amTJ.EndX, amTJ.EndY).mapID); // TODO
                        amMS.mapID = getGrid(amTJ.EndX, amTJ.EndY).mapID;
                        amMS.X     = getGrid(amTJ.EndX, amTJ.EndY).switchX;
                        amMS.Y     = getGrid(amTJ.EndX, amTJ.EndY).switchY;
                        m_actorPod->forward(fromUID, {AM_MAPSWITCH, amMS});
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

void ServerMap::on_AM_TRYMOVE(const ActorMsgPack &rstMPK)
{
    AMTryMove amTM;
    std::memcpy(&amTM, rstMPK.data(), sizeof(amTM));

    auto fnPrintMoveError = [&amTM]()
    {
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::UID           = %" PRIu32 , &amTM, amTM.UID);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::mapID         = %" PRIu32 , &amTM, amTM.mapID);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::X             = %d"       , &amTM, amTM.X);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::Y             = %d"       , &amTM, amTM.Y);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::EndX          = %d"       , &amTM, amTM.EndX);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::EndY          = %d"       , &amTM, amTM.EndY);
        g_monoServer->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::AllowHalfMove = %s"       , &amTM, amTM.AllowHalfMove ? "true" : "false");
    };

    if(!In(amTM.mapID, amTM.X, amTM.Y)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid location: (X, Y)");
        fnPrintMoveError();
        m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
        return;
    }

    // we never allow server to handle motion to invalid grid
    // for client every motion request need to be prepared to avoid this

    if(!In(amTM.mapID, amTM.EndX, amTM.EndY)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid location: (EndX, EndY)");
        fnPrintMoveError();
        m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
        return;
    }

    if(!hasGridUID(amTM.UID, amTM.X, amTM.Y)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Can't find CO at current location: (UID, X, Y)");
        fnPrintMoveError();
        m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
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
                m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
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
        m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
        return;
    }

    if(getGrid(nMostX, nMostY).locked){
        m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
        return;
    }

    AMMoveOK amMOK;
    std::memset(&amMOK, 0, sizeof(amMOK));

    amMOK.UID   = UID();
    amMOK.mapID = ID();
    amMOK.X     = amTM.X;
    amMOK.Y     = amTM.Y;
    amMOK.EndX  = nMostX;
    amMOK.EndY  = nMostY;

    getGrid(nMostX, nMostY).locked = true;
    m_actorPod->forward(rstMPK.from(), {AM_MOVEOK, amMOK}, rstMPK.seqID(), [this, amTM, nMostX, nMostY](const ActorMsgPack &rstRMPK)
    {
        if(!getGrid(nMostX, nMostY).locked){
            throw fflerror("cell lock released before MOVEOK get responsed: mapUID = %llu", to_llu(UID()));
        }
        getGrid(nMostX, nMostY).locked = false;

        switch(rstRMPK.type()){
            case AM_OK:
                {
                    // the object comfirm to move
                    // and it's internal state has changed

                    // 1. leave last cell
                    if(!removeGridUID(amTM.UID, amTM.X, amTM.Y)){
                        throw fflerror("CO location error: (UID = %llu, X = %d, Y = %d)", to_llu(amTM.UID), amTM.X, amTM.Y);
                    }

                    // 2. push to the new cell
                    //    check if it should switch the map
                    addGridUID(amTM.UID, nMostX, nMostY, true);
                    if(uidf::getUIDType(amTM.UID) == UID_PLY){
                        postGroundItemIDList(amTM.UID, nMostX, nMostY);
                        postGroundFireWallList(amTM.UID, nMostX, nMostY); // doesn't help if switched map
                    }

                    if(uidf::getUIDType(amTM.UID) == UID_PLY && getGrid(nMostX, nMostY).mapID){
                        AMMapSwitch amMS;
                        std::memset(&amMS, 0, sizeof(amMS));

                        amMS.UID   = uidf::getMapUID(getGrid(nMostX, nMostY).mapID); // TODO
                        amMS.mapID = getGrid(nMostX, nMostY).mapID;
                        amMS.X     = getGrid(nMostX, nMostY).switchX;
                        amMS.Y     = getGrid(nMostX, nMostY).switchY;
                        m_actorPod->forward(amTM.UID, {AM_MAPSWITCH, amMS});
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

void ServerMap::on_AM_TRYLEAVE(const ActorMsgPack &mpk)
{
    const auto amTL = mpk.conv<AMTryLeave>();
    if(In(ID(), amTL.X, amTL.Y) && hasGridUID(mpk.from(), amTL.X, amTL.Y)){
        removeGridUID(mpk.from(), amTL.X, amTL.Y);
        m_actorPod->forward(mpk.from(), AM_OK, mpk.seqID());
        return;
    }

    // otherwise try leave failed
    // we reply AM_ERROR but this is already something wrong, map never prevert leave on purpose

    m_actorPod->forward(mpk.from(), AM_ERROR, mpk.seqID());
    g_monoServer->addLog(LOGTYPE_WARNING, "Leave request failed: UID = %llu, X = %d, Y = %d", to_llu(mpk.from()), amTL.X, amTL.Y);
}

void ServerMap::on_AM_PULLCOINFO(const ActorMsgPack &rstMPK)
{
    AMPullCOInfo amPCOI;
    std::memcpy(&amPCOI, rstMPK.data(), sizeof(amPCOI));

    DoCenterSquare(amPCOI.X, amPCOI.Y, amPCOI.W, amPCOI.H, false, [this, amPCOI](int nX, int nY) -> bool
    {
        if(true || validC(nX, nY)){
            doUIDList(nX, nY, [this, amPCOI](uint64_t nUID) -> bool
            {
                if(nUID != amPCOI.UID){
                    if(uidf::getUIDType(nUID) == UID_PLY || uidf::getUIDType(nUID) == UID_MON){
                        AMQueryCORecord amQCOR;
                        std::memset(&amQCOR, 0, sizeof(amQCOR));

                        amQCOR.UID = amPCOI.UID;
                        m_actorPod->forward(nUID, {AM_QUERYCORECORD, amQCOR});
                    }
                }
                return false;
            });
        }
        return false;
    });
}

void ServerMap::on_AM_TRYMAPSWITCH(const ActorMsgPack &mpk)
{
    const auto reqUID = mpk.from();
    const auto amTMS  = mpk.conv<AMTryMapSwitch>();

    if(!canMove(false, true, amTMS.X, amTMS.Y)){
        m_actorPod->forward(mpk.from(), AM_ERROR, mpk.seqID());
        return;
    }

    AMMapSwitchOK amMSOK;
    std::memset(&amMSOK, 0, sizeof(amMSOK));

    amMSOK.Ptr = this;
    amMSOK.X   = amTMS.X;
    amMSOK.Y   = amTMS.Y;

    getGrid(amTMS.X, amTMS.Y).locked = true;
    m_actorPod->forward(mpk.from(), {AM_MAPSWITCHOK, amMSOK}, mpk.seqID(), [this, reqUID, amMSOK](const ActorMsgPack &rmpk)
    {
        if(!getGrid(amMSOK.X, amMSOK.Y).locked){
            throw fflerror("cell lock released before MAPSWITCHOK get responsed: MapUID = %lld", to_llu(UID()));
        }

        getGrid(amMSOK.X, amMSOK.Y).locked = false;
        switch(rmpk.type()){
            case AM_OK:
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

void ServerMap::on_AM_PATHFIND(const ActorMsgPack &rstMPK)
{
    AMPathFind amPF;
    std::memcpy(&amPF, rstMPK.data(), sizeof(amPF));

    int nX0 = amPF.X;
    int nY0 = amPF.Y;
    int nX1 = amPF.EndX;
    int nY1 = amPF.EndY;

    AMPathFindOK amPFOK;
    std::memset(&amPFOK, 0, sizeof(amPFOK));

    amPFOK.UID   = amPF.UID;
    amPFOK.mapID = ID();

    // we fill all slots with -1 for initialization
    // won't keep a record of ``how many path nodes are valid"
    constexpr auto nPathCount = std::extent<decltype(amPFOK.Point)>::value;
    for(int nIndex = 0; nIndex < to_d(nPathCount); ++nIndex){
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
        m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
        return;
    }

    // drop the first node
    // it's should be the provided start point
    if(!stPathFinder.GetSolutionStart()){
        m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
        return;
    }

    int nCurrN = 0;
    int nCurrX = nX0;
    int nCurrY = nY0;

    while(auto pNode1 = stPathFinder.GetSolutionNext()){
        if(nCurrN >= to_d(nPathCount)){
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
    m_actorPod->forward(rstMPK.from(), {AM_PATHFINDOK, amPFOK}, rstMPK.seqID());
}

void ServerMap::on_AM_UPDATEHP(const ActorMsgPack &rstMPK)
{
    AMUpdateHP amUHP;
    std::memcpy(&amUHP, rstMPK.data(), sizeof(amUHP));

    if(validC(amUHP.X, amUHP.Y)){
        doCircle(amUHP.X, amUHP.Y, 20, [this, amUHP](int nX, int nY) -> bool
        {
            if(true || validC(nX, nY)){
                for(auto nUID: getUIDList(nX, nY)){
                    if(nUID != amUHP.UID){
                        if(uidf::getUIDType(nUID) == UID_PLY || uidf::getUIDType(nUID) == UID_MON){
                            m_actorPod->forward(nUID, {AM_UPDATEHP, amUHP});
                        }
                    }
                }
            }
            return false;
        });
    }
}

void ServerMap::on_AM_DEADFADEOUT(const ActorMsgPack &mpk)
{
    // CO always send AM_DEADFADEOUT to server map to remove the grid occupation
    // and server map then boardcast to all its neighbors to remove this CO from their list to do clean

    const auto amDFO = mpk.conv<AMDeadFadeOut>();
    if(validC(amDFO.X, amDFO.Y)){
        removeGridUID(amDFO.UID, amDFO.X, amDFO.Y);
        doCircle(amDFO.X, amDFO.Y, 20, [this, amDFO](int nX, int nY) -> bool
        {
            if(true || validC(nX, nY)){
                for(auto nUID: getUIDList(nX, nY)){
                    if(nUID != amDFO.UID){
                        if(uidf::getUIDType(nUID) == UID_PLY || uidf::getUIDType(nUID) == UID_MON){
                            m_actorPod->forward(nUID, {AM_DEADFADEOUT, amDFO});
                        }
                    }
                }
            }
            return false;
        });
    }
}

void ServerMap::on_AM_QUERYCOCOUNT(const ActorMsgPack &rstMPK)
{
    AMQueryCOCount amQCOC;
    std::memcpy(&amQCOC, rstMPK.data(), sizeof(amQCOC));

    if(amQCOC.mapID && (amQCOC.mapID != ID())){
        m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
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
    m_actorPod->forward(rstMPK.from(), {AM_COCOUNT, amCOC}, rstMPK.seqID());
}

void ServerMap::on_AM_NEWDROPITEM(const ActorMsgPack &rstMPK)
{
    AMNewDropItem amNDI;
    std::memcpy(&amNDI, rstMPK.data(), sizeof(amNDI));

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

                    if(const auto nCurrCount = getGridItemIDList(stRC.X(), stRC.Y()).size(); to_d(nCurrCount) < nMinCount){
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
                addGridItemID(amNDI.ID, nBestX, nBestY);
            }
            else{

                // we scanned the square but find we can't find a valid place
                // abort current operation since following check should also fail
                return;
            }
        }
    }
}

void ServerMap::on_AM_OFFLINE(const ActorMsgPack &rstMPK)
{
    AMOffline amO;
    std::memcpy(&amO, rstMPK.data(), sizeof(amO));

    // this may fail
    // because player may get offline at try move
    removeGridUID(amO.UID, amO.X, amO.Y);

    doCircle(amO.X, amO.Y, 10, [amO, this](int nX, int nY) -> bool
    {
        if(true || validC(nX, nY)){
            for(auto nUID: getUIDList(nX, nY)){
                if(nUID != amO.UID){
                    m_actorPod->forward(nUID, {AM_OFFLINE, amO});
                }
            }
        }
        return false;
    });
}

void ServerMap::on_AM_PICKUP(const ActorMsgPack &mpk)
{
    const auto amPU = mpk.conv<AMPickUp>();
    if(!(validC(amPU.x, amPU.y) && (uidf::getUIDType(mpk.from()) == UID_PLY))){
        return;
    }

    std::vector<uint32_t> pickedItemIDList;
    auto &itemIDList = getGridItemIDList(amPU.x, amPU.y);

    for(auto p = itemIDList.begin(); p != itemIDList.end();){
        const auto itemID = *p;
        if(to_u8sv(DBCOM_ITEMRECORD(itemID).type) == u8"金币"){
            pickedItemIDList.push_back(itemID);
            p = itemIDList.erase(p);
        }
        else{
            p++;
        }
    }

    size_t pickedWeight = 0;
    while(!itemIDList.empty()){
        const auto itemID = itemIDList.back();
        const auto &ir = DBCOM_ITEMRECORD(itemID);

        if(!ir){
            throw fflerror("bad itemID: %llu", to_llu(itemID));
        }

        if(pickedWeight + ir.weight > amPU.availableWeight){
            break;
        }

        pickedWeight += ir.weight;
        pickedItemIDList.push_back(itemID);
        itemIDList.pop_back();
    }

    const SDPickUpItemIDList sdPUIIDL
    {
        .failedItemID = itemIDList.empty() ? 0 : itemIDList.back(),
        .itemIDList = std::move(pickedItemIDList),
    };

    if(!sdPUIIDL.itemIDList.empty()){
        postGridItemIDList(amPU.x, amPU.y);
    }
    m_actorPod->forward(mpk.from(), {AM_PICKUPITEMIDLIST, cerealf::serialize(sdPUIIDL)}, mpk.seqID());
}

void ServerMap::on_AM_STRIKEFIXEDLOCDAMAGE(const ActorMsgPack &mpk)
{
    const auto amSFLD = mpk.conv<AMStrikeFixedLocDamage>();
    doUIDList(amSFLD.x, amSFLD.y, [fromUID = mpk.from(), amSFLD, this](uint64_t uid) -> bool
    {
        switch(uidf::getUIDType(uid)){
            case UID_PLY:
            case UID_MON:
                {
                    AMAttack amA;
                    std::memset(&amA, 0, sizeof(amA));

                    amA.UID = fromUID;
                    amA.mapID = UID();

                    amA.X = amSFLD.x;
                    amA.Y = amSFLD.y;
                    amA.damage = amSFLD.damage;

                    m_actorPod->forward(uid, {AM_ATTACK, amA});
                    break;
                }
            default:
                {
                    break;
                }
        }
        return false;
    });
}

void ServerMap::on_AM_CASTFIREWALL(const ActorMsgPack &mpk)
{
    const auto amCFW = mpk.conv<AMCastFireWall>();
    if(groundValid(amCFW.x, amCFW.y)){
        getGrid(amCFW.x, amCFW.y).fireWallList.push_back(FireWallMagicNode
        {
            .uid = mpk.from(),

            .minDC = amCFW.minDC,
            .maxDC = amCFW.maxDC,

            .startTime      = g_monoServer->getCurrTick(),
            .lastAttackTime = g_monoServer->getCurrTick(),

            .duration = amCFW.duration,
            .dps      = amCFW.dps,
        });
        postGridFireWallList(amCFW.x, amCFW.y);
    }
}
