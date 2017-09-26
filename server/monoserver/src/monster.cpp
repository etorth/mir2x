/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 09/26/2017 01:02:37
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
#include "netpod.hpp"
#include "dbconst.hpp"
#include "dbcomid.hpp"
#include "monster.hpp"
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
{
    if(!m_MonsterRecord){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid monster record: MonsterID = %d", (int)(MonsterID()));
        g_MonoServer->Restart();
    }

    auto fnRegisterClass = [this]() -> void {
        if(!RegisterClass<Monster, CharObject>()){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Class registration for <Monster, CharObject> failed");
            g_MonoServer->Restart();
        }
    };
    static std::once_flag stFlag;
    std::call_once(stFlag, fnRegisterClass);

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
        // 1. try move ahead
        //    would fail if next grid is not walkable
        {
            int nX = -1;
            int nY = -1;
            if(OneStepReach(Direction(), 1, &nX, &nY) == 1){
                return RequestMove(MOTION_MON_WALK, nX, nY, false, [](){}, [](){});
            }
        }

        // 2. if current direction leads to a *impossible* place
        //    then randomly take a new direction and try
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
                        // TODO
                        // current direction is possible for next move
                        // don't do turn and motion now
                        // report the turn and do motion (by chance) in next update
                        m_Direction = nDirection;
                        DispatchAction({ACTION_STAND, 0, Direction(), X(), Y(), MapID()});

                        // TODO
                        // we won't do ReportStand() for monster
                        // monster's moving is only driven by server currently
                        return true;
                    }
                }
            }
        }

        // 3. more motion method
        //    ....
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
                                                DispatchAction({ACTION_ATTACK, DC_PHY_PLAIN, Direction(), X(), Y(), MapID()});

                                                extern MonoServer *g_MonoServer;
                                                m_LastAttackTime = g_MonoServer->GetTimeTick();

                                                // 2. send attack message to target
                                                //    target can ignore this message directly
                                                DispatchAttack(stRecord.UID, DC_PHY_PLAIN);
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
                                            TrackUID(stRecord.UID);
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
    if(true
            && nUID
            && CanMove()){

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
                                        DispatchAction({ACTION_STAND, 0, Direction(), X(), Y(), MapID()});
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
    CheckTarget();
    while(!m_TargetQ.empty()){
        // 2. ok not expired
        //    check if current UID is valid
        //    track and record the track time if succeed
        extern MonoServer *g_MonoServer;
        if(auto stRecord = g_MonoServer->GetUIDRecord(m_TargetQ.front().UID)){
            // 1. try to attack uid
            for(auto nDC: m_MonsterRecord.DCList()){
                if(AttackUID(stRecord.UID, nDC)){
                    extern MonoServer *g_MonoServer;
                    m_TargetQ.front().ActiveTime = g_MonoServer->GetTimeTick();
                    return true;
                }
            }

            // 2. try to track uid
            if(TrackUID(stRecord.UID)){
                m_TargetQ.front().ActiveTime = g_MonoServer->GetTimeTick();
                return true;
            }

            // 3. not a proper target
            //    try next one
            m_TargetQ.push_back(m_TargetQ.front());
            m_TargetQ.pop_front();
            continue;
        }else{
            // 3. not valid even
            //    delete directly and try next one
            m_TargetQ.pop_front();
            continue;
        }
    }

    // no operation performed or issued
    return false;
}

