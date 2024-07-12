#include "pathf.hpp"
#include "fflerror.hpp"
#include "processrun.hpp"
#include "clientguard.hpp"

ClientGuard::ClientGuard(uint64_t uid, ProcessRun *proc, const ActionNode &action)
    : ClientMonster(uid, proc)
{
    switch(action.type){
        case ACTION_ATTACK:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_ATTACK0,
                    .direction = m_processRun->getAimDirection(action, DIR_BEGIN),
                    .x = action.x,
                    .y = action.y,
                });
                break;
            }
        default:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_STAND,
                    .direction = pathf::dirValid(action.direction) ? action.direction : to_d(DIR_UP),
                    .x = action.x,
                    .y = action.y,
                });
                break;
            }
    }
}

bool ClientGuard::parseAction(const ActionNode &action)
{
    m_lastActive = SDL_GetTicks();
    m_motionQueue.clear();

    switch(action.type){
        case ACTION_JUMP:
        case ACTION_STAND:
        case ACTION_SPAWN:
            {
                if(action.x != m_currMotion->x || action.y != m_currMotion->y){
                    m_currMotion.reset(new MotionNode
                    {
                        .type = MOTION_MON_STAND,
                        .direction = action.direction,
                        .x = action.x,
                        .y = action.y,
                    });
                }
                return true;
            }
        case ACTION_ATTACK:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_ATTACK0,
                    .direction = m_processRun->getAimDirection(action, m_currMotion->direction),
                    .x = action.x,
                    .y = action.y,
                });
                return true;
            }
        default:
            {
                throw fflvalue(actionName(action.type));
            }
    }
}
