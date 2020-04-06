/*
 * =====================================================================================
 *
 *       Filename: motionnode.hpp
 *        Created: 04/05/2017 12:38:46
 *    Description: for field MotionNode::Speed
 *                  
 *                      means % Speed of default speed
 *
 *                 i.e. if default speed is 100 FPS:
 *                  
 *                      MotionNode::Speed :  20 : FPS =  20 : min
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
#include "motion.hpp"
#include "sysconst.hpp"
#include "protocoldef.hpp"

struct MotionNode
{
    // part-1 : const fields
    //          description of this motion
    int Motion;
    int MotionParam;

    int Direction;
    int Speed;

    int X;
    int Y;

    int EndX;
    int EndY;

    // part-2 : mutable field
    //          always initialized as 0 and get updated later
    int Frame;
    int FadeOut;

    struct _InterpMotion
    {
        int Motion      = MOTION_NONE;
        int MotionParam = 0;

        int Frame       = 0;
    }InterpMotion;

    // don't put any simple data memeber check in constructor
    // in main code there is need to return an invalid motion node

    // if put check method in constructor
    // I need to abort if checking failed this is not good
    MotionNode(int nMotion, int nMotionParam, int nDirection, int nSpeed, int nX, int nY, int nEndX, int nEndY)
        : Motion(nMotion)
        , MotionParam(nMotionParam)
        , Direction(nDirection)
        , Speed(nSpeed)
        , X(nX)
        , Y(nY)
        , EndX(nEndX)
        , EndY(nEndY)
        , Frame(0)
        , FadeOut(0)
    {}

    MotionNode(int nMotion, int nMotionParam, int nDirection, int nSpeed, int nX, int nY)
        : MotionNode(nMotion, nMotionParam, nDirection, nSpeed, nX, nY, nX, nY)
    {}

    MotionNode(int nMotion, int nMotionParam, int nDirection, int nX, int nY)
        : MotionNode(nMotion, nMotionParam, nDirection, SYS_DEFSPEED, nX, nY, nX, nY)
    {}

    MotionNode()
        : MotionNode(MOTION_NONE, 0, DIR_NONE, 0, 0)
    {}

    operator bool () const
    {
        return false
            || ((Motion > MOTION_NONE)     && (Motion < MOTION_MAX))
            || ((Motion > MOTION_MON_NONE) && (Motion < MOTION_MON_MAX));
    }

    void Print();

    static const char *name(int motion)
    {
#define _addCaseType(t) case t: return #t;
        switch(motion){
            _addCaseType(MOTION_NONE         )
            _addCaseType(MOTION_STAND        )
            _addCaseType(MOTION_ARROWATTACK  )
            _addCaseType(MOTION_SPELL0       )
            _addCaseType(MOTION_SPELL1       )
            _addCaseType(MOTION_HOLD         )
            _addCaseType(MOTION_PUSHBACK     )
            _addCaseType(MOTION_PUSHBACKFLY  )
            _addCaseType(MOTION_ATTACKMODE   )
            _addCaseType(MOTION_CUT          )
            _addCaseType(MOTION_ONEVSWING    )
            _addCaseType(MOTION_TWOVSWING    )
            _addCaseType(MOTION_ONEHSWING    )
            _addCaseType(MOTION_TWOHSWING    )
            _addCaseType(MOTION_SPEARVSWING  )
            _addCaseType(MOTION_SPEARHSWING  )
            _addCaseType(MOTION_HITTED       )
            _addCaseType(MOTION_WHEELWIND    )
            _addCaseType(MOTION_RANDSWING    )
            _addCaseType(MOTION_BACKDROPKICK )
            _addCaseType(MOTION_DIE          )
            _addCaseType(MOTION_ONHORSEDIE   )
            _addCaseType(MOTION_WALK         )
            _addCaseType(MOTION_RUN          )
            _addCaseType(MOTION_MOODEPO      )
            _addCaseType(MOTION_ROLL         )
            _addCaseType(MOTION_FISHSTAND    )
            _addCaseType(MOTION_FISHHAND     )
            _addCaseType(MOTION_FISHTHROW    )
            _addCaseType(MOTION_FISHPULL     )
            _addCaseType(MOTION_ONHORSESTAND )
            _addCaseType(MOTION_ONHORSEWALK  )
            _addCaseType(MOTION_ONHORSERUN   )
            _addCaseType(MOTION_ONHORSEHITTED)
            _addCaseType(MOTION_MAX          )

            _addCaseType(MOTION_MON_NONE     )
            _addCaseType(MOTION_MON_STAND    )
            _addCaseType(MOTION_MON_WALK     )
            _addCaseType(MOTION_MON_ATTACK0  )
            _addCaseType(MOTION_MON_HITTED   )
            _addCaseType(MOTION_MON_DIE      )
            _addCaseType(MOTION_MON_ATTACK1  )
            _addCaseType(MOTION_MON_SPELL0   )
            _addCaseType(MOTION_MON_SPELL1   )
            _addCaseType(MOTION_MON_APPEAR   )
            _addCaseType(MOTION_MON_SPECIAL  )
            _addCaseType(MOTION_MON_MAX      )

            _addCaseType(MOTION_NPC_NONE     )
            _addCaseType(MOTION_NPC_STAND    )
            _addCaseType(MOTION_NPC_ACT1     )
            _addCaseType(MOTION_NPC_ACT2     )
            _addCaseType(MOTION_NPC_MAX      )
#undef _addCaseType
            default: return "MOTION_UNKNOWN";
        }
    }
};
