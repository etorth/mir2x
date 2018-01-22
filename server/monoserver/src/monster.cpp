/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 01/21/2018 14:32:04
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
#include "motion.hpp"
#include "dbconst.hpp"
#include "dbcomid.hpp"
#include "monster.hpp"
#include "condcheck.hpp"
#include "netdriver.hpp"
#include "actorpod.hpp"
#include "mathfunc.hpp"
#include "memorypn.hpp"
#include "threadpn.hpp"
#include "sysconst.hpp"
#include "friendtype.hpp"
#include "randompick.hpp"
#include "monoserver.hpp"
#include "dbcomrecord.hpp"
#include "messagepack.hpp"
#include "protocoldef.hpp"

Monster::AStarCache::AStarCache()
    : Time(0)
    , MapID(0)
    , Path()
{}

bool Monster::AStarCache::Retrieve(int *pX, int *pY, int nX0, int nY0, int nX1, int nY1, uint32_t nMapID)
{
    extern MonoServer *g_MonoServer;
    if(g_MonoServer->GetTimeTick() < (Time + Refresh)){

        // if cache doesn't match we won't clean it
        // only cleared by timeout

        if((nMapID == MapID) && (Path.size() >= 3)){
            auto fnFindIndex = [this](int nX, int nY) -> int
            {
                for(int nIndex = 0; nIndex < (int)(Path.size()); ++nIndex){
                    if(true
                            && Path[nIndex].X == nX
                            && Path[nIndex].Y == nY){
                        return nIndex;
                    }
                }
                return -1;
            };

            auto nIndex0 = fnFindIndex(nX0, nY0);
            auto nIndex1 = fnFindIndex(nX1, nY1);

            if(true
                    && nIndex0 >= 0
                    && nIndex1 >= nIndex0 + 2){

                if(pX){ *pX = Path[nIndex0 + 1].X; }
                if(pY){ *pY = Path[nIndex0 + 1].Y; }

                return true;
            }
        }
        return false;
    }

    // time out, clean it
    Path.clear();
    return false;
}

void Monster::AStarCache::Cache(std::vector<PathFind::PathNode> stvPathNode, uint32_t nMapID)
{
    MapID = nMapID;
    Path.swap(stvPathNode);

    extern MonoServer *g_MonoServer;
    Time = g_MonoServer->GetTimeTick();
}

Monster::Monster(uint32_t   nMonsterID,
        ServiceCore        *pServiceCore,
        ServerMap          *pServerMap,
        int                 nMapX,
        int                 nMapY,
        int                 nDirection,
        uint8_t             nLifeState,
        uint32_t            nMasterUID)
    : CharObject(pServiceCore, pServerMap, nMapX, nMapY, nDirection, nLifeState)
    , m_MonsterID(nMonsterID)
    , m_MasterUID(nMasterUID)
    , m_MonsterRecord(DBCOM_MONSTERRECORD(nMonsterID))
    , m_AStarCache()
{
    if(!m_MonsterRecord){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid monster record: MonsterID = %d", (int)(MonsterID()));
        g_MonoServer->Restart();
    }

    SetState(STATE_DEAD    , 0);
    SetState(STATE_NEVERDIE, 0);

    // set attack mode
    // SetState(STATE_ATTACKMODE, STATE_ATTACKMODE_NORMAL);
    SetState(STATE_ATTACKMODE, STATE_ATTACKMODE_ATTACKALL);

    m_HP    = m_MonsterRecord.HP;
    m_HPMax = m_MonsterRecord.HP;
    m_MP    = m_MonsterRecord.MP;
    m_MPMax = m_MonsterRecord.MP;
}

