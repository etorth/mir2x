/*
 * =====================================================================================
 *
 *       Filename: creature.cpp
 *        Created: 08/31/2015 10:45:48 PM
 *  Last Modified: 07/03/2017 13:08:38
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
                auto nFrameCount = MotionFrameCount(m_CurrMotion.Motion, m_CurrMotion.Direction);
                if(nFrameCount <= 0){
                    return false;
                }

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
    auto nFrameCount = MotionFrameCount(m_CurrMotion.Motion, m_CurrMotion.Direction);
    if(nFrameCount > 0){
        m_CurrMotion.Frame = (m_CurrMotion.Frame + nDFrame    ) % nFrameCount;
        m_CurrMotion.Frame = (m_CurrMotion.Frame + nFrameCount) % nFrameCount;
        return true;
    }else{
        m_CurrMotion.Print();
        return false;
    }
}

std::vector<PathFind::PathNode> Creature::ParseMovePath(int nX0, int nY0, int nX1, int nY1, bool bCheckGround, bool bCheckCreature)
{
    if(true
            && m_ProcessRun
            && m_ProcessRun->CanMove(false, nX0, nY0)){

        auto nMaxStep = MaxStep();
        switch(auto nLDistance2 = LDistance2(nX0, nY0, nX1, nY1)){
            case 0:
                {
                    return {{nX0, nY0}};
                }
            case 1:
            case 2:
                {
                    // dst is at one-hop distance
                    // so there couldn't be any middle grids blocking

                    if(bCheckGround){
                        if(m_ProcessRun->CanMove(false, nX1, nY1)){
                            // we ignore bCheckCreature
                            // because this always gives the best path
                            return {{nX0, nY0}, {nX1, nY1}};
                        }else{
                            // can't find a path to dst
                            // return the starting node, return empty means errors
                            return {{nX0, nY0}};
                        }
                    }else{
                        // won't check ground
                        // then directly return the unique path
                        return {{nX0, nY0}, {nX1, nY1}};
                    }
                }
            default:
                {
                    // 1. one hop distance
                    // 2. more complex distance

                    if(false
                            || nLDistance2 == nMaxStep * nMaxStep
                            || nLDistance2 == nMaxStep * nMaxStep * 2){

                        // one hop distance
                        // but not with distance = 1 or 2
                        // there could be middle grid blocking this hop

                        if(m_ProcessRun->CanMove(false, nX0, nY0, nX1, nY1)){
                            if(bCheckCreature){
                                if(m_ProcessRun->CanMove(true, nX0, nY0, nX1, nY1)){
                                    // we are checking the creatures
                                    // and no creaturs standing on the one-hop path
                                    return {{nX0, nY0}, {nX1, nY1}};
                                }

                                // can reach in one hop but there is creatures on the path
                                // and we can't ignore the creatures
                                // leave it to the complex path solver

                            }else{
                                // not check creatures
                                // and we can reach dst in one-hop
                                return {{nX0, nY0}, {nX1, nY1}};
                            }
                        }else{
                            // can't reach in one hop
                            // means there is middle grids blocking this path
                            // leave it to the complex path solver
                        }

                    }else{

                        // not one-hop distance 
                        // leave it to the complex path solver
                    }

                    // the complex path solver
                    // we can always use this solver only

                    ClientPathFinder stPathFinder(bCheckGround, bCheckCreature, nMaxStep);
                    if(true
                            && stPathFinder.Search(nX0, nY0, nX1, nY1)
                            && stPathFinder.GetSolutionStart()){

                        // if path find succeed
                        // we retrive all nodes for the path vector

                        std::vector<PathFind::PathNode> stPathNodeV(1, {nX0, nY0});
                        while(auto pNode = stPathFinder.GetSolutionNext()){
                            stPathNodeV.emplace_back(pNode->X(), pNode->Y());
                        }

                        return stPathNodeV;
                    }else{

                        // we can't find a path
                        // return the starting point only
                        return {{nX0, nY0}};
                    }
                }
        }
    }

    // invalid src point means error
    // invalid dst point should be accepted as a valid input
    return {};
}

bool Creature::UpdateGeneralMotion(bool bLooped)
{
    auto nFrameCount = MotionFrameCount(m_CurrMotion.Motion, m_CurrMotion.Direction);
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

bool Creature::StayDead()
{
    // need refine this function for different creatures
    return m_CurrMotion.Motion == MOTION_DIE;
}

bool Creature::DeadFadeOut()
{
    switch(m_CurrMotion.Motion){
        case MOTION_DIE:
            {
                m_CurrMotion.MotionParam = 1;
                return true;
            }
        default:
            {
                break;
            }
    }
    return false;
}
