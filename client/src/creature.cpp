/*
 * =====================================================================================
 *
 *       Filename: creature.cpp
 *        Created: 08/31/2015 10:45:48 PM
 *  Last Modified: 03/28/2017 18:06:13
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
#include <algorithm>
#include <tinyxml2.h>
#include <SDL2/SDL.h>

#include "creature.hpp"
#include "sysconst.hpp"

Creature::Creature(uint32_t nUID, uint32_t nAddTime)
    : m_UID(nUID)
    , m_AddTime(nAddTime)
    , m_X(0)
    , m_Y(0)
    , m_MapID(0)
    , m_LogicDelay(120.0)
    , m_FrameDelay(180.0)
    , m_LogicUpdateTime(0.0)
    , m_FrameUpdateTime(0.0)
    , m_Frame(0)
    , m_FrameCountInNextCell(5)
    , m_Speed(0)
    , m_Action(0)
    , m_Direction(0)
{}

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
    switch(m_Action){
        case ACTION_WALK:
            {
                auto nFrameCount = (int)(FrameCount());
                switch(m_Direction){
                    case DIR_UP:
                        {
                            if(pShiftX){ *pShiftX = 0; }
                            if(pShiftY){
                                if(m_Frame < nFrameCount - m_FrameCountInNextCell){
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
                                if(m_Frame < nFrameCount - m_FrameCountInNextCell){
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (m_Frame + 1) * Speed());
                                }else{
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_Frame + 1)) * Speed());
                                }
                            }
                            if(pShiftY){
                                if(m_Frame < nFrameCount - m_FrameCountInNextCell){
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
                                if(m_Frame < nFrameCount - m_FrameCountInNextCell){
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
                                if(m_Frame < nFrameCount - m_FrameCountInNextCell){
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (m_Frame + 1) * Speed());
                                }else{
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_Frame + 1)) * Speed());
                                }
                            }
                            if(pShiftY){
                                if(m_Frame < nFrameCount - m_FrameCountInNextCell){
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
                                if(m_Frame < nFrameCount - m_FrameCountInNextCell){
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
                                if(m_Frame < nFrameCount - m_FrameCountInNextCell){
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (m_Frame + 1) * Speed());
                                }else{
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_Frame + 1)) * Speed());
                                }
                            }
                            if(pShiftY){
                                if(m_Frame < nFrameCount - m_FrameCountInNextCell){
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
                                if(m_Frame < nFrameCount - m_FrameCountInNextCell){
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
                                if(m_Frame < nFrameCount - m_FrameCountInNextCell){
                                    *pShiftX = -1 * ((SYS_MAPGRIDXP / nFrameCount) * (m_Frame + 1) * Speed());
                                }else{
                                    *pShiftX = ((SYS_MAPGRIDXP / nFrameCount) * (nFrameCount - (m_Frame + 1)) * Speed());
                                }
                            }
                            if(pShiftY){
                                if(m_Frame < nFrameCount - m_FrameCountInNextCell){
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
