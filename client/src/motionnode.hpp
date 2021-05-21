/*
 * =====================================================================================
 *
 *       Filename: motionnode.hpp
 *        Created: 04/05/2017 12:38:46
 *    Description: for field MotionNode::speed
 *
 *                      means % speed of default speed
 *
 *                 i.e. if default speed is 100 FPS:
 *
 *                      MotionNode::speed :  20 : FPS =  20 : min
 *                                           50 : FPS =  50 : slow
 *                                          100 : FPS = 100 : default
 *                                          200 : FPS = 200 : fast
 *                                          500 : FPS = 500 : max
 *
 *                  currently support speed : 20 ~ 500 => speed x5 or d5
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
#include <memory>
#include <functional>
#include "motion.hpp"
#include "sysconst.hpp"
#include "fflerror.hpp"
#include "dbcomid.hpp"
#include "dbcomrecord.hpp"
#include "protocoldef.hpp"

class MagicSpellEffect;
struct MotionNode final
{
    struct MotionExtParam
    {
        struct MotionSpell
        {
            const uint32_t magicID = 0;
            std::unique_ptr<MagicSpellEffect> effect{};
        }
        spell{};

        struct MotionAttack
        {
            const int motion = 0;
        }
        attack{};

        struct MotionDie
        {
            int fadeOut = 0;
        }
        die{};
        
        struct MotionSwing
        {
            const uint32_t magicID = 0;
        }
        swing{};
    };

    ///////////////////////////////////////////////////////////////////////////
    ////                                                                   ////
    ////          AGGREGATE INITIALIZATION MEMEBER LIST                    ////
    ////                                                                   ////
    ///////////////////////////////////////////////////////////////////////////
    /**/                                                                   /**/
    /**/    const int type      = MOTION_NONE;                             /**/
    /**/    const int direction = DIR_NONE;                                /**/
    /**/    /***/ int speed     = SYS_DEFSPEED;                            /**/
    /**/                                                                   /**/
    /**/    const int x = -1;                                              /**/
    /**/    const int y = -1;                                              /**/
    /**/                                                                   /**/
    /**/    const int endX = x;                                            /**/
    /**/    const int endY = y;                                            /**/
    /**/                                                                   /**/
    /**/    int frame = 0;                                                 /**/
    /**/    MotionExtParam extParam{};                                     /**/
    /**/    std::list<std::function<bool(MotionNode *)>> onUpdateCBList{}; /**/
    /**/                                                                   /**/
    ///////////////////////////////////////////////////////////////////////////

    operator bool () const
    {
        return false
            || ((type >= MOTION_BEGIN)     && (type < MOTION_END))
            || ((type >= MOTION_MON_BEGIN) && (type < MOTION_MON_END))
            || ((type >= MOTION_NPC_BEGIN) && (type < MOTION_NPC_END));
    }

    void update();
    void updateSpellEffect(double);
    void print() const;
    void addUpdate(bool, std::function<bool(MotionNode *)>);
};

class MagicSpellEffect
{
    protected:
        double m_accuTime = 0.0;

    protected:
        const MotionNode &m_motion;
        const MagicGfxEntry &m_gfxEntry;

    public:
        MagicSpellEffect(const MotionNode *motionPtr)
            : m_motion([motionPtr]() -> const auto &
              {
                  if(motionPtr){
                      return *motionPtr;
                  }
                  throw fflerror("bind null motion pointer to magic spell effect");
              }())
            , m_gfxEntry([this]() -> const auto &
              {
                  switch(m_motion.type){
                      case MOTION_SPELL0:
                      case MOTION_SPELL1: break;
                      default           : throw fflerror("invalid motion type: %d", m_motion.type);
                  }

                  const auto &mr = DBCOM_MAGICRECORD(m_motion.extParam.spell.magicID);
                  if(!mr){
                      throw fflerror("invalid magic ID: %llu", to_llu(m_motion.extParam.spell.magicID));
                  }

                  const auto &ge = mr.getGfxEntry(u8"启动");
                  if(!ge){
                      throw fflerror("magic ID %llu has no stage: %s", to_llu(m_motion.extParam.spell.magicID), to_cstr(u8"启动"));
                  }

                  if(!ge.checkType(u8"附着")){
                      throw fflerror("magic stage %s type is not type: %s", to_cstr(u8"启动"), to_cstr(u8"附着"));
                  }

                  if(ge.loop){
                      throw fflerror("magic stage %s is looped", to_cstr(u8"启动"));
                  }

                  switch(ge.gfxDirType){
                      case  1: break;
                      case  8: break;
                      default: throw fflerror("effect magic has gfxDirType: %d", ge.gfxDirType);
                  }

                  if(ge.frameCount <= 0){
                      throw fflerror("effect magic has invalid frameCount: %d", ge.frameCount);
                  }

                  if(ge.frameCount > ge.gfxIDCount){
                      throw fflerror("effect magic has invalid gfxIDCount: %d", ge.gfxIDCount);
                  }

                  if(!(ge.speed >= SYS_MINSPEED && ge.speed <= SYS_MAXSPEED)){
                      throw fflerror("effect magic has invalid speed: %d", ge.speed);
                  }

                  if(!(m_motion.direction >= DIR_BEGIN && m_motion.direction < DIR_END)){
                      throw fflerror("invalid motion direction: %d", m_motion.direction);
                  }
                  return ge;
              }())
        {}