bool Monster::Update()
{
    if(HP() > 0){
        if(TrackAttack() ){ return true; }
        if(FollowMaster()){ return true; }
        if(RandomMove()  ){ return true; }
    }else{ GoDie(); }
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

void Monster::ReportCORecord(uint32_t nSessionID)
{
    if(nSessionID){
        SMCORecord stSMCOR;
        // TODO: don't use OBJECT_MONSTER, we need translation
        //       rule of communication, the sender is responsible to translate

        // 1. set type
        stSMCOR.Type = CREATURE_MONSTER;

        // 2. set common info
        stSMCOR.Common.UID   = UID();
        stSMCOR.Common.MapID = MapID();

        stSMCOR.Common.Action      = ACTION_STAND;
        stSMCOR.Common.ActionParam = 0;
        stSMCOR.Common.Speed       = SYS_DEFSPEED;
        stSMCOR.Common.Direction   = Direction();

        stSMCOR.Common.X    = X();
        stSMCOR.Common.Y    = Y();
        stSMCOR.Common.EndX = X();
        stSMCOR.Common.EndY = Y();

        // 3. set specified info
        stSMCOR.Monster.MonsterID = m_MonsterID;

        extern NetPodN *g_NetPodN;
        g_NetPodN->Send(nSessionID, SM_CORECORD, stSMCOR);
    }else{
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid session id");
        g_MonoServer->Restart();
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
        return m_LastMoveTime + m_MonsterRecord.WalkWait <= g_MonoServer->GetTimeTick();
    }
    return false;
}

bool Monster::CanAttack()
{
    if(CharObject::CanAttack()){
        extern MonoServer *g_MonoServer;
        return m_LastAttackTime + m_MonsterRecord.AttackWait <= g_MonoServer->GetTimeTick();
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
        auto fnUIDCmp = [nUID](const TargetRecord &rstRecord) -> bool
        {
            return rstRecord.UID == nUID;
        };

        auto pLoc = std::find_if(m_TargetQ.begin(), m_TargetQ.end(), fnUIDCmp);
        if(pLoc != m_TargetQ.end()){
            m_TargetQ.erase(pLoc);
        }
    }
}

void Monster::AddTarget(uint32_t nUID)
{
    if(true
            && nUID
            && nUID != MasterUID()){

        auto fnFindUID = [nUID](TargetRecord &rstRecord) -> bool
        {
            if(rstRecord.UID == nUID){
                extern MonoServer *g_MonoServer;
                rstRecord.ActiveTime = g_MonoServer->GetTimeTick();
                return true;
            }else{
                return false;
            }
        };

        // fnFindUID will update time stamp if find UID
        // otherwise new record will be inserted to m_TargetQ

        if(std::find_if(m_TargetQ.begin(), m_TargetQ.end(), fnFindUID) == m_TargetQ.end()){
            extern MonoServer *g_MonoServer;
            m_TargetQ.emplace_back(nUID, g_MonoServer->GetTimeTick());
        }
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
                            SetState(STATE_DEAD, 1);

                            RandomDropItem();
                            DispatchHitterExp();
                            DispatchAction({ACTION_DIE, 0, Direction(), X(), Y(), MapID()});

                            extern MonoServer *g_MonoServer;
                            g_MonoServer->AddLog(LOGTYPE_WARNING, "Monster dead: %d", (int)(UID()));

                            Delay(10 * 1000, [this](){ GoGhost(); });
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
        DispatchMHP();

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

bool Monster::GreedyMove(int nX, int nY, std::function<void()> fnOnError)
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
            return RequestMove(MOTION_MON_WALK, nX1, nY1, false, [](){}, fnOnError);
        }

        // else we should call the error handler
        // otherwise combined move can't check next point if (nX1, nY1) is non-walkable
        fnOnError();
    }
    return false;
}

bool Monster::MoveOneStepGreedy(int nX, int nY)
{
    return GreedyMove(nX, nY, [](){});
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
                return RequestMove(MOTION_MON_WALK, nX, nY, false, [](){}, [](){});
            }
        default:
            {
                break;
            }
    }

    int nDX = ((nX > nX0) - (nX < nX0));
    int nDY = ((nY > nY0) - (nY < nY0));

    std::array<PathFind::PathNode, 3> stvPathNode;
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
                return false;
            }
    }

    auto fnOnErrorRound0 = [this, stvPathNode, nX, nY]()
    {
        auto fnOnErrorRound1 = [this, stvPathNode, nX, nY]()
        {
            auto fnOnErrorRound2 = [this, nX, nY]()
            {
                // we have no hope but use a-star
                // or we can just let the monster stop here
                return MoveOneStepAStar(nX, nY);
            };
            return GreedyMove(stvPathNode[2].X, stvPathNode[2].Y, fnOnErrorRound2);
        };
        return GreedyMove(stvPathNode[1].X, stvPathNode[1].Y, fnOnErrorRound1);
    };
    return GreedyMove(stvPathNode[0].X, stvPathNode[0].Y, fnOnErrorRound0);
}

