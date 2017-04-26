/*
 * =====================================================================================
 *
 *       Filename: myhero.cpp
 *        Created: 08/31/2015 08:52:57 PM
 *  Last Modified: 04/26/2017 13:16:04
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
                            auto rstAction = m_ActionQueue.front();
                            m_ActionQueue.pop_front();
                            return ParseNewAction(rstAction, false);
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
            return Hero::ParseNewAction(rstAction, true);
        }

        switch(rstAction.Action){
            case ACTION_MOVE:
                {
                    int nX0 = rstAction.X;
                    int nY0 = rstAction.Y;
                    int nX1 = rstAction.EndX;
                    int nY1 = rstAction.EndY;

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
                                                rstAction.Action,
                                                rstAction.ActionParam,
                                                rstAction.Speed,
                                                DIR_NONE,
                                                nX0,
                                                nY0,
                                                nEndX,
                                                nEndY
                                            };

                                            ActionNode stActionPart1 {
                                                rstAction.Action,
                                                rstAction.ActionParam,
                                                rstAction.Speed,
                                                DIR_NONE,
                                                nEndX,
                                                nEndY,
                                                nX1,
                                                nY1
                                            };

                                            m_ActionQueue.push_back(stActionPart0);
                                            m_ActionQueue.push_back(stActionPart1);
                                            return true;
                                        }
                                    default:
                                        {
                                            extern Log *g_Log;
                                            g_Log->AddLog(LOGTYPE_FATAL, "Invalid path node");
                                            return false;
                                        }
                                }
                            }
                        }
                    }

                    // we failed to get a path to destination currently
                    // just put the action in the action queue
                    m_ActionQueue.push_back(rstAction);
                    return true;
                }
            default:
                {
                    m_ActionQueue.push_back(rstAction);
                    return true;
                }
        }
    }
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

    // 1. assign first pending action node an ID
    m_ActionQueue.front().ID = (m_ActionCounter++);

    // 2. send this action to server for verification
    {
        CMAction stCMA;
        stCMA.Action      = m_ActionQueue.front().Action;
        stCMA.ActionParam = m_ActionQueue.front().ActionParam;
        stCMA.Speed       = m_ActionQueue.front().Speed;
        stCMA.Direction   = m_ActionQueue.front().Direction;
        stCMA.X           = m_ActionQueue.front().X;
        stCMA.Y           = m_ActionQueue.front().Y;
        stCMA.EndX        = m_ActionQueue.front().EndX;
        stCMA.EndY        = m_ActionQueue.front().EndY;
        stCMA.ID          = m_ActionQueue.front().ID;

        extern Game *g_Game;
        g_Game->Send(CM_ACTION, (const uint8_t *)(&stCMA), sizeof(stCMA));
    }

    // 3. parse local action to motion
    //    we show the action without server verification
    //    later if server refused current action we'll do correction
    {
        auto stAction = m_ActionQueue.front();
        m_ActionQueue.pop_front();
        return ParseNewAction(stAction, false);
    }
}
