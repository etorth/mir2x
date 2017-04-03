/*
 * =====================================================================================
 *
 *       Filename: creature.cpp
 *        Created: 08/31/2015 10:45:48 PM
 *  Last Modified: 04/03/2017 10:41:21
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
    , m_NextActionV()
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

void Creature::OnReportAction(int nAction, int nDirection, int nSpeed, int nX, int nY)
{
    // the first action node is the ``current action"
    // could be empty then current action by default is ACTION_STAND
    if(m_NextActionV.size() >= 1){
        m_NextActionV.erase(m_NextActionV.begin() + 1, m_NextActionV.end());
    }

    switch(nAction){
        case ACTION_STAND:
            {
                switch(LDistance2(nX, nY, X(), Y())){
                    case 0:
                        {
                            m_NextActionV.push_back({ACTION_STAND, nDirection, nSpeed, nX, nY});
                            break;
                        }
                    default:
                        {
                            m_NextActionV.push_back({ACTION_WALK, DIR_NONE, nSpeed, nX, nY});
                            m_NextActionV.push_back({ACTION_STAND, nDirection, nSpeed, nX, nY});
                            break;
                        }
                }
                break;
            }
        case ACTION_WALK:
            {
                // push ACTION_WALK even if (X(), Y()) == (nX, nY)
                // since the creature could be leaving and should be called back
                m_NextActionV.push_back({ACTION_WALK, DIR_NONE, nSpeed, nX, nY});
                break;
            }
        default:
            {
                break;
            }
    }
}

void Creature::OnReportState()
{}


void Creature::ReportBadAction()
{
#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
    {
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_INFO, "Wrong action for ID = %d, X = %4d, Y = %4d, Action = %d, Direction = %d, Speed = %d", X(), Y(), Action(), Direction(), Speed());
    }
#endif
}

void Creature::ReportBadActionNode(size_t nNode)
{
    if(nNode < m_NextActionV.size()){
#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
        {
            extern Log *g_Log;
            g_Log->AddLog(LOGTYPE_INFO, "Wrong action node for ID = %d, current : X = %4d, Y = %4d, Action = %d, Direction = %d, Speed = %d", X(), Y(), Action(), Direction(), Speed());
            g_Log->AddLog(LOGTYPE_INFO, "                                  next : X = %4d, Y = %4d, Action = %d, Direction = %d, Speed = %d", m_NextActionV[nNode].X, m_NextActionV[nNode].Y, m_NextActionV[nNode].Action, m_NextActionV[nNode].Direction, m_NextActionV[nNode].Speed);
        }
#endif
    }
}

void Creature::MoveNextFrame(int nDFrame)
{
    auto nFrameCount = (int)(FrameCount());
    if((nFrameCount > 0) && (std::abs(nDFrame) <= nFrameCount)){
        m_Frame = (m_Frame + nDFrame + nFrameCount) % nFrameCount;
    }else{
#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
        {
            extern Log *g_Log;
            g_Log->AddLog(LOGTYPE_FATAL, "Invalid arguments to MoveNextFrame(FrameCount = %d, DFrame = %d)", nFrameCount, nDFrame);
        }
#endif
    }
}

void Creature::OnStand()
{
    if(m_Action != ACTION_STAND){
#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
        {
            extern Log *g_Log;
            g_Log->AddLog(LOGTYPE_INFO, "shouldn't call this function");
        }
#endif
        return;
    }

    if(m_NextActionV.empty()){
        MoveNextFrame(1);
    }else{
        // when in stand action
        // we will immediately switch to other action if requested
        switch(m_NextActionV[0].Action){
            case ACTION_STAND:
                {
                    switch(LDistance2(X(), Y(), m_NextActionV[0].X, m_NextActionV[0].Y)){
                        case 0:
                            {
                                m_Frame     = 0;
                                m_Action    = ACTION_STAND;
                                m_Direction = m_NextActionV[0].Direction;
                                m_Speed     = m_NextActionV[0].Speed;

                                m_NextActionV.clear();
                                break;
                            }
                        default:
                            {
                                ReportBadActionNode(0);
                                MoveNextFrame(1);

                                m_NextActionV.clear();
                                break;
                            }
                    }
                    break;
                }
            case ACTION_WALK:
                {
                    switch(LDistance2(X(), Y(), m_NextActionV[0].X, m_NextActionV[0].Y)){
                        case 0:
                            {
                                MoveNextFrame(1);
                                m_NextActionV.erase(m_NextActionV.begin());
                                break;
                            }
                        default:
                            {
                                bool bFindPath = false;
                                ClientPathFinder stPathFinder(false);
                                if(stPathFinder.Search(X(), Y(), m_NextActionV[0].X, m_NextActionV[0].Y)){
                                    if(stPathFinder.GetSolutionStart()){
                                        if(auto pNode1 = stPathFinder.GetSolutionNext()){
                                            int nDX = pNode1->X() - X() + 1;
                                            int nDY = pNode1->Y() - Y() + 1;

                                            static const int nDirV[][3] = {
                                                {DIR_UPLEFT,   DIR_UP,   DIR_UPRIGHT  },
                                                {DIR_LEFT,     DIR_NONE, DIR_RIGHT    },
                                                {DIR_DOWNLEFT, DIR_DOWN, DIR_DOWNRIGHT}};

                                            bFindPath   = true;
                                            m_Frame     = 0;
                                            m_Action    = ACTION_WALK;
                                            m_Direction = nDirV[nDY][nDX];
                                            m_Speed     = m_NextActionV[0].Speed;
                                        }
                                    }
                                }

                                if(bFindPath){
                                }else{
                                    ReportBadActionNode(0);
                                    m_NextActionV.clear();
                                    MoveNextFrame(1);
                                }
                                break;
                            }
                    }
                    break;
                }
            default:
                {
                    MoveNextFrame(1);
                    m_NextActionV.clear();
                    break;
                }
        }
    }
}

void Creature::OnWalk()
{
    if(m_Action != ACTION_WALK){
#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
        {
            extern Log *g_Log;
            g_Log->AddLog(LOGTYPE_INFO, "shouldn't call this function");
        }
#endif
        return;
    }

    auto nFrameCount = (int)(FrameCount());
    if(m_Frame == (nFrameCount - (((m_Direction == DIR_UPLEFT) ? 2 : 5) + 1))){
        int nEstimatedX = 0;
        int nEstimatedY = 0;
        EstimateLocation(Speed(), &nEstimatedX, &nEstimatedY);
        if(m_ProcessRun->CanMove(true, nEstimatedX, nEstimatedY)){
            m_X = nEstimatedX;
            m_Y = nEstimatedY;
            MoveNextFrame(1);
        }else if(m_ProcessRun->CanMove(false, nEstimatedX, nEstimatedY)){
            // move-able but some one is on the way
            // can't just stay here and wait, since it gives dead-lock
        }else{
            ReportBadAction();
            m_NextActionV.clear();
            m_Frame  = 0;
            m_Action = ACTION_STAND;
        }
    }else if(m_Frame == ((int)(FrameCount()) - 1)){
        switch(m_NextActionV.size()){
            case 0:
            case 1:
                {
                    m_NextActionV.clear();
                    m_Frame  = 0;
                    m_Action = ACTION_STAND;
                    break;
                }
            default:
                {
                    m_NextActionV.erase(m_NextActionV.begin());
                    while(true){
                        if(m_NextActionV.empty()){
                            m_Frame  = 0;
                            m_Action = ACTION_STAND;
                            break;
                        }

                        bool bDone = false;
                        switch(m_NextActionV[0].Action){
                            case ACTION_STAND:
                                {
                                    m_Frame     = 0;
                                    m_Action    = ACTION_STAND;
                                    m_Direction = m_NextActionV[0].Direction;
                                    m_Speed     = m_NextActionV[0].Speed;

                                    m_NextActionV.clear();
                                    bDone = true;
                                    break;
                                }
                            case ACTION_WALK:
                                {
                                    switch(LDistance2(m_NextActionV[0].X, m_NextActionV[0].Y, X(), Y())){
                                        case 0:
                                            {
                                                m_NextActionV.erase(m_NextActionV.begin());
                                                bDone = false;
                                                break;
                                            }
                                        case 1:
                                        case 2:
                                        default:
                                            {
                                                bool bFindPath = false;
                                                ClientPathFinder stPathFinder(false);
                                                if(stPathFinder.Search(X(), Y(), m_NextActionV[0].X, m_NextActionV[0].Y)){
                                                    if(stPathFinder.GetSolutionStart()){
                                                        if(auto pNode1 = stPathFinder.GetSolutionNext()){
                                                            int nDX = pNode1->X() - X() + 1;
                                                            int nDY = pNode1->Y() - Y() + 1;

                                                            static const int nDirV[][3] = {
                                                                {DIR_UPLEFT,   DIR_UP,   DIR_UPRIGHT  },
                                                                {DIR_LEFT,     DIR_NONE, DIR_RIGHT    },
                                                                {DIR_DOWNLEFT, DIR_DOWN, DIR_DOWNRIGHT}};

                                                            bFindPath   = true;
                                                            m_Frame     = 0;
                                                            m_Action    = ACTION_WALK;
                                                            m_Direction = nDirV[nDY][nDX];
                                                            m_Speed     = m_NextActionV[0].Speed;

                                                        }
                                                    }
                                                }

                                                bDone = true;
                                                if(bFindPath){
                                                }else{
                                                    m_NextActionV.clear();
                                                    m_Frame  = 0;
                                                    m_Action = ACTION_STAND;
                                                }

                                                break;
                                            }
                                    }
                                    break;
                                }
                            default:
                                {
                                    m_NextActionV.erase(m_NextActionV.begin());
                                    break;
                                }
                        }

                        if(bDone){ break; }
                    }
                    break;
                }
        }
    }else{
        MoveNextFrame(1);
    }
}

bool Creature::ActionValid(int nAction, int nDirection)
{
    switch(nAction){
        case ACTION_STAND:
        case ACTION_WALK:
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
                        {ACTION_WALK,       1},
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
