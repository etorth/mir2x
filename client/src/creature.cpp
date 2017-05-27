/*
 * =====================================================================================
 *
 *       Filename: creature.cpp
 *        Created: 08/31/2015 10:45:48 PM
 *  Last Modified: 05/26/2017 18:42:26
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
    int nGridSpeed = 0;
    switch(m_CurrMotion.Motion){
        case MOTION_WALK      : nGridSpeed = 1; break;
        case MOTION_RUN       : nGridSpeed = 2; break;
        case MOTION_HORSEWALK : nGridSpeed = 1; break;
        case MOTION_HORSERUN  : nGridSpeed = 3; break;
        default               : return false;
    }

    const int nFrameCountInNextCell = (m_CurrMotion.Direction == DIR_UPLEFT) ? 2 : 5;
    switch(m_CurrMotion.Motion){
        case MOTION_WALK:
        case MOTION_RUN:
        case MOTION_HORSEWALK:
        case MOTION_HORSERUN:
            {
                auto nFrameCount = (int)(MotionFrameCount());
                switch(m_CurrMotion.Direction){
                    case DIR_UP:
                        {
                            if(pShiftX){ *pShiftX = 0; }
                            if(pShiftY){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nGridSpeed;
                                }else{
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nGridSpeed;
                                }
                            }
                            return true;
                        }
                    case DIR_UPRIGHT:
                        {
                            if(pShiftX){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nGridSpeed;
                                }else{
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nGridSpeed;
                                }
                            }
                            if(pShiftY){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nGridSpeed;
                                }else{
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nGridSpeed;
                                }
                            }
                            return true;
                        }
                    case DIR_RIGHT:
                        {
                            if(pShiftX){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nGridSpeed;
                                }else{
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nGridSpeed;
                                }
                            }
                            if(pShiftY){ *pShiftY = 0; }
                            return true;
                        }
                    case DIR_DOWNRIGHT:
                        {
                            if(pShiftX){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nGridSpeed;
                                }else{
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nGridSpeed;
                                }
                            }
                            if(pShiftY){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nGridSpeed;
                                }else{
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nGridSpeed;
                                }
                            }
                            return true;
                        }
                    case DIR_DOWN:
                        {
                            if(pShiftX){ *pShiftX = 0; }
                            if(pShiftY){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nGridSpeed;
                                }else{
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nGridSpeed;
                                }
                            }
                            return true;
                        }
                    case DIR_DOWNLEFT:
                        {
                            if(pShiftX){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nGridSpeed;
                                }else{
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nGridSpeed;
                                }
                            }
                            if(pShiftY){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nGridSpeed;
                                }else{
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nGridSpeed;
                                }
                            }
                            return true;
                        }
                    case DIR_LEFT:
                        {
                            if(pShiftX){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nGridSpeed;
                                }else{
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nGridSpeed;
                                }
                            }
                            if(pShiftY){ *pShiftY = 0; }
                            return true;
                        }
                    case DIR_UPLEFT:
                        {
                            if(pShiftX){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nGridSpeed;
                                }else{
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nGridSpeed;
                                }
                            }
                            if(pShiftY){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextCell){
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nGridSpeed;
                                }else{
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nGridSpeed;
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
    int nMaxStep = 1;
    switch(nMotion){
        case MOTION_WALK      : nMaxStep = 1; break;
        case MOTION_RUN       : nMaxStep = 2; break;
        case MOTION_HORSEWALK : nMaxStep = 1; break;
        case MOTION_HORSERUN  : nMaxStep = 3; break;
        default               : return false;
    }

    if(!(true
                && nSpeed > 0
                && m_ProcessRun
                && m_ProcessRun->CanMove(false, nX0, nY0)
                && m_ProcessRun->CanMove(false, nX1, nY1))){ return false; }

    switch(LDistance2(nX0, nY0, nX1, nY1)){
        case 0:
            {
                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_WARNING, "Invalid argument: (%d, %d) -> (%d, %d)", nX0, nY0, nX1, nY1);
                return false;
            }
        case 1:
        case 2:
        default:
            {
                // we check both creatures and grids
                // creature check : will prefer a path without creatures stand on the way
                //     gird check : will fail if can't pass through valid grids
                ClientPathFinder stPathFinder(true, true);
                if(true
                        && stPathFinder.Search(nX0, nY0, nX1, nY1)
                        && stPathFinder.GetSolutionStart()){

                    // if path find succeed
                    // we retrive all nodes for the path

                    std::vector<PathFind::PathNode> stPathNodeV(1, {nX0, nY0});
                    while(auto pNode = stPathFinder.GetSolutionNext()){
                        stPathNodeV.emplace_back(pNode->X(), pNode->Y());
                    }

                    static const int nDirV[][3] = {
                        {DIR_UPLEFT,   DIR_UP,   DIR_UPRIGHT  },
                        {DIR_LEFT,     DIR_NONE, DIR_RIGHT    },
                        {DIR_DOWNLEFT, DIR_DOWN, DIR_DOWNRIGHT}};

                    while(stPathNodeV.size() > 1){
                        auto nReachNode = PathFind::MaxReachNode(&(stPathNodeV[0]), stPathNodeV.size(), (size_t)(nMaxStep));
                        if(nReachNode > 0){
                            auto &rstNode0 = stPathNodeV[0];
                            auto &rstNode1 = stPathNodeV[nReachNode];

                            int nSDX = 1 + (rstNode1.X > rstNode0.X) - (rstNode1.X < rstNode0.X);
                            int nSDY = 1 + (rstNode1.Y > rstNode0.Y) - (rstNode1.Y < rstNode0.Y);

                            auto nCurrMotion = MOTION_NONE;
                            switch(nReachNode){
                                case 1:
                                    {
                                        switch(nMotion){
                                            case MOTION_WALK      : nCurrMotion = MOTION_WALK     ; break;
                                            case MOTION_RUN       : nCurrMotion = MOTION_WALK     ; break;
                                            case MOTION_HORSEWALK : nCurrMotion = MOTION_HORSEWALK; break;
                                            case MOTION_HORSERUN  : nCurrMotion = MOTION_HORSEWALK; break;
                                            default               : return false;
                                        }
                                        break;
                                    }
                                case 2:
                                    {
                                        nCurrMotion = MOTION_RUN;
                                        break;
                                    }
                                case 3:
                                    {
                                        nCurrMotion = MOTION_HORSERUN;
                                        break;
                                    }
                                default:
                                    {
                                        return false;
                                    }
                            }

                            // 1. push current motion node to motion queue
                            m_MotionQueue.emplace_back(
                                    nCurrMotion,
                                    nDirV[nSDY][nSDX],
                                    nSpeed,
                                    rstNode0.X,
                                    rstNode0.Y,
                                    rstNode1.X,
                                    rstNode1.Y);

                            // 2. remove path node from stPathNodeV
                            //    should be more efficient
                            stPathNodeV.erase(stPathNodeV.begin(), stPathNodeV.begin() + nReachNode);
                        }else{
                            extern Log *g_Log;
                            g_Log->AddLog(LOGTYPE_WARNING, "Invalid argument: (%d, %d) -> (%d, %d)", nX0, nY0, nX1, nY1);
                            return false;
                        }
                    }

                    // path node parsing done
                    return MotionQueueValid();

                }else{
                    // can't find a valid path
                    // no pass exists only passing the valid grids
                    return false;
                }
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

int Creature::UpdateHP(int nHP, int nHPMax)
{
    m_HP    = nHP;
    m_HPMax = nHPMax;

    return true;
}
