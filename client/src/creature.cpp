/*
 * =====================================================================================
 *
 *       Filename: creature.cpp
 *        Created: 08/31/2015 10:45:48 PM
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
#include "fflerror.hpp"
#include "mathf.hpp"
#include "creature.hpp"
#include "sysconst.hpp"
#include "ascendstr.hpp"
#include "processrun.hpp"
#include "protocoldef.hpp"
#include "dbcomrecord.hpp"
#include "clientpathfinder.hpp"

extern Log *g_Log;

std::tuple<int, int> Creature::getShift() const
{
    int shiftX = 0;
    int shiftY = 0;

    switch(m_currMotion.motion){
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
                return {0, 0};
            }
    }

    const auto currStepCount = CurrStep();
    switch(currStepCount){
        case 1:
        case 2:
        case 3:
            {
                break;
            }
        default:
            {
                return {0, 0};
            }
    }

    // we only allow frameCount = 6
    // for other frameCount need to *manually* permitted here

    switch(const auto frameCount = motionFrameCount(m_currMotion.motion, m_currMotion.direction)){
        case 6:
            {
                const auto frameCountInNextGrid = ((m_currMotion.direction == DIR_UPLEFT) ? 2 : 5);
                switch(m_currMotion.direction){
                    case DIR_UP:
                        {
                            shiftX = 0;
                            if(m_currMotion.frame < frameCount - frameCountInNextGrid){
                                shiftY = -1 * ((SYS_MAPGRIDYP / frameCount) * (m_currMotion.frame + 1)) * currStepCount;
                            }else{
                                shiftY = ((SYS_MAPGRIDYP / frameCount) * (frameCount - (m_currMotion.frame + 1))) * currStepCount;
                            }
                            return {shiftX, shiftY};
                        }
                    case DIR_UPRIGHT:
                        {
                            if(m_currMotion.frame < frameCount - frameCountInNextGrid){
                                shiftX = ((SYS_MAPGRIDXP / frameCount) * (m_currMotion.frame + 1)) * currStepCount;
                            }else{
                                shiftX = -1 * ((SYS_MAPGRIDXP / frameCount) * (frameCount - (m_currMotion.frame + 1))) * currStepCount;
                            }
                            if(m_currMotion.frame < frameCount - frameCountInNextGrid){
                                shiftY = -1 * ((SYS_MAPGRIDYP / frameCount) * (m_currMotion.frame + 1)) * currStepCount;
                            }else{
                                shiftY = ((SYS_MAPGRIDYP / frameCount) * (frameCount - (m_currMotion.frame + 1))) * currStepCount;
                            }
                            return {shiftX, shiftY};
                        }
                    case DIR_RIGHT:
                        {
                            if(m_currMotion.frame < frameCount - frameCountInNextGrid){
                                shiftX = ((SYS_MAPGRIDXP / frameCount) * (m_currMotion.frame + 1)) * currStepCount;
                            }else{
                                shiftX = -1 * ((SYS_MAPGRIDXP / frameCount) * (frameCount - (m_currMotion.frame + 1))) * currStepCount;
                            }
                            shiftY = 0;
                            return {shiftX, shiftY};
                        }
                    case DIR_DOWNRIGHT:
                        {
                            if(m_currMotion.frame < frameCount - frameCountInNextGrid){
                                shiftX = ((SYS_MAPGRIDXP / frameCount) * (m_currMotion.frame + 1)) * currStepCount;
                            }else{
                                shiftX = -1 * ((SYS_MAPGRIDXP / frameCount) * (frameCount - (m_currMotion.frame + 1))) * currStepCount;
                            }
                            if(m_currMotion.frame < frameCount - frameCountInNextGrid){
                                shiftY = ((SYS_MAPGRIDYP / frameCount) * (m_currMotion.frame + 1)) * currStepCount;
                            }else{
                                shiftY = -1 * ((SYS_MAPGRIDYP / frameCount) * (frameCount - (m_currMotion.frame + 1))) * currStepCount;
                            }
                            return {shiftX, shiftY};
                        }
                    case DIR_DOWN:
                        {
                            shiftX = 0;
                            if(m_currMotion.frame < frameCount - frameCountInNextGrid){
                                shiftY = ((SYS_MAPGRIDYP / frameCount) * (m_currMotion.frame + 1)) * currStepCount;
                            }else{
                                shiftY = -1 * ((SYS_MAPGRIDYP / frameCount) * (frameCount - (m_currMotion.frame + 1))) * currStepCount;
                            }
                            return {shiftX, shiftY};
                        }
                    case DIR_DOWNLEFT:
                        {
                            if(m_currMotion.frame < frameCount - frameCountInNextGrid){
                                shiftX = -1 * ((SYS_MAPGRIDXP / frameCount) * (m_currMotion.frame + 1)) * currStepCount;
                            }else{
                                shiftX = ((SYS_MAPGRIDXP / frameCount) * (frameCount - (m_currMotion.frame + 1))) * currStepCount;
                            }
                            if(m_currMotion.frame < frameCount - frameCountInNextGrid){
                                shiftY = ((SYS_MAPGRIDYP / frameCount) * (m_currMotion.frame + 1)) * currStepCount;
                            }else{
                                shiftY = -1 * ((SYS_MAPGRIDYP / frameCount) * (frameCount - (m_currMotion.frame + 1))) * currStepCount;
                            }
                            return {shiftX, shiftY};
                        }
                    case DIR_LEFT:
                        {
                            if(m_currMotion.frame < frameCount - frameCountInNextGrid){
                                shiftX = -1 * ((SYS_MAPGRIDXP / frameCount) * (m_currMotion.frame + 1)) * currStepCount;
                            }else{
                                shiftX = ((SYS_MAPGRIDXP / frameCount) * (frameCount - (m_currMotion.frame + 1))) * currStepCount;
                            }
                            shiftY = 0;
                            return {shiftX, shiftY};
                        }
                    case DIR_UPLEFT:
                        {
                            if(m_currMotion.frame < frameCount - frameCountInNextGrid){
                                shiftX = -1 * ((SYS_MAPGRIDXP / frameCount) * (m_currMotion.frame + 1)) * currStepCount;
                            }else{
                                shiftX = ((SYS_MAPGRIDXP / frameCount) * (frameCount - (m_currMotion.frame + 1))) * currStepCount;
                            }
                            if(m_currMotion.frame < frameCount - frameCountInNextGrid){
                                shiftY = -1 * ((SYS_MAPGRIDYP / frameCount) * (m_currMotion.frame + 1)) * currStepCount;
                            }else{
                                shiftY = ((SYS_MAPGRIDYP / frameCount) * (frameCount - (m_currMotion.frame + 1))) * currStepCount;
                            }
                            return {shiftX, shiftY};
                        }
                    default:
                        {
                            return {0, 0};
                        }
                }
            }
        default:
            {
                return {0, 0};
            }
    }
}

bool Creature::MoveNextMotion()
{
    if(!m_forceMotionQueue.empty()){
        m_currMotion = m_forceMotionQueue.front();
        m_forceMotionQueue.pop_front();
        return true;
    }

    if(m_motionQueue.empty()){
        // reset creature to idle state
        // using last direction, speed, location and frame as 0
        m_currMotion = MakeMotionIdle();
        return true;
    }

    // OK we have pending motions
    // check the motion queue and pick the head if valid
    if(MotionQueueValid()){
        m_currMotion = m_motionQueue.front();
        m_motionQueue.pop_front();
        return true;
    }

    // invalid motion queue
    // clear all pending motions and reset creature to idle state

    g_Log->addLog(LOGTYPE_WARNING, "Motion queue invalid, reset idle state");
    m_motionQueue.clear();
    m_currMotion = MakeMotionIdle();
    return false;
}

bool Creature::AdvanceMotionFrame(int nDFrame)
{
    const auto frameCount = motionFrameCount(m_currMotion.motion, m_currMotion.direction);
    if(frameCount > 0){
        m_currMotion.frame = (m_currMotion.frame + nDFrame    ) % frameCount;
        m_currMotion.frame = (m_currMotion.frame + frameCount) % frameCount;
        return true;
    }else{
        m_currMotion.print();
        return false;
    }
}

std::vector<PathFind::PathNode> Creature::ParseMovePath(int nX0, int nY0, int nX1, int nY1, bool bCheckGround, int nCheckCreature)
{
    condcheck(m_ProcessRun);
    if(!m_ProcessRun->CanMove(true, 0, nX0, nY0)){
        return {};
    }

    auto nMaxStep = MaxStep();
    switch(auto nLDistance2 = mathf::LDistance2(nX0, nY0, nX1, nY1)){
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
                    if(m_ProcessRun->CanMove(true, 0, nX1, nY1)){
                        // we ignore nCheckCreature
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

                    if(m_ProcessRun->CanMove(true, 0, nX0, nY0, nX1, nY1)){
                        switch(nCheckCreature){
                            case 0:
                            case 1:
                                {
                                    // not check creatures
                                    // and we can reach dst in one-hop
                                    return {{nX0, nY0}, {nX1, nY1}};
                                }
                            case 2:
                                {
                                    int nDX = (nX1 > nX0) - (nX1 < nX0);
                                    int nDY = (nY1 > nY0) - (nY1 < nY0);

                                    // we need to avoid check the first node
                                    // since it will fail by occupation of itself

                                    if(m_ProcessRun->CanMove(true, 2, nX0 + nDX, nY0 + nDY, nX1, nY1)){
                                        // we are checking the creatures
                                        // and no creaturs standing on the one-hop path
                                        return {{nX0, nY0}, {nX1, nY1}};
                                    }

                                    // can reach in one hop but there is creatures on the path
                                    // and we can't ignore the creatures
                                    // leave it to the complex path solver
                                    break;
                                }
                            default:
                                {
                                    throw fflerror("invalid CheckCreature provided: %d, should be (0, 1, 2)", nCheckCreature);
                                }
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

                ClientPathFinder stPathFinder(bCheckGround, nCheckCreature, nMaxStep);
                if(stPathFinder.Search(nX0, nY0, nX1, nY1)){
                    return stPathFinder.GetPathNode();
                }else{
                    // we can't find a path
                    // return the starting point only
                    return {{nX0, nY0}};
                }
            }
    }
}

void Creature::UpdateAttachMagic(double fUpdateTime)
{
    for(size_t nIndex = 0; nIndex < m_attachMagicList.size();){
        m_attachMagicList[nIndex]->Update(fUpdateTime);
        if(m_attachMagicList[nIndex]->Done()){
            std::swap(m_attachMagicList[nIndex], m_attachMagicList.back());
            m_attachMagicList.pop_back();
        }else{
            nIndex++;
        }
    }
}

bool Creature::UpdateMotion(bool bLooped)
{
    const auto frameCount = motionFrameCount(m_currMotion.motion, m_currMotion.direction);
    if(frameCount >= 0){
        if(bLooped || (m_currMotion.frame < (frameCount - 1))){
            return AdvanceMotionFrame(1);
        }else{
            return MoveNextMotion();
        }
    }
    return false;
}

bool Creature::MotionQueueValid()
{
    if(m_motionQueue.empty()){
        return true;
    }

    const MotionNode *lastMotionPtr = &m_currMotion;
    for(const auto &motion: m_motionQueue){
        if(motionValid(motion)
                && (lastMotionPtr->endX == motion.x)
                && (lastMotionPtr->endY == motion.y)){
            lastMotionPtr = &motion;
        }
        else{
            g_Log->addLog(LOGTYPE_WARNING, "Invalid motion queue:");
            m_currMotion.print();
            for(auto &node: m_motionQueue){
                node.print();
            }
            return false;
        }
    }
    return true;
}

int Creature::UpdateHP(int nHP, int nHPMax)
{
    if(auto nDiffHP = nHP - HP()){
        if(HP() > 0){
            if(m_ProcessRun){

                // TODO
                // when possible add a new function
                // bool Creature::GfxWindow(int *, int *, int *, int *)

                int nX = X() * SYS_MAPGRIDXP + SYS_MAPGRIDXP / 2;
                int nY = Y() * SYS_MAPGRIDYP - SYS_MAPGRIDYP * 1;
                m_ProcessRun->AddAscendStr(ASCENDSTR_NUM0, nDiffHP, nX, nY);
            }
        }
    }

    m_HP    = nHP;
    m_HPMax = nHPMax;

    return true;
}

bool Creature::StayDead()
{
    return false
        || m_currMotion.motion == MOTION_DIE
        || m_currMotion.motion == MOTION_MON_DIE;
}

bool Creature::StayIdle()
{
    return m_forceMotionQueue.empty() && m_motionQueue.empty();
}

bool Creature::DeadFadeOut()
{
    switch(m_currMotion.motion){
        case MOTION_DIE:
        case MOTION_MON_DIE:
            {
                if(!m_currMotion.fadeOut){
                    m_currMotion.fadeOut = 1;
                }
                return true;
            }
        default:
            {
                break;
            }
    }
    return false;
}

bool Creature::Alive()
{
    if(!motionValid(m_currMotion)){
        throw std::runtime_error(str_ffl() + "; Invalid motion detected");
    }

    switch(m_currMotion.motion){
        case MOTION_DIE:
        case MOTION_MON_DIE:
            {
                return false;
            }
        default:
            {
                return true;
            }
    }
}

bool Creature::Active()
{
    if(!motionValid(m_currMotion)){
        throw std::runtime_error(str_ffl() + ": Invalid motion detected");
    }

    switch(m_currMotion.motion){
        case MOTION_DIE:
        case MOTION_MON_DIE:
            {
                if(auto frameCount = motionFrameCount(m_currMotion.motion, m_currMotion.direction); frameCount > 0){
                    return m_currMotion.frame < (frameCount - 1);
                }
                throw std::runtime_error(str_ffl() + ": Invalid motion detected");
            }
        default:
            {
                return true;
            }
    }
}

bool Creature::Visible()
{
    if(!motionValid(m_currMotion)){
        throw std::runtime_error(str_ffl() + ": Invalid motion detected");
    }

    switch(m_currMotion.motion){
        case MOTION_DIE:
        case MOTION_MON_DIE:
            {
                if(auto frameCount = motionFrameCount(m_currMotion.motion, m_currMotion.direction); frameCount > 0){
                    return (m_currMotion.frame < (frameCount - 1)) || (m_currMotion.fadeOut < 255);
                }
                throw std::runtime_error(str_ffl() + ": Invalid motion detected");
            }
        default:
            {
                return true;
            }
    }
}

MotionNode Creature::MakeMotionIdle() const
{
    // I can put this into each derived class
    // but do it in base class if logic structure highly likely

    int nMotion = -1;
    switch(type()){
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

    return {nMotion, 0, m_currMotion.direction, m_currMotion.speed, m_currMotion.endX, m_currMotion.endY};
}

bool Creature::AddAttachMagic(int nMagicID, int nMagicParam, int nMagicStage)
{
    // check if type is u8"附着"
    // otherwise we shouldn't use AttachMagic

    if(auto &rstMR = DBCOM_MAGICRECORD(nMagicID)){
        for(size_t nIndex = 0;; ++nIndex){
            if(auto &rstGE = rstMR.GetGfxEntry(nIndex)){
                if(rstGE.Stage == nMagicStage){
                    switch(rstGE.Type){
                        case EGT_BOUND:
                            {
                                m_attachMagicList.emplace_back(std::make_shared<AttachMagic>(nMagicID, nMagicParam, nMagicStage));
                                return true;
                            }
                        default:
                            {
                                break;
                            }
                    }
                }
            }else{
                // scan all GE and done
                // can't find the stage, stop here
                break;
            }
        }
    }
    return false;
}

double Creature::CurrMotionDelay() const
{
    auto nSpeed = currMotion().speed;
    nSpeed = (std::max<int>)(SYS_MINSPEED, nSpeed);
    nSpeed = (std::min<int>)(SYS_MAXSPEED, nSpeed);

    return (1000.0 / SYS_DEFFPS) * (100.0 / nSpeed);
}

void Creature::QuerySelf()
{
    m_LastQuerySelf = SDL_GetTicks();
    m_ProcessRun->QueryCORecord(UID());
}

std::deque<MotionNode> Creature::MakeMotionWalkQueue(int startX, int startY, int endX, int endY, int speed)
{
    if(mathf::LDistance2(startX, startY, endX, endY) == 0){
        return {};
    }

    const auto pathNodes = ParseMovePath(startX, startY, endX, endY, true, 1);
    switch(pathNodes.size()){
        case 0:
        case 1:
            {
                // 0 means error
                // 1 means can't find a path here since we know LDistance2 != 0
                throw fflerror("Can't find a path: (%d, %d) -> (%d, %d)", startX, startY, endX, endY);
            }
        default:
            {
                // we get a path
                // make a motion list for the path

                std::deque<MotionNode> motionQueue;
                for(size_t nIndex = 1; nIndex < pathNodes.size(); ++nIndex){
                    auto nX0 = pathNodes[nIndex - 1].X;
                    auto nY0 = pathNodes[nIndex - 1].Y;
                    auto nX1 = pathNodes[nIndex    ].X;
                    auto nY1 = pathNodes[nIndex    ].Y;

                    if(const auto motionNode = MakeMotionWalk(nX0, nY0, nX1, nY1, speed)){
                        motionQueue.push_back(motionNode);
                    }
                    else{
                        throw fflerror("Can't make a motioni node: (%d, %d) -> (%d, %d)", nX0, nY0, nX1, nY1);
                    }
                }
                return motionQueue;
            }
    }
}