bool Monster::RandomMove()
{
    if(CanMove()){
        auto fnMoveOneStep = [this]() -> bool
        {
            int nX = -1;
            int nY = -1;
            if(OneStepReach(Direction(), 1, &nX, &nY) == 1){
                return RequestMove(nX, nY, MoveSpeed(), false, [](){}, [](){});
            }
            return false;
        };

        auto fnMakeOneTurn = [this]() -> bool
        {
            static const int nDirV[]
            {
                DIR_UP,
                DIR_UPRIGHT,
                DIR_RIGHT,
                DIR_DOWNRIGHT,
                DIR_DOWN,
                DIR_DOWNLEFT,
                DIR_LEFT,
                DIR_UPLEFT,
            };

            auto nDirCount = (int)(sizeof(nDirV) / sizeof(nDirV[0]));
            auto nDirStart = (int)(std::rand() % nDirCount);

            for(int nIndex = 0; nIndex < nDirCount; ++nIndex){
                auto nDirection = nDirV[(nDirStart + nIndex) % nDirCount];
                if(Direction() != nDirection){
                    int nX = -1;
                    int nY = -1;
                    if(OneStepReach(nDirection, 1, &nX, &nY) == 1){
                        // current direction is possible for next move
                        // report the turn and do motion (by chance) in next update
                        m_Direction = nDirection;
                        DispatchAction(ActionStand(X(), Y(), Direction()));

                        // we won't do ReportStand() for monster
                        // monster's moving is only driven by server currently
                        return true;
                    }
                }
            }
            return false;
        };

        // by 20% make a move
        // if move failed we make a turn

        if(std::rand() % 97 < 20){
            if(!fnMoveOneStep()){
                return fnMakeOneTurn();
            }
            return true;
        }

        // by 20% make a turn only
        // if didn't try the move/trun

        if(std::rand() % 97 < 20){
            return fnMakeOneTurn();
        }

        // do nothing
        // stay as idle state
    }
    return false;
}

bool Monster::AttackUID(uint32_t nUID, int nDC)
{
    if(CanAttack()){
        extern MonoServer *g_MonoServer;
        if(auto stRecord = g_MonoServer->GetUIDRecord(nUID)){
            if(DCValid(nDC, true)){
                auto fnQueryLocationOK = [this, nDC, stRecord](const COLocation &stCOLocation) -> bool
                {
                    auto nX = stCOLocation.X;
                    auto nY = stCOLocation.Y;

                    // if we get inside current block, we should release the attack lock
                    // it can be released immediately if location retrieve succeeds without querying
                    m_AttackLock = false;
                    switch(nDC){
                        case DC_PHY_PLAIN:
                            {
                                switch(LDistance2(X(), Y(), nX, nY)){
                                    case 1:
                                    case 2:
                                        {
                                            m_Direction = PathFind::GetDirection(X(), Y(), nX, nY);
                                            if(CanAttack()){
                                                // 1. dispatch action to all
                                                DispatchAction(ActionAttack(X(), Y(), DC_PHY_PLAIN, AttackSpeed(), stRecord.UID()));

                                                extern MonoServer *g_MonoServer;
                                                m_LastAttackTime = g_MonoServer->GetTimeTick();

                                                // 2. send attack message to target
                                                //    target can ignore this message directly
                                                DispatchAttack(stRecord.UID(), DC_PHY_PLAIN);
                                            }
                                            return true;
                                        }
                                    case 0:
                                    default:
                                        {
                                            // if distance is zero
                                            // means the location cache is out of time

                                            // TODO
                                            // one issue is evertime we do AttackUID() || TrackUID()
                                            // then could be a case everytime we schedule an attack operation
                                            // but actually this attack can't be done successfully
                                            // then we don't do AttackUID() nor TrackUID()
                                            TrackUID(stRecord.UID());
                                            return false;
                                        }
                                }
                            }
                        case DC_MAG_FIRE:
                            {
                                return false;
                            }
                        default:
                            {
                                break;
                            }
                    }
                    return false;
                };

                // retrieving could schedule an location query
                // before the response we can't allow any attack request

                m_AttackLock = true;
                return RetrieveLocation(nUID, fnQueryLocationOK);
            }
        }
    }
    return false;
}

bool Monster::TrackUID(uint32_t nUID)
{
    if(CanMove()){
        return RetrieveLocation(nUID, [this](const COLocation &rstCOLocation) -> bool
        {
            auto nX     = rstCOLocation.X;
            auto nY     = rstCOLocation.Y;
            auto nMapID = rstCOLocation.MapID;

            if(nMapID == MapID()){
                switch(LDistance2(nX, nY, X(), Y())){
                    case 0:
                    case 1:
                    case 2:
                        {
                            return true;
                        }
                    default:
                        {
                            return MoveOneStep(nX, nY);
                        }
                }
            }
            return false;
        });
    }
    return false;
}

