/*
 * =====================================================================================
 *
 *       Filename: clientcreature.hpp
 *        Created: 04/07/2016 03:48:41
 *    Description: should I use factory method to create all creatures? seems I have to
 *                 allow to create creatures with current motion as MOTION_NONE
 *
 *                 reason:
 *                      class ClientCreature is supposed to be base of many derived
 *                      classes, then if define its constructor as
 *
 *                          ClientCreature(uid, pRun, stAction)
 *
 *                      means every time if we wanna create it, we should provide
 *                      currently its motion, however, think about class Monster
 *                      public derived from ClientCreature:
 *
 *                          Monster(uid, nMonsterID, pRun, stAction)
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
 *                          MonsterD(uid, pRun, stAction)
 *                              : Monster(uid, pRun, stAction) // <-- fail
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
 *                          Monster *Create(uid, pRun, stAction)
 *                          {
 *                              p = new Monster(uid, pRun);
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
 *                      not, make it valid and then call the public parseAction()
 *
 *                      but at the beginning how we to make it valid?? We have to make
 *                      one explicitly, which means we have to define a motion valid
 *                      for *all* creatures. MOTION_STAND? no, some creatures may not
 *                      stand even. actually this is the hard step.
 *
 *                      then immediately I will use MOTION_NONE
 *                      but if MOTION_NONE is valid, we can just use
 *
 *                          auto p = new Monster(uid, pRun);
 *                          p->parseAction(stAction);
 *
 *                      we don't even need the factory method any more. the cost is for
 *                      all function of Monster, we have to take care of MOTION_NONE,
 *                      this is a nightmare, every place I have to check for MOTION_NONE
 *                      
 *                      to get rid of this noise, currently how work around
 *                          Monster *Create(uid, pRun, stAction)
 *                          {
 *                              1. p = new Monster(uid, pRun);
 *                              2. p->MakeMotion(MOTION_STAND, stAction.X, stAciton.Y)
 *                              3. p->parseAction(stAction)
 *
 *                              return p;
 *                          }
 *
 *                       
 *                      this method will make up a motion based on stAction, nasty hack
 *                      but this can give use the assume:
 *
 *                          1. all creatures are made by Create(uid, ...)
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

#include "uidf.hpp"
#include "toll.hpp"
#include "colorf.hpp"
#include "fflerror.hpp"
#include "focustype.hpp"
#include "actionnode.hpp"
#include "labelboard.hpp"
#include "motionnode.hpp"
#include "protocoldef.hpp"
#include "attachmagic.hpp"

 class ProcessRun;
 class ClientCreature
{
    protected:
        const uint64_t m_UID;

    protected:
        ProcessRun *m_processRun;

    protected:
        uint32_t m_HP;
        uint32_t m_MP;
        uint32_t m_maxHP;
        uint32_t m_maxMP;

    protected:
        MotionNode m_currMotion;

    protected:
        std::vector<std::unique_ptr<AttachMagic>> m_attachMagicList;

    protected:
        uint32_t m_lastActive;
        uint32_t m_lastQuerySelf;
        double   m_lastUpdateTime;

    protected:
        LabelBoard m_nameBoard;

    protected:
        ClientCreature(uint64_t uid, ProcessRun *pRun)
            : m_UID(uid)
            , m_processRun(pRun)
            , m_HP(0)
            , m_MP(0)
            , m_maxHP(0)
            , m_maxMP(0)
            , m_lastActive(0)
            , m_lastQuerySelf(0)
            , m_lastUpdateTime(0.0)
            , m_nameBoard(0, 0, u8"ClientCreature", 1, 12, 0, colorf::RGBA(0XFF, 0XFF, 0XFF, 0X00))
        {
            if(!(m_UID && m_processRun)){
                throw fflerror("invalid argument: UID = %llu, processRun = %p", to_llu(m_UID), to_cvptr(m_processRun));
            }
        }

    public:
        virtual ~ClientCreature() = default;

    public:
        uint64_t UID() const
        {
            return m_UID;
        }

        int type() const
        {
            return uidf::getUIDType(UID());
        }

    public:
        static SDL_Color focusColor(int focusChan)
        {
            switch(focusChan){
                case FOCUS_MOUSE : return colorf::RGBA2Color(0XFF, 0X86, 0X00, 0XFF);
                case FOCUS_MAGIC : return colorf::RGBA2Color(0X92, 0XC6, 0X20, 0XFF);
                case FOCUS_FOLLOW: return colorf::RGBA2Color(0X00, 0XC6, 0XF0, 0XFF);
                case FOCUS_ATTACK: return colorf::RGBA2Color(0XD0, 0X2C, 0X70, 0XFF);
                default          : return colorf::RGBA2Color(0XFF, 0XFF, 0XFF, 0XFF);
            }
        }

    public:
        virtual bool canFocus(int, int) const = 0;

    public:
        uint32_t lastActive() const
        {
            return m_lastActive;
        }

        uint32_t lastQuerySelf() const
        {
            return m_lastQuerySelf;
        }

    public:
        int x() const
        {
            return std::get<0>(location());
        }

        int y() const
        {
            return std::get<1>(location());
        }

    public:
        const MotionNode &currMotion() const
        {
            return m_currMotion;
        }

    public:
        virtual std::tuple<int, int> location() const = 0;

    public:
        virtual int motionFrameCount(int, int) const = 0;

    protected:
        virtual bool moveNextMotion() = 0;

    public:
        virtual bool parseAction(const ActionNode &) = 0;

    protected:
        virtual bool advanceMotionFrame(int);

    protected:
        bool updateMotion(bool);
        void updateAttachMagic(double);

    public:
        virtual bool update(double) = 0;
        virtual bool draw(int, int, int) = 0;

    public:
        virtual bool motionValid(const MotionNode &) const = 0;

    public:
        void motionValidEx(const MotionNode &motion) const
        {
            if(!motionValid(motion)){
                throw fflerror("invalid motion");
            }
        }

    public:
        void updateHealth(int, int);

    public:
        int HP() const { return m_HP; }
        int MP() const { return m_MP; }

        int maxHP() const { return m_maxHP; }
        int maxMP() const { return m_maxMP; }

    public:
        bool deadFadeOut();

    protected:
        virtual int gfxMotionID(int) const = 0;

    public:
        bool   alive() const;
        bool  active() const;
        bool visible() const;

    protected:
        MotionNode makeMotionIdle() const;

    public:
        bool addAttachMagic(int, int, int);

    protected:
        double currMotionDelay() const;

    public:
        void querySelf();
};
