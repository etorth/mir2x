/*
 * =====================================================================================
 *
 *       Filename: creature.hpp
 *        Created: 04/07/2016 03:48:41
 *  Last Modified: 04/07/2017 13:43:08
 *
 *    Description: should I use factory method to create all creatures? seems I have to
 *                 allow to create creatures with current motion as MOTION_NONE
 *
 *                 reason:
 *                      class Creature is supposed to be base of many derived
 *                      classes, then if define its constructor as
 *
 *                          Creature(nUID, pRun, stAction)
 *
 *                      means every time if we wanna create it, we should provide
 *                      currently its motion, however, think about class Monster
 *                      public derived from Creature:
 *
 *                          Monster(nUID, nMonsterID, pRun, stAction)
 *
 *                      this class is for general monsters, then it only allow
 *
 *                          MOTION_STAND
 *                          MOTION_WALK
 *                          MOTION_ATTACK
 *                          MOTION_UNDERATTACK
 *                          MOTION_DIE
 *
 *                      means other action -> motion would fail.
 *
 *                      now, if we further derive MonsterD from Monster, MonsterD
 *                      is a specified monster which can't walk. then in the ctor:
 *
 *                          MonsterD(nUID, pRun, stAction)
 *                              : Monster(nUID, pRun, stAction)
 *                          {...}
 *
 *                      then here we call Monster() and it fails
 *
 *                      So if we split Action handling from construction from it,
 *                      as the interface
 *
 *                          p = new Monster(nUID, pRun);
 *                          p->ParseNewAction(stAction);
 *
 *                      then what's the motion when it's created? I have no idea
 *                      but put a MOTION_NONE there
 *
 *                      but if I have to use factory mode, why not just use the two
 *                      phases method?
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

#include <deque>
#include <cstddef>
#include <cstdint>

#include "statenode.hpp"
#include "actionnode.hpp"
#include "motionnode.hpp"
#include "protocoldef.hpp"

class ProcessRun;
class Creature
{
    protected:
        const uint32_t m_UID;

    protected:
        ProcessRun *m_ProcessRun;

    protected:
        MotionNode m_CurrMotion;

    protected:
        std::deque<MotionNode> m_MotionQueue;

    protected:
        Creature(uint32_t nUID, ProcessRun *pRun)
            : m_UID(nUID)
            , m_ProcessRun(pRun)
            , m_CurrMotion()
            , m_MotionQueue()
        {
            assert(m_UID);
            assert(m_ProcessRun);
        }

    public:
        virtual ~Creature() = default;

    public:
        uint32_t UID() { return m_UID; }

    public:
        int X() { int nX; return Location(&nX, nullptr) ? nX : -1; }
        int Y() { int nY; return Location(nullptr, &nY) ? nY : -1; }

    public:
        virtual bool Location(int *, int *) = 0;

    public:
        bool EstimateLocation(int, int *, int *);
        bool EstimatePixelShift(int *, int *);

    public:
        virtual int Type() = 0;
        virtual size_t MotionFrameCount() = 0;

    public:
        virtual bool ParseNewState (const StateNode &) = 0;
        virtual bool ParseNewAction(const ActionNode&) = 0;

    protected:
        virtual bool AdvanceMotionFrame(int);

    protected:
        virtual bool UpdateGeneralMotion(bool);

    public:
        virtual bool ValidG() = 0;
        virtual bool Draw(int, int) = 0;
        virtual bool Update() = 0;

    public:
        virtual bool MoveNextMotion();
        virtual bool ParseMovePath(int, int, int, int, int, int);

    protected:
        virtual bool MotionQueueValid();

    public:
        virtual bool ActionValid(const ActionNode &) = 0;
        virtual bool MotionValid(const MotionNode &) = 0;
};