bool Monster::FollowMaster()
{
    auto nMasterUID = MasterUID();
    if(true
            && nMasterUID
            && CanMove()){

        // followMaster works almost like TrackUID(), but
        // 1. follower always try to stand at the back of the master
        // 2. when distance is too far or even located at different map, follower takes space move

        return RetrieveLocation(nMasterUID, [nMasterUID, this](const COLocation &rstCOLocation) -> bool
        {
            if(nMasterUID == MasterUID()){

                // check if it's still my master?
                // possible during the location query master changed

                auto nMapID     = rstCOLocation.MapID;
                auto nX         = rstCOLocation.X;
                auto nY         = rstCOLocation.Y;
                auto nDirection = rstCOLocation.Direction;

                // get back location with different distance
                // here we use different distance if space move and follow

                auto fnGetBack = [nX, nY, nDirection](int *pX, int *pY, int nLD) -> bool
                {
                    if(!PathFind::GetBackLocation(pX, pY, nX, nY, nDirection, nLD)){
                        // randomly pick a location
                        // for some COs it doesn't have direction
                        auto nRandDir = (std::rand() % 8) + (DIR_NONE + 1);
                        if(!PathFind::GetBackLocation(pX, pY, nX, nY, nRandDir, nLD)){
                            return false;
                        }
                    }
                    return true;
                };

                if(false
                        || (nMapID != MapID())
                        || (LDistance<double>(nX, nY, X(), Y()) > 10.0)){

                    // long distance
                    // slave have to do space move

                    int nBackX = -1;
                    int nBackY = -1;

                    if(fnGetBack(&nBackX, &nBackY, 3)){
                        return RequestSpaceMove(nMapID, nBackX, nBackY, false, [](){}, [](){});
                    }
                    return false;
                }else{

                    // not that long
                    // slave should move step by step

                    int nBackX = -1;
                    int nBackY = -1;

                    if(fnGetBack(&nBackX, &nBackY, 1)){
                        switch(LDistance2(nBackX, nBackY, X(), Y())){
                            case 0:
                                {
                                    // already get there
                                    // need to make a turn if needed

                                    if(Direction() != nDirection){
                                        m_Direction= nDirection;
                                        DispatchAction(ActionStand(X(), Y(), Direction()));
                                    }
                                    return true;
                                }
                            default:
                                {
                                    return MoveOneStep(nBackX, nBackY);
                                }
                        }
                    }
                    return false;
                }
            }

            return false;
        });
    }
    return false;
}

bool Monster::TrackAttack()
{
    do{
        CheckCurrTarget();
        if(!m_TargetQueue.Empty()){

            extern MonoServer *g_MonoServer;
            if(auto stRecord = g_MonoServer->GetUIDRecord(m_TargetQueue[0].UID)){

                // 1. try to attack uid
                for(auto nDC: m_MonsterRecord.DCList()){
                    if(AttackUID(stRecord.UID(), nDC)){
                        m_TargetQueue[0].ActiveTime = g_MonoServer->GetTimeTick();
                        return true;
                    }
                }

                // 2. try to track uid
                if(TrackUID(stRecord.UID())){
                    m_TargetQueue[0].ActiveTime = g_MonoServer->GetTimeTick();
                    return true;
                }

                // 3. not a proper target now
                //    we keep the target (for 1min) and try next one
                m_TargetQueue.Rotate(1);

            }else{

                // head target is not valid anymore
                // remove it from the target queue and try next

                m_TargetQueue.PopHead();
                continue;
            }
        }

    }while(!m_TargetQueue.Empty());

    // no target
    // this track-attack failed
    return false;
}

bool Monster::Update()
{
    if(HP() > 0){
        if(TrackAttack()){
            return true;
        }
        if(FollowMaster()){
            return true;
        }
        if(RandomMove()){
            return true;
        }
    }else{
        GoDie();
    }
    return true;
}

