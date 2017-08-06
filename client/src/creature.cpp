/*
 * =====================================================================================
 *
 *       Filename: creature.cpp
 *        Created: 08/31/2015 10:45:48 PM
 *  Last Modified: 08/06/2017 01:46:02
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
#include "ascendstr.hpp"
#include "processrun.hpp"
#include "protocoldef.hpp"
#include "dbcomrecord.hpp"
#include "clientpathfinder.hpp"

bool Creature::EstimateShift(int *pShiftX, int *pShiftY)
{
    switch(m_CurrMotion.Motion){
        case MOTION_WALK:           // human
        case MOTION_RUN:            // human
        case MOTION_ONHORSEWALK:    // human
        case MOTION_ONHORSERUN:     // human
        case MOTION_MON_WALK:       // monster
            {
                break;
            }
        default:
            {
                return false;
            }
    }

    auto nCurrStep = CurrStep();
    switch(nCurrStep){
        case 1:
        case 2:
        case 3:
            {
                break;
            }
        default:
            {
                return false;
            }
    }

    // currently we only allow frameCount = 6
    // for other frameCount need to manually permitted here

    switch(auto nFrameCount = MotionFrameCount(m_CurrMotion.Motion, m_CurrMotion.Direction)){
        case 6:
            {
                auto nFrameCountInNextGrid = ((m_CurrMotion.Direction == DIR_UPLEFT) ? 2 : 5);
                switch(m_CurrMotion.Direction){
                    case DIR_UP:
                        {
                            if(pShiftX){ *pShiftX = 0; }
                            if(pShiftY){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextGrid){
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nCurrStep;
                                }else{
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nCurrStep;
                                }
                            }
                            return true;
                        }
                    case DIR_UPRIGHT:
                        {
                            if(pShiftX){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextGrid){
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nCurrStep;
                                }else{
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nCurrStep;
                                }
                            }
                            if(pShiftY){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextGrid){
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nCurrStep;
                                }else{
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nCurrStep;
                                }
                            }
                            return true;
                        }
                    case DIR_RIGHT:
                        {
                            if(pShiftX){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextGrid){
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nCurrStep;
                                }else{
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nCurrStep;
                                }
                            }
                            if(pShiftY){ *pShiftY = 0; }
                            return true;
                        }
                    case DIR_DOWNRIGHT:
                        {
                            if(pShiftX){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextGrid){
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nCurrStep;
                                }else{
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nCurrStep;
                                }
                            }
                            if(pShiftY){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextGrid){
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nCurrStep;
                                }else{
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nCurrStep;
                                }
                            }
                            return true;
                        }
                    case DIR_DOWN:
                        {
                            if(pShiftX){ *pShiftX = 0; }
                            if(pShiftY){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextGrid){
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nCurrStep;
                                }else{
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nCurrStep;
                                }
                            }
                            return true;
                        }
                    case DIR_DOWNLEFT:
                        {
                            if(pShiftX){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextGrid){
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nCurrStep;
                                }else{
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nCurrStep;
                                }
                            }
                            if(pShiftY){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextGrid){
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nCurrStep;
                                }else{
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nCurrStep;
                                }
                            }
                            return true;
                        }
                    case DIR_LEFT:
                        {
                            if(pShiftX){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextGrid){
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nCurrStep;
                                }else{
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nCurrStep;
                                }
                            }
                            if(pShiftY){ *pShiftY = 0; }
                            return true;
                        }
                    case DIR_UPLEFT:
                        {
                            if(pShiftX){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextGrid){
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nCurrStep;
                                }else{
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nCurrStep;
                                }
                            }
                            if(pShiftY){
                                if(m_CurrMotion.Frame < nFrameCount - nFrameCountInNextGrid){
                                    *pShiftY = -1 * ((SYS_MAPGRIDYP / nFrameCount) * (m_CurrMotion.Frame + 1)) * nCurrStep;
                                }else{
                                    *pShiftY = ((SYS_MAPGRIDYP / nFrameCount) * (nFrameCount - (m_CurrMotion.Frame + 1))) * nCurrStep;
                                }
                            }
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
                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_WARNING, "Current motion is not valid: frameCount = %d", nFrameCount);

                m_CurrMotion.Print();
                return false;
            }
    }
}

bool Creature::MoveNextMotion()
{
    if(m_MotionQueue.empty()){

        // reset creature to idle state
        // using last direction, speed, location and frame as 0
        m_CurrMotion = MakeMotionIdle();
        return true;
    }

    // OK we have pending motions
    // check the motion queue and pick the head if valid

    if(MotionQueueValid()){
        m_CurrMotion = m_MotionQueue.front();
        m_MotionQueue.pop_front();
        return true;
    }

    // invalid motion queue
    // clear all pending motions and reset creature to idle state

    // 1. reset creature to idle state
    m_CurrMotion = MakeMotionIdle();

    // 2. print the motion queue
    extern Log *g_Log;
    g_Log->AddLog(LOGTYPE_WARNING, "Invalid motion queue:");

    m_CurrMotion.Print();
    for(auto &rstMotion: m_MotionQueue){
        rstMotion.Print();
    }

    extern Log *g_Log;
    g_Log->AddLog(LOGTYPE_WARNING, "Current motion is not valid");

    // 3. clear the motion queue
    m_MotionQueue.clear();
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

                                int nDX = (nX1 > nX0) - (nX1 < nX0);
                                int nDY = (nY1 > nY0) - (nY1 < nY0);

                                // we need to avoid check the first node
                                // since it will fail by occupation of itself

                                if(m_ProcessRun->CanMove(true, nX0 + nDX, nY0 + nDY, nX1, nY1)){
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

bool Creature::UpdateEffect()
{
    for(auto pRecord = m_EffectQueue.begin(); pRecord != m_EffectQueue.end();){
        if(auto &rstER = DBCOM_MAGICRECORD(pRecord->Effect)){
            if(pRecord->Frame >= rstER.EffectFrameCount - 1){
                // one round of current effect finished
                if(rstER.EffectLoop){
                    pRecord->Frame = 0;
                    pRecord++;
                }else{
                    pRecord = m_EffectQueue.erase(pRecord);
                }
            }else{
                pRecord->Frame++;
                pRecord++;
            }
        }else{
            extern Log *g_Log;
            g_Log->AddLog(LOGTYPE_WARNING, "Invalid effect detected: %p", &(*pRecord));
            pRecord->Print();

            pRecord = m_EffectQueue.erase(pRecord);
        }
    }
    return true;
}

bool Creature::UpdateMotion(bool bLooped)
{
    auto nFrameCount = MotionFrameCount(m_CurrMotion.Motion, m_CurrMotion.Direction);
    if(nFrameCount >= 0){
        if(bLooped || (m_CurrMotion.Frame < (nFrameCount - 1))){
            return AdvanceMotionFrame(1);
        }else{
            return MoveNextMotion();
        }
    }
    return false;
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
    if(auto nDiffHP = nHP - HP()){
        if(m_ProcessRun){

            // TODO
            // when possible add a new function
            // bool Creature::GfxWindow(int *, int *, int *, int *)

            int nX = X() * SYS_MAPGRIDXP + SYS_MAPGRIDXP / 2;
            int nY = Y() * SYS_MAPGRIDYP - SYS_MAPGRIDYP * 1;
            m_ProcessRun->AddAscendStr(ASCENDSTR_NUM0, nDiffHP, nX, nY);
        }
    }

    m_HP    = nHP;
    m_HPMax = nHPMax;

    return true;
}

bool Creature::StayDead()
{
    return false
        || m_CurrMotion.Motion == MOTION_DIE
        || m_CurrMotion.Motion == MOTION_MON_DIE;
}

bool Creature::StayIdle()
{
    return m_MotionQueue.empty();
}

bool Creature::DeadFadeOut()
{
    switch(m_CurrMotion.Motion){
        case MOTION_DIE:
        case MOTION_MON_DIE:
            {
                m_CurrMotion.FadeOut = 1;
                return true;
            }
        default:
            {
                break;
            }
    }
    return false;
}

void Creature::Focus(int nFocusType, bool bFocus)
{
    if(nFocusType < (int)(m_FocusV.size())){
        m_FocusV[nFocusType] = bFocus;
    }
}

bool Creature::Focus(int nFocusType) const
{
    return (nFocusType < (int)(m_FocusV.size())) ? m_FocusV[nFocusType] : false;
}

bool Creature::Active()
{
    if(MotionValid(m_CurrMotion)){
        switch(m_CurrMotion.Motion){
            case MOTION_DIE:
            case MOTION_MON_DIE:
                {
                    auto nFrameCount = MotionFrameCount(m_CurrMotion.Motion, m_CurrMotion.Direction);
                    if(nFrameCount > 0){
                        if(true
                                && m_CurrMotion.Frame   == (nFrameCount - 1)
                                && m_CurrMotion.FadeOut == (255)){
                            return false;
                        }

                        return true;
                    }else{
                        extern Log *g_Log;
                        g_Log->AddLog(LOGTYPE_WARNING, "Current motion is not valid");
                        return false;
                    }
                }
            default:
                {
                    return true;
                }
        }
    }

    return false;
}

MotionNode Creature::MakeMotionIdle() const
{
    // I can put this into each derived class
    // but do it in base class if logic structure highly likely

    int nMotion = -1;
    switch(Type()){
        case CREATURE_PLAYER:
            {
                nMotion = MOTION_STAND;
                break;
            }
        case CREATURE_MONSTER:
            {
                nMotion = MOTION_MON_STAND;
                break;
            }
        default:
            {
                return {};
            }
    }

    return {nMotion, 0, m_CurrMotion.Direction, m_CurrMotion.Speed, m_CurrMotion.EndX, m_CurrMotion.EndY};
}
