/*
 * =====================================================================================
 *
 *       Filename: creature.hpp
 *        Created: 04/07/2016 03:48:41
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
 *                      say this class is for general monsters, then it allows
 *
 *                          MOTION_MON_STAND
 *                          MOTION_MON_WALK
 *                          MOTION_MON_ATTACK
 *                          MOTION_MON_UNDERATTACK
 *                          MOTION_MON_DIE
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
 *                              p->PrivateParseAction(stAction);
 *
 *                              return p;
 *                          }
 *
 *                      then when get p, it's already in valid stAction. but this
 *                      need to define PrivateParseAction(), which can handle
 *                      creature in invalid state.
 *
 *                      PrivateParseAction() first check if current state valid? if
 *                      not, make it valid and then call the public ParseAction()
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
 *                          p->ParseAction(stAction);
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
 *                              3. p->ParseAction(stAction)
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
#include <memory>
#include <cstddef>
#include <cstdint>

#include "colorfunc.hpp"
#include "focustype.hpp"
#include "condcheck.hpp"
#include "actionnode.hpp"
#include "labelboard.hpp"
#include "motionnode.hpp"
#include "pathfinder.hpp"
#include "magicrecord.hpp"
#include "protocoldef.hpp"
#include "attachmagic.hpp"

class ProcessRun;
class Creature
{
    protected:
        const uint64_t m_UID;

    protected:
        ProcessRun *m_ProcessRun;

    protected:
        uint32_t m_HP;
        uint32_t m_MP;
        uint32_t m_HPMax;
        uint32_t m_MPMax;

    protected:
        MotionNode m_CurrMotion;

    protected:
        std::deque<MotionNode> m_MotionQueue;
        std::deque<MotionNode> m_forceMotionQueue;

    protected:
        std::vector<std::shared_ptr<AttachMagic>> m_AttachMagicList;

    protected:
        uint32_t m_LastActive;
        uint32_t m_LastQuerySelf;

    protected:
        double m_LastUpdateTime;

    protected:
        labelBoard m_nameBoard;

    protected:
        Creature(uint64_t nUID, ProcessRun *pRun)
            : m_UID(nUID)
            , m_ProcessRun(pRun)
            , m_HP(0)
            , m_MP(0)
            , m_HPMax(0)
            , m_MPMax(0)
            , m_CurrMotion()
            , m_MotionQueue()
            , m_forceMotionQueue()
            , m_AttachMagicList()
            , m_LastActive(0)
            , m_LastQuerySelf(0)
            , m_LastUpdateTime(0.0)
            , m_nameBoard(0, 0, "creature", 1, 12, 0, ColorFunc::RGBA(0XFF, 0XFF, 0XFF, 0X00))
        {
            condcheck(m_UID);
            condcheck(m_ProcessRun);
        }

    public:
        virtual ~Creature() = default;

    public:
        uint64_t UID() const
        {
            return m_UID;
        }

    public:
        static SDL_Color FocusColor(int nFocusChan)
        {
            switch(nFocusChan){
                case FOCUS_MOUSE:
                    {
                        return ColorFunc::RGBA2Color(0XFF, 0X86, 0X00, 0XFF);
                    }
                case FOCUS_MAGIC:
                    {
                        return ColorFunc::RGBA2Color(0X92, 0XC6, 0X20, 0XFF);
                    }
                case FOCUS_FOLLOW:
                    {
                        return ColorFunc::RGBA2Color(0X00, 0XC6, 0XF0, 0XFF);
                    }
                case FOCUS_ATTACK:
                    {
                        return ColorFunc::RGBA2Color(0XD0, 0X2C, 0X70, 0XFF);
                    }
                default:
                    {
                        return ColorFunc::RGBA2Color(0XFF, 0XFF, 0XFF, 0XFF);
                    }
            }
        }

    public:
        virtual bool canFocus(int, int) = 0;

    public:
        bool Alive();
        bool Active();
        bool Visible();

    public:
        uint32_t LastActive() const
        {
            return m_LastActive;
        }

        uint32_t LastQuerySelf() const
        {
            return m_LastQuerySelf;
        }

    public:
        int X() { int nX; return Location(&nX, nullptr) ? nX : -1; }
        int Y() { int nY; return Location(nullptr, &nY) ? nY : -1; }

    public:
        const MotionNode &CurrMotion() const
        {
            return m_CurrMotion;
        }

    public:
        virtual bool Location(int *, int *) = 0;

    public:
        bool GetShift(int *, int *);

    public:
        virtual int Type() const = 0;

    public:
        virtual int MotionFrameCount(int, int) const = 0;

    public:
        virtual bool StayDead();
        virtual bool StayIdle();

    public:
        virtual bool ParseAction(const ActionNode &) = 0;

    protected:
        virtual bool AdvanceMotionFrame(int);

    protected:
        virtual bool UpdateMotion(bool);
        virtual void UpdateAttachMagic(double);

    public:
        virtual bool Update(double) = 0;
        virtual bool Draw(int, int, int) = 0;

    protected:
        virtual bool MoveNextMotion();

    protected:
        // parse a motion path for src -> dst for current creature
        // parameters:
        //      src            : (nX0, nY0)
        //      dst            : (nX1, nY1)
        //      bCheckGround   :
        //      bCheckCreature :
        //
        //  1. src point should be valid grid on map
        //  2. this function allows dst to be invalid location if bCheckGround is not set
        //  3. this function automatically use the minmal time path by calling Creature::MaxStep()
        //  4. check ClientPathFinder for bCheckGround and bCheckCreature
        //
        // return vector size:
        //      0 : error happens
        //      1 : can't find a path and keep standing on (nX0, nY0)
        //     >1 : path found
        virtual std::vector<PathFind::PathNode> ParseMovePath(
                int, int,       // src
                int, int,       // dst
                bool,           // bCheckGround
                int);           // nCheckCreature

    protected:
        virtual bool MotionQueueValid();

    public:
        virtual bool MotionValid(const MotionNode &) const = 0;

    public:
        virtual int UpdateHP(int, int);

    public:
        int HP() const { return m_HP; }
        int MP() const { return m_MP; }

        int HPMax() const { return m_HPMax; }
        int MPMax() const { return m_MPMax; }

    public:
        virtual bool DeadFadeOut();

    protected:
        double CurrMotionDelay() const;

    protected:
        virtual int  MaxStep() const = 0;
        virtual int CurrStep() const = 0;

    protected:
        virtual int GfxMotionID(int) const = 0;

    protected:
        virtual MotionNode MakeMotionIdle() const;
        virtual MotionNode MakeMotionWalk(int, int, int, int, int) const = 0;

    protected:
        std::deque<MotionNode> MakeMotionWalkQueue(int, int, int, int, int);

    public:
        bool AddAttachMagic(int, int, int);

    public:
        void QuerySelf();
};
