/*
 * =====================================================================================
 *
 *       Filename: creature.cpp
 *        Created: 08/31/2015 10:45:48 PM
 *  Last Modified: 05/05/2017 18:30:48
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

bool Creature::EstimatePixelShift(int *pShiftX, int *pShiftY)
{
    const int nFrameCountInNextCell = (m_CurrMotion.Direction == DIR_UPLEFT) ? 2 : 5;
    switch(m_CurrMotion.Motion){
        case MOTION_WALK:
        case MOTION_RUN:
            {
                auto nFrameCount = (int)(MotionFrameCount());
                switch(m_CurrMotion.Direction){
                    case DIR_UP:
                        {
                            if(pShiftX){ *pShiftX = 0; }
                            if(pShiftY){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (m_CurrMotion.Frame + 1));
                                }else{
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1)));
                                }
                            }
                            return true;
                        }
                    case DIR_UPRIGHT:
                        {
                            if(pShiftX){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (m_CurrMotion.Frame + 1));
                                }else{
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1)));
                                }
                            }
                            if(pShiftY){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (m_CurrMotion.Frame + 1));
                                }else{
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1)));
                                }
                            }
                            return true;
                        }
                    case DIR_RIGHT:
                        {
                            if(pShiftX){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (m_CurrMotion.Frame + 1));
                                }else{
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1)));
                                }
                            }
                            if(pShiftY){ *pShiftY = 0; }
                            return true;
                        }
                    case DIR_DOWNRIGHT:
                        {
                            if(pShiftX){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (m_CurrMotion.Frame + 1));
                                }else{
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1)));
                                }
                            }
                            if(pShiftY){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (m_CurrMotion.Frame + 1));
                                }else{
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1)));
                                }
                            }
                            return true;
                        }
                    case DIR_DOWN:
                        {
                            if(pShiftX){ *pShiftX = 0; }
                            if(pShiftY){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (m_CurrMotion.Frame + 1));
                                }else{
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1)));
                                }
                            }
                            return true;
                        }
                    case DIR_DOWNLEFT:
                        {
                            if(pShiftX){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (m_CurrMotion.Frame + 1));
                                }else{
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1)));
                                }
                            }
                            if(pShiftY){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (m_CurrMotion.Frame + 1));
                                }else{
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1)));
                                }
                            }
                            return true;
                        }
                    case DIR_LEFT:
                        {
                            if(pShiftX){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (m_CurrMotion.Frame + 1));
                                }else{
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1)));
                                }
                            }
                            if(pShiftY){ *pShiftY = 0; }
                            return true;
                        }
                    case DIR_UPLEFT:
                        {
                            if(pShiftX){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (m_CurrMotion.Frame + 1));
                                }else{
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1)));
                                }
                            }
                            if(pShiftY){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (m_CurrMotion.Frame + 1));
                                }else{
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1)));
                                }
                            }
                            return true;
                        }
                    default:
                        {
                            if(pShiftX){ *pShiftX = 0; }
                            if(pShiftY){ *pShiftY = 0; }
                            return true;
                        }
                }
            }
        default:
            {
                if(pShiftX){ *pShiftX = 0; }
                if(pShiftY){ *pShiftY = 0; }
                return true;
            }
    }

    return false;
}

bool Creature::MoveNextMotion()
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

bool Creature::AdvanceMotionFrame(int nDFrame)
{
    auto nFrameCount = (int)(MotionFrameCount());
    if(nFrameCount > 0){
        m_CurrMotion.Frame = (m_CurrMotion.Frame + nDFrame    ) % nFrameCount;
        m_CurrMotion.Frame = (m_CurrMotion.Frame + nFrameCount) % nFrameCount;
        return true;
    }else{
        m_CurrMotion.Print();
        return false;
    }
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
                            int nEndX = pNode1->X();
                            int nEndY = pNode1->Y();
                            switch(LDistance2(nCurrX, nCurrY, nEndX, nEndY)){
                                case 1:
                                case 2:
                                    {
                                        static const int nDirV[][3] = {
                                            {DIR_UPLEFT,   DIR_UP,   DIR_UPRIGHT  },
                                            {DIR_LEFT,     DIR_NONE, DIR_RIGHT    },
                                            {DIR_DOWNLEFT, DIR_DOWN, DIR_DOWNRIGHT}};

                                        int nDX = nEndX - nCurrX + 1;
                                        int nDY = nEndY - nCurrY + 1;

                                        m_MotionQueue.push_back({nMotion, nDirV[nDY][nDX], nSpeed, nCurrX, nCurrY, nEndX, nEndY});

                                        nCurrX = nEndX;
                                        nCurrY = nEndY;
                                        break;
                                    }
                                case 0:
                                default:
                                    {
                                        extern Log *g_Log;
                                        g_Log->AddLog(LOGTYPE_FATAL, "Invalid path node");
                                        return false;
                                    }
                            }
                        }
                        return MotionQueueValid();
                    }
                }
                return false;
            }
    }
}

bool Creature::UpdateGeneralMotion(bool bLooped)
{
    auto nFrameCount = (int)(MotionFrameCount());
    if(nFrameCount >= 0){
        if(bLooped || (m_CurrMotion.Frame < (nFrameCount - 1))){
            return AdvanceMotionFrame(1);
        }else{
            return MoveNextMotion();
        }
    }
    return true;
}

bool Creature::MotionQueueValid()
{
    if(m_MotionQueue.empty()){ return true; }
    auto pLast = &m_CurrMotion;
    for(auto &rstMotion: m_MotionQueue){
        if(true
                && MotionValid(rstMotion)
                && (pLast->EndX == rstMotion.X)
                && (pLast->EndY == rstMotion.Y)){
            pLast = &rstMotion;
        }else{ return false; }
    }
    return true;
}
