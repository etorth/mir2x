/*
 * =====================================================================================
 *
 *       Filename: creature.cpp
 *        Created: 08/31/2015 10:45:48 PM
 *  Last Modified: 03/30/2017 14:06:44
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

#include "creature.hpp"
#include "sysconst.hpp"

Creature::Creature(uint32_t nUID, ProcessRun *pRun)
    : m_UID(nUID)
    , m_X(0)
    , m_Y(0)
    , m_MoveDstX(0)
    , m_MoveDstY(0)
    , m_ProcessRun(pRun)
    , m_LogicDelay(120.0)
    , m_FrameDelay(180.0)
    , m_LogicUpdateTime(0.0)
    , m_FrameUpdateTime(0.0)
    , m_Frame(0)
    , m_Speed(0)
    , m_NextSpeed(0)
    , m_Action(ACTION_NONE)
    , m_Direction(0)
{
    assert(m_ProcessRun);
}

Creature::~Creature()
{}

void Creature::EstimateLocation(int nDistance, int *pNextX, int *pNextY)
{
    static const int nDX[] = { 0, +1, +1, +1,  0, -1, -1, -1};
    static const int nDY[] = {-1, -1,  0, +1, +1, +1,  0, -1};

    if(pNextX){ *pNextX = m_X + nDistance * nDX[m_Direction]; }
    if(pNextY){ *pNextY = m_Y + nDistance * nDY[m_Direction]; }
}

void Creature::EstimatePixelShift(int *pShiftX, int *pShiftY)
{
    int nFrameCountInNextCell = (m_Direction == DIR_UPLEFT) ? 5 : 2;
    switch(m_Action){
        case ACTION_WALK:
            {
                auto nFrameCount = (int)(FrameCount());
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

void Creature::OnActionState(int nAction, int nDirection, int nSpeed, int nX, int nY)
{
    switch(nAction){
        case ACTION_WALK:
            {
                switch(m_Action){
                    case ACTION_WALK:
                        {
                            m_MoveDstX = nX;
                            m_MoveDstY = nY;

                            m_NextSpeed = nSpeed;
                            break;
                        }
                    case ACTION_STAND:
                        {
                            if((X() != nX) || (Y() != nY)){
                                m_MoveDstX = nX;
                                m_MoveDstY = nY;
                                
                                m_Frame  = 0;
                                m_Speed  = nSpeed;
                                m_Action = ACTION_WALK;
                            }else{
                                m_Speed = nSpeed;
                                m_Direction = nDirection;
                            }
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
                break;
            }
        case ACTION_STAND:
            {
                switch(m_Action){
                    case ACTION_WALK:
                        {
                            m_MoveDstX = nX;
                            m_MoveDstY = nY;
                            break;
                        }
                    case ACTION_STAND:
                        {
                            if((X() != nX) || (Y() != nY)){
                                m_MoveDstX = nX;
                                m_MoveDstY = nY;

                                m_Frame  = 0;
                                m_Speed  = nSpeed;
                                m_Action = ACTION_WALK;
                            }else{
                                m_Speed  = nSpeed;
                                m_Direction = nDirection;
                            }
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
            }
        default:
            {
                ResetAction(nAction);
                ResetSpeed(nSpeed);
                ResetLocation(nX, nY);
                ResetDirection(nDirection);

                break;
            }
    }
}

void Creature::OnCORecord(int nAction, int nDirection, int nSpeed, int nX, int nY)
{
    OnActionState(nAction, nDirection, nSpeed, nX, nY);
}
