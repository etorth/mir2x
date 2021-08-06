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
#include "motioneffect.hpp"

struct MotionNode final
{
    struct MotionExtParam
    {
        struct MotionSpell
        {
            const uint32_t magicID = 0;
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
    /**/    MotionExtParam extParam {};                                    /**/
    /**/    std::unique_ptr<MotionEffect> effect {};                       /**/
    /**/                                                                   /**/
    ///////////////////////////////////////////////////////////////////////////

    // private members
    // make it public to support init by initializer_list
    std::list<std::function<bool(MotionNode *)>> m_triggerList {};

    operator bool () const
    {
        return false
            || ((type >= MOTION_BEGIN)     && (type < MOTION_END))
            || ((type >= MOTION_MON_BEGIN) && (type < MOTION_MON_END))
            || ((type >= MOTION_NPC_BEGIN) && (type < MOTION_NPC_END));
    }

    void runTrigger();
    void addTrigger(bool, std::function<bool(MotionNode *)>);

    void print() const;
    double frameDelay() const;
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
        _add_motion_type_case(MOTION_MON_SPAWN    )
        _add_motion_type_case(MOTION_MON_SPECIAL  )

        _add_motion_type_case(MOTION_NPC_STAND    )
        _add_motion_type_case(MOTION_NPC_ACT      )
        _add_motion_type_case(MOTION_NPC_ACTEXT   )
#undef _add_motion_type_case
        default: return "MOTION_UNKNOWN";
    }
}
