/*
 * =====================================================================================
 *
 *       Filename: motionnode.cpp
 *        Created: 04/05/2017 12:40:09
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
#include <cinttypes>
#include "log.hpp"
#include "motionnode.hpp"

void MotionNode::Print()
{
    auto fnGetMotionName = [](int nMotion) -> const char *
    {
        switch(nMotion){
            case MOTION_NONE          : return "MOTION_NONE";
            case MOTION_STAND         : return "MOTION_STAND";
            case MOTION_ARROWATTACK   : return "MOTION_ARROWATTACK";
            case MOTION_SPELL0        : return "MOTION_SPELL0";
            case MOTION_SPELL1        : return "MOTION_SPELL1";
            case MOTION_HOLD          : return "MOTION_HOLD";
            case MOTION_PUSHBACK      : return "MOTION_PUSHBACK";
            case MOTION_PUSHBACKFLY   : return "MOTION_PUSHBACKFLY";
            case MOTION_ATTACKMODE    : return "MOTION_ATTACKMODE";
            case MOTION_CUT           : return "MOTION_CUT";
            case MOTION_ONEVSWING     : return "MOTION_ONEVSWING";
            case MOTION_TWOVSWING     : return "MOTION_TWOVSWING";
            case MOTION_ONEHSWING     : return "MOTION_ONEHSWING";
            case MOTION_TWOHSWING     : return "MOTION_TWOHSWING";
            case MOTION_SPEARVSWING   : return "MOTION_SPEARVSWING";
            case MOTION_SPEARHSWING   : return "MOTION_SPEARHSWING";
            case MOTION_HITTED        : return "MOTION_HITTED";
            case MOTION_WHEELWIND     : return "MOTION_WHEELWIND";
            case MOTION_RANDSWING     : return "MOTION_RANDSWING";
            case MOTION_BACKDROPKICK  : return "MOTION_BACKDROPKICK";
            case MOTION_DIE           : return "MOTION_DIE";
            case MOTION_ONHORSEDIE    : return "MOTION_ONHORSEDIE";
            case MOTION_WALK          : return "MOTION_WALK";
            case MOTION_RUN           : return "MOTION_RUN";
            case MOTION_MOODEPO       : return "MOTION_MOODEPO";
            case MOTION_ROLL          : return "MOTION_ROLL";
            case MOTION_FISHSTAND     : return "MOTION_FISHSTAND";
            case MOTION_FISHHAND      : return "MOTION_FISHHAND";
            case MOTION_FISHTHROW     : return "MOTION_FISHTHROW";
            case MOTION_FISHPULL      : return "MOTION_FISHPULL";
            case MOTION_ONHORSESTAND  : return "MOTION_ONHORSESTAND";
            case MOTION_ONHORSEWALK   : return "MOTION_ONHORSEWALK";
            case MOTION_ONHORSERUN    : return "MOTION_ONHORSERUN";
            case MOTION_ONHORSEHITTED : return "MOTION_ONHORSEHITTED";
            case MOTION_MAX           : return "MOTION_MAX";
            case MOTION_MON_NONE      : return "MOTION_MON_NONE";
            case MOTION_MON_STAND     : return "MOTION_MON_STAND";
            case MOTION_MON_WALK      : return "MOTION_MON_WALK";
            case MOTION_MON_ATTACK0   : return "MOTION_MON_ATTACK0";
            case MOTION_MON_HITTED    : return "MOTION_MON_HITTED";
            case MOTION_MON_DIE       : return "MOTION_MON_DIE";
            case MOTION_MON_ATTACK1   : return "MOTION_MON_ATTACK1";
            case MOTION_MON_SPELL0    : return "MOTION_MON_SPELL0";
            case MOTION_MON_SPELL1    : return "MOTION_MON_SPELL1";
            case MOTION_MON_APPEAR    : return "MOTION_MON_APPEAR";
            case MOTION_MON_SPECIAL   : return "MOTION_MON_SPECIAL";
            case MOTION_MON_MAX       : return "MOTION_MON_MAX";
            default                   : return "MOTION_UNKNOWN";
        }
    };

    extern Log *g_Log;
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::Motion      = %s", (int)(2 * sizeof(this)), (uintptr_t)(this), fnGetMotionName(Motion));
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::MotionParam = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), MotionParam            );
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::Direction   = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), Direction              );
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::Speed       = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), Speed                  );
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::X           = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), X                      );
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::Y           = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), Y                      );
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::EndX        = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), EndX                   );
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::EndY        = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), EndY                   );
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::Frame       = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), Frame                  );
    g_Log->AddLog(LOGTYPE_INFO, "MotionNode::0X%0*" PRIXPTR "::FadeOut     = %d", (int)(2 * sizeof(this)), (uintptr_t)(this), FadeOut                );
}
