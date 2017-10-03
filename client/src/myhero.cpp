/*
 * =====================================================================================
 *
 *       Filename: myhero.cpp
 *        Created: 08/31/2015 08:52:57 PM
 *  Last Modified: 10/02/2017 23:28:44
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

#include "log.hpp"
#include "game.hpp"
#include "myhero.hpp"
#include "message.hpp"
#include "mathfunc.hpp"
#include "processrun.hpp"
#include "clientpathfinder.hpp"

MyHero::MyHero(uint32_t nUID, uint32_t nDBID, bool bMale, uint32_t nDressID, ProcessRun *pRun, const ActionNode &rstAction)
	: Hero(nUID, nDBID, bMale, nDressID, pRun, rstAction)
    , m_ActionQueue()
{}

bool MyHero::Update(double fUpdateTime)
{
    // 1. independent from time control
    //    attached magic could take different speed
    UpdateAttachMagic(fUpdateTime);

    // 2. update this monster
    //    need fps control for current motion
    double fTimeNow = SDL_GetTicks() * 1.0;
    if(fTimeNow > CurrMotionDelay() + m_LastUpdateTime){

        // 1. record update time
        //    needed for next update
        m_LastUpdateTime = fTimeNow;

        // 2. do the update
        switch(m_CurrMotion.Motion){
            case MOTION_STAND:
                {
                    if(m_MotionQueue.empty()){
                        if(m_ActionQueue.empty()){
                            return AdvanceMotionFrame(1);
                        }else{
                            return ParseActionQueue();
                        }
                    }else{
                        // move to next motion will reset frame as 0
                        // if current there is no more motion pending
                        // it will add a MOTION_STAND
                        //
                        // we don't want to reset the frame here
                        return MoveNextMotion();
                    }
                }
            case MOTION_HITTED:
                {
                    if(m_MotionQueue.empty()){
                        if(m_ActionQueue.empty()){
                            return UpdateMotion(false);
                        }else{
                            return ParseActionQueue();
                        }
                    }else{
                        // move to next motion will reset frame as 0
                        // if current there is no more motion pending
                        // it will add a MOTION_STAND
                        //
                        // we don't want to reset the frame here
                        return MoveNextMotion();
                    }
                }
            default:
                {
                    return UpdateMotion(false);
                }
        }
    }
    return true;
}

bool MyHero::MoveNextMotion()
{
    if(m_MotionQueue.empty()){
        if(m_ActionQueue.empty()){
            m_CurrMotion = {
                MOTION_STAND,
                0,
                m_CurrMotion.Direction,
                m_CurrMotion.EndX,
                m_CurrMotion.EndY
            };
            return true;
        }else{
            // there is pending action in the queue
            // try to present it and return false if failed
            return ParseActionQueue();
        }
    }

    if(MotionQueueValid()){
        m_CurrMotion = m_MotionQueue.front();
        m_MotionQueue.pop_front();
        return true;
    }

    // oops we get invalid motion queue
    extern Log *g_Log;
    g_Log->AddLog(LOGTYPE_INFO, "Invalid motion queue:");

    m_CurrMotion.Print();
    for(auto &rstMotion: m_MotionQueue){
        rstMotion.Print();
    }

    extern Log *g_Log;
    g_Log->AddLog(LOGTYPE_FATAL, "Current motion is not valid");
    return false;
}

bool MyHero::ParseNewAction(const ActionNode &rstAction, bool bRemote)
{
    if(ActionValid(rstAction, bRemote)){
        if(bRemote){
            m_ActionQueue.clear();
            return Hero::ParseNewAction(rstAction, true);
        }else{
            // OK it's a local action
            // put it into the action queue for next update
            m_ActionQueue.clear();
            m_ActionQueue.push_back(rstAction);
            return true;
        }
    }

    return false;
}

bool MyHero::DecompMove(bool bCheckGround, bool bCheckCreature, bool bCheckMove, int nX0, int nY0, int nX1, int nY1, int *pXm, int *pYm)
{
    auto stPathNodeV = ParseMovePath(nX0, nY0, nX1, nY1, bCheckGround, bCheckCreature);
    switch(stPathNodeV.size()){
        case 0:
        case 1:
            {
                return false;
            }
        default:
            {
                if(bCheckMove){

                    // request to check if the first step is possible
                    // to avoid creature to move into invalid or occupied grids

                    auto nXt = stPathNodeV[1].X;
                    auto nYt = stPathNodeV[1].Y;

                    int nDX = (nXt > nX0) - (nXt < nX0);
                    int nDY = (nYt > nY0) - (nYt < nY0);

                    int nIndexMax = 0;
                    switch(LDistance2(nX0, nY0, nXt, nYt)){
                        case 1:
                        case 2:
                            {
                                nIndexMax = 1;
                                break;
                            }
                        case 4:
                        case 8:
                            {
                                nIndexMax = 2;
                                break;
                            }
                        case  9:
                        case 18:
                            {
                                nIndexMax = 3;
                                break;
                            }
                        default:
                            {
                                return false;
                            }
                    }

                    int nReachIndexMax = 0;
                    for(int nIndex = 1; nIndex <= nIndexMax; ++nIndex){
                        if(m_ProcessRun->CanMove(true, nX0 + nDX * nIndex, nY0 + nDY * nIndex)){
                            nReachIndexMax = nIndex;
                        }else{ break; }
                    }

                    switch(nReachIndexMax){
                        case 0:
                            {
                                // we find an impossible step
                                // need to reject and report failure
                                return false;
                            }
                        default:
                            {
                                // we allow step size as 1, 2, 3
                                // every creature has two possible step size 1, MaxStep

                                // so if it failed to reach grid with MaxStep size
                                // it should use sizeStep as 1 only

                                // like if human on horse failed for a MOTION_HORSERUN
                                // then it should only use MOTION_HORSEWALK, rather than MOTION_RUN

                                if(pXm){ *pXm = (nReachIndexMax == nIndexMax) ? nXt : (nX0 + nDX); }
                                if(pYm){ *pYm = (nReachIndexMax == nIndexMax) ? nYt : (nY0 + nDY); }

                                return true;
                            }
                    }

                }else{

                    // won't check if srcLoc -> midLoc is possible
                    // 1. could contain invalid grids if not set bCheckGround
                    // 2. could contain occuped grids if not set bCheckCreature

                    if(pXm){ *pXm = stPathNodeV[1].X; }
                    if(pYm){ *pYm = stPathNodeV[1].Y; }

                    return true;
                }
            }
    }
}

bool MyHero::ParseActionMove()
{
    if(true
            && !m_ActionQueue.empty()
            &&  m_ActionQueue.front().Action == ACTION_MOVE){

        auto stCurrAction = m_ActionQueue.front();
        m_ActionQueue.pop_front();

        int nX0 = stCurrAction.X;
        int nY0 = stCurrAction.Y;
        int nX1 = stCurrAction.AimX;
        int nY1 = stCurrAction.AimY;

        if(!m_ProcessRun->CanMove(false, nX0, nY0)){
            extern Log *g_Log;
            g_Log->AddLog(LOGTYPE_WARNING, "Motion start from invalid grid (%d, %d)", nX0, nY0);

            m_ActionQueue.clear();
            return false;
        }

        switch(LDistance2(nX0, nY0, nX1, nY1)){
            case 0:
                {
                    extern Log *g_Log;
                    g_Log->AddLog(LOGTYPE_WARNING, "Motion invalid (%d, %d) -> (%d, %d)", nX0, nY0, nX1, nY1);

                    m_ActionQueue.clear();
                    return false;
                }
            default:
                {
                    bool bCheckGround = m_ProcessRun->CanMove(false, nX1, nY1);
                    auto stPathNodeV  = ParseMovePath(nX0, nY0, nX1, nY1, bCheckGround, true);
                    switch(stPathNodeV.size()){
                        case 0:
                        case 1:
                            {
                                extern Log *g_Log;
                                g_Log->AddLog(LOGTYPE_WARNING, "Motion invalid (%d, %d) -> (%d, %d)", nX0, nY0, nX1, nY1);

                                m_ActionQueue.clear();
                                return false;
                            }
                        default:
                            {
                                // need to check the first step it gives
                                // to avoid hero to move into an invalid grid on the map

                                auto nXm = stPathNodeV[1].X;
                                auto nYm = stPathNodeV[1].Y;

                                int nDX = (nXm > nX0) - (nXm < nX0);
                                int nDY = (nYm > nY0) - (nYm < nY0);

                                int nIndexMax = 0;
                                switch(LDistance2(nX0, nY0, nXm, nYm)){
                                    case 1:
                                    case 2:
                                        {
                                            nIndexMax = 1;
                                            break;
                                        }
                                    case 4:
                                    case 8:
                                        {
                                            nIndexMax = 2;
                                            break;
                                        }
                                    case  9:
                                    case 18:
                                        {
                                            nIndexMax = 3;
                                            break;
                                        }
                                    default:
                                        {
                                            extern Log *g_Log;
                                            g_Log->AddLog(LOGTYPE_WARNING, "Motion invalid (%d, %d) -> (%d, %d)", nX0, nY0, nXm, nYm);

                                            m_ActionQueue.clear();
                                            return false;
                                        }
                                }

                                // reset (nXm, nYm) to be the max possible point
                                {
                                    nXm = nX0;
                                    nYm = nY0;

                                    for(int nIndex = 1; nIndex <= nIndexMax; ++nIndex){
                                        if(m_ProcessRun->CanMove(true, nX0 + nDX * nIndex, nY0 + nDY * nIndex)){
                                            nXm = nX0 + nDX * nIndex;
                                            nYm = nY0 + nDY * nIndex;
                                        }else{ break; }
                                    }
                                }

                                switch(LDistance2(nX0, nY0, nXm, nYm)){
                                    case 0:
                                        {
                                            // can't move since next is invalid
                                            // but this doesn't means the parse failed

                                            // make a turn if needed
                                            static const int nDirV[][3] = {
                                                {DIR_UPLEFT,   DIR_UP,   DIR_UPRIGHT  },
                                                {DIR_LEFT,     DIR_NONE, DIR_RIGHT    },
                                                {DIR_DOWNLEFT, DIR_DOWN, DIR_DOWNRIGHT}};

                                            m_ActionQueue.emplace_front(
                                                    ACTION_STAND,
                                                    0,
                                                    stCurrAction.Speed,
                                                    nDirV[nDY + 1][nDX + 1],
                                                    nX0,
                                                    nY0,
                                                    m_ProcessRun->MapID());
                                            break;
                                        }
                                    default:
                                        {
                                            // decomposition succeeds
                                            // need now to check if we are at the final point

                                            if(LDistance2(nXm, nYm, nX1, nY1)){

                                                // if current last grid is not the dst point
                                                // we still have grid to cross

                                                m_ActionQueue.emplace_front(
                                                        ACTION_MOVE,
                                                        stCurrAction.ActionParam,
                                                        stCurrAction.Speed,
                                                        DIR_NONE,
                                                        nXm,
                                                        nYm,
                                                        nX1,
                                                        nY1,
                                                        m_ProcessRun->MapID());
                                            }

                                            m_ActionQueue.emplace_front(
                                                    ACTION_MOVE,
                                                    stCurrAction.ActionParam,
                                                    stCurrAction.Speed,
                                                    DIR_NONE,
                                                    nX0,
                                                    nY0,
                                                    nXm,
                                                    nYm,
                                                    m_ProcessRun->MapID());
                                            break;
                                        }
                                }

                                return true;
                            }
                    }
                    break;
                }
        }
    }
    return false;
}

bool MyHero::ParseActionAttack()
{
    if(true
            && !m_ActionQueue.empty()
            &&  m_ActionQueue.front().Action == ACTION_ATTACK){

        auto stCurrAction = m_ActionQueue.front();
        m_ActionQueue.pop_front();

        int nX0 = m_CurrMotion.EndX;
        int nY0 = m_CurrMotion.EndY;
        int nX1 = stCurrAction.AimX;
        int nY1 = stCurrAction.AimY;

        switch(LDistance2(nX0, nY0, nX1, nY1)){
            case 0:
                {
                    extern Log *g_Log;
                    g_Log->AddLog(LOGTYPE_WARNING, "Invalid attack location (%d, %d) -> (%d, %d)", nX0, nY0, nX1, nY1);

                    m_ActionQueue.clear();
                    return false;
                }
            case 1:
            case 2:
                {
                    // don't parse more than one action at one call
                    // ok we are at the place for attack, assign direction here

                    static const int nDirV[][3] = {
                        {DIR_UPLEFT,   DIR_UP,   DIR_UPRIGHT  },
                        {DIR_LEFT,     DIR_NONE, DIR_RIGHT    },
                        {DIR_DOWNLEFT, DIR_DOWN, DIR_DOWNRIGHT}};

                    auto nSDX = nX1 - nX0 + 1;
                    auto nSDY = nY1 - nY0 + 1;

                    m_ActionQueue.emplace_front(
                            ACTION_ATTACK,
                            stCurrAction.ActionParam,
                            stCurrAction.Speed,
                            nDirV[nSDY][nSDX],
                            m_CurrMotion.EndX,
                            m_CurrMotion.EndY,
                            stCurrAction.AimX,
                            stCurrAction.AimY,
                            m_ProcessRun->MapID());
                    return true;
                }
            default:
                {
                    // complex part
                    // not close enough for the PLAIN_PHY_ATTACK
                    // need to schedule a path to move closer and then attack

                    int nXt = -1;
                    int nYt = -1;

                    if(DecompMove(true, true, true, nX0, nY0, nX1, nY1, &nXt, &nYt)){
                        if(false
                                || nXt != nX1
                                || nYt != nY1){

                            // we find a distinct point as middle point
                            // we firstly move to (nXt, nYt) then parse the attack action
                            m_ActionQueue.emplace_front(
                                    ACTION_ATTACK,
                                    stCurrAction.ActionParam,
                                    stCurrAction.Speed,
                                    DIR_NONE,
                                    nXt,
                                    nYt,
                                    nX1,
                                    nY1,
                                    m_ProcessRun->MapID());

                            m_ActionQueue.emplace_front(
                                    ACTION_MOVE,
                                    OnHorse() ? 1 : 0,
                                    stCurrAction.Speed,
                                    DIR_NONE,
                                    nX0,
                                    nY0,
                                    nXt,
                                    nYt,
                                    m_ProcessRun->MapID());
                        }else{

                            // one hop we can reach the attack location
                            // but we know step size between (nX0, nY0) and (nX1, nY1) > 1

                            int nDX = (nXt > nX0) - (nXt < nX0);
                            int nDY = (nYt > nY0) - (nYt < nY0);

                            m_ActionQueue.emplace_front(
                                    ACTION_ATTACK,
                                    stCurrAction.ActionParam,
                                    stCurrAction.Speed,
                                    DIR_NONE,
                                    nX0 + nDX,
                                    nY0 + nDY,
                                    nX1,
                                    nY1,
                                    m_ProcessRun->MapID());

                            m_ActionQueue.emplace_front(
                                    ACTION_MOVE,
                                    OnHorse() ? 1 : 0,
                                    stCurrAction.Speed,
                                    DIR_NONE,
                                    nX0,
                                    nY0,
                                    nX0 + nDX,
                                    nY0 + nDY,
                                    m_ProcessRun->MapID());
                        }

                        // ok action decomposition succeeds
                        // now the first action node is proper to parse & send
                        return true;

                    }else{

                        // decompse failed, why it failed?
                        // if can't reach we need to reject current action
                        // if caused by occupied grids of creatures, we need to keep it

                        if(DecompMove(true, false, false, nX0, nY0, nX1, nY1, nullptr, nullptr)){

                            // keep it
                            m_ActionQueue.emplace_front(stCurrAction);
                            return true;
                        }else{

                            m_ActionQueue.clear();
                            return false;
                        }
                    }
                }
        }
    }
    return false;
}

bool MyHero::ParseActionQueue()
{
    if(m_ActionQueue.empty()){
        return true;
    }

    // all actions in action queue is local and not verfified
    // present the action list immediately and sent it to server for verification
    // the server will keep silent if permitted, or send pull-back message if rejected

    // 1. decompose action into simple action if needed
    // 2. send the simple action to server for verification
    // 3. at the same time present the actions

    // decompose the first action if complex
    // make sure the first action is simple enough
    switch(m_ActionQueue.front().Action){
        case ACTION_MOVE:
            {
                if(!ParseActionMove()){
                    return false;
                }
                break;
            }
        case ACTION_ATTACK:
            {
                if(!ParseActionAttack()){
                    return false;
                }
                break;
            }
        case ACTION_SPELL:
            {
                break;
            }
        default:
            {
                break;
            }
    }

    // pick the first simple action and handle it
    {
        auto stCurrAction = m_ActionQueue.front();
        m_ActionQueue.pop_front();

        // 2. send this action to server for verification
        {
            CMAction stCMA;
            stCMA.UID         = UID();
            stCMA.MapID       = stCurrAction.MapID;
            stCMA.Action      = stCurrAction.Action;
            stCMA.ActionParam = stCurrAction.ActionParam;
            stCMA.Speed       = stCurrAction.Speed;
            stCMA.Direction   = stCurrAction.Direction;
            stCMA.X           = stCurrAction.X;
            stCMA.Y           = stCurrAction.Y;
            stCMA.AimX        = stCurrAction.AimX;
            stCMA.AimY        = stCurrAction.AimY;
            stCMA.AimUID      = stCurrAction.AimUID;

            extern Game *g_Game;
            g_Game->Send(CM_ACTION, stCMA);
        }

        // 3. present current *local* action
        //    we show the action without server verification
        //    later if server refused current action we'll do correction
        if(Hero::ParseNewAction(stCurrAction, false)){
            m_CurrMotion = m_MotionQueue.front();
            m_MotionQueue.pop_front();
        }
    }

    return true;
}

bool MyHero::StayIdle()
{
    return true
        && m_MotionQueue.empty()
        && m_ActionQueue.empty();
}