bool Monster::MoveOneStepAStar(int nX, int nY)
{
    int nMoveMode = MOTION_MON_WALK;

    int nMaxStep = 0;
    switch(nMoveMode){
        case MOTION_WALK        : nMaxStep = 1; break;
        case MOTION_RUN         : nMaxStep = 2; break;
        case MOTION_ONHORSEWALK : nMaxStep = 1; break;
        case MOTION_ONHORSERUN  : nMaxStep = 3; break;
        case MOTION_MON_WALK    : nMaxStep = 1; break;
        default                 : return false;
    }

    int nDX = std::abs<int>(X() - nX);
    int nDY = std::abs<int>(Y() - nY);

    // +---+---+---+---+---+
    // |   |   |   |   |   |    one-hop motion can reach:
    // +---+---+---+---+---+
    // | 3 |   |   | 3 |   |    1: walk, horse_walk
    // +---+---+---+---+---+    2: run
    // | 2 |   | 2 |   |   |    3: horse_run
    // +---+---+---+---+---+
    // | 1 | 1 |   |   |   |
    // +---+---+---+---+---+
    // | O | 1 | 2 | 3 |   |
    // +---+---+---+---+---+

    if((nDX == 0) && (nDY == 0)){ return true; }
    if(true
            && ((std::max<int>(nDX, nDY) <= nMaxStep))
            && ((std::min<int>(nDX, nDY) == 0) || (nDX == nDY))){
        // can reach (nX, nY) in one-hop
        // possible reuslt for the motion request:
        //      1. completed fully      : MPK_MOVEOK
        //      2. completed partially  : MPK_MOVEOK
        //      3. failed               : MPK_ERROR
        // DON'T issue next motion if not complete the request
        // no matter how far it's from the destination, it only do one step and stop
        return RequestMove(nMoveMode, nX, nY, true, [](){}, [](){});
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

    auto fnOnResp = [this, nMoveMode, nMaxStep](const MessagePack &rstRMPK, const Theron::Address &){
        switch(rstRMPK.Type()){
            case MPK_PATHFINDOK:
                {
                    AMPathFindOK stAMPFOK;
                    std::memcpy(&stAMPFOK, rstRMPK.Data(), sizeof(stAMPFOK));

                    // 1. for all received result
                    //    perform strict argument checking

                    constexpr auto nPathNodeCount = sizeof(stAMPFOK.Point) / sizeof(stAMPFOK.Point[0]);
                    static_assert(nPathNodeCount >= 2, "Invalid structure AMPathFindOK: Path node count should be at least 2");

                    auto fnReportBadPath = [&stAMPFOK, nPathNodeCount](size_t nBadNodeIndex){
                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid path finding result: %p", &stAMPFOK);
                        for(size_t nIndex = 0; nIndex <= std::min<size_t>(nPathNodeCount - 1, nBadNodeIndex); ++nIndex){
                            g_MonoServer->AddLog(LOGTYPE_WARNING, "[%p]::Point[%d]::X = %d", &stAMPFOK, (int)(nIndex), stAMPFOK.Point[nIndex].X);
                            g_MonoServer->AddLog(LOGTYPE_WARNING, "[%p]::Point[%d]::Y = %d", &stAMPFOK, (int)(nIndex), stAMPFOK.Point[nIndex].Y);
                        }
                    };

                    for(size_t nIndex = 0; nIndex < nPathNodeCount; ++nIndex){
                        if(true
                                && m_Map
                                && m_Map->In(stAMPFOK.MapID, stAMPFOK.Point[nIndex].X, stAMPFOK.Point[nIndex].Y)){
                            if(nIndex > 0){
                                int nX0 = stAMPFOK.Point[nIndex - 1].X;
                                int nY0 = stAMPFOK.Point[nIndex - 1].Y;
                                int nX1 = stAMPFOK.Point[nIndex    ].X;
                                int nY1 = stAMPFOK.Point[nIndex    ].Y;
                                switch(LDistance2(nX0, nY0, nX1, nY1)){
                                    case 1:
                                    case 2:
                                        {
                                            break;
                                        }
                                    default:
                                        {
                                            // nMaxStep could be more than 1
                                            // but path finding result should always be one-grid distance
                                            fnReportBadPath(nIndex);
                                            return;
                                        }
                                }
                            }
                        }else{
                            // we allow invalid point (-1, -1) to indicate end
                            // but we should at least have [0] and [1] valid in the result
                            if(nIndex < 2){ fnReportBadPath(nIndex); return; }
                            break;
                        }
                    }

                    // 2. minimize the hop count
                    //    start to check from Point[2]
                    //    Point[0] is the staring place and Point[1] is always OK

                    auto nMaxStepCheck = std::min<int>(nMaxStep, nPathNodeCount - 1);
                    auto nMaxStepReach = nMaxStepCheck;

                    for(int nStep = 1; nStep <= nMaxStepCheck; ++nStep){
                        int nDX = std::abs<int>(stAMPFOK.Point[nStep].X - stAMPFOK.Point[0].X);
                        int nDY = std::abs<int>(stAMPFOK.Point[nStep].Y - stAMPFOK.Point[0].Y);
                        if(true
                                // 1. current point is valid
                                //    invalid (-1, -1) stays at the ending part
                                && m_Map
                                && m_Map->In(stAMPFOK.MapID, stAMPFOK.Point[nStep].X, stAMPFOK.Point[nStep].Y)

                                // 2. current path stays straight
                                && ((std::max<int>(nDX, nDY) == nStep))
                                && ((std::min<int>(nDX, nDY) == 0) || (nDX == nDY))){ continue; }

                        nMaxStepReach = nStep - 1;
                        break;
                    }

                    RequestMove(nMoveMode, stAMPFOK.Point[nMaxStepReach].X, stAMPFOK.Point[nMaxStepReach].Y, true, [](){}, [](){});
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

void Monster::RandomDropItem()
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

void Monster::CheckTarget()
{
    for(auto pInst = m_TargetQ.begin(); pInst != m_TargetQ.end();){
        extern MonoServer *g_MonoServer;
        if(pInst->ActiveTime + 60 * 1000 <= g_MonoServer->GetTimeTick()){
            pInst = m_TargetQ.erase(pInst);
        }else{
            pInst++;
        }
    }

    for(auto &rstTarget: m_TargetQ){
        auto fnOnFriend = [this, nUID = rstTarget.UID](int nFriendType)
        {
            switch(nFriendType){
                case FRIENDTYPE_ENEMY:
                    {
                        break;
                    }
                default:
                    {
                        RemoveTarget(nUID);
                        break;
                    }
            }
        };
        CheckFriend(rstTarget.UID, fnOnFriend);
    }
}

void Monster::CheckFriend(uint32_t nCheckUID, std::function<void(int)> fnOnFriend)
{
    enum UIDFromType: int
    {
        UIDFROM_NONE             = 0,
        UIDFROM_PLAYER           = 1,
        UIDFROM_SUMMON           = 2,
        UIDFROM_MONSTER_TAMEABLE = 3,
        UIDFROM_MONSTER          = 4,
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
                switch(stUIDRecord.Desp.Monster.MonsterID){
                    case DBCOM_MONSTERID(u8"神兽"):
                    case DBCOM_MONSTERID(u8"变异骷髅"):
                        {
                            return UIDFROM_SUMMON;
                        }
                    default:
                        {
                            if(auto &rstMR = DBCOM_MONSTERRECORD(stUIDRecord.Desp.Monster.MonsterID)){
                                return rstMR.Tameable ? UIDFROM_MONSTER_TAMEABLE : UIDFROM_MONSTER;
                            }
                            break;
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
                        case UIDFROM_MONSTER_TAMEABLE:
                            {
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
                        case UIDFROM_MONSTER_TAMEABLE:
                            {
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
            case UIDFROM_MONSTER_TAMEABLE:
                {
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

void Monster::DispatchSpaceMove()
{
    DispatchAction({ACTION_SPACEMOVE, X(), Y(), MapID()});
}

InvarData Monster::GetInvarData() const
{
    InvarData stData;
    stData.Monster.MonsterID = MonsterID();
    return stData;
}
