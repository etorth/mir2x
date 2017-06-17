/*
 * =====================================================================================
 *
 *       Filename: creature.hpp
 *        Created: 04/07/2016 03:48:41
 *  Last Modified: 06/16/2017 22:55:54
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
 *                              : Monster(nUID, pRun, stAction) // <-- fail
 *                          {...}
 *
 *                      then here we call Monster() and it fails
 *
 *                      My object is to find a method to ensure that if a creature
 *                      is created by new, then it's already in valid state.
 *
 *                      this helps to simplify the logic, everytime if we have new
 *                      input, we always are sure current it's valid, and if the
 *                      input is valid, then it keeps valid after the operation.
 *
 *                      so we should define a valid motion to *all* creatures, then
 *                      when constructed it's automatically in that motion state.
 *
 *                      so I use MOTION_NONE
 *
 *                      factory method works as a two phases solution:
 *
 *                          Monster *Create(nUID, pRun, stAction)
 *                          {
 *                              p = new Monster(nUID, pRun);
 *                              p->PrivateParseNewAction(stAction);
 *
 *                              return p;
 *                          }
 *
 *                      then when get p, it's already in valid stAction. but this
 *                      need to define PrivateParseNewAction(), which can handle
 *                      creature in invalid state.
 *
 *                      PrivateParseNewAction() first check if current state valid? if
 *                      not, make it valid and then call the public ParseNewAction()
 *
 *                      but at the beginning how we to make it valid?? We have to make
 *                      one explicitly, which means we have to define a motion valid
 *                      for *all* creatures. MOTION_STAND? no, some creatures may not
 *                      stand even. actually this is the hard step.
 *
 *                      then immediately I will use MOTION_NONE
 *                      but if MOTION_NONE is valid, we can just use
 *
 *                          auto p = new Monster(nUID, pRun);
 *                          p->ParseNewAction(stAction);
 *
 *                      we don't even need the factory method any more. the cost is for
 *                      all function of Monster, we have to take care of MOTION_NONE,
 *                      this is a nightmare, every place I have to check for MOTION_NONE
 *                      
 *                      to get rid of this noise, currently how work around
 *                          Monster *Create(nUID, pRun, stAction)
 *                          {
 *                              1. p = new Monster(nUID, pRun);
 *                              2. p->MakeMotion(MOTION_STAND, stAction.X, stAciton.Y)
 *                              3. p->ParseNewAction(stAction)
 *
 *                              return p;
 *                          }
 *
 *                       
 *                      this method will make up a motion based on stAction, nasty hack
 *                      but this can give use the assume:
 *
 *                          1. all creatures are made by Create(nUID, ...)
 *                          2. once made, it's guarenteed to be in valid state
 *
 *                      then only constructor can leave creature with MOTION_NONE, but
 *                      we make it protected, and make interface as Monster::Create()
 *
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
#include <cassert>
#include <cstddef>
#include <cstdint>

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
        uint32_t m_HP;
        uint32_t m_MP;
        uint32_t m_HPMax;
        uint32_t m_MPMax;

    protected:
        bool m_Active;
        bool m_Focus;

    protected:
        MotionNode m_CurrMotion;

    protected:
        std::deque<MotionNode> m_MotionQueue;

    protected:
        double m_UpdateDelay;
        double m_LastUpdateTime;

    protected:
        Creature(uint32_t nUID, ProcessRun *pRun)
            : m_UID(nUID)
            , m_ProcessRun(pRun)
            , m_HP(0)
            , m_MP(0)
            , m_HPMax(0)
            , m_MPMax(0)
            , m_Active(true)
            , m_CurrMotion()
            , m_MotionQueue()
            , m_UpdateDelay(100.0)
            , m_LastUpdateTime(0.0)
        {
            assert(m_UID);
            assert(m_ProcessRun);
        }

    public:
        virtual ~Creature() = default;

    public:
        uint32_t UID() { return m_UID; }

    public:
        void Focus(bool bFocus)
        {
            m_Focus = bFocus;
        }

        bool Focus() const
        {
            return m_Focus;
        }

        virtual bool CanFocus(int, int) = 0;

    public:
        bool Active() const
        {
            return m_Active;
        }

        void Deactivate()
        {
            m_Active = false;
        }

    public:
        int X() { int nX; return Location(&nX, nullptr) ? nX : -1; }
        int Y() { int nY; return Location(nullptr, &nY) ? nY : -1; }

    public:
        const MotionNode &CurrMotion()
        {
            return m_CurrMotion;
        }

    public:
        virtual bool Location(int *, int *) = 0;

    public:
        bool EstimatePixelShift(int *, int *);

    public:
        virtual int Type() = 0;
        virtual size_t MotionFrameCount() = 0;

    public:
        virtual bool StayDead();

    public:
        virtual bool ParseNewAction(const ActionNode &, bool) = 0;

    protected:
        virtual bool AdvanceMotionFrame(int);

    protected:
        virtual bool UpdateGeneralMotion(bool);

    public:
        virtual bool ValidG() = 0;
        virtual bool Draw(int, int) = 0;
        virtual bool Update() = 0;

    protected:
        virtual bool MoveNextMotion();

    protected:
        // parse motion request (src -> dst)
        // motion    : motion used for this motion request
        // speed     :
        // src point :
        // dst point :
        //
        // return true  if successfully add motion node to m_MotionQueue
        //        false if there is no path
        //
        // 1. return true if there is path but creature blocks it
        // 2. small motions like MOTION_WALK/HORSEWALK can be inserted
        virtual bool ParseMovePath(int,     // motion
                int,                        // speed
                int, int,                   // src point
                int, int);                  // dst point

    protected:
        virtual bool MotionQueueValid();

    public:
        virtual bool ActionValid(const ActionNode &) = 0;
        virtual bool MotionValid(const MotionNode &) = 0;

    public:
        virtual int UpdateHP(int, int);

    public:
        virtual bool DeadFadeOut();
};
