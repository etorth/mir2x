#include "fflerror.hpp"
#include "processrun.hpp"
#include "clientzumamonster.hpp"

ClientZumaMonster::ClientZumaMonster(uint64_t uid, ProcessRun *proc, const ActionNode &action)
    : ClientStandMonster(uid, proc)
{
    fflassert(isMonster(u8"祖玛雕像") || isMonster(u8"祖玛卫士"));
    switch(action.type){
        case ACTION_SPAWN:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_STAND,
                    .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = false;
                break;
            }
        case ACTION_STAND:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_STAND,
                    .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = (bool)(action.extParam.stand.zumaMonster.standMode);
                break;
            }
        case ACTION_ATTACK:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_ATTACK0,
                    .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = true;
                break;
            }
        case ACTION_MOVE:
            {
                // use MOTION_MON_STAND
                // MOTION_MON_WALK needs to figure the destination grid

                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_STAND,
                    .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = true;
                break;
            }
        case ACTION_TRANSF:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_SPAWN,
                    .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = (bool)(action.extParam.transf.zumaMonster.standModeReq);
                break;
            }
        case ACTION_HITTED:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = MOTION_MON_HITTED,
                    .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
                    .x = action.x,
                    .y = action.y,
                });

                m_standMode = true; // can't be hitted if stay in the soil
                break;
            }
        default:
            {
                throw fflerror("invalid action: %s", actionName(action));
            }
    }
}

bool ClientZumaMonster::onActionSpawn(const ActionNode &action)
{
    fflassert(m_forcedMotionQueue.empty());
    m_currMotion.reset(new MotionNode
    {
        .type = MOTION_MON_STAND,
        .direction = directionValid(action.direction) ? to_d(action.direction) : DIR_UP,
        .x = action.x,
        .y = action.y,
    });

    m_standMode = false;
    return true;
}

bool ClientZumaMonster::onActionStand(const ActionNode &action)
{
    if(finalStandMode() != (bool)(action.extParam.stand.zumaMonster.standMode)){
        addActionTransf();
    }
    return true;
}

bool ClientZumaMonster::onActionTransf(const ActionNode &action)
{
    const auto standReq = (bool)(action.extParam.transf.zumaMonster.standModeReq);
    if(finalStandMode() != standReq){
        addActionTransf();
    }
    return true;
}

bool ClientZumaMonster::onActionAttack(const ActionNode &action)
{
    if(!finalStandMode()){
        addActionTransf();
    }
    return ClientMonster::onActionAttack(action);
}
