/*
 * =====================================================================================
 *
 *       Filename: creature.hpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 03/29/2017 16:51:41
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

#pragma once

#include <array>
#include <vector>

#include "protocoldef.hpp"

class ProcessRun;
class Creature
{
    protected:
        uint32_t m_UID;

        int m_X;
        int m_Y;

        int m_MoveDstX;
        int m_MoveDstY;

    protected:
        ProcessRun *m_ProcessRun;

    protected:
        // we split logic update and frame update here
        double m_LogicDelay;
        double m_FrameDelay;
        double m_LogicUpdateTime;
        double m_FrameUpdateTime;

    protected:
        int    m_Frame;

    protected:
        int    m_Speed;
        int    m_NextSpeed;
        int    m_Action;
        int    m_Direction;

    public:
        Creature(uint32_t, ProcessRun *);
        virtual ~Creature();

    public:
        int Speed()
        {
            return m_Speed;
        }

    public:
        void ResetAction(int nAction)
        {
            if(nAction == m_Action){ return; }
            // TODO: this frame issue is a bug
            //       here we reset it to zero, but in Update() it then will be 1, so
            //       the first frame can't be shown
            m_Frame = 0;
            m_Action = nAction;
        }

        void ResetSpeed(int nSpeed)
        {
            m_Speed = nSpeed;
        }

        void ResetDirection(int nDirection)
        {
            if(nDirection == m_Direction){ return; }

            // TODO: bug for frame
            m_Frame = 0;
            m_Direction = nDirection;
        }

        void ResetLocation(int nX, int nY)
        {
            m_X = nX;
            m_Y = nY;
        }

    public:
        virtual void OnCORecord(int, int, int, int, int);
        virtual void OnActionState(int, int, int, int, int);

    public:
        int X()
        {
            return m_X;
        }

        int Y()
        {
            return m_Y;
        }

    public:
        uint32_t UID()
        {
            return m_UID;
        }

    public:
        // update the next possible position based on current state
        void EstimateLocation(int, int *, int *);
        void EstimatePixelShift(int *, int *);

    public:
        virtual int Type() = 0;
        virtual size_t FrameCount() = 0;

    public:
        virtual bool ValidG() = 0;
        virtual void Draw(int, int) = 0;
        virtual void Update() = 0;
};