void Monster::OperateAM(const MessagePack &rstMPK, const Theron::Address &rstAddress)
{
    switch(rstMPK.Type()){
        case MPK_METRONOME:
            {
                On_MPK_METRONOME(rstMPK, rstAddress);
                break;
            }
        case MPK_NOTIFYDEAD:
            {
                On_MPK_NOTIFYDEAD(rstMPK, rstAddress);
                break;
            }
        case MPK_UPDATEHP:
            {
                On_MPK_UPDATEHP(rstMPK, rstAddress);
                break;
            }
        case MPK_EXP:
            {
                On_MPK_EXP(rstMPK, rstAddress);
                break;
            }
        case MPK_ACTION:
            {
                On_MPK_ACTION(rstMPK, rstAddress);
                break;
            }
        case MPK_ATTACK:
            {
                On_MPK_ATTACK(rstMPK, rstAddress);
                break;
            }
        case MPK_MAPSWITCH:
            {
                On_MPK_MAPSWITCH(rstMPK, rstAddress);
                break;
            }
        case MPK_QUERYLOCATION:
            {
                On_MPK_QUERYLOCATION(rstMPK, rstAddress);
                break;
            }
        case MPK_PULLCOINFO:
            {
                On_MPK_PULLCOINFO(rstMPK, rstAddress);
                break;
            }
        case MPK_BADACTORPOD:
            {
                On_MPK_BADACTORPOD(rstMPK, rstAddress);
                break;
            }
        case MPK_OFFLINE:
            {
                On_MPK_OFFLINE(rstMPK, rstAddress);
                break;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "Unsupported message: %s", rstMPK.Name());
                g_MonoServer->Restart();
                break;
            }
    }
}

void Monster::SearchViewRange()
{
}

void Monster::ReportCORecord(uint32_t nUID)
{
    if(nUID){
        extern MonoServer *g_MonoServer;
        if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
            AMCORecord stAMCOR;
            std::memset(&stAMCOR, 0, sizeof(stAMCOR));

            // TODO: don't use OBJECT_MONSTER, we need translation
            //       rule of communication, the sender is responsible to translate

            // 1. set type
            stAMCOR.COType = CREATURE_MONSTER;

            // 2. set current action
            stAMCOR.Action.UID   = UID();
            stAMCOR.Action.MapID = MapID();

            stAMCOR.Action.Action    = ACTION_STAND;
            stAMCOR.Action.Speed     = SYS_DEFSPEED;
            stAMCOR.Action.Direction = Direction();

            stAMCOR.Action.X    = X();
            stAMCOR.Action.Y    = Y();
            stAMCOR.Action.AimX = X();
            stAMCOR.Action.AimY = Y();

            stAMCOR.Action.AimUID      = 0;
            stAMCOR.Action.ActionParam = 0;

            // 3. set specified co information
            stAMCOR.Monster.MonsterID = MonsterID();

            // don't reply to server map
            // even get co information pull request from map
            m_ActorPod->Forward({MPK_CORECORD, stAMCOR}, stUIDRecord.GetAddress());
        }
    }
}

bool Monster::InRange(int nRangeType, int nX, int nY)
{
    if(true
            && m_Map
            && m_Map->ValidC(nX, nY)){
        switch(nRangeType){
            case RANGE_VISIBLE:
                {
                    return LDistance2(X(), Y(), nX, nY) < 20 * 20;
                }
            case RANGE_ATTACK:
                {
                    // inside this range
                    // monster will decide to make an attack
                    return LDistance2(X(), Y(), nX, nY) < 10 * 10;
                }
        }
    }
    return false;
}

DamageNode Monster::GetAttackDamage(int nDC)
{
    switch(nDC){
        case DC_PHY_PLAIN:
            {
                return {UID(), nDC, m_MonsterRecord.DC + std::rand() % (1 + std::max<int>(m_MonsterRecord.DCMax - m_MonsterRecord.DC, 0)), EC_NONE};
            }
        case DC_MAG_FIRE:
            {
                return {UID(), nDC, m_MonsterRecord.MDC + std::rand() % (1 + std::max<int>(m_MonsterRecord.MDCMax - m_MonsterRecord.MDC, 0)), EC_FIRE};
            }
        default:
            {
                return {};
            }
    }
}

bool Monster::CanMove()
{
    if(CharObject::CanMove()){
        extern MonoServer *g_MonoServer;
        return g_MonoServer->GetTimeTick() >= m_LastMoveTime + m_MonsterRecord.WalkWait;
    }
    return false;
}

bool Monster::CanAttack()
{
    if(CharObject::CanAttack()){
        extern MonoServer *g_MonoServer;
        return g_MonoServer->GetTimeTick() >= m_LastAttackTime + m_MonsterRecord.AttackWait;
    }
    return false;
}

