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
#include "totype.hpp"
#include "fflerror.hpp"
#include "motionnode.hpp"

extern Log *g_log;

MotionNode::MagicSpellEffect::MagicSpellEffect(const MotionNode *motionPtr)
    : MotionNode::MotionEffect()
    , m_gfxEntry([motionPtr, this]()
      {
          if(!motionPtr){
              throw fflerror("invalid argument: motionPtr = nullptr");
          }

          switch(motionPtr->type){
              case MOTION_SPELL0:
              case MOTION_SPELL1: break;
              default           : throw fflerror("invalid motion type: %d", motionPtr->type);
          }

          const auto &mr = DBCOM_MAGICRECORD(motionPtr->param.spell.magicID);
          if(!mr){
              throw fflerror("invalid magic ID: %d", motionPtr->param.spell.magicID);
          }

          const auto &ge = mr.getGfxEntry(u8"启动");
          if(!ge){
              throw fflerror("magic ID %d has no stage: %s", motionPtr->param.spell.magicID, to_cstr(u8"启动"));
          }

          if(!ge.checkType(u8"附着")){
              throw fflerror("magic stage %s type is not type: %s", to_cstr(u8"启动"), to_cstr(u8"附着"));
          }

          if(ge.loop){
              throw fflerror("magic stage %s is looped", to_cstr(u8"启动"));
          }
          return &ge;
      }())
{}

int MotionNode::MagicSpellEffect::frame() const
{
    return m_frame;
}

int MotionNode::MagicSpellEffect::frameCount() const
{
    return m_gfxEntry->frameCount;
}

void MotionNode::MagicSpellEffect::drawShift(int, int, bool)
{}

void MotionNode::print() const
{
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::motion      = %s", to_cvptr(this), MotionNode::name(type));
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::direction   = %d", to_cvptr(this), direction             );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::speed       = %d", to_cvptr(this), speed                 );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::x           = %d", to_cvptr(this), x                     );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::y           = %d", to_cvptr(this), y                     );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::endX        = %d", to_cvptr(this), endX                  );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::endY        = %d", to_cvptr(this), endY                  );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::frame       = %d", to_cvptr(this), frame                 );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::fadeOut     = %d", to_cvptr(this), fadeOut               );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::effect      = %s", to_cvptr(this), effect   ? "1" : "0"  );
    g_log->addLog(LOGTYPE_INFO, "MotionNode::0x%016p::onUpdate    = %s", to_cvptr(this), onUpdate ? "1" : "0"  );
}

const char *MotionNode::name(int type)
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
