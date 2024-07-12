#pragma once
enum MotionType: int
{
    //===================================
    MOTION_NONE  = 0,
    MOTION_BEGIN = MOTION_NONE + 1,
    MOTION_STAND = MOTION_BEGIN,
    MOTION_ARROWATTACK,
    MOTION_SPELL0,
    MOTION_SPELL1,
    MOTION_HOLD,
    MOTION_PUSHBACK,
    MOTION_PUSHBACKFLY,
    MOTION_ATTACKMODE,
    MOTION_CUT,
    MOTION_ONEVSWING,
    MOTION_TWOVSWING,
    MOTION_ONEHSWING,
    MOTION_TWOHSWING,
    MOTION_SPEARVSWING,
    MOTION_SPEARHSWING,
    MOTION_HITTED,
    MOTION_WHEELWIND,
    MOTION_RANDSWING,
    MOTION_SPINKICK,
    MOTION_DIE,
    MOTION_ONHORSEDIE,
    MOTION_WALK,
    MOTION_RUN,
    MOTION_MOODEPO,
    MOTION_ROLL,
    MOTION_FISHSTAND,
    MOTION_FISHHAND,
    MOTION_FISHTHROW,
    MOTION_FISHPULL,
    MOTION_ONHORSESTAND,
    MOTION_ONHORSEWALK,
    MOTION_ONHORSERUN,
    MOTION_ONHORSEHITTED,
    MOTION_END,

    //===================================
    MOTION_MON_NONE,
    MOTION_MON_BEGIN = MOTION_MON_NONE + 1,
    MOTION_MON_STAND = MOTION_MON_BEGIN,
    MOTION_MON_WALK,
    MOTION_MON_ATTACK0,
    MOTION_MON_HITTED,
    MOTION_MON_DIE,

    MOTION_MON_ATTACK1,
    MOTION_MON_SPELL0,
    MOTION_MON_SPELL1,
    MOTION_MON_SPAWN,
    MOTION_MON_SPECIAL,
    MOTION_MON_END,

    //===================================
    MOTION_NPC_NONE,
    MOTION_NPC_BEGIN = MOTION_NPC_NONE + 1,
    MOTION_NPC_STAND = MOTION_NPC_BEGIN,
    MOTION_NPC_ACT,
    MOTION_NPC_ACTEXT,
    MOTION_NPC_END,

    //===================================
    MOTION_EXT_NONE,
    MOTION_EXT_BEGIN = MOTION_EXT_NONE + 1,
    MOTION_EXT_COMBINED,
    MOTION_EXT_END,
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
        _add_motion_type_case(MOTION_SPINKICK     )
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