bool Monster::DCValid(int nDC, bool bCheck)
{
    if(std::find(m_MonsterRecord.DCList().begin(), m_MonsterRecord.DCList().end(), nDC) != m_MonsterRecord.DCList().end()){
        if(bCheck){
            if(auto &rstDCR = DB_DCRECORD(nDC)){
                if(m_HP < rstDCR.HP){ return false; }
                if(m_MP < rstDCR.MP){ return false; }
            }
        }
        return true;
    }
    return false;
}

void Monster::RemoveTarget(uint32_t nUID)
{
    if(nUID){

        // cache queue don't support remove in middle
        // we implement here

        auto fnRemoveIndex = [this](int nIndex)
        {
            if(nIndex >= 0 && nIndex < (int)(m_TargetQueue.Length())){
                m_TargetQueue.Rotate(nIndex);
                m_TargetQueue.PopHead();
                m_TargetQueue.Rotate(-1 * nIndex);
            }
        };

        for(size_t nIndex = 0; nIndex < m_TargetQueue.Length();){
            if(m_TargetQueue[nIndex].UID == nUID){
                fnRemoveIndex(nIndex);
            }else{
                nIndex++;
            }
        }
    }
}

void Monster::AddTarget(uint32_t nUID)
{
    if(true
            && nUID
            && nUID != MasterUID()){

        extern MonoServer *g_MonoServer;
        for(size_t nIndex = 0; nIndex < m_TargetQueue.Length(); ++nIndex){
            if(m_TargetQueue[nIndex].UID == nUID){
                m_TargetQueue[nIndex].ActiveTime = g_MonoServer->GetTimeTick();
                return;
            }
        }

        m_TargetQueue.PushBack(TargetRecord(nUID, g_MonoServer->GetTimeTick()));
    }
}

bool Monster::GoDie()
{
    switch(GetState(STATE_NEVERDIE)){
        case 0:
            {
                switch(GetState(STATE_DEAD)){
                    case 0:
                        {
                            RandomDrop();
                            DispatchHitterExp();
                            DispatchAction(ActionDie(X(), Y(), Direction()));

                            // let's dispatch ActionDie before mark it dead
                            // theoratically dead actor shouldn't dispatch anything

                            SetState(STATE_DEAD, 1);

                            Delay(2 * 1000, [this](){ GoGhost(); });
                            return true;
                        }
                    default:
                        {
                            return true;
                        }
                }
            }
        default:
            {
                return false;
            }
    }
}

bool Monster::GoGhost()
{
    switch(GetState(STATE_NEVERDIE)){
        case 0:
            {
                switch(GetState(STATE_DEAD)){
                    case 0:
                        {
                            return false;
                        }
                    default:
                        {
                            // 1. setup state and inform all others
                            SetState(STATE_GHOST, 1);

                            AMDeadFadeOut stAMDFO;
                            stAMDFO.UID   = UID();
                            stAMDFO.MapID = MapID();
                            stAMDFO.X     = X();
                            stAMDFO.Y     = Y();

                            if(true
                                    && ActorPodValid()
                                    && m_Map
                                    && m_Map->ActorPodValid()){
                                m_ActorPod->Forward({MPK_DEADFADEOUT, stAMDFO}, m_Map->GetAddress());
                            }

                            // 2. deactivate the actor here
                            //    disable the actorpod then no source can drive it
                            //    then current *this* can't be refered by any actor threads after this invocation
                            //    then MonoServer::EraseUID() is safe to delete *this*
                            //
                            //    don't do delete m_ActorPod to disable the actor
                            //    since currently we are in the actor thread which accquired by m_ActorPod
                            Deactivate();

                            // 3. without message driving it
                            //    the char object will be inactive and activities after this
                            GoSuicide();
                            return true;

                            // there is an time gap after Deactivate() and before deletion handler called in GoSuicide
                            // then during this gap even if the actor is scheduled we won't have data race anymore
                            // since we called Deactivate() which deregistered Innhandler refers *this*
                            //
                            // note that even if during this gap we have functions call GetAddress()
                            // we are still OK since m_ActorPod is still valid
                            // but if then send to this address, it will drain to the default message handler
                        }
                }
            }
        default:
            {
                return false;
            }
    }
}

