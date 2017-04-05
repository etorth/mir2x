/*
 * =====================================================================================
 *
 *       Filename: creature.hpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 04/05/2017 14:25:41
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

#include <list>
#include <cstdint>
#include <cstddef>

#include "motionnode.hpp"
#include "protocoldef.hpp"

class ProcessRun;
class Creature
{
    protected:

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
        std::list<MotionNode> m_MotionList;

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
        virtual bool OnReportState() = 0;
        virtual bool OnReportAction(int, int, int, int, int, int) = 0;

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
        virtual size_t MotionFrameCount() = 0;

    protected:
        virtual bool UpdateMotion() = 0;
        virtual bool AdvanceMotionFrame(int);

    protected:
        virtual bool UpdateMotionOnStand()       = 0;
        virtual bool UpdateMotionOnWalk()        = 0;
        virtual bool UpdateMotionOnAttack()      = 0;
        virtual bool UpdateMotionOnUnderAttack() = 0;
        virtual bool UpdateMotionOnDie()         = 0;

    public:
        virtual bool ValidG() = 0;
        virtual void Draw(int, int) = 0;
        virtual void Update() = 0;

    public:
        virtual bool MoveNextMotion();
        virtual bool EraseNextMotion();
        virtual bool ParseMovePath(int, int, int, int, int, int);

    public:
        virtual int GfxID();

    public:
        virtual bool MotionValid(int, int);
        virtual bool ActionValid(int, int);
};
