/*
 * =====================================================================================
 *
 *       Filename: myhero.cpp
 *        Created: 08/31/2015 08:52:57 PM
 *  Last Modified: 07/04/2017 15:44:56
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
    auto fnGetUpdateDelay = [](int nSpeed, double fStandDelay) -> double
    {
        nSpeed = std::max<int>(SYS_MINSPEED, nSpeed);
        nSpeed = std::min<int>(SYS_MAXSPEED, nSpeed);

        return fStandDelay * 100.0 / nSpeed;
    };

    double fTimeNow = SDL_GetTicks() * 1.0;
    if(fTimeNow > fnGetUpdateDelay(m_CurrMotion.Speed, m_UpdateDelay) + m_LastUpdateTime){
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
            case MOTION_UNDERATTACK:
                {
                    if(m_MotionQueue.empty()){
                        if(m_ActionQueue.empty()){
                            return UpdateGeneralMotion(false);
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
    {
        auto stCurrAction = m_ActionQueue.front();
        m_ActionQueue.pop_front();

        switch(stCurrAction.Action){
            case ACTION_MOVE:
                {
                    int nX0 = stCurrAction.X;
                    int nY0 = stCurrAction.Y;
                    int nX1 = stCurrAction.EndX;
                    int nY1 = stCurrAction.EndY;

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
                                                    if(m_ProcessRun->CanMove(true, nX0, nY0, nX0 + nDX * nIndex, nY0 + nDY * nIndex)){
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
                                            break;
                                        }
                                }
                                break;
                            }
                    }

                    break;
                }
            default:
                {
                    break;
                }
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
            stCMA.EndX        = stCurrAction.EndX;
            stCMA.EndY        = stCurrAction.EndY;

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

int MyHero::MaxStep()
{
    return 2;
}