bool Monster::GoSuicide()
{
    if(true
            && GetState(STATE_DEAD)
            && GetState(STATE_GHOST)){

        // 1. register a operationi to the thread pool to delete
        // 2. don't pass *this* to any other threads, pass UID instead
        extern ThreadPN *g_ThreadPN;
        return g_ThreadPN->Add([nUID = UID()](){
            if(nUID){
                extern MonoServer *g_MonoServer;
                g_MonoServer->EraseUID(nUID);
            }else{
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "Suicide with empty UID");
            }
        });

        // after this line
        // *this* is invalid and should never be refered
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "GoSuicide(this = %p, UID = %" PRIu32 ") failed", this, UID());
    return false;
}

bool Monster::StruckDamage(const DamageNode &rstDamage)
{
    if(rstDamage){
        m_HP = std::max<int>(0, HP() - rstDamage.Damage);
        DispatchHealth();

        if(HP() <= 0){
            GoDie();
        }
        return true;
    }
    return false;
}

bool Monster::MoveOneStep(int nX, int nY)
{
    switch(FindPathMethod()){
        case FPMETHOD_ASTAR   : return MoveOneStepAStar  (nX, nY);
        case FPMETHOD_GREEDY  : return MoveOneStepGreedy (nX, nY);
        case FPMETHOD_COMBINE : return MoveOneStepCombine(nX, nY);
        default               : return false;
    }
}

bool Monster::MoveOneStepGreedy(int nX, int nY)
{
    int nX0 = X();
    int nY0 = Y();
    int nX1 = -1;
    int nY1 = -1;

    switch(LDistance2(nX0, nY0, nX, nY)){
        case 0:
            {
                return false;
            }
        case 1:
        case 2:
            {
                nX1 = nX;
                nY1 = nY;
                break;
            }
        default:
            {
                nX1 = nX0 + ((nX > nX0) - (nX < nX0));
                nY1 = nY0 + ((nY > nY0) - (nY < nY0));
                break;
            }
    }

    if(CanMove()){
        if(m_Map && m_Map->GroundValid(nX1, nY1)){
            return RequestMove(nX1, nY1, MoveSpeed(), false, [](){}, [](){});
        }
    }
    return false;
}

bool Monster::MoveOneStepCombine(int nX, int nY)
{
    int nX0 = X();
    int nY0 = Y();

    switch(LDistance2(nX0, nY0, nX, nY)){
        case 0:
            {
                return false;
            }
        case 1:
        case 2:
            {
                return RequestMove(nX, nY, MoveSpeed(), false, [](){}, [](){});
            }
        default:
            {
                break;
            }
    }

    int nXm = -1;
    int nYm = -1;

    if(m_AStarCache.Retrieve(&nXm, &nYm, X(), Y(), nX, nY, MapID())){
        return RequestMove(nXm, nYm, MoveSpeed(), false, [](){}, [](){});
    }

    // not a simple hop
    // and the a-star cache can't help

    auto stvPathNode = GetChaseGrid(nX, nY);
    auto fnOnErrorRound0 = [this, stvPathNode, nX, nY]()
    {
        auto fnOnErrorRound1 = [this, stvPathNode, nX, nY]()
        {
            auto fnOnErrorRound2 = [this, nX, nY]()
            {
                return MoveOneStepAStar(nX, nY);
            };
            return RequestMove(stvPathNode[2].X, stvPathNode[2].Y, MoveSpeed(), false, [](){}, fnOnErrorRound2);
        };
        return RequestMove(stvPathNode[1].X, stvPathNode[1].Y, MoveSpeed(), false, [](){}, fnOnErrorRound1);
    };
    return RequestMove(stvPathNode[0].X, stvPathNode[0].Y, MoveSpeed(), false, [](){}, fnOnErrorRound0);
}

