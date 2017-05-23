/*
 * =====================================================================================
 *
 *       Filename: myhero.cpp
 *        Created: 08/31/2015 08:52:57 PM
 *  Last Modified: 05/20/2017 21:32:24
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

bool MyHero::Update()
{
    double fTimeNow = SDL_GetTicks() * 1.0;
    if(fTimeNow > m_UpdateDelay + m_LastUpdateTime){
        // 1. record the update time
        m_LastUpdateTime = fTimeNow;

        // 2. logic update

        // 3. motion update
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
            default:
                {
                    return UpdateGeneralMotion(false);
                }
        }
    }
    return true;
}

bool MyHero::RequestMove(int nX, int nY)
{
    m_MotionQueue.clear();
    return ParseMovePath(MOTION_WALK, 1, m_CurrMotion.EndX, m_CurrMotion.EndY, nX, nY);
}

bool MyHero::MoveNextMotion()
{
    if(m_MotionQueue.empty()){
        if(m_ActionQueue.empty()){
            m_CurrMotion.Motion = MOTION_STAND;
            m_CurrMotion.Speed  = 0;
            m_CurrMotion.X      = m_CurrMotion.EndX;
            m_CurrMotion.Y      = m_CurrMotion.EndY;
            m_CurrMotion.Frame  = 0;
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
    if(ActionValid(rstAction)){
        if(bRemote){
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

bool MyHero::ParseNewNotice(const NoticeNode &, bool)
{
    return true;
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

    switch(m_ActionQueue.front().Action){
        case ACTION_MOVE:
            {
                int nX0 = m_ActionQueue.front().X;
                int nY0 = m_ActionQueue.front().Y;
                int nX1 = m_ActionQueue.front().EndX;
                int nY1 = m_ActionQueue.front().EndY;

                if(!m_ProcessRun->CanMove(false, nX0, nY0)){
                    extern Log *g_Log;
                    g_Log->AddLog(LOGTYPE_FATAL, "Motion start from invalid grid (%d, %d)", nX0, nY0);

                    m_ActionQueue.clear();
                    return false;
                }

                if(LDistance2(nX0, nY0, nX1, nY1) > 2){
                    ClientPathFinder stPathFinder(false, true);
                    if(true
                            && stPathFinder.Search(nX0, nY0, nX1, nY1)
                            && stPathFinder.GetSolutionStart()){

                        // prepare the path node vector
                        // won't check the ground during the search
                        // [  0] : src point
                        // [end] : dst point or next point of the furthest point one hop can reach

                        std::vector<PathFind::PathNode> stPathNodeV(1, {nX0, nY0});
                        while(auto pNode = stPathFinder.GetSolutionNext()){
                            // [  0] :      starting point
                            // [end] : next starting point
                            if(stPathNodeV.size() < (m_OnHorse ? 5 : 4)){
                                stPathNodeV.emplace_back(pNode->X(), pNode->Y());
                            }else{ break; }
                        }

                        // we do path search if LDistance2() > 2
                        // means at least we have two steps to reach the dst point

                        if(stPathNodeV.size() < 3){
                            extern Log *g_Log;
                            g_Log->AddLog(LOGTYPE_FATAL, "Invalid path node found from (%d, %d) -> (%d, %d)", nX0, nY0, nX1, nY1);
                            return false;
                        }

                        // furthest point it can reach
                        // 1. doesn't check ground
                        // 2. doesn't check creatures
                        auto nReachNode = PathFind::MaxReachNode(&(stPathNodeV[0]), stPathNodeV.size(), m_OnHorse ? 3 : 2);
                        if(nReachNode <= 0){
                            extern Log *g_Log;
                            g_Log->AddLog(LOGTYPE_FATAL, "Invalid path node found from (%d, %d) -> (%d, %d)", nX0, nY0, nX1, nY1);
                            return false;
                        }

                        // 1. check ground
                        //    remove grids can't move into
                        for(int nIndex = 1; nIndex <= nReachNode; ++nIndex){
                            if(true
                                    && m_ProcessRun
                                    && m_ProcessRun->CanMove(false, stPathNodeV[nIndex].X, stPathNodeV[nIndex].Y)){ continue; }
                            nReachNode = nIndex - 1;
                            break;
                        }

                        if(nReachNode == 0){
                            m_ActionQueue.clear();
                            return false;
                        }


                        // 2. check creatures
                        //    if there is creatures on the way
                        //    it means we can't find a better path
                        //
                        //    if there is no creatures we at least can reach node[1]
                        for(int nIndex = 1; nIndex <= nReachNode; ++nIndex){
                            if(true
                                    && m_ProcessRun
                                    && m_ProcessRun->CanMove(true, stPathNodeV[nIndex].X, stPathNodeV[nIndex].Y)){ continue; }
                            nReachNode = nIndex - 1;
                            break;
                        }

                        int nActionParam = MOTION_NONE;
                        switch(nReachNode){
                            case 0:
                                {
                                    // creatures is on the way
                                    // but we can move to next grid, try next time
                                    return false;
                                }
                            case 1:
                                {
                                    nActionParam = (m_OnHorse ? MOTION_HORSEWALK : MOTION_WALK);
                                    break;
                                }
                            case 2:
                                {
                                    // if on horse we have to use MOTION_HORSEWALK
                                    // else we can use MOTION_RUN
                                    if(m_OnHorse){
                                        nReachNode = 1;
                                        nActionParam = MOTION_HORSEWALK;
                                    }else{
                                        nReachNode = 2;
                                        nActionParam = MOTION_RUN;
                                    }
                                    break;
                                }
                            case 3:
                                {
                                    nActionParam = MOTION_HORSERUN;
                                    break;
                                }
                            default:
                                {
                                    extern Log *g_Log;
                                    g_Log->AddLog(LOGTYPE_FATAL, "Invalid internal logic");
                                    return false;
                                }
                        }

                        auto stFrontAction = m_ActionQueue.front();
                        m_ActionQueue.pop_front();

                        if(false
                                || stPathNodeV[nReachNode].X != nX1
                                || stPathNodeV[nReachNode].Y != nY1){

                            // if current last grid is not the dst point
                            // we still have grid to cross
                            m_ActionQueue.emplace_front(
                                    ACTION_MOVE,
                                    stFrontAction.ActionParam,
                                    stFrontAction.Speed,
                                    DIR_NONE,
                                    stPathNodeV[nReachNode].X,
                                    stPathNodeV[nReachNode].Y,
                                    nX1,
                                    nY1,
                                    m_ProcessRun->MapID());
                        }

                        m_ActionQueue.emplace_front(
                                ACTION_MOVE,
                                nActionParam,
                                stFrontAction.Speed,
                                DIR_NONE,
                                nX0,
                                nY0,
                                stPathNodeV[nReachNode].X,
                                stPathNodeV[nReachNode].Y,
                                m_ProcessRun->MapID());
                    }else{
                        // shouldn't be
                        // current clientpathfinder always succeeds for any map
                        // see logic description in clientpathfinder.cpp
                        extern Log *g_Log;
                        g_Log->AddLog(LOGTYPE_FATAL, "Path search failed for (%d, %d) -> (%d, %d)", nX0, nY0, nX1, nY1);
                        return false;
                    }
                }else{
                    // it's a distance of 1
                    // use single grid hop to present the motion
                    m_ActionQueue.front().ActionParam = (m_OnHorse ? MOTION_HORSEWALK : MOTION_WALK);
                }

                // 1. succefully decomposed
                // 2. only single-hop and no decomposition needed
                break;
            }
        default:
            {
                break;
            }
    }

    // pop if off to parse it
    auto stAction = m_ActionQueue.front();
    m_ActionQueue.pop_front();

    // 2. send this action to server for verification
    {
        CMAction stCMA;
        stCMA.UID         = UID();
        stCMA.MapID       = stAction.MapID;
        stCMA.Action      = stAction.Action;
        stCMA.ActionParam = stAction.ActionParam;
        stCMA.Speed       = stAction.Speed;
        stCMA.Direction   = stAction.Direction;
        stCMA.X           = stAction.X;
        stCMA.Y           = stAction.Y;
        stCMA.EndX        = stAction.EndX;
        stCMA.EndY        = stAction.EndY;

        extern Game *g_Game;
        g_Game->Send(CM_ACTION, stCMA);
    }

    // 3. present current *local* action
    //    we show the action without server verification
    //    later if server refused current action we'll do correction
    if(Hero::ParseNewAction(stAction, false)){
        m_CurrMotion = m_MotionQueue.front();
        m_MotionQueue.pop_front();
    }

    return true;
}

int MyHero::MaxStep()
{
    return 2;
}
