/*
 * =====================================================================================
 *
 *       Filename: creature.hpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 04/04/2017 14:13:40
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
        typedef struct _ActionNode
        {
            int Action;
            int Direction;
            int Speed;

            int X;
            int Y;

            _ActionNode(int nAction, int nDirection, int nSpeed, int nX, int nY)
                : Action(nAction)
                , Direction(nDirection)
                , Speed(nSpeed)
                , X(nX)
                , Y(nY)
            {}

            _ActionNode()
                : Action(ACTION_NONE)
            {}
        }ActionNode;

    protected:
        const uint32_t m_UID;

    protected:
        ProcessRun *m_ProcessRun;

    protected:
        int m_X;
        int m_Y;

        int m_Action;
        int m_Direction;
        int m_Speed;

    protected:
        std::vector<ActionNode> m_NextActionV;

    protected:
        int m_Frame;

    public:
        Creature(uint32_t,      // UID
                ProcessRun *,   //
                int,            // map x
                int,            // map y
                int,            // action
                int,            // direction
                int);           // speed
        virtual ~Creature() = default;

    public:
        virtual int Speed()
        {
            return m_Speed;
        }

    public:
        virtual void OnReportState();
        virtual void OnReportAction(int, int, int, int, int, int);

    public:
        uint32_t UID() { return m_UID; }

    public:
        int X() { return m_X; }
        int Y() { return m_Y; }

    public:
        int Action   () { return m_Action;    }
        int Direction() { return m_Direction; }

    public:
        // update the next possible position based on current state
        void EstimateLocation(int, int *, int *);
        void EstimatePixelShift(int *, int *);

    public:
        virtual int Type() = 0;
        virtual size_t FrameCount() = 0;

    protected:
        virtual void MoveNextFrame(int);

    public:
        virtual bool ValidG() = 0;
        virtual void Draw(int, int) = 0;
        virtual void Update() = 0;

    public:
        virtual bool ActionValid(int, int);

    public:
        virtual int GfxID();

    public:
        virtual void ReportBadAction();
        virtual void ReportBadActionNode(size_t);

    public:
        virtual void OnWalk();
        virtual void OnStand();
};