bool Monster::MoveOneStepAStar(int nX, int nY)
{
    switch(LDistance2(X(), Y(), nX, nY)){
        case 0:
            {
                return false;
            }
        case 1:
        case 2:
            {
                return RequestMove(nX, nY, MoveSpeed(), false, [](){}, [](){});
            }
        default:
            {
                break;
            }
    }

    // try a-star cache first
    // if failed we need send resequst to server map

    int nXm = -1;
    int nYm = -1;

    if(m_AStarCache.Retrieve(&nXm, &nYm, X(), Y(), nX, nY, MapID())){
        return RequestMove(nXm, nYm, MoveSpeed(), false, [](){}, [](){});
    }

    // can't reach in one hop
    // need firstly do path finding by server map

    AMPathFind stAMPF;
    stAMPF.UID     = UID();
    stAMPF.MapID   = MapID();
    stAMPF.MaxStep = 1;
    stAMPF.CheckCO = true;
    stAMPF.X       = X();
    stAMPF.Y       = Y();
    stAMPF.EndX    = nX;
    stAMPF.EndY    = nY;

    auto fnOnResp = [this, nX, nY](const MessagePack &rstRMPK, const Theron::Address &)
    {
        switch(rstRMPK.Type()){
            case MPK_PATHFINDOK:
                {
                    AMPathFindOK stAMPFOK;
                    std::memcpy(&stAMPFOK, rstRMPK.Data(), sizeof(stAMPFOK));

                    // cache current result
                    // use it for next path finding
                    constexpr auto nNodeCount = std::extent<decltype(stAMPFOK.Point)>::value;
                    static_assert(nNodeCount >= 2, "");

                    auto pBegin = stAMPFOK.Point;
                    auto pEnd   = stAMPFOK.Point + nNodeCount; 

                    std::vector<PathFind::PathNode> stvPathNode;
                    for(auto pCurr = pBegin; pCurr != pEnd; ++pCurr){
                        if(m_Map->GroundValid(pCurr->X, pCurr->Y)){
                            stvPathNode.emplace_back(pCurr->X, pCurr->Y);
                        }else{
                            break;
                        }
                    }

                    stvPathNode.emplace_back(nX, nY);
                    m_AStarCache.Cache(stvPathNode, MapID());

                    // done cache
                    // do request move as normal

                    RequestMove(stAMPFOK.Point[1].X, stAMPFOK.Point[1].Y, MoveSpeed(), false, [](){}, [](){});
                    break;
                }
            default:
                {
                    break;
                }
        }
    };
    return m_ActorPod->Forward({MPK_PATHFIND, stAMPF}, m_Map->GetAddress(), fnOnResp);
}

int Monster::FindPathMethod()
{
    return FPMETHOD_COMBINE;
}

void Monster::RandomDrop()
{
    for(auto &rstGroupRecord: DB_MONSTERDROPITEM(MonsterID())){
        for(auto &rstItemRecord: rstGroupRecord.second){
            if(std::rand() % rstItemRecord.ProbRecip == 0){
                AMNewDropItem stAMNDI;
                stAMNDI.UID   = UID();
                stAMNDI.X     = X();
                stAMNDI.Y     = Y();
                stAMNDI.ID    = rstItemRecord.ID;
                stAMNDI.Value = rstItemRecord.Value;

                // suggest server map to add a new drop item, but server map
                // may reject this suggestion silently.
                //
                // and if we are not in group-0
                // break if we select the first one item
                m_ActorPod->Forward({MPK_NEWDROPITEM, stAMNDI}, m_Map->GetAddress());
                if(rstGroupRecord.first != 0){
                    break;
                }
            }
        }
    }
}

void Monster::CheckCurrTarget()
{
    // make the first target valid, means
    // 1. not time out
    // 2. not friend

    while(!m_TargetQueue.Empty()){

        // 1. check time-out
        {
            extern MonoServer *g_MonoServer;
            if(g_MonoServer->GetTimeTick() >= m_TargetQueue[0].ActiveTime + 60 * 1000){
                m_TargetQueue.PopHead();
                continue;
            }
        }

        // 2. remove friend
        {
            bool bFriend  = false;
            auto nCurrUID = m_TargetQueue[0].UID;

            CheckFriend(nCurrUID, [&bFriend](int nFriendType)
            {
                switch(nFriendType){
                    case FRIENDTYPE_ENEMY:
                        {
                            break;
                        }
                    default:
                        {
                            bFriend = true;
                            break;
                        }
                }
            });

            if(bFriend){
                m_TargetQueue.PopHead();
                continue;
            }
        }

        // pass tests
        // keep the head as current target

        return;
    }
}

