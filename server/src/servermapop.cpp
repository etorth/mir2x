#include <cinttypes>
#include "player.hpp"
#include "npchar.hpp"
#include "dbcomid.hpp"
#include "monster.hpp"
#include "strf.hpp"
#include "uidf.hpp"
#include "uidsf.hpp"
#include "mathf.hpp"
#include "sysconst.hpp"
#include "actorpod.hpp"
#include "raiitimer.hpp"
#include "servermap.hpp"
#include "server.hpp"
#include "rotatecoord.hpp"
#include "serverargparser.hpp"

extern Server *g_server;
extern ServerArgParser *g_serverArgParser;

corof::awaitable<> ServerMap::on_AM_METRONOME(const ActorMsgPack &)
{
    updateMapGrid();
    if(!g_serverArgParser->sharedConfig().disableMapScript){
        m_luaRunner->resume(m_mainScriptThreadKey);
    }
}

corof::awaitable<> ServerMap::on_AM_BADACTORPOD(const ActorMsgPack &)
{
}

corof::awaitable<> ServerMap::on_AM_ACTION(const ActorMsgPack &rstMPK)
{
    const auto amA = rstMPK.conv<AMAction>();
    if(!mapBin()->validC(amA.action.x, amA.action.y)){
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
        if(true || mapBin()->validC(nX, nY)){
            doUIDList(nX, nY, [this, amA](uint64_t nUID) -> bool
            {
                if(nUID != amA.UID){
                    switch(uidf::getUIDType(nUID)){
                        case UID_PLY:
                        case UID_MON:
                        case UID_NPC:
                            {
                                m_actorPod->post(nUID, {AM_ACTION, amA});
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

corof::awaitable<> ServerMap::on_AM_TRYSPACEMOVE(const ActorMsgPack &mpk)
{
    const auto amTSM = mpk.conv<AMTrySpaceMove>();
    const auto fnPrintMoveError = [&amTSM]()
    {
        g_server->addLog(LOGTYPE_WARNING, "TRYSPACEMOVE[%p]::X          = %d", &amTSM, amTSM.X);
        g_server->addLog(LOGTYPE_WARNING, "TRYSPACEMOVE[%p]::Y          = %d", &amTSM, amTSM.Y);
        g_server->addLog(LOGTYPE_WARNING, "TRYSPACEMOVE[%p]::EndX       = %d", &amTSM, amTSM.EndX);
        g_server->addLog(LOGTYPE_WARNING, "TRYSPACEMOVE[%p]::EndY       = %d", &amTSM, amTSM.EndY);
        g_server->addLog(LOGTYPE_WARNING, "TRYSPACEMOVE[%p]::StrictMove = %s", &amTSM, to_boolcstr(amTSM.StrictMove));
    };

    if(!in(UID(), amTSM.X, amTSM.Y)){
        g_server->addLog(LOGTYPE_WARNING, "Invalid location: (X, Y)");
        fnPrintMoveError();
        m_actorPod->post(mpk.fromAddr(), AM_REJECTSPACEMOVE);
        return;
    }

    if(!hasGridUID(mpk.from(), amTSM.X, amTSM.Y)){
        g_server->addLog(LOGTYPE_WARNING, "Can't find CO at current location: (UID, X, Y)");
        fnPrintMoveError();
        m_actorPod->post(mpk.fromAddr(), AM_REJECTSPACEMOVE);
        return;
    }

    int nDstX = amTSM.EndX;
    int nDstY = amTSM.EndY;

    if(!mapBin()->validC(amTSM.X, amTSM.Y)){
        if(amTSM.StrictMove){
            m_actorPod->post(mpk.fromAddr(), AM_REJECTSPACEMOVE);
            return;
        }

        nDstX = mathf::rand() % mapBin()->w();
        nDstY = mathf::rand() % mapBin()->h();
    }

    const auto loc = getRCValidGrid(false, false, amTSM.StrictMove ? 1 : 100, nDstX, nDstY);
    if(!loc.has_value()){
        m_actorPod->post(mpk.fromAddr(), AM_REJECTSPACEMOVE);
        return;
    }

    std::tie(nDstX, nDstY) = loc.value();

    AMAllowSpaceMove amASM;
    std::memset(&amASM, 0, sizeof(amASM));

    amASM.X = amTSM.X;
    amASM.Y = amTSM.Y;
    amASM.EndX = nDstX;
    amASM.EndY = nDstY;

    m_actorPod->send(mpk.fromAddr(), {AM_ALLOWSPACEMOVE, amASM}, [this, fromUID = mpk.from(), amTSM, nDstX, nDstY](const ActorMsgPack &rmpk)
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
                    amA.mapUID = amSMOK.mapUID;
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
                                            m_actorPod->post(gridUID, {AM_ACTION, amA});
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
                                            m_actorPod->post(gridUID, {AM_ACTION, amA});
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

corof::awaitable<> ServerMap::on_AM_TRYJUMP(const ActorMsgPack &mpk)
{
    const auto amTJ = mpk.conv<AMTryJump>();
    const auto fnPrintJumpError = [&amTJ]()
    {
        g_server->addLog(LOGTYPE_WARNING, "TRYJUMP[%p]::X    = %d", &amTJ, amTJ.X);
        g_server->addLog(LOGTYPE_WARNING, "TRYJUMP[%p]::Y    = %d", &amTJ, amTJ.Y);
        g_server->addLog(LOGTYPE_WARNING, "TRYJUMP[%p]::EndX = %d", &amTJ, amTJ.EndX);
        g_server->addLog(LOGTYPE_WARNING, "TRYJUMP[%p]::EndY = %d", &amTJ, amTJ.EndY);
    };

    if(!in(UID(), amTJ.X, amTJ.Y)){
        g_server->addLog(LOGTYPE_WARNING, "Invalid location: (X, Y)");
        fnPrintJumpError();
        m_actorPod->post(mpk.fromAddr(), AM_REJECTJUMP);
        return;
    }

    // we never allow server to handle motion to invalid grid
    // for client every motion request need to be prepared to avoid this

    if(!in(UID(), amTJ.EndX, amTJ.EndY)){
        g_server->addLog(LOGTYPE_WARNING, "Invalid location: (EndX, EndY)");
        fnPrintJumpError();
        m_actorPod->post(mpk.fromAddr(), AM_REJECTJUMP);
        return;
    }

    if(!hasGridUID(mpk.from(), amTJ.X, amTJ.Y)){
        g_server->addLog(LOGTYPE_WARNING, "Can't find CO at current location: (UID, X, Y)");
        fnPrintJumpError();
        m_actorPod->post(mpk.fromAddr(), AM_REJECTJUMP);
        return;
    }

    AMAllowJump amAJ;
    std::memset(&amAJ, 0, sizeof(amAJ));

    amAJ.EndX = amTJ.EndX;
    amAJ.EndY = amTJ.EndY;

    m_actorPod->send(mpk.fromAddr(), {AM_ALLOWJUMP, amAJ}, [this, amTJ, fromUID = mpk.from()](const ActorMsgPack &rmpk)
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
                    amA.mapUID = amJOK.mapUID;
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
                                            m_actorPod->post(gridUID, {AM_ACTION, amA});
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
                                            m_actorPod->post(gridUID, {AM_ACTION, amA});
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

                    if(uidf::isPlayer(fromUID) && getGrid(amTJ.EndX, amTJ.EndY).mapUID){
                        AMMapSwitchTrigger amMST;
                        std::memset(&amMST, 0, sizeof(amMST));

                        amMST.mapUID = getGrid(amTJ.EndX, amTJ.EndY).mapUID;
                        amMST.X      = getGrid(amTJ.EndX, amTJ.EndY).switchX;
                        amMST.Y      = getGrid(amTJ.EndX, amTJ.EndY).switchY;
                        m_actorPod->post(fromUID, {AM_MAPSWITCHTRIGGER, amMST});
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

corof::awaitable<> ServerMap::on_AM_TRYMOVE(const ActorMsgPack &rstMPK)
{
    AMTryMove amTM;
    std::memcpy(&amTM, rstMPK.data(), sizeof(amTM));

    auto fnPrintMoveError = [&amTM]()
    {
        g_server->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::UID           = %" PRIu32 , &amTM, amTM.UID);
        g_server->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::mapUID        = %" PRIu64 , &amTM, amTM.mapUID);
        g_server->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::X             = %d"       , &amTM, amTM.X);
        g_server->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::Y             = %d"       , &amTM, amTM.Y);
        g_server->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::EndX          = %d"       , &amTM, amTM.EndX);
        g_server->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::EndY          = %d"       , &amTM, amTM.EndY);
        g_server->addLog(LOGTYPE_WARNING, "TRYMOVE[%p]::AllowHalfMove = %s"       , &amTM, amTM.AllowHalfMove ? "true" : "false");
    };

    if(!in(amTM.mapUID, amTM.X, amTM.Y)){
        g_server->addLog(LOGTYPE_WARNING, "Invalid location: (X, Y)");
        fnPrintMoveError();
        m_actorPod->post(rstMPK.fromAddr(), AM_REJECTMOVE);
        return;
    }

    // we never allow server to handle motion to invalid grid
    // for client every motion request need to be prepared to avoid this

    if(!in(amTM.mapUID, amTM.EndX, amTM.EndY)){
        g_server->addLog(LOGTYPE_WARNING, "Invalid location: (EndX, EndY)");
        fnPrintMoveError();
        m_actorPod->post(rstMPK.fromAddr(), AM_REJECTMOVE);
        return;
    }

    if(!hasGridUID(amTM.UID, amTM.X, amTM.Y)){
        g_server->addLog(LOGTYPE_WARNING, "Can't find CO at current location: (UID, X, Y)");
        fnPrintMoveError();
        m_actorPod->post(rstMPK.fromAddr(), AM_REJECTMOVE);
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
                g_server->addLog(LOGTYPE_WARNING, "Invalid move request: (X, Y) -> (EndX, EndY)");
                fnPrintMoveError();
                m_actorPod->post(rstMPK.fromAddr(), AM_REJECTMOVE);
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
        m_actorPod->post(rstMPK.fromAddr(), AM_REJECTMOVE);
        return;
    }

    if(getGrid(nMostX, nMostY).locked){
        m_actorPod->post(rstMPK.fromAddr(), AM_REJECTMOVE);
        return;
    }

    AMAllowMove amAM;
    std::memset(&amAM, 0, sizeof(amAM));

    amAM.UID   = UID();
    amAM.mapUID = UID();
    amAM.X     = amTM.X;
    amAM.Y     = amTM.Y;
    amAM.EndX  = nMostX;
    amAM.EndY  = nMostY;

    getGrid(nMostX, nMostY).locked = true;
    m_actorPod->send(rstMPK.fromAddr(), {AM_ALLOWMOVE, amAM}, [this, amTM, nMostX, nMostY](const ActorMsgPack &rstRMPK)
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
                    amA.mapUID = amMOK.mapUID;
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
                                            m_actorPod->post(gridUID, {AM_ACTION, amA});
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
                                            m_actorPod->post(gridUID, {AM_ACTION, amA});
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

                    if(uidf::isPlayer(amTM.UID) && getGrid(nMostX, nMostY).mapUID){
                        AMMapSwitchTrigger amMST;
                        std::memset(&amMST, 0, sizeof(amMST));

                        amMST.mapUID = getGrid(nMostX, nMostY).mapUID;
                        amMST.X      = getGrid(nMostX, nMostY).switchX;
                        amMST.Y      = getGrid(nMostX, nMostY).switchY;
                        m_actorPod->post(amTM.UID, {AM_MAPSWITCHTRIGGER, amMST});
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

corof::awaitable<> ServerMap::on_AM_TRYLEAVE(const ActorMsgPack &mpk)
{
    const auto fromUID = mpk.from();
    const auto amTL = mpk.conv<AMTryLeave>();

    if(!(in(UID(), amTL.X, amTL.Y) && hasGridUID(mpk.from(), amTL.X, amTL.Y))){
        m_actorPod->post(mpk.fromAddr(), AM_REJECTLEAVE);
        co_return;
    }

    const auto rmpk = co_await m_actorPod->send(mpk.fromAddr(), AM_ALLOWLEAVE);
    if(rmpk.type() != AM_LEAVEOK){
        co_return;
    }

    const auto amLOK = rmpk.conv<AMLeaveOK>();

    AMAction amA;
    std::memset(&amA, 0, sizeof(amA));
    amA.UID = rmpk.from();
    amA.mapUID = amLOK.mapUID; // new map CO that switched into
    amA.action = amLOK.action;

    if(!removeGridUID(fromUID, amTL.X, amTL.Y)){
        throw fflerror("CO location error: (UID = %llu, X = %d, Y = %d)", to_llu(fromUID), amTL.X, amTL.Y);
    }

    // broadcast new CO location to leaving map
    // then all old neighbors can remove it from their inview list

    std::unordered_set<uint64_t> seenUIDList;
    doCircle(amTL.X, amTL.Y, SYS_VIEWR, [fromUID, &amA, &seenUIDList, this](int gridX, int gridY)
    {
        doUIDList(gridX, gridY, [fromUID, &amA, &seenUIDList, this](uint64_t gridUID)
        {
            switch(uidf::getUIDType(gridUID)){
                case UID_PLY:
                case UID_MON:
                case UID_NPC:
                    {
                        if((gridUID != fromUID) && !seenUIDList.contains(gridUID)){
                            seenUIDList.insert(gridUID);
                            m_actorPod->post(gridUID, {AM_ACTION, amA});
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

    m_actorPod->post({fromUID, rmpk.seqID()}, AM_FINISHLEAVE);
}

corof::awaitable<> ServerMap::on_AM_TRYMAPSWITCH(const ActorMsgPack &mpk)
{
    const auto fromUID = mpk.from();
    const auto amTMS = mpk.conv<AMTryMapSwitch>();

    if(!canMove(false, true, amTMS.X, amTMS.Y)){
        m_actorPod->post(mpk.fromAddr(), AM_REJECTMAPSWITCH);
        return;
    }

    AMAllowMapSwitch amAMS;
    std::memset(&amAMS, 0, sizeof(amAMS));
    amAMS.X = amTMS.X;
    amAMS.Y = amTMS.Y;

    getGrid(amAMS.X, amAMS.Y).locked = true;
    m_actorPod->send(mpk.fromAddr(), {AM_ALLOWMAPSWITCH, amAMS}, [this, fromUID, amAMS](const ActorMsgPack &rmpk)
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
                    amA.mapUID = amMSOK.mapUID;
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
                                            m_actorPod->post(gridUID, {AM_ACTION, amA});
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

corof::awaitable<> ServerMap::on_AM_PATHFIND(const ActorMsgPack &rstMPK)
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

    amPFOK.UID    = amPF.UID;
    amPFOK.mapUID = UID();

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
        g_server->addLog(LOGTYPE_WARNING, "Invalid MaxStep: %d, should be (1, 2, 3)", amPF.MaxStep);
    }

    ServerPathFinder stPathFinder(this, amPF.MaxStep, amPF.CheckCO);
    if(!stPathFinder.search(nX0, nY0, nDir, nX1, nY1).hasPath()){
        m_actorPod->post(rstMPK.fromAddr(), AM_ERROR);
        return;
    }

    // drop the first node
    // it's should be the provided start point
    const auto pathList = stPathFinder.getPathNode();
    if(pathList.size() < 2){
        m_actorPod->post(rstMPK.fromAddr(), AM_ERROR);
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
                    g_server->addLog(LOGTYPE_WARNING, "Invalid path node found");
                    break;
                }
        }
    }
    m_actorPod->post(rstMPK.fromAddr(), {AM_PATHFINDOK, amPFOK});
}

corof::awaitable<> ServerMap::on_AM_UPDATEHP(const ActorMsgPack &rstMPK)
{
    AMUpdateHP amUHP;
    std::memcpy(&amUHP, rstMPK.data(), sizeof(amUHP));

    if(mapBin()->validC(amUHP.X, amUHP.Y)){
        doCircle(amUHP.X, amUHP.Y, 20, [this, amUHP](int nX, int nY) -> bool
        {
            if(true || mapBin()->validC(nX, nY)){
                for(auto nUID: getUIDList(nX, nY)){
                    if(nUID != amUHP.UID){
                        if(uidf::getUIDType(nUID) == UID_PLY || uidf::getUIDType(nUID) == UID_MON){
                            m_actorPod->post(nUID, {AM_UPDATEHP, amUHP});
                        }
                    }
                }
            }
            return false;
        });
    }
}

corof::awaitable<> ServerMap::on_AM_DEADFADEOUT(const ActorMsgPack &mpk)
{
    // CO always send AM_DEADFADEOUT to server map to remove the grid occupation
    // and server map then boardcast to all its neighbors to remove this CO from their list to do clean

    const auto amDFO = mpk.conv<AMDeadFadeOut>();
    if(mapBin()->validC(amDFO.X, amDFO.Y)){
        removeGridUID(amDFO.UID, amDFO.X, amDFO.Y);
        doCircle(amDFO.X, amDFO.Y, 20, [this, amDFO](int nX, int nY) -> bool
        {
            if(true || mapBin()->validC(nX, nY)){
                for(auto nUID: getUIDList(nX, nY)){
                    if(nUID != amDFO.UID){
                        if(uidf::getUIDType(nUID) == UID_PLY || uidf::getUIDType(nUID) == UID_MON){
                            m_actorPod->post(nUID, {AM_DEADFADEOUT, amDFO});
                        }
                    }
                }
            }
            return false;
        });
    }
}

corof::awaitable<> ServerMap::on_AM_QUERYCOCOUNT(const ActorMsgPack &rstMPK)
{
    AMQueryCOCount amQCOC;
    std::memcpy(&amQCOC, rstMPK.data(), sizeof(amQCOC));

    if(amQCOC.mapUID && (amQCOC.mapUID != UID())){
        m_actorPod->post(rstMPK.fromAddr(), AM_ERROR);
        return;
    }

    size_t nCOCount = 0;
    for(int nX = 0; nX < to_d(mapBin()->w()); ++nX){
        for(int nY = 0; nY < to_d(mapBin()->h()); ++nY){
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
    m_actorPod->post(rstMPK.fromAddr(), {AM_COCOUNT, amCOC});
}

corof::awaitable<> ServerMap::on_AM_DROPITEM(const ActorMsgPack &mpk)
{
    auto sdDI = mpk.deserialize<SDDropItem>();
    fflassert(sdDI.item);
    fflassert(mapBin()->groundValid(sdDI.x, sdDI.y));

    int bestX = -1;
    int bestY = -1;
    int checkGridCount = 0;
    int minGridItemCount = SYS_MAXDROPITEM + 1;

    RotateCoord rc(sdDI.x, sdDI.y, 0, 0, mapBin()->w(), mapBin()->h());
    do{
        if(mapBin()->groundValid(rc.x(), rc.y())){
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

    if(mapBin()->groundValid(bestX, bestY)){
        addGridItem(std::move(sdDI.item), bestX, bestY);
    }
}

corof::awaitable<> ServerMap::on_AM_OFFLINE(const ActorMsgPack &rstMPK)
{
    AMOffline amO;
    std::memcpy(&amO, rstMPK.data(), sizeof(amO));

    // this may fail
    // because player may get offline at try move
    removeGridUID(amO.UID, amO.X, amO.Y);

    doCircle(amO.X, amO.Y, 10, [amO, this](int nX, int nY) -> bool
    {
        if(true || mapBin()->validC(nX, nY)){
            for(auto nUID: getUIDList(nX, nY)){
                if(nUID != amO.UID){
                    m_actorPod->post(nUID, {AM_OFFLINE, amO});
                }
            }
        }
        return false;
    });
}

corof::awaitable<> ServerMap::on_AM_PICKUP(const ActorMsgPack &mpk)
{
    const auto amPU = mpk.conv<AMPickUp>();

    fflassert(mapBin()->validC(amPU.x, amPU.y));
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
    m_actorPod->post(mpk.fromAddr(), {AM_PICKUPITEMLIST, cerealf::serialize(sdPUIL)});
}

corof::awaitable<> ServerMap::on_AM_STRIKEFIXEDLOCDAMAGE(const ActorMsgPack &mpk)
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
                    amA.mapUID = UID();

                    amA.X = amSFLD.x;
                    amA.Y = amSFLD.y;
                    amA.damage = amSFLD.damage;

                    m_actorPod->post(uid, {AM_ATTACK, amA});
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

corof::awaitable<> ServerMap::on_AM_CASTFIREWALL(const ActorMsgPack &mpk)
{
    const auto amCFW = mpk.conv<AMCastFireWall>();
    if(mapBin()->groundValid(amCFW.x, amCFW.y)){
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

corof::awaitable<> ServerMap::on_AM_REMOTECALL(const ActorMsgPack &mpk)
{
    auto sdRC = mpk.deserialize<SDRemoteCall>();
    m_luaRunner->spawn(m_threadKey++, mpk.fromAddr(), std::move(sdRC.code), std::move(sdRC.args));
}
