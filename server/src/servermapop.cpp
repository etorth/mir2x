#include <cinttypes>
#include "player.hpp"
#include "npchar.hpp"
#include "dbcomid.hpp"
#include "monster.hpp"
#include "strf.hpp"
#include "mathf.hpp"
#include "sysconst.hpp"
#include "actorpod.hpp"
#include "raiitimer.hpp"
#include "servermap.hpp"
#include "monoserver.hpp"
#include "rotatecoord.hpp"
#include "serverargparser.hpp"

extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;

void ServerMap::on_AM_METRONOME(const ActorMsgPack &)
{
    updateMapGrid();
    if(!g_serverArgParser->disableMapScript){
        m_luaRunner->resume(m_mainScriptThreadKey);
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

    doCircle(amA.action.x, amA.action.y, SYS_VIEWR, [this, amA](int nX, int nY) -> bool
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

void ServerMap::on_AM_ADDCO(const ActorMsgPack &rstMPK)
{
    const auto amACO = rstMPK.conv<AMAddCharObject>();
    const auto nX = amACO.x;
    const auto nY = amACO.y;
    const bool bStrictLoc = amACO.strictLoc;

    AMUID amUID;
    std::memset(&amUID, 0, sizeof(amUID));

    switch(amACO.type){
        case UID_MON:
            {
                const auto nMonsterID = amACO.monster.monsterID;
                const auto nMasterUID = amACO.monster.masterUID;

                if(auto monPtr = addMonster(nMonsterID, nMasterUID, nX, nY, bStrictLoc)){
                    amUID.UID = monPtr->UID();
                }

                m_actorPod->forward(rstMPK.from(), {AM_UID, amUID}, rstMPK.seqID());
                return;
            }
        case UID_PLY:
            {
                const auto initParam = amACO.buf.deserialize<SDInitPlayer>();
                if(auto playerPtr = addPlayer(initParam)){
                    AMBindChannel amBC;
                    std::memset(&amBC, 0, sizeof(amBC));
                    amBC.channID = initParam.channID;

                    m_actorPod->forward(playerPtr->UID(), {AM_BINDCHANNEL, amBC});
                    doCircle(nX, nY, 20, [this](int nX, int nY) -> bool
                    {
                        if(true || validC(nX, nY)){
                            // ReportGroundItem(nChannID, nX, nY);
                        }
                        return false;
                    });
                    amUID.UID = playerPtr->UID();
                }

                m_actorPod->forward(rstMPK.from(), {AM_UID, amUID}, rstMPK.seqID());
                return;
            }
        case UID_NPC:
            {
                if(auto npcPtr = addNPChar(amACO.buf.deserialize<SDInitNPChar>())){
                    doCircle(nX, nY, 20, [this](int nX, int nY) -> bool
                    {
                        if(true || validC(nX, nY)){
                            // ReportGroundItem(nChannID, nX, nY);
                        }
                        return false;
                    });
                    amUID.UID = npcPtr->UID();
                }

                m_actorPod->forward(rstMPK.from(), {AM_UID, amUID}, rstMPK.seqID());
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

    if(!in(ID(), amTSM.X, amTSM.Y)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid location: (X, Y)");
        fnPrintMoveError();
        m_actorPod->forward(mpk.from(), AM_REJECTSPACEMOVE, mpk.seqID());
        return;
    }

    if(!hasGridUID(mpk.from(), amTSM.X, amTSM.Y)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Can't find CO at current location: (UID, X, Y)");
        fnPrintMoveError();
        m_actorPod->forward(mpk.from(), AM_REJECTSPACEMOVE, mpk.seqID());
        return;
    }

    int nDstX = amTSM.EndX;
    int nDstY = amTSM.EndY;

    if(!validC(amTSM.X, amTSM.Y)){
        if(amTSM.StrictMove){
            m_actorPod->forward(mpk.from(), AM_REJECTSPACEMOVE, mpk.seqID());
            return;
        }

        nDstX = std::rand() % W();
        nDstY = std::rand() % H();
    }

    const auto loc = getRCValidGrid(false, false, amTSM.StrictMove ? 1 : 100, nDstX, nDstY);
    if(!loc.has_value()){
        m_actorPod->forward(mpk.from(), AM_REJECTSPACEMOVE, mpk.seqID());
        return;
    }

    std::tie(nDstX, nDstY) = loc.value();

    AMAllowSpaceMove amASM;
    std::memset(&amASM, 0, sizeof(amASM));

    amASM.X = amTSM.X;
    amASM.Y = amTSM.Y;
    amASM.EndX = nDstX;
    amASM.EndY = nDstY;

    m_actorPod->forward(mpk.from(), {AM_ALLOWSPACEMOVE, amASM}, mpk.seqID(), [this, fromUID = mpk.from(), amTSM, nDstX, nDstY](const ActorMsgPack &rmpk)
    {
        switch(rmpk.type()){
            case AM_SPACEMOVEOK:
                {
                    // the object comfirm to move
                    // and it's internal state has changed

                    // for space move
                    // 1. we won't take care of map switch
                    // 2. we don't take reservation of the dstination cell

                    const auto amSMOK = rmpk.conv<AMSpaceMoveOK>();

                    AMAction amA;
                    std::memset(&amA, 0, sizeof(amA));

                    amA.UID = fromUID;
                    amA.mapID = amSMOK.mapID;
                    amA.action = amSMOK.action;

                    if(!removeGridUID(fromUID, amTSM.X, amTSM.Y)){
                        throw fflerror("CO location error: (UID = %llu, X = %d, Y = %d)", to_llu(fromUID), amTSM.X, amTSM.Y);
                    }

                    std::unordered_set<uint64_t> seenUIDList;
                    doCircle(amTSM.X, amTSM.Y, SYS_VIEWR, [amA, fromUID, &seenUIDList, this](int gridX, int gridY)
                    {
                        doUIDList(gridX, gridY, [amA, fromUID, &seenUIDList, this](uint64_t gridUID)
                        {
                            switch(uidf::getUIDType(gridUID)){
                                case UID_PLY:
                                case UID_MON:
                                case UID_NPC:
                                    {
                                        if((gridUID != fromUID) && (seenUIDList.count(gridUID) == 0)){
                                            m_actorPod->forward(gridUID, {AM_ACTION, amA});
                                            seenUIDList.insert(gridUID);
                                        }
                                        break;
                                    }
                                default:
                                    {
                                        break;
                                    }
                            }
                            return false;
                        });
                        return false;
                    });

                    addGridUID(fromUID, nDstX, nDstY, true);
                    if(uidf::getUIDType(fromUID) == UID_PLY){
                        postGroundItemIDList(fromUID, nDstX, nDstY); // doesn't help if switched map
                        postGroundFireWallList(fromUID, nDstX, nDstY);
                    }

                    doCircle(nDstX, nDstY, SYS_VIEWR, [amA, fromUID, &seenUIDList, this](int gridX, int gridY)
                    {
                        doUIDList(gridX, gridY, [amA, fromUID, &seenUIDList, this](uint64_t gridUID)
                        {
                            switch(uidf::getUIDType(gridUID)){
                                case UID_PLY:
                                case UID_MON:
                                case UID_NPC:
                                    {
                                        if((gridUID != fromUID) && (seenUIDList.count(gridUID) == 0)){
                                            m_actorPod->forward(gridUID, {AM_ACTION, amA});
                                            seenUIDList.insert(gridUID);
                                        }
                                        break;
                                    }
                                default:
                                    {
                                        break;
                                    }
                            }
                            return false;
                        });
                        return false;
                    });
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

    if(!in(ID(), amTJ.X, amTJ.Y)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid location: (X, Y)");
        fnPrintJumpError();
        m_actorPod->forward(mpk.from(), AM_REJECTJUMP, mpk.seqID());
        return;
    }

    // we never allow server to handle motion to invalid grid
    // for client every motion request need to be prepared to avoid this

    if(!in(ID(), amTJ.EndX, amTJ.EndY)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid location: (EndX, EndY)");
        fnPrintJumpError();
        m_actorPod->forward(mpk.from(), AM_REJECTJUMP, mpk.seqID());
        return;
    }

    if(!hasGridUID(mpk.from(), amTJ.X, amTJ.Y)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Can't find CO at current location: (UID, X, Y)");
        fnPrintJumpError();
        m_actorPod->forward(mpk.from(), AM_REJECTJUMP, mpk.seqID());
        return;
    }

    AMAllowJump amAJ;
    std::memset(&amAJ, 0, sizeof(amAJ));

    amAJ.EndX = amTJ.EndX;
    amAJ.EndY = amTJ.EndY;

    m_actorPod->forward(mpk.from(), {AM_ALLOWJUMP, amAJ}, mpk.seqID(), [this, amTJ, fromUID = mpk.from()](const ActorMsgPack &rmpk)
    {
        switch(rmpk.type()){
            case AM_JUMPOK:
                {
                    // the object comfirm to move
                    // and it's internal state has changed

                    const auto amJOK = rmpk.conv<AMJumpOK>();

                    AMAction amA;
                    std::memset(&amA, 0, sizeof(amA));

                    amA.UID = amJOK.uid;
                    amA.mapID = amJOK.mapID;
                    amA.action = amJOK.action;

                    if(!hasGridUID(fromUID, amTJ.X, amTJ.Y)){
                        throw fflerror("CO location error: (UID = %llu, X = %d, Y = %d)", to_llu(fromUID), amTJ.X, amTJ.Y);
                    }

                    removeGridUID(fromUID, amTJ.X, amTJ.Y);

                    std::unordered_set<uint64_t> seenUIDList;
                    doCircle(amTJ.X, amTJ.Y, SYS_VIEWR, [amTJ, amA, fromUID, &seenUIDList, this](int gridX, int gridY)
                    {
                        doUIDList(gridX, gridY, [amTJ, amA, fromUID, &seenUIDList, this](uint64_t gridUID)
                        {
                            switch(uidf::getUIDType(gridUID)){
                                case UID_PLY:
                                case UID_MON:
                                case UID_NPC:
                                    {
                                        if((gridUID != fromUID) && (seenUIDList.count(gridUID) == 0)){
                                            m_actorPod->forward(gridUID, {AM_ACTION, amA});
                                            seenUIDList.insert(gridUID);
                                        }
                                        break;
                                    }
                                default:
                                    {
                                        break;
                                    }
                            }
                            return false;
                        });
                        return false;
                    });

                    // push to the new cell
                    // check if it should switch the map
                    addGridUID(fromUID, amTJ.EndX, amTJ.EndY, true);
                    if(uidf::getUIDType(fromUID) == UID_PLY){
                        postGroundItemIDList(fromUID, amTJ.EndX, amTJ.EndY); // doesn't help if switched map
                        postGroundFireWallList(fromUID, amTJ.EndX, amTJ.EndY);
                    }

                    doCircle(amTJ.EndX, amTJ.EndY, SYS_VIEWR, [amTJ, amA, fromUID, &seenUIDList, this](int gridX, int gridY)
                    {
                        doUIDList(gridX, gridY, [amTJ, amA, fromUID, &seenUIDList, this](uint64_t gridUID)
                        {
                            switch(uidf::getUIDType(gridUID)){
                                case UID_PLY:
                                case UID_MON:
                                case UID_NPC:
                                    {
                                        if((gridUID != fromUID) && (seenUIDList.count(gridUID) == 0)){
                                            m_actorPod->forward(gridUID, {AM_ACTION, amA});
                                            seenUIDList.insert(gridUID);
                                        }
                                        break;
                                    }
                                default:
                                    {
                                        break;
                                    }
                            }
                            return false;
                        });
                        return false;
                    });

                    if(uidf::isPlayer(fromUID) && getGrid(amTJ.EndX, amTJ.EndY).mapID){
                        AMMapSwitchTrigger amMST;
                        std::memset(&amMST, 0, sizeof(amMST));

                        amMST.UID   = uidf::getMapBaseUID(getGrid(amTJ.EndX, amTJ.EndY).mapID); // TODO
                        amMST.mapID = getGrid(amTJ.EndX, amTJ.EndY).mapID;
                        amMST.X     = getGrid(amTJ.EndX, amTJ.EndY).switchX;
                        amMST.Y     = getGrid(amTJ.EndX, amTJ.EndY).switchY;
                        m_actorPod->forward(fromUID, {AM_MAPSWITCHTRIGGER, amMST});
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

    if(!in(amTM.mapID, amTM.X, amTM.Y)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid location: (X, Y)");
        fnPrintMoveError();
        m_actorPod->forward(rstMPK.from(), AM_REJECTMOVE, rstMPK.seqID());
        return;
    }

    // we never allow server to handle motion to invalid grid
    // for client every motion request need to be prepared to avoid this

    if(!in(amTM.mapID, amTM.EndX, amTM.EndY)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid location: (EndX, EndY)");
        fnPrintMoveError();
        m_actorPod->forward(rstMPK.from(), AM_REJECTMOVE, rstMPK.seqID());
        return;
    }

    if(!hasGridUID(amTM.UID, amTM.X, amTM.Y)){
        g_monoServer->addLog(LOGTYPE_WARNING, "Can't find CO at current location: (UID, X, Y)");
        fnPrintMoveError();
        m_actorPod->forward(rstMPK.from(), AM_REJECTMOVE, rstMPK.seqID());
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
                m_actorPod->forward(rstMPK.from(), AM_REJECTMOVE, rstMPK.seqID());
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

                fflassert(nStepSize == 2 || nStepSize == 3, nStepSize);

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
        m_actorPod->forward(rstMPK.from(), AM_REJECTMOVE, rstMPK.seqID());
        return;
    }

    if(getGrid(nMostX, nMostY).locked){
        m_actorPod->forward(rstMPK.from(), AM_REJECTMOVE, rstMPK.seqID());
        return;
    }

    AMAllowMove amAM;
    std::memset(&amAM, 0, sizeof(amAM));

    amAM.UID   = UID();
    amAM.mapID = ID();
    amAM.X     = amTM.X;
    amAM.Y     = amTM.Y;
    amAM.EndX  = nMostX;
    amAM.EndY  = nMostY;

    getGrid(nMostX, nMostY).locked = true;
    m_actorPod->forward(rstMPK.from(), {AM_ALLOWMOVE, amAM}, rstMPK.seqID(), [this, amTM, nMostX, nMostY](const ActorMsgPack &rstRMPK)
    {
        fflassert(getGrid(nMostX, nMostY).locked);
        getGrid(nMostX, nMostY).locked = false;

        switch(rstRMPK.type()){
            case AM_MOVEOK:
                {
                    const auto amMOK = rstRMPK.conv<AMMoveOK>();

                    AMAction amA;
                    std::memset(&amA, 0, sizeof(amA));

                    amA.UID = amMOK.uid;
                    amA.mapID = amMOK.mapID;
                    amA.action = amMOK.action;

                    // the object comfirm to move
                    // and it's internal state has changed

                    // leave last cell
                    if(!removeGridUID(amTM.UID, amTM.X, amTM.Y)){
                        throw fflerror("CO location error: (UID = %llu, X = %d, Y = %d)", to_llu(amTM.UID), amTM.X, amTM.Y);
                    }

                    std::unordered_set<uint64_t> seenUIDList;
                    doCircle(amTM.X, amTM.Y, SYS_VIEWR, [amTM, amA, &seenUIDList, this](int gridX, int gridY)
                    {
                        doUIDList(gridX, gridY, [amTM, amA, &seenUIDList, this](uint64_t gridUID)
                        {
                            switch(uidf::getUIDType(gridUID)){
                                case UID_PLY:
                                case UID_MON:
                                case UID_NPC:
                                    {
                                        if((gridUID != amTM.UID) && (seenUIDList.count(gridUID) == 0)){
                                            m_actorPod->forward(gridUID, {AM_ACTION, amA});
                                            seenUIDList.insert(gridUID);
                                        }
                                        break;
                                    }
                                default:
                                    {
                                        break;
                                    }
                            }
                            return false;
                        });
                        return false;
                    });

                    // push to the new cell
                    // check if it should switch the map
                    addGridUID(amTM.UID, nMostX, nMostY, true);
                    if(uidf::isPlayer(amTM.UID)){
                        postGroundItemIDList(amTM.UID, nMostX, nMostY);
                        postGroundFireWallList(amTM.UID, nMostX, nMostY); // doesn't help if switched map
                    }

                    doCircle(nMostX, nMostY, SYS_VIEWR, [amTM, amA, &seenUIDList, this](int gridX, int gridY)
                    {
                        doUIDList(gridX, gridY, [amTM, amA, &seenUIDList, this](uint64_t gridUID)
                        {
                            switch(uidf::getUIDType(gridUID)){
                                case UID_PLY:
                                case UID_MON:
                                case UID_NPC:
                                    {
                                        if((gridUID != amTM.UID) && (seenUIDList.count(gridUID) == 0)){
                                            m_actorPod->forward(gridUID, {AM_ACTION, amA});
                                            seenUIDList.insert(gridUID);
                                        }
                                        break;
                                    }
                                default:
                                    {
                                        break;
                                    }
                            }
                            return false;
                        });
                        return false;
                    });

                    if(uidf::isPlayer(amTM.UID) && getGrid(nMostX, nMostY).mapID){
                        AMMapSwitchTrigger amMST;
                        std::memset(&amMST, 0, sizeof(amMST));

                        amMST.UID   = uidf::getMapBaseUID(getGrid(nMostX, nMostY).mapID); // TODO
                        amMST.mapID = getGrid(nMostX, nMostY).mapID;
                        amMST.X     = getGrid(nMostX, nMostY).switchX;
                        amMST.Y     = getGrid(nMostX, nMostY).switchY;
                        m_actorPod->forward(amTM.UID, {AM_MAPSWITCHTRIGGER, amMST});
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
    const auto fromUID = mpk.from();
    const auto amTL = mpk.conv<AMTryLeave>();

    const auto fromGridX = amTL.X;
    const auto fromGridY = amTL.Y;

    if(!(in(ID(), fromGridX, fromGridY) && hasGridUID(mpk.from(), fromGridX, fromGridY))){
        m_actorPod->forward(mpk.from(), AM_REJECTLEAVE, mpk.seqID());
        g_monoServer->addLog(LOGTYPE_WARNING, "Leave request failed: UID = %llu, X = %d, Y = %d", to_llu(fromUID), fromGridX, fromGridY);
        return;
    }

    m_actorPod->forward(fromUID, AM_ALLOWLEAVE, mpk.seqID(), [fromUID, fromGridX, fromGridY, this](const ActorMsgPack &rmpk)
    {
        switch(rmpk.type()){
            case AM_LEAVEOK:
                {
                    const auto amLOK = rmpk.conv<AMLeaveOK>();
                    AMAction amA;
                    std::memset(&amA, 0, sizeof(amA));

                    amA.UID = fromUID;
                    amA.mapID = amLOK.mapID; // new map CO that switched into
                    amA.action = amLOK.action;

                    if(!removeGridUID(fromUID, fromGridX, fromGridY)){
                        throw fflerror("CO location error: (UID = %llu, X = %d, Y = %d)", to_llu(fromUID), fromGridX, fromGridY);
                    }

                    // broadcast new CO location to leaving map
                    // then all old neighbors can remove it from their inview list

                    std::unordered_set<uint64_t> seenUIDList;
                    doCircle(fromGridX, fromGridY, SYS_VIEWR, [fromUID, amA, &seenUIDList, this](int gridX, int gridY)
                    {
                        doUIDList(gridX, gridY, [fromUID, amA, &seenUIDList, this](uint64_t gridUID)
                        {
                            switch(uidf::getUIDType(gridUID)){
                                case UID_PLY:
                                case UID_MON:
                                case UID_NPC:
                                    {
                                        if((gridUID != fromUID) && (seenUIDList.count(gridUID) == 0)){
                                            m_actorPod->forward(gridUID, {AM_ACTION, amA});
                                            seenUIDList.insert(gridUID);
                                        }
                                        break;
                                    }
                                default:
                                    {
                                        break;
                                    }
                            }
                            return false;
                        });
                        return false;
                    });

                    m_actorPod->forward(fromUID, AM_FINISHLEAVE, rmpk.seqID());
                    return;
                }
            default:
                {
                    return;
                }
        }
    });
}

void ServerMap::on_AM_TRYMAPSWITCH(const ActorMsgPack &mpk)
{
    const auto fromUID = mpk.from();
    const auto amTMS = mpk.conv<AMTryMapSwitch>();

    if(!canMove(false, true, amTMS.X, amTMS.Y)){
        m_actorPod->forward(mpk.from(), AM_REJECTMAPSWITCH, mpk.seqID());
        return;
    }

    AMAllowMapSwitch amAMS;
    std::memset(&amAMS, 0, sizeof(amAMS));

    amAMS.Ptr = this;
    amAMS.X   = amTMS.X;
    amAMS.Y   = amTMS.Y;

    getGrid(amAMS.X, amAMS.Y).locked = true;
    m_actorPod->forward(mpk.from(), {AM_ALLOWMAPSWITCH, amAMS}, mpk.seqID(), [this, fromUID, amAMS](const ActorMsgPack &rmpk)
    {
        fflassert(getGrid(amAMS.X, amAMS.Y).locked);
        getGrid(amAMS.X, amAMS.Y).locked = false;

        switch(rmpk.type()){
            case AM_MAPSWITCHOK:
                {
                    const auto amMSOK = rmpk.conv<AMMapSwitchOK>();

                    AMAction amA;
                    std::memset(&amA, 0, sizeof(amA));

                    amA.UID = amMSOK.uid;
                    amA.mapID = amMSOK.mapID;
                    amA.action = amMSOK.action;

                    // didn't check map switch here
                    // map switch should be triggered by move request

                    addGridUID(fromUID, amAMS.X, amAMS.Y, true);
                    if(uidf::isPlayer(fromUID)){
                        postGroundItemIDList(fromUID, amAMS.X, amAMS.Y);
                        postGroundFireWallList(fromUID, amAMS.X, amAMS.Y); // doesn't help if switched map
                    }

                    std::unordered_set<uint64_t> seenUIDList;
                    doCircle(amAMS.X, amAMS.Y, SYS_VIEWR, [fromUID, amA, &seenUIDList, this](int gridX, int gridY)
                    {
                        doUIDList(gridX, gridY, [fromUID, amA, &seenUIDList, this](uint64_t gridUID)
                        {
                            switch(uidf::getUIDType(gridUID)){
                                case UID_PLY:
                                case UID_MON:
                                case UID_NPC:
                                    {
                                        if((gridUID != fromUID) && (seenUIDList.count(gridUID) == 0)){
                                            m_actorPod->forward(gridUID, {AM_ACTION, amA});
                                            seenUIDList.insert(gridUID);
                                        }
                                        break;
                                    }
                                default:
                                    {
                                        break;
                                    }
                            }
                            return false;
                        });
                        return false;
                    });
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

    int nX0  = amPF.X;
    int nY0  = amPF.Y;
    int nDir = amPF.direction;

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
    if(!stPathFinder.search(nX0, nY0, nDir, nX1, nY1).hasPath()){
        m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
        return;
    }

    // drop the first node
    // it's should be the provided start point
    const auto pathList = stPathFinder.getPathNode();
    if(pathList.size() < 2){
        m_actorPod->forward(rstMPK.from(), AM_ERROR, rstMPK.seqID());
        return;
    }

    int nCurrN = 0;
    int nCurrX = nX0;
    int nCurrY = nY0;

    for(size_t i = 1; i < pathList.size(); ++i){
        const auto pNode1 = pathList.data() + i;
        if(nCurrN >= to_d(nPathCount)){
            break;
        }
        int nEndX = pNode1->X;
        int nEndY = pNode1->Y;
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

void ServerMap::on_AM_DROPITEM(const ActorMsgPack &mpk)
{
    auto sdDI = mpk.deserialize<SDDropItem>();
    fflassert(sdDI.item);
    fflassert(groundValid(sdDI.x, sdDI.y));

    int bestX = -1;
    int bestY = -1;
    int checkGridCount = 0;
    int minGridItemCount = SYS_MAXDROPITEM + 1;

    RotateCoord rc(sdDI.x, sdDI.y, 0, 0, W(), H());
    do{
        if(groundValid(rc.x(), rc.y())){
            if(const auto currCount = to_d(getGridItemList(rc.x(), rc.y()).size()); currCount < minGridItemCount){
                bestX = rc.x();
                bestY = rc.y();
                minGridItemCount = currCount;
                if(minGridItemCount == 0){
                    break;
                }
            }
        }
    }while(rc.forward() && (checkGridCount++ <= SYS_MAXDROPITEMGRID));

    if(groundValid(bestX, bestY)){
        addGridItem(std::move(sdDI.item), bestX, bestY);
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

    fflassert(validC(amPU.x, amPU.y));
    fflassert(uidf::getUIDType(mpk.from()) == UID_PLY);

    std::vector<SDItem> pickedItemList;
    auto &itemList = getGridItemList(amPU.x, amPU.y);

    size_t goldCount = 0;
    for(auto p = itemList.begin(); p != itemList.end();){
        if(p->item.isGold()){
            goldCount += p->item.count;
            p = itemList.erase(p);
        }
        else{
            p++;
        }
    }

    if(goldCount > 0){
        pickedItemList.push_back(SDItem
        {
            .itemID = DBCOM_ITEMID(u8"金币（小）"),
            .count  = goldCount,
        });
    }

    size_t pickedWeight = 0;
    while(!itemList.empty()){
        const auto itemID = itemList.back().item.itemID;
        const auto &ir = DBCOM_ITEMRECORD(itemID);

        fflassert(ir);
        if(pickedWeight + ir.weight > amPU.availableWeight){
            break;
        }

        pickedWeight += ir.weight;
        pickedItemList.push_back(std::move(itemList.back().item));
        itemList.pop_back();
    }

    const SDPickUpItemList sdPUIL
    {
        .failedItemID = itemList.empty() ? 0 : itemList.back().item.itemID,
        .itemList = std::move(pickedItemList),
    };

    if(!sdPUIL.itemList.empty()){
        postGridItemIDList(amPU.x, amPU.y);
    }
    m_actorPod->forward(mpk.from(), {AM_PICKUPITEMLIST, cerealf::serialize(sdPUIL)}, mpk.seqID());
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

            .startTime      = hres_tstamp().to_msec(),
            .lastAttackTime = hres_tstamp().to_msec(),

            .duration = amCFW.duration,
            .dps      = amCFW.dps,
        });

        m_fireWallLocList.insert({amCFW.x, amCFW.y});
        postGridFireWallList(amCFW.x, amCFW.y);
    }
}

void ServerMap::on_AM_REMOTECALL(const ActorMsgPack &mpk)
{
    const auto sdRC = mpk.deserialize<SDRemoteCall>();
    m_luaRunner->spawn(m_threadKey++, mpk.fromAddr(), sdRC.code);
}