    public:
        int absFrame() const
        {
            return (m_accuTime / 1000.0) * SYS_DEFFPS * (m_gfxEntry.speed / 100.0);
        }

    public:
        virtual int frame() const
        {
            return absFrame();
        }

        virtual int frameCount() const
        {
            return m_gfxEntry.frameCount;
        }

        int speed() const
        {
            return m_gfxEntry.speed;
        }

    protected:
        virtual uint32_t frameTexID() const
        {
            if(m_gfxEntry.gfxDirType > 1){
                return m_gfxEntry.gfxID + frame() + (m_motion.direction - DIR_BEGIN) * m_gfxEntry.gfxIDCount;
            }
            return m_gfxEntry.gfxID + frame();
        }

    public:
        virtual void drawShift(int, int, bool);

    public:
        virtual void update(double ms)
        {
            m_accuTime += ms;
        }
};

class CastMagicMotionEffect: public MagicSpellEffect
{
    public:
        CastMagicMotionEffect(const MotionNode *motionPtr)
            : MagicSpellEffect(motionPtr)
        {}

    public:
        int frameCount() const override
        {
            switch(m_motion.type){
                case MOTION_SPELL0: return std::max<int>(MagicSpellEffect::frameCount(),  8);
                case MOTION_SPELL1: return std::max<int>(MagicSpellEffect::frameCount(), 10);
                default           : throw bad_reach();
            }
        }

        uint32_t frameTexID() const override
        {
            if(frame() < MagicSpellEffect::frameCount()){
                return MagicSpellEffect::frameTexID();
            }
            return SYS_TEXNIL;
        }
};

inline const char *motionName(int type)
{
#define _add_motion_type_case(t) case t: return #t;
    switch(type){
        _add_motion_type_case(MOTION_STAND        )
        _add_motion_type_case(MOTION_ARROWATTACK  )
        _add_motion_type_case(MOTION_SPELL0       )
        _add_motion_type_case(MOTION_SPELL1       )
        _add_motion_type_case(MOTION_HOLD         )
        _add_motion_type_case(MOTION_PUSHBACK     )
        _add_motion_type_case(MOTION_PUSHBACKFLY  )
        _add_motion_type_case(MOTION_ATTACKMODE   )
        _add_motion_type_case(MOTION_CUT          )
        _add_motion_type_case(MOTION_ONEVSWING    )
        _add_motion_type_case(MOTION_TWOVSWING    )
        _add_motion_type_case(MOTION_ONEHSWING    )
        _add_motion_type_case(MOTION_TWOHSWING    )
        _add_motion_type_case(MOTION_SPEARVSWING  )
        _add_motion_type_case(MOTION_SPEARHSWING  )
        _add_motion_type_case(MOTION_HITTED       )
        _add_motion_type_case(MOTION_WHEELWIND    )
        _add_motion_type_case(MOTION_RANDSWING    )
        _add_motion_type_case(MOTION_BACKDROPKICK )
        _add_motion_type_case(MOTION_DIE          )
        _add_motion_type_case(MOTION_ONHORSEDIE   )
        _add_motion_type_case(MOTION_WALK         )
        _add_motion_type_case(MOTION_RUN          )
        _add_motion_type_case(MOTION_MOODEPO      )
        _add_motion_type_case(MOTION_ROLL         )
        _add_motion_type_case(MOTION_FISHSTAND    )
        _add_motion_type_case(MOTION_FISHHAND     )
        _add_motion_type_case(MOTION_FISHTHROW    )
        _add_motion_type_case(MOTION_FISHPULL     )
        _add_motion_type_case(MOTION_ONHORSESTAND )
        _add_motion_type_case(MOTION_ONHORSEWALK  )
        _add_motion_type_case(MOTION_ONHORSERUN   )
        _add_motion_type_case(MOTION_ONHORSEHITTED)

        _add_motion_type_case(MOTION_MON_STAND    )
        _add_motion_type_case(MOTION_MON_WALK     )
        _add_motion_type_case(MOTION_MON_ATTACK0  )
        _add_motion_type_case(MOTION_MON_HITTED   )
        _add_motion_type_case(MOTION_MON_DIE      )
        _add_motion_type_case(MOTION_MON_ATTACK1  )
        _add_motion_type_case(MOTION_MON_SPELL0   )
        _add_motion_type_case(MOTION_MON_SPELL1   )
        _add_motion_type_case(MOTION_MON_APPEAR   )
        _add_motion_type_case(MOTION_MON_SPECIAL  )

        _add_motion_type_case(MOTION_NPC_STAND    )
        _add_motion_type_case(MOTION_NPC_ACT      )
        _add_motion_type_case(MOTION_NPC_ACTEXT   )
#undef _add_motion_type_case
        default: return "MOTION_UNKNOWN";
    }
}