void Monster::CheckFriend(uint32_t nCheckUID, const std::function<void(int)> &fnOnFriend)
{
    enum UIDFromType: int
    {
        UIDFROM_NONE    = 0,
        UIDFROM_PLAYER  = 1,
        UIDFROM_SUMMON  = 2,
        UIDFROM_SLAVE   = 3,
        UIDFROM_MONSTER = 4,
    };

    auto fnUIDFrom = [](uint32_t nUID) -> int
    {
        // define return code
        // <= 0: error
        //    1: player
        //    2: 变异骷髅 or 神兽
        //    3: monster, but tameble
        //    4: monster

        extern MonoServer *g_MonoServer;
        if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
            if(stUIDRecord.ClassFrom<Player>()){
                return UIDFROM_PLAYER;
            }else if(stUIDRecord.ClassFrom<Monster>()){
                switch(stUIDRecord.GetInvarData().Monster.MonsterID){
                    case DBCOM_MONSTERID(u8"神兽"):
                    case DBCOM_MONSTERID(u8"变异骷髅"):
                        {
                            return UIDFROM_SUMMON;
                        }
                    default:
                        {
                            return UIDFROM_MONSTER;
                        }
                }
            }
        }
        return UIDFROM_NONE;
    };

    if(MasterUID()){
        switch(fnUIDFrom(MasterUID())){
            case UIDFROM_PLAYER:
                {
                    switch(fnUIDFrom(nCheckUID)){
                        case UIDFROM_PLAYER:
                        case UIDFROM_SUMMON:
                            {
                                fnOnFriend(FRIENDTYPE_FRIEND);
                                return;
                            }
                        case UIDFROM_MONSTER:
                            {
                                fnOnFriend(FRIENDTYPE_ENEMY);
                                return;
                            }
                        default:
                            {
                                return;
                            }
                    }
                }
            case UIDFROM_MONSTER:
                {
                    switch(fnUIDFrom(nCheckUID)){
                        case UIDFROM_PLAYER:
                        case UIDFROM_SUMMON:
                            {
                                fnOnFriend(FRIENDTYPE_ENEMY);
                                return;
                            }
                        case UIDFROM_MONSTER:
                            {
                                fnOnFriend(FRIENDTYPE_FRIEND);
                                return;
                            }
                        default:
                            {
                                return;
                            }
                    }
                }
            default:
                {
                    return;
                }
        }
    }else{

        // a monster without master
        // do everything decided by itself

        switch(fnUIDFrom(nCheckUID)){
            case UIDFROM_PLAYER:
            case UIDFROM_SUMMON:
                {
                    fnOnFriend(FRIENDTYPE_ENEMY);
                    return;
                }
            case UIDFROM_MONSTER:
                {
                    fnOnFriend(FRIENDTYPE_FRIEND);
                    return;
                }
            default:
                {
                    return;
                }
        }
    }
}

InvarData Monster::GetInvarData() const
{
    InvarData stData;
    stData.Monster.MonsterID = MonsterID();
    return stData;
}

std::array<PathFind::PathNode, 3> Monster::GetChaseGrid(int nX, int nY)
{
    // always get the next step to chase
    // this function won't check if (nX, nY) is valid

    int nX0 = X();
    int nY0 = Y();

    std::array<PathFind::PathNode, 3> stvPathNode
    {{
        {-1, -1},
        {-1, -1},
        {-1, -1},
    }};

    int nDX = ((nX > nX0) - (nX < nX0));
    int nDY = ((nY > nY0) - (nY < nY0));

    switch(std::abs(nDX) + std::abs(nDY)){
        case 1:
            {
                if(nDY){
                    stvPathNode[0] = {nX0 + 0, nY0 + nDY};
                    stvPathNode[1] = {nX0 - 1, nY0 + nDY};
                    stvPathNode[2] = {nX0 + 1, nY0 + nDY};
                }else{
                    stvPathNode[0] = {nX0 + nDX, nY0 + 0};
                    stvPathNode[1] = {nX0 + nDX, nY0 - 1};
                    stvPathNode[2] = {nX0 + nDX, nY0 + 1};
                }
                break;
            }
        case 2:
            {
                stvPathNode[0] = {nX0 + nDX, nY0 + nDY};
                stvPathNode[1] = {nX0      , nY0 + nDY};
                stvPathNode[2] = {nX0 + nDX, nY0      };
                break;
            }
        default:
            {
                break;
            }
    }
    return stvPathNode;
}
