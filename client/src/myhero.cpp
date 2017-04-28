/*
 * =====================================================================================
 *
 *       Filename: myhero.cpp
 *        Created: 08/31/2015 08:52:57 PM
 *  Last Modified: 04/28/2017 02:08:15
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
#include "clientpathfinder.hpp"

MyHero::MyHero(uint32_t nUID, uint32_t nDBID, bool bMale, uint32_t nDressID, ProcessRun *pRun, const ActionNode &rstAction)
	: Hero(nUID, nDBID, bMale, nDressID, pRun, rstAction)
    , m_ActionQueue()
    , m_ActionCounter(1)
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
        m_CurrMotion.Motion = MOTION_STAND;
        m_CurrMotion.Speed  = 0;
        m_CurrMotion.X      = m_CurrMotion.EndX;
        m_CurrMotion.Y      = m_CurrMotion.EndY;
        m_CurrMotion.Frame  = 0;

        return true;
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
            if(rstAction.ID){
                if(m_CurrMotion.ID >= rstAction.ID){
                    return true;
                }else{
                    return Hero::ParseNewAction(rstAction, true);
                }
            }else{
                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_WARNING, "Remote action with ID = %" PRIu32, rstAction.ID);
                rstAction.Print();
                return false;
            }
        }

        // OK it's a local action
        // put it into the action queue for next update
        m_ActionQueue.clear();
        m_ActionQueue.push_back(rstAction);
        return true;
    }

    rstAction.Print();
    return false;
}

bool MyHero::ParseNewState(const StateNode &, bool)
{
    return true;
}

bool MyHero::ParseActionQueue()
{
    if(m_ActionQueue.empty()){
        return true;
    }

    // all actions in action queue is local and un-verfified
    // 1. decompose action into simple action if needed
    // 2. send the simple action to server for verification
    // 3. at the same time present the actions

    // 1. assign first pending action node an ID
    //    then decompose current action with the same ID
    if(m_ActionQueue.front().ID){
    }else{
        m_ActionQueue.front().ID = (m_ActionCounter++);
    }

    switch(m_ActionQueue.front().Action){
        case ACTION_MOVE:
            {
                int nX0 = m_ActionQueue.front().X;
                int nY0 = m_ActionQueue.front().Y;
                int nX1 = m_ActionQueue.front().EndX;
                int nY1 = m_ActionQueue.front().EndY;

                if(LDistance2(nX0, nY0, nX1, nY1) > 2){
                    ClientPathFinder stPathFinder(true);
                    if(stPathFinder.Search(nX0, nY0, nX1, nY1)){
                        if(stPathFinder.GetSolutionStart()){
                            if(auto pNode1 = stPathFinder.GetSolutionNext()){
                                int nEndX = pNode1->X();
                                int nEndY = pNode1->Y();
                                switch(LDistance2(nX0, nY0, nEndX, nEndY)){
                                    case 1:
                                    case 2:
                                        {
                                            ActionNode stActionPart0 {
                                                m_ActionQueue.front().Action,
                                                m_ActionQueue.front().ActionParam,
                                                m_ActionQueue.front().Speed,
                                                DIR_NONE,
                                                nX0,
                                                nY0,
                                                nEndX,
                                                nEndY
                                            };

                                            ActionNode stActionPart1 {
                                                m_ActionQueue.front().Action,
                                                m_ActionQueue.front().ActionParam,
                                                m_ActionQueue.front().Speed,
                                                DIR_NONE,
                                                nEndX,
                                                nEndY,
                                                nX1,
                                                nY1
                                            };

                                            stActionPart0.ID = m_ActionQueue.front().ID;
                                            stActionPart1.ID = m_ActionQueue.front().ID;

                                            m_ActionQueue.pop_front();
                                            m_ActionQueue.push_front(stActionPart1);
                                            m_ActionQueue.push_front(stActionPart0);
                                            break;
                                        }
                                    default:
                                        {
                                            extern Log *g_Log;
                                            g_Log->AddLog(LOGTYPE_WARNING, "Invalid path node");
                                            return false;
                                        }
                                }
                            }
                        }
                    }else{
                        // return and try next time
                        // currently I can't find a way to do one-hop move
                        return true;
                    }
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
        stCMA.Action      = stAction.Action;
        stCMA.ActionParam = stAction.ActionParam;
        stCMA.Speed       = stAction.Speed;
        stCMA.Direction   = stAction.Direction;
        stCMA.X           = stAction.X;
        stCMA.Y           = stAction.Y;
        stCMA.EndX        = stAction.EndX;
        stCMA.EndY        = stAction.EndY;
        stCMA.ID          = stAction.ID;

        if(LDistance2(stAction.X, stAction.Y, stAction.EndX, stAction.EndY) > 2){
            stAction.Print();
        }

        extern Game *g_Game;
        g_Game->Send(CM_ACTION, stCMA);
    }

    // 3. present current *local* action
    //    we show the action without server verification
    //    later if server refused current action we'll do correction
    return Hero::ParseNewAction(stAction, false);
}
