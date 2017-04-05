/*
 * =====================================================================================
 *
 *       Filename: creature.cpp
 *        Created: 08/31/2015 10:45:48 PM
 *  Last Modified: 04/05/2017 14:08:45
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

#include <string>
#include <cassert>
#include <algorithm>
#include <tinyxml2.h>
#include <SDL2/SDL.h>

#include "log.hpp"
#include "motion.hpp"
#include "mathfunc.hpp"
#include "creature.hpp"
#include "sysconst.hpp"
#include "processrun.hpp"
#include "protocoldef.hpp"
#include "clientpathfinder.hpp"

Creature::Creature(uint32_t nUID, ProcessRun *pRun, int nX, int nY, int nAction, int nDirection, int nSpeed)
    : m_UID(nUID)
    , m_ProcessRun(pRun)
    , m_X(nX)
    , m_Y(nY)
    , m_Action(nAction)
    , m_Direction(nDirection)
    , m_Speed(nSpeed)
    , m_MotionList()
    , m_Frame(0)
{
    assert(true
            && m_ProcessRun
            && m_ProcessRun->ValidC(nX, nY));
}

void Creature::EstimateLocation(int nDistance, int *pNextX, int *pNextY)
{
    static const int nDX[] = { 0, +1, +1, +1,  0, -1, -1, -1};
    static const int nDY[] = {-1, -1,  0, +1, +1, +1,  0, -1};

    if(pNextX){ *pNextX = m_X + nDistance * nDX[m_Direction]; }
    if(pNextY){ *pNextY = m_Y + nDistance * nDY[m_Direction]; }
}

void Creature::EstimatePixelShift(int *pShiftX, int *pShiftY)
{
    int nFrameCountInNextCell = (m_Direction == DIR_UPLEFT) ? 2 : 5;
    switch(m_Action){
        case ACTION_MOVE:
            {
                auto nFrameCount = (int)(MotionFrameCount());
                switch(m_Direction){
                    case DIR_UP:
                        {
                            if(pShiftX){ *pShiftX = 0; }
                            if(pShiftY){
                                if(m_Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (m_Frame + 1) * Speed());
                                }else{
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_Frame + 1)) * Speed());
                                }
                            }
                            return;
                        }
                    case DIR_UPRIGHT:
                        {
                            if(pShiftX){
                                if(m_Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (m_Frame + 1) * Speed());
                                }else{
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_Frame + 1)) * Speed());
                                }
                            }
                            if(pShiftY){
                                if(m_Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (m_Frame + 1) * Speed());
                                }else{
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_Frame + 1)) * Speed());
                                }
                            }
                            return;
                        }
                    case DIR_RIGHT:
                        {
                            if(pShiftX){
                                if(m_Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (m_Frame + 1) * Speed());
                                }else{
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_Frame + 1)) * Speed());
                                }
                            }
                            if(pShiftY){ *pShiftY = 0; }
                            return;
                        }
                    case DIR_DOWNRIGHT:
                        {
                            if(pShiftX){
                                if(m_Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (m_Frame + 1) * Speed());
                                }else{
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_Frame + 1)) * Speed());
                                }
                            }
                            if(pShiftY){
                                if(m_Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (m_Frame + 1) * Speed());
                                }else{
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_Frame + 1)) * Speed());
                                }
                            }
                            return;
                        }
                    case DIR_DOWN:
                        {
                            if(pShiftX){ *pShiftX = 0; }
                            if(pShiftY){
                                if(m_Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (m_Frame + 1) * Speed());
                                }else{
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_Frame + 1)) * Speed());
                                }
                            }
                            return;
                        }
                    case DIR_DOWNLEFT:
                        {
                            if(pShiftX){
                                if(m_Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (m_Frame + 1) * Speed());
                                }else{
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_Frame + 1)) * Speed());
                                }
                            }
                            if(pShiftY){
                                if(m_Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (m_Frame + 1) * Speed());
                                }else{
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_Frame + 1)) * Speed());
                                }
                            }
                            return;
                        }
                    case DIR_LEFT:
                        {
                            if(pShiftX){
                                if(m_Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (m_Frame + 1) * Speed());
                                }else{
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_Frame + 1)) * Speed());
                                }
                            }
                            if(pShiftY){ *pShiftY = 0; }
                            return;
                        }
                    case DIR_UPLEFT:
                        {
                            if(pShiftX){
                                if(m_Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (m_Frame + 1) * Speed());
                                }else{
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_Frame + 1)) * Speed());
                                }
                            }
                            if(pShiftY){
                                if(m_Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (m_Frame + 1) * Speed());
                                }else{
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_Frame + 1)) * Speed());
                                }
                            }
                            return;
                        }
                    default:
                        {
                            if(pShiftX){ *pShiftX = 0; }
                            if(pShiftY){ *pShiftY = 0; }
                            return;
                        }
                }
            }
        default:
            {
                if(pShiftX){ *pShiftX = 0; }
                if(pShiftY){ *pShiftY = 0; }
                return;
            }
    }
}

bool Creature::MoveNextMotion()
{
    switch(m_MotionList.size()){
        case 0:
            {
                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_FATAL, "Empty motion list");
                return false;
            }
        case 1:
            {
                m_MotionList.push_back({MOTION_STAND, m_MotionList.front().Direction, m_MotionList.front().NextX, m_MotionList.front().NextY});
                m_MotionList.pop_front();
                return true;
            }
        default:
            {
                m_MotionList.pop_front();
                return true;
            }
    }
}

bool Creature::EraseNextMotion()
{
    switch(m_MotionList.size()){
        case 0:
            {
                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_FATAL, "Empty motion list");
                return false;
            }
        case 1:
            {
                return true;
            }
        default:
            {
                m_MotionList.erase(std::next(m_MotionList.begin(), 1), m_MotionList.end());
                return true;
            }
    }
}

bool Creature::AdvanceMotionFrame(int nDFrame)
{
    auto nFrameCount = (int)(MotionFrameCount());
    if(nFrameCount > 0){
        m_Frame = (m_Frame + nDFrame    ) % nFrameCount;
        m_Frame = (m_Frame + nFrameCount) % nFrameCount;
        return true;
    }else{
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_FATAL, "Invalid arguments to MoveNextFrame(FrameCount = %d, DFrame = %d)", nFrameCount, nDFrame);
        return false;
    }
}

bool Creature::MotionValid(int nMotion, int nDirection)
{
    switch(nMotion){
        case MOTION_STAND:
        case MOTION_WALK:
        case MOTION_ATTACK:
        case MOTION_UNDERATTACK:
        case MOTION_DIE:
            {
                switch(nDirection){
                    case DIR_UP:
                    case DIR_UPRIGHT:
                    case DIR_RIGHT:
                    case DIR_DOWNRIGHT:
                    case DIR_DOWN:
                    case DIR_DOWNLEFT:
                    case DIR_LEFT:
                    case DIR_UPLEFT:
                        {
                            return true;
                        }
                    default:
                        {
                            return false;
                        }
                }
            }
        default:
            {
                return false;
            }
    }
}

bool Creature::ActionValid(int nAction, int nDirection)
{
    switch(nAction){
        case ACTION_STAND:
        case ACTION_MOVE:
        case ACTION_ATTACK:
        case ACTION_DIE:
            {
                switch(nDirection){
                    case DIR_UP:
                    case DIR_UPRIGHT:
                    case DIR_RIGHT:
                    case DIR_DOWNRIGHT:
                    case DIR_DOWN:
                    case DIR_DOWNLEFT:
                    case DIR_LEFT:
                    case DIR_UPLEFT:
                        {
                            return true;
                        }
                    case DIR_NONE:
                    default:
                        {
                            return false;
                        }
                }
                break;
            }
        default:
            {
                return false;
            }
    }
}

int Creature::GfxID()
{
    if(ActionValid(m_Action, m_Direction)){
        switch(m_Action){
            case ACTION_NONE:
                {
                    return -1;
                }
            default:
                {
                    static const std::unordered_map<int, int> stActionGfxIDRecord = {
                        {ACTION_STAND,      0},
                        {ACTION_MOVE,       1},
                        {ACTION_ATTACK,     2},
                        {ACTION_DIE,        3}};

                    if(stActionGfxIDRecord.find(m_Action) != stActionGfxIDRecord.end()){
                        switch(m_Direction){
                            case DIR_UP         : return ( 0 + (stActionGfxIDRecord.at(m_Action) << 3));
                            case DIR_DOWN       : return ( 4 + (stActionGfxIDRecord.at(m_Action) << 3));
                            case DIR_LEFT       : return ( 6 + (stActionGfxIDRecord.at(m_Action) << 3));
                            case DIR_RIGHT      : return ( 2 + (stActionGfxIDRecord.at(m_Action) << 3));
                            case DIR_UPLEFT     : return ( 7 + (stActionGfxIDRecord.at(m_Action) << 3));
                            case DIR_UPRIGHT    : return ( 1 + (stActionGfxIDRecord.at(m_Action) << 3));
                            case DIR_DOWNLEFT   : return ( 5 + (stActionGfxIDRecord.at(m_Action) << 3));
                            case DIR_DOWNRIGHT  : return ( 3 + (stActionGfxIDRecord.at(m_Action) << 3));
                            case DIR_NONE       : return (-1 + 0);
                            default             : return (-1 + 0);
                        }
                    }

                    return -1;
                }
        }
    }
    return -1;
}

bool Creature::ParseMovePath(int nMotion, int nSpeed, int nX0, int nY0, int nX1, int nY1)
{
    switch(LDistance2(nX0, nY0, nX1, nY1)){
        case 0:
            {
                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_FATAL, "Invalid argument: (%d, %d, %d, %d)", nX0, nY0, nX1, nY1);
                return false;
            }
        case 1:
        case 2:
        default:
            {
                ClientPathFinder stPathFinder(false);
                if(stPathFinder.Search(nX0, nY0, nX1, nY1)){
                    if(stPathFinder.GetSolutionStart()){
                        int nCurrX = nX0;
                        int nCurrY = nY0;
                        while(auto pNode1 = stPathFinder.GetSolutionNext()){
                            int nNextX = pNode1->X();
                            int nNextY = pNode1->Y();
                            switch(LDistance2(nCurrX, nCurrY, nNextX, nNextY)){
                                case 1:
                                case 2:
                                    {
                                        static const int nDirV[][3] = {
                                            {DIR_UPLEFT,   DIR_UP,   DIR_UPRIGHT  },
                                            {DIR_LEFT,     DIR_NONE, DIR_RIGHT    },
                                            {DIR_DOWNLEFT, DIR_DOWN, DIR_DOWNRIGHT}};

                                        int nDX = nNextX - nCurrX + 1;
                                        int nDY = nNextY - nCurrY + 1;
                                        m_MotionList.push_back({nMotion, nDirV[nDY][nDX], nSpeed, nCurrX, nCurrY, nNextX, nNextY});

                                        break;
                                    }
                                case 0:
                                default:
                                    {
                                        extern Log *g_Log;
                                        g_Log->AddLog(LOGTYPE_FATAL, "Invalid path find result");
                                        return false;
                                    }
                            }
                        }
                        return true;
                    }
                }
                return false;
            }
    }
}
